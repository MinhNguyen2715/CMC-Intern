// ChatApp.cpp

#include "pch.h"
#include "framework.h"
#include "ChatApp.h"
#include "CLoginDlg.h"
#include "CChatDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChatAppApp

BEGIN_MESSAGE_MAP(CChatAppApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CChatAppApp construction

CChatAppApp::CChatAppApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CChatAppApp object

CChatAppApp theApp;


// CChatAppApp initialization

BOOL CChatAppApp::InitInstance()
{
    AfxInitRichEdit2();
    CWinApp::InitInstance();
    AfxEnableControlContainer();

    // Winsock init
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        AfxMessageBox(_T("Failed to initialize Winsock!"));
        return FALSE;
    }

    CLoginDlg loginDlg;
    if (loginDlg.DoModal() == IDOK)
    {
        CChatDlg chatDlg;
        chatDlg.m_hSocket = loginDlg.m_hSocket;            
        chatDlg.m_strUsername = loginDlg.m_strUsername;  

        m_pMainWnd = &chatDlg;
        chatDlg.DoModal(); 
    }

    WSACleanup();
    return FALSE;
}
