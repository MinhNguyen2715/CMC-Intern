
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
	DDX_Control(pDX, IDC_LIST_OVERVIEW, m_listOverview); 
	DDX_Control(pDX, IDC_LIST_DETAIL, m_listDetail);    
}

BEGIN_MESSAGE_MAP(CChatAppDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_LIST_OVERVIEW, &CChatAppDlg::OnNMClickListOverview)
END_MESSAGE_MAP()


// CChatAppDlg message handlers

BOOL CChatAppDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_fontList.CreateFont(
        30,                        // nHeight: Font height in pixels 
        0,                         // nWidth: Average character width (0 = auto)
        0,                         // nEscapement
        0,                         // nOrientation
        FW_NORMAL,                 // nWeight: FW_NORMAL / FW_BOLD
        FALSE,                     // bItalic
        FALSE,                     // bUnderline
        0,                         // cStrikeOut
        DEFAULT_CHARSET,           // nCharSet
        OUT_DEFAULT_PRECIS,        // nOutPrecision
        CLIP_DEFAULT_PRECIS,       // nClipPrecision
        DEFAULT_QUALITY,           // nQuality
        DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
        _T("Segoe UI")             // lpszFacename: Font face name
    );

    m_fontHeader.CreateFont(
        30,                        
        0,                         
        0,                         
        0,                         
        FW_BOLD,                  
        FALSE,                     
        FALSE,                     
        0,                         
        DEFAULT_CHARSET,           
        OUT_DEFAULT_PRECIS,        
        CLIP_DEFAULT_PRECIS,       
        DEFAULT_QUALITY,          
        DEFAULT_PITCH | FF_SWISS,  
        _T("Segoe UI")             
    );

    m_listOverview.SetFont(&m_fontList);
    m_listDetail.SetFont(&m_fontList);

    CHeaderCtrl* pHeaderOverview = m_listOverview.GetHeaderCtrl();
    if (pHeaderOverview != NULL)
    {
        pHeaderOverview->SetFont(&m_fontHeader);
    }

    CHeaderCtrl* pHeaderDetail = m_listDetail.GetHeaderCtrl();
    if (pHeaderDetail != NULL)
    {
        pHeaderDetail->SetFont(&m_fontHeader);
    }

    // Hide listDetail
    m_bShowDetailView = FALSE;

    // Styles
    m_listOverview.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
    m_listDetail.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

    // ListOverview (Overview: Time | Sender | Message)
    m_listOverview.InsertColumn(0, _T("Time"), LVCFMT_LEFT);
    m_listOverview.InsertColumn(1, _T("Sender"), LVCFMT_LEFT);
    m_listOverview.InsertColumn(2, _T("Message"), LVCFMT_LEFT);

    int nIndex = m_listOverview.InsertItem(0, _T("09:03 AM"));
    m_listOverview.SetItemText(nIndex, 1, _T("Alice"));
    m_listOverview.SetItemText(nIndex, 2, _T("Sounds good. Send me the calendar invite."));

    nIndex = m_listOverview.InsertItem(1, _T("10:21 AM"));
    m_listOverview.SetItemText(nIndex, 1, _T("Bob"));
    m_listOverview.SetItemText(nIndex, 2, _T("Checking it right now."));

    nIndex = m_listOverview.InsertItem(2, _T("11:35 AM"));
    m_listOverview.SetItemText(nIndex, 1, _T("Charlie"));
    m_listOverview.SetItemText(nIndex, 2, _T("Will do. See you later!"));

    nIndex = m_listOverview.InsertItem(3, _T("01:16 PM"));
    m_listOverview.SetItemText(nIndex, 1, _T("David"));
    m_listOverview.SetItemText(nIndex, 2, _T("Received! Thanks a lot!"));

    nIndex = m_listOverview.InsertItem(4, _T("02:50 PM"));
    m_listOverview.SetItemText(nIndex, 1, _T("Emma"));
    m_listOverview.SetItemText(nIndex, 2, _T("Got it! See you at the studio."));

    // ListDetail (Detail: Time | Sender: | Me)
    m_listDetail.InsertColumn(0, _T("Time"), LVCFMT_LEFT);
    m_listDetail.InsertColumn(1, _T("Sender:"), LVCFMT_LEFT);
    m_listDetail.InsertColumn(2, _T("Me"), LVCFMT_LEFT);

    m_listDetail.ShowWindow(SW_HIDE);

    // Maximize dialog window
    ShowWindow(SW_MAXIMIZE);

    return TRUE;
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

void CChatAppDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    // Verify HWNDs are valid before performing window layout repositioning
    if (m_listOverview.GetSafeHwnd() == NULL || m_listDetail.GetSafeHwnd() == NULL)
        return;

    if (!m_bShowDetailView)
    {
        // Detail view hidden -> Overview occupies full area
        m_listOverview.MoveWindow(0, 0, cx, cy, TRUE);

        // Adjust column widths proportionally for Left ListCtrl: 10 / 10 / 80
        m_listOverview.SetColumnWidth(0, cx / 10);
        m_listOverview.SetColumnWidth(1, cx / 10);
        m_listOverview.SetColumnWidth(2, cx / 10 * 8);
    }
    else
    {
        // Detail view visible -> Split area 50/50
        int nNewWidth = cx / 3;

        // Left ListCtrl occupies [0 -> nNewWidth]
        m_listOverview.MoveWindow(0, 0, nNewWidth, cy, TRUE);
        m_listOverview.SetColumnWidth(0, nNewWidth / 4);
        m_listOverview.SetColumnWidth(1, nNewWidth / 4);
        m_listOverview.SetColumnWidth(2, nNewWidth / 2);

        // Right ListCtrl occupies [nNewWidth -> cx]
        m_listDetail.MoveWindow(nNewWidth, 0, cx - nNewWidth, cy, TRUE);
        m_listDetail.SetColumnWidth(0, (cx - nNewWidth) / 10 * 2);
        m_listDetail.SetColumnWidth(1, (cx - nNewWidth) / 10 * 4);
        m_listDetail.SetColumnWidth(2, (cx - nNewWidth) / 10 * 4);
    }
}


// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CChatAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CChatAppDlg::OnNMClickListOverview(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    int nSelectedRow = pNMItemActivate->iItem;

    // Check if a valid item row was clicked
    if (nSelectedRow != -1)
    {
        UpdateDetailView(nSelectedRow);

        if (!m_bShowDetailView)
        {
            m_bShowDetailView = TRUE;

            // Reveal right ListCtrl using WinAPI ShowWindow
            m_listDetail.ShowWindow(SW_SHOW);

            // Trigger WM_SIZE to recalculate 50/50 layout
            CRect rectClient;
            GetClientRect(&rectClient);
            OnSize(SIZE_RESTORED, rectClient.Width(), rectClient.Height());
        }
    }

    *pResult = 0;
}

