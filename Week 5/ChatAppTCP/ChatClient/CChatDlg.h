#pragma once
#include "afxdialogex.h"

// CChatDlg dialog

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

private:
	SOCKET m_hSocket;
	BOOL m_bRunning;

	CFont m_font;
	void WrapMessageToLines(CDC* pDC, const CString& strInput, int nMaxWidth, CStringArray& outLines);
	void LayoutControls();
	static DWORD WINAPI SocketClientThread(LPVOID lpParam);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnSend();
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnSocketMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);

public:
	void AppendMessage(const CString& strSender, const CString& strTextOriginal);
};
