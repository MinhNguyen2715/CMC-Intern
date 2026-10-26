#include "ChatAppService.h"
#include "Utils.h"

using json = nlohmann::json;

// Global service variables
SERVICE_STATUS        g_SvcStatus;
SERVICE_STATUS_HANDLE g_SvcStatusHandle = NULL;
HANDLE                g_hServerThread = NULL;
BOOL                  g_bRunning = FALSE;
SOCKET                g_listenSocket = INVALID_SOCKET;

// Thread-safe active sessions management (Username -> SOCKET)
std::map<std::string, SOCKET> g_activeUsers;
std::vector<SOCKET>           g_clientSockets; // Track all client sockets for cleanup
CRITICAL_SECTION              g_csClients;

DatabaseManager               g_dbManager;

// Service main entry point
VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    g_SvcStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, SvcCtrlHandler);
    if (!g_SvcStatusHandle) return;

    g_SvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_SvcStatus.dwServiceSpecificExitCode = 0;

    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Initialize critical section for thread-safe socket & session management
    InitializeCriticalSection(&g_csClients);

    // Initialize Winsock library
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        DeleteCriticalSection(&g_csClients);
        ReportSvcStatus(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR, 0);
        return;
    }

    // Initialize SQLite Database
    if (!g_dbManager.InitDatabase(GetDatabaseFullPath()))
    {
        WSACleanup();
        DeleteCriticalSection(&g_csClients);
        ReportSvcStatus(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR, 0);
        return;
    }

    g_bRunning = TRUE;
    g_hServerThread = CreateThread(NULL, 0, TcpServerThread, NULL, 0, NULL);

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    if (g_hServerThread)
    {
        WaitForSingleObject(g_hServerThread, INFINITE);
        CloseHandle(g_hServerThread);
        g_hServerThread = NULL;
    }

    // Clean up all active sockets when terminate
    EnterCriticalSection(&g_csClients);
    std::vector<SOCKET> socketsToClose = g_clientSockets;
    g_clientSockets.clear();
    g_activeUsers.clear();
    LeaveCriticalSection(&g_csClients);

    for (SOCKET s : socketsToClose)
    {
        shutdown(s, SD_BOTH);
        closesocket(s);
    }

    Sleep(200);

    WSACleanup();
    DeleteCriticalSection(&g_csClients);
    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

// Service control handler callback
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 5000);
        g_bRunning = FALSE;

        // Close listening socket -> unblock accept()
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

// Report status update to Service Control Manager
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

// Main thread listening for incoming TCP client connections
DWORD WINAPI TcpServerThread(LPVOID lpParam)
{
    // Create a listening port at PORT_CHAT
    g_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_listenSocket == INVALID_SOCKET) return 0;

    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_CHAT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(g_listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR ||
        listen(g_listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
        return 0;
    }

    // Listen for new connection
    while (g_bRunning)
    {
        sockaddr_in clientAddr = { 0 };
        int nAddrLen = sizeof(clientAddr);

        SOCKET clientSocket = accept(g_listenSocket, (SOCKADDR*)&clientAddr, &nAddrLen);
        if (clientSocket != INVALID_SOCKET)
        {
            // Add new client socket 
            EnterCriticalSection(&g_csClients);
            g_clientSockets.push_back(clientSocket);
            LeaveCriticalSection(&g_csClients);

            // Allocate memory
            SOCKET* pClientSock = new SOCKET(clientSocket);
            HANDLE hClientThread = CreateThread(NULL, 0, ClientWorkerThread, pClientSock, 0, NULL);
            if (hClientThread)
            {
                CloseHandle(hClientThread); // Thread runs independently
            }
            else
            {
                delete pClientSock;
                closesocket(clientSocket);
                RemoveClientSession(clientSocket, "");
            }
        }
        else
        {
            break; // Socket closed 
        }
    }

    return 0;
}

