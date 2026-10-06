
// ChatAppNamedPipe.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "ChatAppNamedPipe.h"
#include "ChatAppNamedPipeDlg.h"
#include "CChatDlgA.h"
#include "CChatDlgB.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChatAppNamedPipeApp

BEGIN_MESSAGE_MAP(CChatAppNamedPipeApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CChatAppNamedPipeApp construction

CChatAppNamedPipeApp::CChatAppNamedPipeApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CChatAppNamedPipeApp object

CChatAppNamedPipeApp theApp;


// CChatAppNamedPipeApp initialization

BOOL CChatAppNamedPipeApp::InitInstance()
{
	CWinApp::InitInstance();

	// Dialog B in modeless
	CChatDlgB* pDlgB = new CChatDlgB();
	if (pDlgB->Create(IDD_CHAT_DLG_B))
	{
		pDlgB->ShowWindow(SW_SHOW);
		pDlgB->UpdateWindow();
	}

	// Dialog A in Modal
	CChatDlgA dlgA;
	m_pMainWnd = &dlgA;
	INT_PTR nResponse = dlgA.DoModal();

	// Cleanup Dlg B when Dlg A is destroyed
	if (pDlgB != NULL)
	{
		if (::IsWindow(pDlgB->GetSafeHwnd()))
		{
			pDlgB->DestroyWindow();
		}
		delete pDlgB;
		pDlgB = NULL;
	}

	::PostQuitMessage(0);

	return FALSE;
}

