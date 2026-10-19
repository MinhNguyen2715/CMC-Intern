
// ChatClient.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "ChatClient.h"
#include "CChatDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChatClientApp

BEGIN_MESSAGE_MAP(CChatClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CChatClientApp construction

CChatClientApp::CChatClientApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CChatClientApp object

CChatClientApp theApp;


// CChatClientApp initialization

BOOL CChatClientApp::InitInstance()
{
    CWinApp::InitInstance();
    if (!AfxSocketInit()) return FALSE;

    CChatDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();

    return FALSE;
}