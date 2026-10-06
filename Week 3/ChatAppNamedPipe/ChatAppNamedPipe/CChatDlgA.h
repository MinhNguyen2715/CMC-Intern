#pragma once


// CChatDlgA dialog
#pragma once
#include "afxdialogex.h"

class CChatDlgA : public CDialogEx
{
	DECLARE_DYNAMIC(CChatDlgA)

public:
	CChatDlgA(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CChatDlgA();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHAT_DLG_A };
#endif

private:
	HANDLE m_hPipe;          // Handle of Named Pipe Server
	HANDLE m_hListenThread;  // Handle of Worker Thread
	BOOL   m_bRunning;       // Flag for thread status
	static DWORD WINAPI PipeServerThread(LPVOID lpParam); 

	HWND m_hWndPartner; // Save Dialog B's HWND 
	CFont m_font;
	void WrapMessageToLines(CDC* pDC, const CString& strInput, int nMaxWidth, CStringArray& outLines);
	void LayoutControls();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnPipeMsgReceived(WPARAM wParam, LPARAM lParam); 
	afx_msg void OnBnClickedBtnSendA();
	DECLARE_MESSAGE_MAP()

public:
	void AppendMessage(const CString& strSender, const CString& strTextOriginal);
};
