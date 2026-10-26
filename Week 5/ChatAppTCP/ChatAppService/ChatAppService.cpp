#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

#define SERVICE_NAME  _T("ChatAppService")
#define PORT_ECHO     9999

SERVICE_STATUS        g_SvcStatus;
SERVICE_STATUS_HANDLE g_SvcStatusHandle = NULL;
HANDLE                g_hServerThread = NULL;
BOOL                  g_bRunning = FALSE;
SOCKET                g_listenSocket = INVALID_SOCKET;

// Thread-safe client socket management
std::vector<SOCKET>   g_clientSockets;
CRITICAL_SECTION      g_csClients;

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv);
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl);
VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
DWORD WINAPI TcpEchoServerThread(LPVOID lpParam);
DWORD WINAPI ClientWorkerThread(LPVOID lpParam);

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    g_SvcStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, SvcCtrlHandler);
    if (!g_SvcStatusHandle) return;

    g_SvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

    g_SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

    g_SvcStatus.dwServiceSpecificExitCode = 0;

    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Initialize critical section for socket vector protection
    InitializeCriticalSection(&g_csClients);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        DeleteCriticalSection(&g_csClients);
        ReportSvcStatus(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR, 0);
        return;
    }

    g_bRunning = TRUE;
    g_hServerThread = CreateThread(NULL, 0, TcpEchoServerThread, NULL, 0, NULL);

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    if (g_hServerThread)
    {
        WaitForSingleObject(g_hServerThread, INFINITE);
        CloseHandle(g_hServerThread);
        g_hServerThread = NULL;
    }

    // Close all connected client sockets upon service termination
    EnterCriticalSection(&g_csClients);
    for (SOCKET s : g_clientSockets)
    {
        shutdown(s, SD_BOTH);
        closesocket(s);
    }
    g_clientSockets.clear();
    LeaveCriticalSection(&g_csClients);

    WSACleanup();
    DeleteCriticalSection(&g_csClients);
    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 5000);
        g_bRunning = FALSE;

        // Close listening socket to unblock accept() immediately (avoid freeze)
        if (g_listenSocket != INVALID_SOCKET)
        {
            closesocket(g_listenSocket);
            g_listenSocket = INVALID_SOCKET;
        }
        break;

    default:
        break;
    }
}

VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    g_SvcStatus.dwCurrentState = dwCurrentState;
    g_SvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    g_SvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING || dwCurrentState == SERVICE_STOP_PENDING)
        g_SvcStatus.dwCheckPoint = dwCheckPoint++;
    else
        g_SvcStatus.dwCheckPoint = 0;

    SetServiceStatus(g_SvcStatusHandle, &g_SvcStatus);
}

// Main thread accepting incoming TCP connections
DWORD WINAPI TcpEchoServerThread(LPVOID lpParam)
{
    g_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_listenSocket == INVALID_SOCKET) return 0;

    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_ECHO); // Port 9999
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(g_listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR ||
        listen(g_listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
        return 0;
    }

    while (g_bRunning)
    {
        sockaddr_in clientAddr = { 0 };
        int nAddrLen = sizeof(clientAddr);

        SOCKET clientSocket = accept(g_listenSocket, (SOCKADDR*)&clientAddr, &nAddrLen);
        if (clientSocket != INVALID_SOCKET)
        {
            // Add client socket to management list
            EnterCriticalSection(&g_csClients);
            g_clientSockets.push_back(clientSocket);
            LeaveCriticalSection(&g_csClients);

            // Create a worker thread for newly connected client
            SOCKET* pClientSock = new SOCKET(clientSocket);
            HANDLE hClientThread = CreateThread(NULL, 0, ClientWorkerThread, pClientSock, 0, NULL);
            if (hClientThread) CloseHandle(hClientThread);
        }
        else
        {
            break; // Socket closed during SvcCtrlHandler stop request
        }
    }

    return 0;
}

