// CChatDlg.cpp : implementation file
//

#include "pch.h"
#include "ChatApp.h"
#include "Resource.h"
#include "CChatDlg.h"
#include "json.hpp"

using json = nlohmann::json;

// CChatDlg dialog

IMPLEMENT_DYNAMIC(CChatDlg, CDialogEx)

CChatDlg::CChatDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_CHAT_DLG, pParent), m_hSocket(INVALID_SOCKET), m_hRecvThread(NULL), m_bRunning(TRUE)
{
}

CChatDlg::~CChatDlg()
{
}

void CChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_USER, m_listUser);
    DDX_Control(pDX, IDC_LIST_CHAT, m_richChat);
}


BEGIN_MESSAGE_MAP(CChatDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_LIST_USER, &CChatDlg::OnNMClickListUser)
	ON_BN_CLICKED(IDC_BTN_SEND, &CChatDlg::OnBnClickedBtnSend)
    ON_MESSAGE(WM_RECV_JSON, &CChatDlg::OnRecvJsonMessage)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CChatDlg message handlers

BOOL CChatDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_fontUI.CreatePointFont(120, _T("Segoe UI"));
    m_listUser.SetFont(&m_fontUI);
    m_richChat.SetFont(&m_fontUI);
    m_richChat.SendMessage(EM_SETTARGETDEVICE, 0, 0); // word wrapping 
    if (GetDlgItem(IDC_EDIT_MESSAGE)) GetDlgItem(IDC_EDIT_MESSAGE)->SetFont(&m_fontUI);

    if (GetDlgItem(IDC_BTN_SEND)) GetDlgItem(IDC_BTN_SEND)->SetFont(&m_fontUI);
    ModifyStyle(0, WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME);
    ModifyStyleEx(0, WS_EX_APPWINDOW);
    SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    CString strTitle;
    strTitle.Format(_T("ChatApp - [%s]"), m_strUsername.GetString());
    SetWindowText(strTitle);

    InitUserListControl();
    ToggleChatUI(FALSE);

    m_bRunning = TRUE;
    m_hRecvThread = CreateThread(NULL, 0, ReceiverThreadProc, this, 0, NULL);

    RequestUserList();
    ShowWindow(SW_SHOWMAXIMIZED);

    return TRUE;
}

void CChatDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (nType == SIZE_MINIMIZED || cx <= 0 || cy <= 0 || !::IsWindow(m_hWnd)) return;
    int margin = 10;        
    int spacing = 10;       
    int inputHeight = 60;   

    int leftWidth = static_cast<int>(cx * 0.30);
    if (leftWidth < 300) leftWidth = 300;
    int rightWidth = cx - leftWidth - (margin * 2) - spacing;
    int chatHeight = cy - inputHeight - (margin * 2) - spacing;

    if (m_listUser.GetSafeHwnd() && ::IsWindow(m_listUser.GetSafeHwnd()))
    {
        m_listUser.MoveWindow(margin, margin, leftWidth, cy - (margin * 2));

        int timeColWidth = 130;
        int userColWidth = 180;
        int msgColWidth = leftWidth - userColWidth - timeColWidth - 8;
        if (msgColWidth < 50) msgColWidth = 50;
        m_listUser.SetColumnWidth(0, userColWidth);  
        m_listUser.SetColumnWidth(1, msgColWidth);   
        m_listUser.SetColumnWidth(2, timeColWidth);  
    }

    if (m_richChat.GetSafeHwnd() && ::IsWindow(m_richChat.GetSafeHwnd()))
    {
        m_richChat.MoveWindow(margin + leftWidth + spacing, margin, rightWidth, chatHeight);

        CRect rect;
        m_richChat.GetClientRect(&rect);
        rect.DeflateRect(15, 10, 15, 10);
        m_richChat.SetRect(&rect);
    }

    CWnd* pEdit = GetDlgItem(IDC_EDIT_MESSAGE);
    if (pEdit && pEdit->GetSafeHwnd())
    {
        int btnWidth = 90;
        int editWidth = rightWidth - btnWidth - spacing;
        pEdit->MoveWindow(margin + leftWidth + spacing, margin + chatHeight + spacing, editWidth, inputHeight);
    }

    CWnd* pBtn = GetDlgItem(IDC_BTN_SEND);
    if (pBtn && pBtn->GetSafeHwnd())
    {
        int btnWidth = 90;
        pBtn->MoveWindow(cx - margin - btnWidth, margin + chatHeight + spacing, btnWidth, inputHeight);
    }
}

