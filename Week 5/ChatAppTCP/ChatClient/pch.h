// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <afxsock.h>
#include "resource.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define WM_SOCKET_MSG_RECEIVED (WM_USER + 1)

#define PORT_CHAT 9999
#define SERVER_IP _T("127.0.0.1")

inline BOOL RegisterAutoRunSilently(const CString& strAppName)
{
    HKEY hKey = NULL;
    LSTATUS lStatus = ::RegOpenKeyEx(HKEY_CURRENT_USER,
        _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        0, KEY_SET_VALUE, &hKey);

    if (lStatus != ERROR_SUCCESS) return FALSE;

    TCHAR szPath[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szPath, MAX_PATH);

    CString strPathWithQuotes;
    strPathWithQuotes.Format(_T("\"%s\""), szPath);

    lStatus = ::RegSetValueEx(hKey, strAppName, 0, REG_SZ,
        (const BYTE*)strPathWithQuotes.GetString(),
        (DWORD)(strPathWithQuotes.GetLength() + 1) * sizeof(TCHAR));

    ::RegCloseKey(hKey);
    return (lStatus == ERROR_SUCCESS);
}

#endif //PCH_H