// Worker thread handling TCP Echo response for each connected client
DWORD WINAPI ClientWorkerThread(LPVOID lpParam)
{
    SOCKET clientSocket = *(reinterpret_cast<SOCKET*>(lpParam));
    delete reinterpret_cast<SOCKET*>(lpParam);

    TCHAR szBuffer[1024] = { 0 };

    UINT_PTR nClientID = (UINT_PTR)clientSocket;

    while (g_bRunning)
    {
        int nBytesRecv = recv(clientSocket, (char*)szBuffer, sizeof(szBuffer) - sizeof(TCHAR), 0);
        if (nBytesRecv > 0)
        {
            // null termination
            szBuffer[nBytesRecv / sizeof(TCHAR)] = _T('\0');

            // Format echo message tagged with unique Client ID
            // Tag format: [Service echo to client #ID]: <Original message>
            TCHAR szEchoBuffer[2048] = { 0 };

            _sntprintf_s(
                szEchoBuffer,
                _countof(szEchoBuffer),
                _TRUNCATE,
                _T("[Service echo to client #%Iu]: %s"),
                nClientID,
                szBuffer
            );

            DWORD cbToSend = (DWORD)(_tcslen(szEchoBuffer) + 1) * sizeof(TCHAR);

            // Echo
            send(clientSocket, (const char*)szEchoBuffer, cbToSend, 0);
            ZeroMemory(szBuffer, sizeof(szBuffer));
        }
        else
        {
            break; // Client disconnected
        }
    }

    // Remove socket from management list upon disconnection
    EnterCriticalSection(&g_csClients);
    for (auto it = g_clientSockets.begin(); it != g_clientSockets.end(); ++it)
    {
        if (*it == clientSocket)
        {
            g_clientSockets.erase(it);
            break;
        }
    }
    LeaveCriticalSection(&g_csClients);

    shutdown(clientSocket, SD_BOTH);   
    closesocket(clientSocket);
    return 0;
}

BOOL InstallService()
{
    TCHAR szPath[MAX_PATH];
    if (GetModuleFileName(NULL, szPath, MAX_PATH) == 0) return FALSE;

    // Open Service Control Manager
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager) return FALSE;

    // Call WinAPI CreateService to register automatically
    SC_HANDLE hService = CreateService(
        hSCManager,
        SERVICE_NAME,               // Service Name
        _T("Chat App Service"),     // Display Name
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,         // Auto-start with Windows
        SERVICE_ERROR_NORMAL,
        szPath,                     // Executable path
        NULL, NULL, NULL, NULL, NULL
    );

    if (!hService)
    {
        CloseServiceHandle(hSCManager);
        return FALSE;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return TRUE;
}

BOOL UninstallService()
{
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) return FALSE;

    SC_HANDLE hService = OpenService(hSCManager, SERVICE_NAME, DELETE);
    if (!hService)
    {
        CloseServiceHandle(hSCManager);
        return FALSE;
    }

    BOOL bResult = DeleteService(hService);

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return bResult;
}

// Main entry point with command-line arguments parsing
int _tmain(int argc, TCHAR* argv[])
{
    if (argc > 1)
    {
        if (_tcsicmp(argv[1], _T("-install")) == 0 || _tcsicmp(argv[1], _T("/i")) == 0)
        {
            if (InstallService())
                _tprintf(_T("Windows Service '%s' installed SUCCESSFUL!\n"), SERVICE_NAME);
            else
                _tprintf(_T("Installation FAILED! (Make sure you run Command Prompt as Administrator).\n"));
            return 0;
        }
        else if (_tcsicmp(argv[1], _T("-uninstall")) == 0 || _tcsicmp(argv[1], _T("/u")) == 0)
        {
            if (UninstallService())
                _tprintf(_T("Windows Service '%s' uninstalled SUCCESSFUL!\n"), SERVICE_NAME);
            else
                _tprintf(_T("Uninstallation FAILED! (Make sure you run Command Prompt as Administrator).\n"));
            return 0;
        }
    }

    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        { (LPTSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcher(ServiceTable))
    {
        _tprintf(_T("Usage:\n"));
        _tprintf(_T("  Install service   : ChatAppService.exe -install\n"));
        _tprintf(_T("  Uninstall service : ChatAppService.exe -uninstall\n"));
    }
    return 0;
}