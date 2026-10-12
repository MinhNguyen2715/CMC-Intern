
// ChatServer.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "ChatServer.h"
#include "CChatDlgA.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChatServerApp

BEGIN_MESSAGE_MAP(CChatServerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CChatServerApp construction

CChatServerApp::CChatServerApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CChatServerApp object

CChatServerApp theApp;


// CChatServerApp initialization

BOOL CChatServerApp::InitInstance()
{
    CWinApp::InitInstance();
    if (!AfxSocketInit()) return FALSE;

    CChatDlgA dlgA;
    m_pMainWnd = &dlgA;
    dlgA.DoModal();

    return FALSE;
}