void CChatDlg::ToggleChatUI(BOOL bShow)
{
    if (!::IsWindow(m_hWnd)) return;
    int nCmdShow = bShow ? SW_SHOW : SW_HIDE;
    if (GetDlgItem(IDC_LIST_CHAT)) GetDlgItem(IDC_LIST_CHAT)->ShowWindow(nCmdShow);
    if (GetDlgItem(IDC_EDIT_MESSAGE)) GetDlgItem(IDC_EDIT_MESSAGE)->ShowWindow(nCmdShow);
    if (GetDlgItem(IDC_BTN_SEND)) GetDlgItem(IDC_BTN_SEND)->ShowWindow(nCmdShow);
}

bool LoadPNG(CImage& img, UINT nID)
{
    HINSTANCE hInst = AfxGetInstanceHandle();
    HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(nID), _T("PNG"));
    if (!hResource) return false;

    DWORD dwSize = SizeofResource(hInst, hResource);
    HGLOBAL hResData = LoadResource(hInst, hResource);
    if (!hResData) return false;

    LPVOID pData = LockResource(hResData);
    if (!pData) return false;

    HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, dwSize);
    if (!hBuffer) return false;
    LPVOID pBuffer = GlobalLock(hBuffer);
    if (pBuffer)
    {
        CopyMemory(pBuffer, pData, dwSize);
        GlobalUnlock(hBuffer);
        IStream* pStream = NULL;
        if (SUCCEEDED(CreateStreamOnHGlobal(hBuffer, TRUE, &pStream)))

        {
            img.Load(pStream);
            pStream->Release();
            return !img.IsNull();
        }
    }
    GlobalFree(hBuffer);
    return false;
}

BOOL CALLBACK EnumAvatar(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam)
{
    std::vector<UINT>* pResourceIDs = reinterpret_cast<std::vector<UINT>*>(lParam);
    if (IS_INTRESOURCE(lpszName))
    {
        pResourceIDs->push_back(static_cast<UINT>(reinterpret_cast<ULONG_PTR>(lpszName)));
    }
    return TRUE;
}

void CChatDlg::InitUserListControl()
{
    if (!m_listUser.GetSafeHwnd() || !::IsWindow(m_listUser.GetSafeHwnd())) return;

    m_listUser.ModifyStyle(0, LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS);
    m_listUser.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);
    m_listUser.InsertColumn(0, _T("User"), LVCFMT_LEFT, 130);
    m_listUser.InsertColumn(1, _T("LastMsg"), LVCFMT_LEFT, 80);
    m_listUser.InsertColumn(2, _T("Time"), LVCFMT_RIGHT, 65);
    m_imgListUser.Create(48, 48, ILC_COLOR32 | ILC_MASK, 5, 5);

    std::vector<UINT> avatarResIDs;
    EnumResourceNames(
        AfxGetInstanceHandle(),
        _T("PNG"),
        EnumAvatar,
        reinterpret_cast<LONG_PTR>(&avatarResIDs)
    );

    HINSTANCE hInst = AfxGetInstanceHandle();
    CWindowDC dcScreen(NULL);

    for (size_t i = 0; i < avatarResIDs.size(); ++i)
    {
        CImage img;
        bool bLoaded = false;
        if (LoadPNG(img, avatarResIDs[i])) bLoaded = true;
    
        if (bLoaded)
        {
            CDC dcMem;
            dcMem.CreateCompatibleDC(&dcScreen);
            CBitmap bmp;
            bmp.CreateCompatibleBitmap(&dcScreen, 48, 48);
            CBitmap* pOldBmp = dcMem.SelectObject(&bmp);
            dcMem.SetStretchBltMode(HALFTONE);
            img.Draw(dcMem.GetSafeHdc(), 0, 0, 48, 48);
            dcMem.SelectObject(pOldBmp);
            m_imgListUser.Add(&bmp, (COLORREF)CLR_NONE);
        }
        else
        {
            HICON hDefault = ::LoadIcon(NULL, IDI_APPLICATION);
            m_imgListUser.Add(hDefault);
            ::DestroyIcon(hDefault);
        }
    }

    m_listUser.SetImageList(&m_imgListUser, LVSIL_SMALL);
}

