#pragma once
#include <atlimage.h>

// CChatDlg dialog
#define WM_RECV_JSON (WM_USER + 101)

class CChatDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChatDlg)

public:
	CChatDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CChatDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHAT_DLG };
#endif
public:
	SOCKET  m_hSocket;
	CString m_strUsername;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    virtual void OnOK();

    // Các Control trên Giao diện
    CListCtrl  m_listUser;      // IDC_LIST_USER (Danh sách bạn bè/users)
    CListCtrl  m_listChat;      // IDC_LIST_CHAT (Lịch sử chat)
    CImageList m_imgListUser;   // Danh sách Icon / Avatar
    CFont m_fontUI;

    CString    m_strSelectedUser;
    HANDLE     m_hRecvThread;
    BOOL       m_bRunning;

    // Event Handlers
    afx_msg void OnNMClickListUser(NMHDR* pNMHDR, LRESULT* pResult); // Sự kiện click chọn User
    afx_msg void OnBnClickedBtnSend();                              // Sự kiện bấm nút Send
    afx_msg LRESULT OnRecvJsonMessage(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()

private:
    void InitUserListControl();
    void InitChatListControl();
    void ToggleChatUI(BOOL bShow);  // Hàm bật/tắt hiển thị khung Chat
    void RequestUserList();
    void RequestChatHistory(const CString& withUser);
    void UpdateUserLastMessage(const CString& strUser, const CString& strMsg, const CString& strTime);
    void AppendChatMessage(const CString& strSender, const CString& strContentOriginal, const CString& strTime);
    void WrapMessageToLines(CDC* pDC, const CString& strInput, int nMaxWidth, CStringArray& outLines);
    CString FormatTimestamp(int64_t ts);           // Dùng cho khung chat bên phải (%d/%m/%Y %H:%M)
    CString FormatUserListTimestamp(int64_t ts);   // Dùng cho danh sách bên trái (Tự phân loại Hôm nay vs Khác ngày)

    static DWORD WINAPI ReceiverThreadProc(LPVOID lpParam);
public:
};
