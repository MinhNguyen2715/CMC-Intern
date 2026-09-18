
// ChatApp.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "ChatApp.h"
#include "ChatAppDlg.h"
#include "LoginDlg.h"

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
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Set login UI to be init first
	CLoginDlg loginDlg;
	INT_PTR nResponse = loginDlg.DoModal();
	
	// Launch chat dialog only if authentication is successful
	if (nResponse == IDOK)
	{
		CChatAppDlg mainDlg;
		m_pMainWnd = &mainDlg;
		mainDlg.DoModal(); 
	}

	// User closed or canceled login -> Exit application
	return FALSE;
}

