// CLoginDlg.cpp : implementation file
//

#include "pch.h"
#include "ChatApp.h"
#include "Resource.h"    
#include "CLoginDlg.h"
#include "CRegisterDlg.h"
#include "Utils.h"       
#include "json.hpp"      
#include <ws2tcpip.h>

using json = nlohmann::json;
// CLoginDlg dialog

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOGIN_DLG, pParent), m_hSocket(INVALID_SOCKET)
{
}

CLoginDlg::~CLoginDlg(){

}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_LOGIN, &CLoginDlg::OnBnClickedBtnLogin)
	ON_BN_CLICKED(IDC_BTN_REGISTER, &CLoginDlg::OnBnClickedBtnRegister)
END_MESSAGE_MAP()

// CLoginDlg message handlers
BOOL CLoginDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Title font
    m_fontTitle.CreatePointFont(140, _T("Segoe UI")); 
    LOGFONT lf;
    m_fontTitle.GetLogFont(&lf);
    lf.lfWeight = FW_BOLD; 
    m_fontTitle.DeleteObject();
    m_fontTitle.CreateFontIndirect(&lf);

    // Normal font
    m_fontNormal.CreatePointFont(100, _T("Segoe UI"));

    // Title
    if (GetDlgItem(IDC_STATIC_TITLE))
        GetDlgItem(IDC_STATIC_TITLE)->SetFont(&m_fontTitle);

    // Static labels
    if (GetDlgItem(IDC_STATIC_USERNAME)) GetDlgItem(IDC_STATIC_USERNAME)->SetFont(&m_fontNormal);
    if (GetDlgItem(IDC_STATIC_PASSWORD)) GetDlgItem(IDC_STATIC_PASSWORD)->SetFont(&m_fontNormal);

    // Edit Controls
    if (GetDlgItem(IDC_EDIT_USERNAME)) GetDlgItem(IDC_EDIT_USERNAME)->SetFont(&m_fontNormal);
    if (GetDlgItem(IDC_EDIT_PASSWORD)) GetDlgItem(IDC_EDIT_PASSWORD)->SetFont(&m_fontNormal);

    // Buttons
    if (GetDlgItem(IDC_BTN_LOGIN)) GetDlgItem(IDC_BTN_LOGIN)->SetFont(&m_fontNormal);
    if (GetDlgItem(IDC_BTN_REGISTER)) GetDlgItem(IDC_BTN_REGISTER)->SetFont(&m_fontNormal);

    return TRUE;
}

void CLoginDlg::OnBnClickedBtnRegister()
{
    CRegisterDlg regDlg;
    if (regDlg.DoModal() == IDOK)
    {
        AfxMessageBox(_T("Account created succesfully!\nLogin to continue"), MB_OK | MB_ICONINFORMATION);
        SetDlgItemText(IDC_EDIT_USERNAME, regDlg.m_strUsername);
        SetDlgItemText(IDC_EDIT_PASSWORD, _T(""));
        
        CWnd* pPass = GetDlgItem(IDC_EDIT_PASSWORD);
        if (pPass) pPass->SetFocus();
    }
}

bool CLoginDlg::ConnectToServer()
{
    if (m_hSocket != INVALID_SOCKET) return true;

    struct addrinfo hints = { 0 }, * result = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Server IP + Port
    if (getaddrinfo("192.168.1.18", "9999", &hints, &result) != 0)
    {
        return false;
    }

    m_hSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_hSocket == INVALID_SOCKET)
    {
        freeaddrinfo(result);
        return false;
    }

    if (connect(m_hSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        freeaddrinfo(result);
        return false;
    }

    freeaddrinfo(result);
    return true;
}

void CLoginDlg::OnBnClickedBtnLogin()
{
    CString strUser, strPass;
    GetDlgItemText(IDC_EDIT_USERNAME, strUser);
    GetDlgItemText(IDC_EDIT_PASSWORD, strPass);

    strUser.Trim();
    strPass.Trim();

    std::string userA = CW2A(strUser.GetString(), CP_UTF8);
    std::string passA = CW2A(strPass.GetString(), CP_UTF8);

    // Check username + password
    if (!Utils::IsValidUsername(userA) || !Utils::IsValidPassword(passA))
    {
        AfxMessageBox(_T("Wrong username or password!"),MB_OK | MB_ICONWARNING);
        return;
    }

    // Connect to server
    if (!ConnectToServer())
    {
        AfxMessageBox(_T("Cannot connect to ChatAppService!"));
        return;
    }

    // Send JSON request login
    json req = {
        {"action", "login"},
        {"data", {
            {"username", userA},
            {"password", passA}
        }}
    };
    std::string strReq = req.dump() + '\0';
    send(m_hSocket, strReq.c_str(), static_cast<int>(strReq.length()) + 1, 0);

    // Receive JSON reply
    char szBuf[1024] = { 0 };
    int nRecv = recv(m_hSocket, szBuf, sizeof(szBuf) - 1, 0);
    if (nRecv > 0)
    {
        try {
            auto res = json::parse(szBuf);
            if (res.value("status", "") == "success")
            {
                m_strUsername = strUser;
                CDialogEx::OnOK();
                return;
            }
            else
            {
                std::string msg = res.value("message", "Login fails!");
                AfxMessageBox(CA2T(msg.c_str(), CP_UTF8), MB_OK | MB_ICONWARNING);
            }
        }
        catch (...) {
            AfxMessageBox(_T("Invalid server reply!"));
        }
    }
    if (m_hSocket != INVALID_SOCKET)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }
}

void CLoginDlg::OnCancel()
{
    if (m_hSocket != INVALID_SOCKET)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }
    CDialogEx::OnCancel();
}