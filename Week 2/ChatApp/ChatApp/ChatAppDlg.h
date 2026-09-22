
// ChatAppDlg.h : header file
//

#pragma once


// CChatAppDlg dialog
class CChatAppDlg : public CDialogEx
{
// Construction
public:
	CChatAppDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATAPP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
private: 
	CListCtrl m_listOverview;
	CListCtrl m_listDetail;
	CFont m_fontList;
	CFont m_fontHeader;
	BOOL m_bShowDetailView;
	void UpdateDetailView(int nRowIndex);

protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMClickListOverview(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
public:
	
};