void CChatDlg::OnNMClickListUser(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    *pResult = 0;
    int nItem = pNMItemActivate->iItem;
    if (nItem != -1){ 
        m_strSelectedUser = m_listUser.GetItemText(nItem, 0);
        ToggleChatUI(TRUE);
        m_richChat.SetWindowText(_T(""));
        RequestChatHistory(m_strSelectedUser);
    }
}

void CChatDlg::RequestUserList()
{
    json req = { {"action", "get_users"} };
    std::string strReq = req.dump();
    send(m_hSocket, strReq.c_str(), static_cast<int>(strReq.length()) + 1, 0);
}

void CChatDlg::RequestChatHistory(const CString& withUser)
{
    json req = {
        {"action", "get_chat_history"},
        {"data", {
            {"with_user", CW2A(withUser.GetString(), CP_UTF8)}
        }}
    };
    std::string strReq = req.dump();
    send(m_hSocket, strReq.c_str(), static_cast<int>(strReq.length()) + 1, 0);
}

void CChatDlg::OnBnClickedBtnSend()
{
    if (m_strSelectedUser.IsEmpty()) return;
    CString strMsg;
    GetDlgItemText(IDC_EDIT_MESSAGE, strMsg);
    strMsg.Trim();
    if (strMsg.IsEmpty()) return;

    json req = {
        {"action", "send_msg"},
        {"data", {
            {"to", CW2A(m_strSelectedUser.GetString(), CP_UTF8)},
            {"content", CW2A(strMsg.GetString(), CP_UTF8)}
        }}
    };

    std::string strReq = req.dump();
    send(m_hSocket, strReq.c_str(), static_cast<int>(strReq.length()) + 1, 0);

    CTime now = CTime::GetCurrentTime();
    int64_t currentTs = now.GetTime();
    CString strTimeFull = FormatTimestamp(currentTs);
    AppendChatMessage(m_strUsername, strMsg, strTimeFull);
    CString strTimeList = FormatUserListTimestamp(currentTs);
    UpdateUserLastMessage(m_strSelectedUser, strMsg, strTimeList);

    SetDlgItemText(IDC_EDIT_MESSAGE, _T(""));
}

DWORD WINAPI CChatDlg::ReceiverThreadProc(LPVOID lpParam)
{
    CChatDlg* pDlg = reinterpret_cast<CChatDlg*>(lpParam);
    std::string accumulatedBuffer;
    char szChunk[1024] = { 0 };

    while (pDlg->m_bRunning && pDlg->m_hSocket != INVALID_SOCKET){
        int nBytesRecv = recv(pDlg->m_hSocket, szChunk, sizeof(szChunk) - 1, 0);
        if (nBytesRecv > 0){
            accumulatedBuffer.append(szChunk, nBytesRecv);

            size_t nullPos = 0;
            while ((nullPos = accumulatedBuffer.find('\0')) != std::string::npos){
                std::string rawJson = accumulatedBuffer.substr(0, nullPos);
                accumulatedBuffer.erase(0, nullPos + 1);

                if (!rawJson.empty()){
                    std::string* pStrData = new std::string(rawJson);
                    pDlg->PostMessage(WM_RECV_JSON, 0, reinterpret_cast<LPARAM>(pStrData));
                }
            }
            ZeroMemory(szChunk, sizeof(szChunk));
        }
        else{
            break;
        }
    }
    return 0;
}

