#pragma once


// CRegisterDlg dialog

class CRegisterDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRegisterDlg)

public:
	CRegisterDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CRegisterDlg();
    CString m_strUsername;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REGISTER_DLG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedBtnSubmitRegister();
    DECLARE_MESSAGE_MAP()

private:
    bool ConnectAndRegister(const std::string& username, const std::string& password, std::string& outMsg);
    CFont m_fontTitle;
    CFont m_fontNormal;
};
