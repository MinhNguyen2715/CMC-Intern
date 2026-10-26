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
	enum { IDD = IDD_LOGIN_DLG };
#endif
	
public:
	SOCKET  m_hSocket;
	CString m_strUsername;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnBnClickedBtnLogin();
	afx_msg void OnBnClickedBtnRegister();

	DECLARE_MESSAGE_MAP()

private:
	bool ConnectToServer();
	CFont m_fontTitle;  
	CFont m_fontNormal; 
};