// Process JSON payloads received from connected clients
std::string ProcessJsonRequest(const std::string& rawJson, SOCKET clientSocket, std::string& outAuthenticatedUser)
{
    json response;

    try
    {
        auto req = json::parse(rawJson);
        std::string action = req.value("action", "");
        auto data = req.value("data", json::object());
        // Register
        if (action == "register")
        {
            std::string username = data.value("username", "");
            std::string password = data.value("password", "");

            if (!Utils::IsValidUsername(username))
            {
                response = {
                    {"action", action + "_res"},
                    {"status", "fail"},
                    {"message", "Invalid username! Must be 8-20 alphanumeric characters or underscores."}
                };
                return response.dump() + '\0';
            }

            if (!Utils::IsValidPassword(password))
            {
                response = {
                    {"action", action + "_res"},
                    {"status", "fail"},
                    {"message", "Invalid password! Must be 8-32 characters without spaces."}
                };
                return response.dump() + '\0';
            }

            if (g_dbManager.RegisterUser(username, password))
            {
                response = {
                    {"action", "register_res"},
                    {"status", "success"},
                    {"message", "User registered successfully."}
                };
            }
            else
            {
                response = {
                    {"action", "register_res"},
                    {"status", "fail"},
                    {"message", "Username already exists or database error."}
                };
            }
        }

        // Login
        else if (action == "login")
        {
            std::string username = data.value("username", "");
            std::string password = data.value("password", "");

            if (g_dbManager.AuthenticateUser(username, password))
            {
                EnterCriticalSection(&g_csClients);

                auto it = g_activeUsers.find(username);
                if (it != g_activeUsers.end())
                {
                    LeaveCriticalSection(&g_csClients);
                    response = {
                        {"action", "login_res"},
                        {"status", "fail"},
                        {"message", "User is already logged in."}
                    };
                    return response.dump() + '\0';
                }

                if (!outAuthenticatedUser.empty())
                {
                    g_activeUsers.erase(outAuthenticatedUser);
                }

                // Bind active username to client socket session
                g_activeUsers[username] = clientSocket;
                outAuthenticatedUser = username;
                LeaveCriticalSection(&g_csClients);

                response = {
                    {"action", "login_res"},
                    {"status", "success"},
                    {"message", "Login successful."}
                };
            }
            else
            {
                response = {
                    {"action", "login_res"},
                    {"status", "fail"},
                    {"message", "Invalid username or password."}
                };
            }
        }

        // Send messages
        else if (action == "send_msg")
        {
            if (outAuthenticatedUser.empty())
            {
                response = {
                    {"action", "send_msg_res"},
                    {"status", "fail"},
                    {"message", "Unauthorized. Please log in first."}
                };
            }
            else
            {
                std::string toUser = data.value("to", "");
                std::string content = data.value("content", "");
                int64_t timestamp = static_cast<int64_t>(std::time(nullptr));

                if (toUser.empty() || content.empty())
                {
                    return "";
                }

                // Save message into database
                g_dbManager.SaveMessage(outAuthenticatedUser, toUser, content, timestamp);

                // Check if target recipient is online
                SOCKET targetSocket = INVALID_SOCKET;
                EnterCriticalSection(&g_csClients);
                auto it = g_activeUsers.find(toUser);
                if (it != g_activeUsers.end())
                {
                    targetSocket = it->second;
                }
                LeaveCriticalSection(&g_csClients);

                if (targetSocket != INVALID_SOCKET)
                {
                    // Send message to destination client
                    json forwardMsg = {
                        {"action", "receive_msg"},
                        {"data", {
                            {"from", outAuthenticatedUser},
                            {"content", content},
                            {"timestamp", timestamp}
                        }}
                    };

                    std::string forwardStr = forwardMsg.dump() + '\0';
                    send(targetSocket, forwardStr.c_str(), static_cast<int>(forwardStr.length()) + 1, 0);
                }

                response = {
                    {"action", "send_msg_res"},
                    {"status", "success"},
                    {"message", "Message sent successfully."}
                };
            }
        }

        // Get users list
        else if (action == "get_users")
            {
            if (outAuthenticatedUser.empty())
            {
                response = {
                    {"action", "get_users_res"},
                    {"status", "fail"},
                    {"message", "Unauthorized. Please log in first."}
                };
            }
            else
            {
                auto userList = g_dbManager.GetAllUsers(outAuthenticatedUser);
                response = {
                    {"action", "get_users_res"},
                    {"status", "success"},
                    {"data", {
                        {"users", userList}
                    }}
                };
            }
        }

        // Get chat history
        else if (action == "get_chat_history")
        {
        if (outAuthenticatedUser.empty())
        {
            response = {
                {"action", "get_chat_history_res"},
                {"status", "fail"},
                {"message", "Unauthorized. Please log in first."}
            };
        }
        else
        {
            std::string withUser = data.value("with_user", "");
            auto history = g_dbManager.GetChatHistory(outAuthenticatedUser, withUser);

            json msgList = json::array();
            for (const auto& msg : history)
            {
                msgList.push_back({
                    {"sender", msg.sender},
                    {"receiver", msg.receiver},
                    {"content", msg.content},
                    {"timestamp", msg.timestamp}
                    });
            }

            response = {
                {"action", "get_chat_history_res"},
                {"status", "success"},
                {"data", {
                    {"with_user", withUser},
                    {"messages", msgList}
                }}
            };
        }
        }
        
        // Fallback
        else
        {
            response = {
                {"action", "error"},
                {"status", "fail"},
                {"message", "Unknown or unsupported action."}
            };
        }
    }
    catch (const std::exception&)
    {
        response = {
            {"action", "error"},
            {"status", "fail"},
            {"message", "Malformed JSON payload."}
        };
    }

    return response.dump() + '\0';
}

