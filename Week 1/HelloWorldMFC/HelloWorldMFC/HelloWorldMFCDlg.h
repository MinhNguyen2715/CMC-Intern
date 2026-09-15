
// HelloWorldMFCDlg.h : header file
//

#pragma once


// CHelloWorldMFCDlg dialog
class CHelloWorldMFCDlg : public CDialogEx
{
// Construction
public:
	CHelloWorldMFCDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HELLOWORLDMFC_DIALOG };
#endif

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	HICON m_hIcon;
	CString m_strFolderPath;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEnChangeEdit();
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnBnClickedBtnLoad();

	DECLARE_MESSAGE_MAP()
};
