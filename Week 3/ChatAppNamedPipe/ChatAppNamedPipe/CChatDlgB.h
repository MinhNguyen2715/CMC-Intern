#pragma once
#include "afxdialogex.h"

// CChatDlgB dialog

class CChatDlgB : public CDialogEx
{
	DECLARE_DYNAMIC(CChatDlgB)

public:
	CChatDlgB(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CChatDlgB();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHAT_DLG_B };
#endif

private:
	HWND m_hWndPartner; // Save Dialog A's HWND 
	CFont m_font;
	void WrapMessageToLines(CDC* pDC, const CString& strInput, int nMaxWidth, CStringArray& outLines);
	void LayoutControls();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnSendB();
	DECLARE_MESSAGE_MAP()

public:
	void AppendMessage(const CString& strSender, const CString& strTextOriginal);

	
};