// Remove client socket and active user mapping
void RemoveClientSession(SOCKET clientSocket, const std::string& username)
{
    EnterCriticalSection(&g_csClients);

    // Remove from active authenticated users
    if (!username.empty())
    {
        g_activeUsers.erase(username);
    }

    // Remove from socket list
    for (auto it = g_clientSockets.begin(); it != g_clientSockets.end(); ++it)
    {
        if (*it == clientSocket)
        {
            g_clientSockets.erase(it);
            break;
        }
    }

    LeaveCriticalSection(&g_csClients);
}

// Worker thread 
DWORD WINAPI ClientWorkerThread(LPVOID lpParam)
{
    SOCKET clientSocket = *(reinterpret_cast<SOCKET*>(lpParam));
    delete reinterpret_cast<SOCKET*>(lpParam);

    std::string accumulatedBuffer;
    char szChunk[1024] = { 0 };
    std::string currentUsername = "";

    while (g_bRunning)
    {
        int nBytesRecv = recv(clientSocket, szChunk, sizeof(szChunk) - 1, 0);
        if (nBytesRecv > 0)
        {
            szChunk[nBytesRecv] = '\0';
            accumulatedBuffer.append(szChunk, nBytesRecv);

            // Process full JSON messages delimited by null-terminator '\0'
            size_t nullPos = 0;
            while ((nullPos = accumulatedBuffer.find('\0')) != std::string::npos)
            {
                std::string rawJson = accumulatedBuffer.substr(0, nullPos);
                accumulatedBuffer.erase(0, nullPos + 1);

                if (!rawJson.empty())
                {
                    std::string responseStr = ProcessJsonRequest(rawJson, clientSocket, currentUsername);
                    if (!responseStr.empty())
                    {
                        send(clientSocket, responseStr.c_str(), static_cast<int>(responseStr.length()) + 1, 0);
                    }
                }
            }
            ZeroMemory(szChunk, sizeof(szChunk));
        }
        else
        {
            break; // Connection closed or socket error
        }
    }

    // Cleanup session and close socket 
    RemoveClientSession(clientSocket, currentUsername);

    shutdown(clientSocket, SD_BOTH);
    closesocket(clientSocket);
    return 0;
}

// Install Windows Service
BOOL InstallService()
{
    TCHAR szPath[MAX_PATH];
    if (GetModuleFileName(NULL, szPath, MAX_PATH) == 0) return FALSE;

    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager) return FALSE;

    SC_HANDLE hService = CreateService(
        hSCManager,
        SERVICE_NAME,
        _T("Chat App Service"),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        szPath,
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

// Uninstall Windows Service
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

// Set database path in the same folder as .exe file
std::string GetDatabaseFullPath()
{
    char szPath[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, szPath, MAX_PATH);
    std::string exePath(szPath);

    size_t lastSlash = exePath.find_last_of("\\/");
    if (lastSlash != std::string::npos)
    {
        return exePath.substr(0, lastSlash + 1) + DB_FILE_PATH;
    }
    return DB_FILE_PATH;
}

// Application entry point for service startup + CLI argument parsing
int _tmain(int argc, TCHAR* argv[])
{
    if (argc > 1)
    {
        if (_tcsicmp(argv[1], _T("-install")) == 0 || _tcsicmp(argv[1], _T("/i")) == 0)
        {
            if (InstallService())
                _tprintf_s(_T("Windows Service '%s' installed SUCCESSFUL!\n"), SERVICE_NAME);
            else
                _tprintf_s(_T("Installation FAILED! (Make sure you run Command Prompt as Administrator).\n"));
            return 0;
        }
        else if (_tcsicmp(argv[1], _T("-uninstall")) == 0 || _tcsicmp(argv[1], _T("/u")) == 0)
        {
            if (UninstallService())
                _tprintf_s(_T("Windows Service '%s' uninstalled SUCCESSFUL!\n"), SERVICE_NAME);
            else
                _tprintf_s(_T("Uninstallation FAILED! (Make sure you run Command Prompt as Administrator).\n"));
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
        _tprintf_s(_T("Usage:\n"));
        _tprintf_s(_T("  Install service   : ChatAppService.exe -install\n"));
        _tprintf_s(_T("  Uninstall service : ChatAppService.exe -uninstall\n"));
    }
    return 0;
}