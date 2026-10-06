
// ChatAppWinMessage.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "ChatAppWinMessage.h"
#include "ChatAppWinMessageDlg.h"
#include "CChatDlgA.h"
#include "CChatDlgB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChatAppWinMessageApp

BEGIN_MESSAGE_MAP(CChatAppWinMessageApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CChatAppWinMessageApp construction

CChatAppWinMessageApp::CChatAppWinMessageApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CChatAppWinMessageApp object

CChatAppWinMessageApp theApp;


// CChatAppWinMessageApp initialization

BOOL CChatAppWinMessageApp::InitInstance()
{
	CWinApp::InitInstance();
	// Show B in Modeless
	CChatDlgB* pDlgB = new CChatDlgB();
	pDlgB->Create(IDD_CHAT_DLG_B);
	pDlgB->ShowWindow(SW_SHOW);

	// Show A in Model
	CChatDlgA dlgA;
	m_pMainWnd = &dlgA;
	INT_PTR nResponse = dlgA.DoModal();

	// Cleanup B when turn off
	if (pDlgB != NULL)
	{
		pDlgB->DestroyWindow();
		delete pDlgB;
		pDlgB = NULL;
	}

	return FALSE;
}

