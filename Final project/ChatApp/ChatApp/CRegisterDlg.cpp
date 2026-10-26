// CRegisterDlg.cpp : implementation file
//

#include "pch.h"
#include "ChatApp.h"
#include "Resource.h"
#include "CRegisterDlg.h"
#include "Utils.h"
#include "json.hpp"
#include <ws2tcpip.h>

using json = nlohmann::json;
// CRegisterDlg dialog

IMPLEMENT_DYNAMIC(CRegisterDlg, CDialogEx)

CRegisterDlg::CRegisterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REGISTER_DLG, pParent)
{
}

CRegisterDlg::~CRegisterDlg()
{
}

void CRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRegisterDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SUBMIT_REGISTER, &CRegisterDlg::OnBnClickedBtnSubmitRegister)
END_MESSAGE_MAP()

// CRegisterDlg message handlers
BOOL CRegisterDlg::OnInitDialog()
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

    // Assign font
    if (GetDlgItem(IDC_STATIC_REG_TITLE))
        GetDlgItem(IDC_STATIC_REG_TITLE)->SetFont(&m_fontTitle);

    if (GetDlgItem(IDC_STATIC_REG_USER)) GetDlgItem(IDC_STATIC_REG_USER)->SetFont(&m_fontNormal);
    if (GetDlgItem(IDC_STATIC_REG_PASS)) GetDlgItem(IDC_STATIC_REG_PASS)->SetFont(&m_fontNormal);
    if (GetDlgItem(IDC_STATIC_REG_CONFIRM)) GetDlgItem(IDC_STATIC_REG_CONFIRM)->SetFont(&m_fontNormal);

    if (GetDlgItem(IDC_EDIT_REG_USERNAME)) GetDlgItem(IDC_EDIT_REG_USERNAME)->SetFont(&m_fontNormal);
    if (GetDlgItem(IDC_EDIT_REG_PASSWORD)) GetDlgItem(IDC_EDIT_REG_PASSWORD)->SetFont(&m_fontNormal);
    if (GetDlgItem(IDC_EDIT_REG_CONFIRM)) GetDlgItem(IDC_EDIT_REG_CONFIRM)->SetFont(&m_fontNormal);

    if (GetDlgItem(IDC_BTN_SUBMIT_REGISTER)) GetDlgItem(IDC_BTN_SUBMIT_REGISTER)->SetFont(&m_fontNormal);
    if (GetDlgItem(IDC_BTN_RETURN)) GetDlgItem(IDC_BTN_RETURN)->SetFont(&m_fontNormal);

    return TRUE;
}

// Temporarily connect to server to send register request
bool CRegisterDlg::ConnectAndRegister(const std::string& username, const std::string& password, std::string& outMsg)
{
    struct addrinfo hints = { 0 }, * result = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Server IP + Port
    if (getaddrinfo("192.168.1.18", "9999", &hints, &result) != 0)
    {
        outMsg = "Cannot connect to server!";
        return false;
    }

    // Initiate socket
    SOCKET hSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (hSocket == INVALID_SOCKET)
    {
        freeaddrinfo(result);
        outMsg = "Cannot initiate socket!";
        return false;
    }

    // Connect to Server
    if (connect(hSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
    {
        closesocket(hSocket);
        freeaddrinfo(result);
        outMsg = "Cannot connect to ChatAppService!";
        return false;
    }

    freeaddrinfo(result);

    // Send JSON register request
    json req = {
        {"action", "register"},
        {"data", {
            {"username", username},
            {"password", password}
        }}
    };

    std::string strReq = req.dump() + '\0';
    send(hSocket, strReq.c_str(), static_cast<int>(strReq.length()) + 1, 0);

    // Receive JSON reply
    char szBuf[1024] = { 0 };
    int nRecv = recv(hSocket, szBuf, sizeof(szBuf) - 1, 0);

    shutdown(hSocket, SD_BOTH);
    closesocket(hSocket);

    if (nRecv > 0)
    {
        szBuf[nRecv] = '\0';
        try {
            auto res = json::parse(szBuf);
            outMsg = res.value("message", "");
            return (res.value("status", "") == "success");
        }
        catch (...) {
            outMsg = "Invalid Server reply!";
            return false;
        }
    }

    outMsg = "Cannot receive Server reply!";
    return false;
}

void CRegisterDlg::OnBnClickedBtnSubmitRegister()
{
    CString strUser, strPass, strConfirmPass;
    GetDlgItemText(IDC_EDIT_REG_USERNAME, strUser);
    GetDlgItemText(IDC_EDIT_REG_PASSWORD, strPass);
    GetDlgItemText(IDC_EDIT_REG_CONFIRM, strConfirmPass);

    strUser.Trim();
    strPass.Trim();
    strConfirmPass.Trim();

    std::string userA = CW2A(strUser.GetString(), CP_UTF8);
    std::string passA = CW2A(strPass.GetString(), CP_UTF8);
    std::string confirmA = CW2A(strConfirmPass.GetString(), CP_UTF8);

    // Check username + password
    if (!Utils::IsValidUsername(userA))
    {
        AfxMessageBox(_T("Invalid username format!\n(8-20 charecters, numbers or underscore)"), MB_OK | MB_ICONWARNING);
        return;
    }

    if (!Utils::IsValidPassword(passA))
    {
        AfxMessageBox(_T("Invalid password format!\n(8-32 characters, no spaces)"), MB_OK | MB_ICONWARNING);
        return;
    }

    // Checkn confirm password
    if (passA != confirmA)
    {
        AfxMessageBox(_T("Passwords do not match!"), MB_OK | MB_ICONWARNING);
        return;
    }

    // Send to ChatAppService to save into database
    std::string serverMsg;
    if (ConnectAndRegister(userA, passA, serverMsg))
    {
        m_strUsername = strUser;
        AfxMessageBox(CA2T(serverMsg.c_str()), MB_OK | MB_ICONINFORMATION);
        EndDialog(IDOK); 
    }
    else
    {
        AfxMessageBox(CA2T(serverMsg.c_str()));
    }
}