void CChatAppDlg::UpdateDetailView(int nRowIndex)
{
    // Clear previous detail items
    m_listDetail.DeleteAllItems();

    CString strSender = m_listOverview.GetItemText(nRowIndex, 1); 
    CString strMessage = m_listOverview.GetItemText(nRowIndex, 2); 

    // Rename column -> sender
    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT;
    CString strHeader = strSender;
    lvc.pszText = strHeader.GetBuffer();
    m_listDetail.SetColumn(1, &lvc);
    strHeader.ReleaseBuffer();

    // Load specific conversation messages into m_listDetail based on nRowIndex
    switch (nRowIndex)
    {
    case 0: // Conversation 0: Alice
    {
        int nIdx = m_listDetail.InsertItem(0, _T("09:00 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Hey, are you free for a quick meeting today?"));
        m_listDetail.SetItemText(nIdx, 2, _T(""));

        nIdx = m_listDetail.InsertItem(1, _T("09:01 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T(""));
        m_listDetail.SetItemText(nIdx, 2, _T("Sure, I have time around 2 PM."));

        nIdx = m_listDetail.InsertItem(2, _T("09:02 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Perfect! We need to discuss the new MFC UI updates."));
        m_listDetail.SetItemText(nIdx, 2, _T(""));

        // Last message matching Overview row 0
        nIdx = m_listDetail.InsertItem(3, _T("09:03 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T(""));
        m_listDetail.SetItemText(nIdx, 2, _T("Sounds good. Send me the calendar invite."));
        break;
    }

    case 1: // Conversation 1: Bob
    {
        int nIdx = m_listDetail.InsertItem(0, _T("10:15 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Did you finish analyzing the binary file logs?"));
        m_listDetail.SetItemText(nIdx, 2, _T(""));

        nIdx = m_listDetail.InsertItem(1, _T("10:16 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T(""));
        m_listDetail.SetItemText(nIdx, 2, _T("Yes, I identified the memory leak issue in the buffer stream."));

        nIdx = m_listDetail.InsertItem(2, _T("10:18 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Great work! Can you push the code fix to the repository?"));
        m_listDetail.SetItemText(nIdx, 2, _T(""));

        nIdx = m_listDetail.InsertItem(3, _T("10:20 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T(""));
        m_listDetail.SetItemText(nIdx, 2, _T("Already pushed. Please review the pull request whenever you are ready."));

        // Last message matching Overview row 1
        nIdx = m_listDetail.InsertItem(4, _T("10:21 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Checking it right now."));
        m_listDetail.SetItemText(nIdx, 2, _T(""));
        break;
    }

    case 2: // Conversation 2: Charlie
    {
        int nIdx = m_listDetail.InsertItem(0, _T("11:30 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Are we still playing game on Steam tonight?"));
        m_listDetail.SetItemText(nIdx, 2, _T(""));

        nIdx = m_listDetail.InsertItem(1, _T("11:32 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T(""));
        m_listDetail.SetItemText(nIdx, 2, _T("Yes! I will be online around 8 PM."));

        nIdx = m_listDetail.InsertItem(2, _T("11:33 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Awesome, invite the rest of the team as well."));
        m_listDetail.SetItemText(nIdx, 2, _T(""));

        // Last message matching Overview row 2
        nIdx = m_listDetail.InsertItem(3, _T("11:35 AM"));
        m_listDetail.SetItemText(nIdx, 1, _T(""));
        m_listDetail.SetItemText(nIdx, 2, _T("Will do. See you later!"));
        break;
    }

    case 3: // Conversation 3: David
    {
        int nIdx = m_listDetail.InsertItem(0, _T("01:10 PM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Hi! Do you have the CCNA study materials?"));
        m_listDetail.SetItemText(nIdx, 2, _T(""));

        nIdx = m_listDetail.InsertItem(1, _T("01:12 PM"));
        m_listDetail.SetItemText(nIdx, 1, _T(""));
        m_listDetail.SetItemText(nIdx, 2, _T("Yes, I have the full course notes and Packet Tracer labs."));

        nIdx = m_listDetail.InsertItem(2, _T("01:13 PM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Could you share the Google Drive link with me?"));
        m_listDetail.SetItemText(nIdx, 2, _T(""));

        nIdx = m_listDetail.InsertItem(3, _T("01:15 PM"));
        m_listDetail.SetItemText(nIdx, 1, _T(""));
        m_listDetail.SetItemText(nIdx, 2, _T("Sent to your email address."));

        // Last message matching Overview row 3
        nIdx = m_listDetail.InsertItem(4, _T("01:16 PM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Received! Thanks a lot!"));
        m_listDetail.SetItemText(nIdx, 2, _T(""));
        break;
    }

    case 4: // Conversation 4: Emma
    {
        int nIdx = m_listDetail.InsertItem(0, _T("02:45 PM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Don't forget guitar training lesson tomorrow evening."));
        m_listDetail.SetItemText(nIdx, 2, _T(""));

        nIdx = m_listDetail.InsertItem(1, _T("02:46 PM"));
        m_listDetail.SetItemText(nIdx, 1, _T(""));
        m_listDetail.SetItemText(nIdx, 2, _T("Thanks for reminding me! I almost forgot."));

        nIdx = m_listDetail.InsertItem(2, _T("02:47 PM"));
        m_listDetail.SetItemText(nIdx, 1, _T("Bring your acoustic guitar this time."));
        m_listDetail.SetItemText(nIdx, 2, _T(""));

        // Last message matching Overview row 4
        nIdx = m_listDetail.InsertItem(3, _T("02:50 PM"));
        m_listDetail.SetItemText(nIdx, 1, _T(""));
        m_listDetail.SetItemText(nIdx, 2, _T("Got it! See you at the studio."));
        break;
    }

    default:
        break;
    }
}