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

    CListCtrl  m_listUser;     
    CRichEditCtrl  m_richChat;  
    CImageList m_imgListUser;   
    CFont m_fontUI;

    CString m_strSelectedUser;
    HANDLE m_hRecvThread;
    BOOL m_bRunning;

    afx_msg void OnNMClickListUser(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedBtnSend();                              
    afx_msg LRESULT OnRecvJsonMessage(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()

private:
    void InitUserListControl();
    void ToggleChatUI(BOOL bShow);  
    void RequestUserList();
    void RequestChatHistory(const CString& withUser);
    void UpdateUserLastMessage(const CString& strUser, const CString& strMsg, const CString& strTime);
    void AppendChatMessage(const CString& strSender, const CString& strContentOriginal, const CString& strTime);
    CString FormatTimestamp(int64_t ts);           
    CString FormatUserListTimestamp(int64_t ts);   

    static DWORD WINAPI ReceiverThreadProc(LPVOID lpParam);
public:
};