LRESULT CChatDlg::OnRecvJsonMessage(WPARAM wParam, LPARAM lParam)
{
    std::string* pRawJson = reinterpret_cast<std::string*>(lParam);
    if (!pRawJson) return 0;

    if (!::IsWindow(m_hWnd)){
        delete pRawJson;
        return 0;
    }

    try{
        auto res = json::parse(*pRawJson);
        std::string action = res.value("action", "");
        std::string status = res.value("status", "");

        if (action == "get_users_res" && status == "success"){ 
            if (m_listUser.GetSafeHwnd() && ::IsWindow(m_listUser.GetSafeHwnd())){
                m_listUser.DeleteAllItems();
                auto users = res["data"].value("users", std::vector<std::string>());

                int nTotalAvatars = m_imgListUser.GetImageCount();
                if (nTotalAvatars <= 0) nTotalAvatars = 1;

                int index = 0;
                for (const auto& user : users){
                    CString strUser(CA2T(user.c_str(), CP_UTF8));
                    int avatarIndex = index % nTotalAvatars;
                    m_listUser.InsertItem(index, strUser, avatarIndex);
                    m_listUser.SetItemText(index, 1, _T(""));
                    m_listUser.SetItemText(index, 2, _T(""));
                    index++;

                    RequestChatHistory(strUser);
                }
            }
        } 

        else if (action == "get_chat_history_res" && status == "success"){
            std::string withUser = res["data"].value("with_user", "");
            CString strWithUser(CA2T(withUser.c_str(), CP_UTF8));
            auto messages = res["data"]["messages"];

            if (messages.size() > 0){
                auto lastMsg = messages.back();
                std::string lastContent = lastMsg.value("content", "");
                int64_t lastTs = lastMsg.value("timestamp", (int64_t)0);
                CString strLastContent(CA2T(lastContent.c_str(), CP_UTF8));
                CString strLastTime = FormatUserListTimestamp(lastTs);
                UpdateUserLastMessage(strWithUser, strLastContent, strLastTime);
            }

            if (strWithUser.CompareNoCase(m_strSelectedUser) == 0 && m_richChat.GetSafeHwnd() && ::IsWindow(m_richChat.GetSafeHwnd())){
                m_richChat.SetRedraw(FALSE);
                m_richChat.SetWindowText(_T(""));

                for (const auto& msg : messages){
                    std::string sender = msg.value("sender", "");
                    std::string content = msg.value("content", "");
                    int64_t ts = msg.value("timestamp", (int64_t)0);

                    CString strSender(CA2T(sender.c_str(), CP_UTF8));
                    CString strContent(CA2T(content.c_str(), CP_UTF8));
                    CString strTime = FormatTimestamp(ts);

                    AppendChatMessage(strSender, strContent, strTime);
                }
                m_richChat.SetRedraw(TRUE);
                m_richChat.Invalidate();
                m_richChat.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
            }
        }

        else if (action == "receive_msg"){
            std::string fromUser = res["data"].value("from", "");
            std::string content = res["data"].value("content", "");
            int64_t ts = res["data"].value("timestamp", (int64_t)std::time(nullptr));

            CString strFromUser(CA2T(fromUser.c_str(), CP_UTF8));
            CString strContent(CA2T(content.c_str(), CP_UTF8));
            CString strTime = FormatTimestamp(ts);
            CString strTimeList = FormatUserListTimestamp(ts);

            if (strFromUser.CompareNoCase(m_strSelectedUser) == 0 && m_richChat.GetSafeHwnd() && ::IsWindow(m_richChat.GetSafeHwnd())){
                AppendChatMessage(strFromUser, strContent, strTime);
            }
            UpdateUserLastMessage(strFromUser, strContent, strTimeList);
        }
    }
    catch (...) {}

    delete pRawJson;
    return 0;
}

void CChatDlg::OnCancel()
{
    m_bRunning = FALSE;

    if (m_hSocket != INVALID_SOCKET){
        shutdown(m_hSocket, SD_BOTH);
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }

    if (m_hRecvThread){
        WaitForSingleObject(m_hRecvThread, 1000);
        CloseHandle(m_hRecvThread);
        m_hRecvThread = NULL;
    }

    CDialogEx::OnCancel();
}

void CChatDlg::UpdateUserLastMessage(const CString& strUser, const CString& strMsg, const CString& strTime)
{
    if (!m_listUser.GetSafeHwnd() || !::IsWindow(m_listUser.GetSafeHwnd())) return;

    CString strPreview = strMsg;
    strPreview.Replace(_T("\r\n"), _T(" "));
    strPreview.Replace(_T("\r"), _T(" "));
    strPreview.Replace(_T("\n"), _T(" "));

    for (int i = 0; i < m_listUser.GetItemCount(); ++i) {
        if (m_listUser.GetItemText(i, 0).CompareNoCase(strUser) == 0) {
            m_listUser.SetItemText(i, 1, strPreview);
            m_listUser.SetItemText(i, 2, strTime);
            break;
        }
    }
}

CString CChatDlg::FormatTimestamp(int64_t ts)
{
    if (ts <= 0) ts = static_cast<int64_t>(std::time(nullptr));
    CTime time(ts);
    return time.Format(_T("%d/%m/%Y %H:%M"));
}

