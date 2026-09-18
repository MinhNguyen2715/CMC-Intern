
// ChatAppDlg.cpp : implementation file
//

#include "pch.h"          
#include "ChatApp.h"
#include "ChatAppDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using json = nlohmann::json;

// CChatAppDlg dialog

CChatAppDlg::CChatAppDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATAPP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChatAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MESSAGE, m_listMessages);
}

BEGIN_MESSAGE_MAP(CChatAppDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CChatAppDlg message handlers

BOOL CChatAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	InitMessageList();
	PopulateFakeData();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CChatAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CChatAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CChatAppDlg::InitMessageList()
{
	// Enable full-row selection and grid lines
	m_listMessages.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// Insert Columns: InsertColumn(index, header_text, alignment, width_px)
	m_listMessages.InsertColumn(0, _T("Time"), LVCFMT_LEFT, 80);
	m_listMessages.InsertColumn(1, _T("Sender"), LVCFMT_LEFT, 100);
	m_listMessages.InsertColumn(2, _T("Message"), LVCFMT_LEFT, 280);


}

// 2. Load Mock / Seed Data
void CChatAppDlg::PopulateFakeData()
{
	// Clear existing items --> avoid duplication
	m_listMessages.DeleteAllItems();

    // 1. Open messages.json file
    ifstream inFile("message.json");
    if (!inFile.is_open())
    {
        AfxMessageBox(_T("Could not open messages.json file!"), MB_ICONWARNING);
        return;
    }

    json messageDb;
    try
    {
        inFile >> messageDb;
        inFile.close();
    }
    catch (const exception& e)
    {
        inFile.close();
        CString errStr(e.what());
        AfxMessageBox(_T("JSON Parsing Error: ") + errStr, MB_ICONERROR);
        return;
    }

    // 2. Iterate through JSON array and populate CListCtrl
    int nIndex = 0;
    for (const auto& item : messageDb)
    {
        // Safely check for required JSON fields
        if (item.contains("time") && item.contains("sender") && item.contains("message"))
        {
            string strTime = item["time"].get<std::string>();
            string strSender = item["sender"].get<std::string>();
            string strMessage = item["message"].get<std::string>();

            // Convert std::string (UTF-8/ANSI) to MFC CString
            CString cstrTime(strTime.c_str());
            CString cstrSender(strSender.c_str());
            CString cstrMessage(strMessage.c_str());

            // Insert new row into CListCtrl
            int nRow = m_listMessages.InsertItem(nIndex, cstrTime);
            m_listMessages.SetItemText(nRow, 1, cstrSender);
            m_listMessages.SetItemText(nRow, 2, cstrMessage);

            nIndex++;
        }
    }

    // Auto-scroll to the bottom of the message list
    if (m_listMessages.GetItemCount() > 0)
    {
        m_listMessages.EnsureVisible(m_listMessages.GetItemCount() - 1, FALSE);
    }
}

