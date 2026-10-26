#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include "md5.h"
#include "json.hpp"
#include "DatabaseManager.h"

#pragma comment(lib, "Ws2_32.lib")

// Constants
#define SERVICE_NAME  _T("ChatAppService")
#define PORT_CHAT     9999
#define DB_FILE_PATH  "chat_app.db"

// Service functions
VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv);
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl);
VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
BOOL InstallService();
BOOL UninstallService();

// Network & Worker thread functions
DWORD WINAPI TcpServerThread(LPVOID lpParam);
DWORD WINAPI ClientWorkerThread(LPVOID lpParam);
std::string ProcessJsonRequest(const std::string& rawJson, SOCKET clientSocket, std::string& outAuthenticatedUser);
void RemoveClientSession(SOCKET clientSocket, const std::string& username);

// Helper functions
std::string GetDatabaseFullPath();