
// ChatAppNamedPipeDlg.h : header file
//

#pragma once


// CChatAppNamedPipeDlg dialog
class CChatAppNamedPipeDlg : public CDialogEx
{
// Construction
public:
	CChatAppNamedPipeDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATAPPNAMEDPIPE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
