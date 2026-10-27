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
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnCancel();
    virtual void OnOK();

    // UI Control
    CListCtrl  m_listUser;      // IDC_LIST_USER
    CRichEditCtrl  m_richChat;  // IDC_LIST_CHAT 
    CImageList m_imgListUser;   // Icon / Avatar
    CFont m_fontUI;

    CString    m_strSelectedUser;
    HANDLE     m_hRecvThread;
    BOOL       m_bRunning;

    // Event Handlers
    afx_msg void OnNMClickListUser(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedBtnSend();                              
    afx_msg LRESULT OnRecvJsonMessage(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()

private:
    void InitUserListControl();
    // void InitChatListControl();
    void ToggleChatUI(BOOL bShow);  
    void RequestUserList();
    void RequestChatHistory(const CString& withUser);
    void UpdateUserLastMessage(const CString& strUser, const CString& strMsg, const CString& strTime);
    void AppendChatMessage(const CString& strSender, const CString& strContentOriginal, const CString& strTime);
    // void WrapMessageToLines(CDC* pDC, const CString& strInput, int nMaxWidth, CStringArray& outLines);
    CString FormatTimestamp(int64_t ts);           
    CString FormatUserListTimestamp(int64_t ts);   

    static DWORD WINAPI ReceiverThreadProc(LPVOID lpParam);
public:
};