CString CChatDlg::FormatUserListTimestamp(int64_t ts)
{
    if (ts <= 0) ts = static_cast<int64_t>(std::time(nullptr));
    CTime time(ts);
    CTime now = CTime::GetCurrentTime();

    if (time.GetYear() == now.GetYear() &&
        time.GetMonth() == now.GetMonth() &&
        time.GetDay() == now.GetDay())
    {
        return time.Format(_T("%H:%M"));
    }
    else{
        return time.Format(_T("%d/%m/%Y"));
    }
}

void CChatDlg::OnOK()
{
    OnBnClickedBtnSend();
}

void CChatDlg::AppendChatMessage(const CString& strSender, const CString& strContentOriginal, const CString& strTime)
{
    if (!m_richChat.GetSafeHwnd() || !::IsWindow(m_richChat.GetSafeHwnd())) return;

    BOOL bIsMe = (strSender.CompareNoCase(m_strUsername) == 0);
    LONG nLen = m_richChat.GetTextLength();
    m_richChat.SetSel(nLen, nLen);

    PARAFORMAT2 pf;
    ZeroMemory(&pf, sizeof(PARAFORMAT2));
    pf.cbSize = sizeof(PARAFORMAT2);

    pf.dwMask = PFM_ALIGNMENT | PFM_STARTINDENT | PFM_RIGHTINDENT;
    pf.wAlignment = bIsMe ? PFA_RIGHT : PFA_LEFT;
    pf.dxStartIndent = 100;
    pf.dxRightIndent = 100;
    m_richChat.SetParaFormat(pf);

    CHARFORMAT2 cfHeader;
    ZeroMemory(&cfHeader, sizeof(CHARFORMAT2));
    cfHeader.cbSize = sizeof(CHARFORMAT2);
    cfHeader.dwMask = CFM_COLOR | CFM_BOLD | CFM_FACE | CFM_SIZE;
    cfHeader.dwEffects = CFE_BOLD;          
    cfHeader.yHeight = 240;                
    _tcscpy_s(cfHeader.szFaceName, _T("Segoe UI"));

    if (bIsMe){
        cfHeader.crTextColor = RGB(220, 53, 69); // red
    }
    else {
        cfHeader.crTextColor = RGB(13, 110, 253); // blue
    }
    m_richChat.SetSelectionCharFormat(cfHeader);

    CString strHeader;
    if (bIsMe) {
        strHeader.Format(_T("Me [%s]\r\n"), strTime.GetString());
    }
    else { strHeader.Format(_T("%s [%s]\r\n"), strSender.GetString(), strTime.GetString()); }

    m_richChat.ReplaceSel(strHeader);

    CHARFORMAT2 cfContent;
    ZeroMemory(&cfContent, sizeof(CHARFORMAT2));
    cfContent.cbSize = sizeof(CHARFORMAT2);
    cfContent.dwMask = CFM_COLOR | CFM_BOLD | CFM_FACE | CFM_SIZE;
    cfContent.dwEffects = 0;                
    cfContent.crTextColor = RGB(30, 30, 30); 
    cfContent.yHeight = 240;                 
    _tcscpy_s(cfContent.szFaceName, _T("Segoe UI"));
    m_richChat.SetSelectionCharFormat(cfContent);

    CString strBody = strContentOriginal + _T("\r\n\r\n");
    m_richChat.ReplaceSel(strBody);
    m_richChat.SetSel(-1, -1);
    m_richChat.SendMessage(EM_SCROLLCARET, 0, 0);
    m_richChat.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

BOOL CChatDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN){
        CWnd* pEdit = GetDlgItem(IDC_EDIT_MESSAGE);
        if (pEdit && pMsg->hwnd == pEdit->GetSafeHwnd()){
            // Shift + Enter / Ctrl + Enter -> newline
            if ((GetKeyState(VK_SHIFT) & 0x8000) || (GetKeyState(VK_CONTROL) & 0x8000)){
                CEdit* pEditCtrl = reinterpret_cast<CEdit*>(pEdit);
                pEditCtrl->ReplaceSel(_T("\r\n"), TRUE); 
                return TRUE; 
            }
            // Enter -> Send
            else {
                OnBnClickedBtnSend();
                return TRUE;
            }
        }
    }
    return CDialogEx::PreTranslateMessage(pMsg);
}