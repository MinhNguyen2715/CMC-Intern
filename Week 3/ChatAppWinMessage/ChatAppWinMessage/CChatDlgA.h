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
	HWND m_hWndPartner; // Save Dialog B's HWND 
	CFont m_font;
	void AppendMessage(const CString& strSender, const CString& strText);
	void WrapMessageToLines(CDC* pDC, const CString& strInput, int nMaxWidth, CStringArray& outLines);
	void LayoutControls();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedBtnSendA();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);

	DECLARE_MESSAGE_MAP()
};
