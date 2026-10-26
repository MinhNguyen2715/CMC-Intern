// ChatApp.cpp

#include "pch.h"
#include "framework.h"
#include "ChatApp.h"
#include "ChatAppDlg.h"
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
    CWinApp::InitInstance();
    AfxEnableControlContainer();

    // 1. Khởi tạo Winsock 2.2 cho Client
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        AfxMessageBox(_T("Khởi tạo Winsock thất bại!"));
        return FALSE;
    }

    // 2. Mở CLoginDlg trước
    CLoginDlg loginDlg;
    if (loginDlg.DoModal() == IDOK)
    {
        // 3. Đăng nhập thành công -> Chuyển Socket & Username sang CChatDlg
        CChatDlg chatDlg;
        chatDlg.m_hSocket = loginDlg.m_hSocket;            // Dùng lại Socket đã kết nối
        chatDlg.m_strUsername = loginDlg.m_strUsername;   // Lưu username đã đăng nhập

        m_pMainWnd = &chatDlg;
        chatDlg.DoModal(); // Mở màn hình Chat chính
    }

    // Dọn dẹp Winsock khi thoát ứng dụng
    WSACleanup();
    return FALSE;
}
