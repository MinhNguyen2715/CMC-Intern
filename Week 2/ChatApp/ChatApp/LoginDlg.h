#pragma once


// CLoginDlg dialog

class CLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLoginDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LOGIN };
#endif
private:
	CFont m_fontStatic;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;

	afx_msg void OnBnClickedBtnLogReg();
	afx_msg void OnBnClickedBtnLogLog();
	afx_msg BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()

};
