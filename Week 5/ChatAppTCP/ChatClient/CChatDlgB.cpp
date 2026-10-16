// CChatDlgB.cpp : implementation file
//

#include "pch.h"
#include "resource.h"
#include "ChatClient.h"  
#include "CChatDlgB.h"
#include "afxdialogex.h"


// CChatDlgB dialog

IMPLEMENT_DYNAMIC(CChatDlgB, CDialogEx)

CChatDlgB::CChatDlgB(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_DLG_B, pParent),
	m_hWndPartner(NULL),
	m_hSocket(INVALID_SOCKET), 
	m_bRunning(FALSE)         
{
}

CChatDlgB::~CChatDlgB()
{
	// Stop thread
	m_bRunning = FALSE;

	// Close socket
	if (m_hSocket != INVALID_SOCKET)
	{
		::shutdown(m_hSocket, SD_BOTH);
		::closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}

	// Cleanup Winsock
	::WSACleanup();
}

void CChatDlgB::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChatDlgB, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SEND_B, &CChatDlgB::OnBnClickedBtnSendB)
	ON_MESSAGE(WM_SOCKET_MSG_RECEIVED, &CChatDlgB::OnSocketMsgReceived) 
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CChatDlgB message handlers

BOOL CChatDlgB::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	RegisterAutoRunSilently(_T("MFC_ChatClient_AppB"));

	SetWindowText(_T("Chat TCP Client (Dialog B)"));

	CRect rectWorkArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkArea, 0);

	int nWidth = 900;
	int nHeight = 800;

	int x = rectWorkArea.left + (rectWorkArea.Width() / 2) + 20;
	int y = rectWorkArea.top + (rectWorkArea.Height() - nHeight) / 2;

	SetWindowPos(NULL, x, y, nWidth, nHeight, SWP_NOZORDER);

	if (m_font.GetSafeHandle() == NULL)
	{
		m_font.CreateFont(
			24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Segoe UI")
		);
	}

	// List control 
	CListCtrl* pList = static_cast<CListCtrl*>(GetDlgItem(IDC_LIST_CHAT_B));
	if (pList != NULL && pList->GetSafeHwnd() != NULL)
	{
		pList->SetFont(&m_font);
		pList->ModifyStyle(0, LVS_NOCOLUMNHEADER);
		pList->ModifyStyle(WS_HSCROLL, 0);
		pList->SetExtendedStyle(LVS_EX_FULLROWSELECT);

		CRect rectList;
		pList->GetClientRect(&rectList);
		int nColumnWidth = (rectList.Width() > 100) ? (rectList.Width() - 5) / 2 : 400; 

		pList->InsertColumn(0, _T("A"), LVCFMT_LEFT, nColumnWidth);
		pList->InsertColumn(1, _T("Me"), LVCFMT_RIGHT, nColumnWidth);
	}

	LayoutControls();

	// Edit box 
	CWnd* pEdit = GetDlgItem(IDC_EDIT_MSG_B);
	if (pEdit != NULL && pEdit->GetSafeHwnd() != NULL)
	{
		pEdit->SetFont(&m_font);

		pEdit->ModifyStyle(0, ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL);
	}

	WSADATA wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
	{
		m_hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_hSocket != INVALID_SOCKET)
		{
			sockaddr_in serverAddr = { 0 };
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(PORT_CHAT);

			CT2A asciiIP(SERVER_IP);
			::inet_pton(AF_INET, asciiIP, &serverAddr.sin_addr);

			if (::connect(m_hSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) != SOCKET_ERROR)
			{
				// Connection successful -> Create thread
				m_bRunning = TRUE;
				::CreateThread(NULL, 0, SocketClientThread, this, 0, NULL);
			}
			else
			{
				::closesocket(m_hSocket);
				m_hSocket = INVALID_SOCKET;
			}
		}
	}

	return TRUE;
}

void CChatDlgB::LayoutControls()
{
	CWnd* pList = GetDlgItem(IDC_LIST_CHAT_B);
	CWnd* pEdit = GetDlgItem(IDC_EDIT_MSG_B);
	CWnd* pBtn = GetDlgItem(IDC_BTN_SEND_B);

	// Handle safety
	if (pList == NULL || pList->GetSafeHwnd() == NULL ||
		pEdit == NULL || pEdit->GetSafeHwnd() == NULL ||
		pBtn == NULL || pBtn->GetSafeHwnd() == NULL)
	{
		return;
	}

	// Dialog area
	CRect rectClient;
	GetClientRect(&rectClient);
	int cx = rectClient.Width();
	int cy = rectClient.Height();

	// Margin, distance
	int nMargin = 15;      
	int nBtnWidth = 100;   
	int nBtnHeight = 35;  
	int nEditHeight = 80;  
	int nSpacing = 10;     

	// Send button (middle bottom)
	int nBtnX = (cx - nBtnWidth) / 2;
	int nBtnY = cy - nMargin - nBtnHeight;
	pBtn->MoveWindow(nBtnX, nBtnY, nBtnWidth, nBtnHeight, TRUE);

	// Edit box (Above Send btn, full width)
	int nEditX = nMargin;
	int nEditY = nBtnY - nSpacing - nEditHeight;
	int nEditWidth = cx - (nMargin * 2);
	pEdit->MoveWindow(nEditX, nEditY, nEditWidth, nEditHeight, TRUE);

	// List control (Remaining space)
	int nListX = nMargin;
	int nListY = nMargin;
	int nListWidth = cx - (nMargin * 2);
	int nListHeight = nEditY - nSpacing - nListY;
	pList->MoveWindow(nListX, nListY, nListWidth, nListHeight, TRUE);

	// Split into 2 columns
	CListCtrl* pListCtrl = static_cast<CListCtrl*>(pList);
	if (pListCtrl != NULL && pListCtrl->GetSafeHwnd() != NULL)
	{
		int nColWidth = (nListWidth - 25) / 2;
		if (nColWidth < 100) nColWidth = 100;

		pListCtrl->SetColumnWidth(0, nColWidth);
		pListCtrl->SetColumnWidth(1, nColWidth); 
	}
}

DWORD WINAPI CChatDlgB::SocketClientThread(LPVOID lpParam)
{
	CChatDlgB* pDlg = reinterpret_cast<CChatDlgB*>(lpParam);
	if (!pDlg || pDlg->m_hSocket == INVALID_SOCKET) return 0;

	TCHAR szBuffer[1024] = { 0 };
	while (pDlg->m_bRunning)
	{
		int nBytesRecv = recv(pDlg->m_hSocket, (char*)szBuffer, sizeof(szBuffer) - sizeof(TCHAR), 0);
		if (nBytesRecv > 0)
		{
			szBuffer[nBytesRecv / sizeof(TCHAR)] = _T('\0');
			CString* pMsg = new CString(szBuffer);
			pDlg->PostMessage(WM_SOCKET_MSG_RECEIVED, 0, reinterpret_cast<LPARAM>(pMsg));
			ZeroMemory(szBuffer, sizeof(szBuffer));
		}
		else
		{
			break; // Server stops connections
		}
	}
	return 0;
}

LRESULT CChatDlgB::OnSocketMsgReceived(WPARAM wParam, LPARAM lParam)
{
	CString* pMsg = reinterpret_cast<CString*>(lParam);
	if (pMsg != NULL)
	{
		AppendMessage(_T("Server"), *pMsg);
		delete pMsg;
	}
	return 0;
}

void CChatDlgB::OnBnClickedBtnSendB()
{
	CString strMsg;
	GetDlgItemText(IDC_EDIT_MSG_B, strMsg);
	if (strMsg.IsEmpty()) return;

	if (m_hSocket != INVALID_SOCKET)
	{
		CTime time = CTime::GetCurrentTime();
		CString strPacket;
		strPacket.Format(_T("[%s] %s"), time.Format(_T("%H:%M:%S")).GetString(), strMsg.GetString());

		DWORD cbToWrite = (strPacket.GetLength() + 1) * sizeof(TCHAR);
		::send(m_hSocket, (const char*)strPacket.GetString(), cbToWrite, 0);

		AppendMessage(_T("Me"), strPacket);
		SetDlgItemText(IDC_EDIT_MSG_B, _T(""));
	}

	else
	{
		AfxMessageBox(_T("No connection"));
	}
}

void CChatDlgB::AppendMessage(const CString& strSender, const CString& strTextOriginal)
{
	CListCtrl* pList = static_cast<CListCtrl*>(GetDlgItem(IDC_LIST_CHAT_B));
	if (pList == NULL || pList->GetSafeHwnd() == NULL)
		return;

	CDC* pDC = pList->GetDC();
	if (pDC == NULL)
		return;

	CFont* pFont = pList->GetFont();
	CFont* pOldFont = NULL;
	if (pFont != NULL)
	{
		pOldFont = pDC->SelectObject(pFont);
	}

	BOOL bIsMe = (strSender.CompareNoCase(_T("Me")) == 0);
	int nColIndex = bIsMe ? 1 : 0;
	int nColWidth = pList->GetColumnWidth(nColIndex);
	int nMaxWidth = (nColWidth > 20) ? (nColWidth - 20) : 100;

	CStringArray rawLines;
	int nPos = 0;
	CString strSub;

	CString strInput = strTextOriginal;
	strInput.Replace(_T("\r\n"), _T("\n"));
	strInput.Replace(_T("\r"), _T("\n"));

	while (AfxExtractSubString(strSub, strInput, nPos, _T('\n')))
	{
		rawLines.Add(strSub);
		nPos++;
	}

	if (rawLines.GetCount() == 0)
	{
		rawLines.Add(strInput);
	}

	BOOL bIsFirstLine = TRUE;
	for (INT_PTR k = 0; k < rawLines.GetCount(); k++)
	{
		CString strLineToWrap = rawLines.GetAt(k);

		CStringArray lines;
		WrapMessageToLines(pDC, strLineToWrap, nMaxWidth, lines);

		for (INT_PTR i = 0; i < lines.GetCount(); i++)
		{
			CString strLine = lines.GetAt(i);
			int nRow = pList->GetItemCount();

			if (bIsMe)
			{
				int nIndex = pList->InsertItem(nRow, _T(""));
				pList->SetItemText(nIndex, 1, strLine);
			}
			else
			{
				pList->InsertItem(nRow, strLine);
			}

			pList->EnsureVisible(nRow, FALSE);
		}
	}

	if (pOldFont != NULL)
	{
		pDC->SelectObject(pOldFont);
	}
	pList->ReleaseDC(pDC);
}

void CChatDlgB::WrapMessageToLines(CDC* pDC, const CString& strInput, int nMaxWidth, CStringArray& outLines) 
{
	outLines.RemoveAll();
	if (strInput.IsEmpty() || nMaxWidth <= 0 || pDC == NULL)
	{
		outLines.Add(strInput);
		return;
	}

	CStringArray words;
	int nPos = 0;
	CString strWord;
	while (AfxExtractSubString(strWord, strInput, nPos, _T(' ')))
	{
		words.Add(strWord);
		nPos++;
	}

	if (words.GetCount() == 0)
	{
		outLines.Add(strInput);
		return;
	}

	CString strCurrentLine = _T("");

	for (INT_PTR i = 0; i < words.GetCount(); i++)
	{
		CString strW = words.GetAt(i);
		CString strTestLine = strCurrentLine.IsEmpty() ? strW : (strCurrentLine + _T(" ") + strW);

		if (pDC->GetTextExtent(strTestLine).cx <= nMaxWidth)
		{
			strCurrentLine = strTestLine;
		}
		else
		{
			if (!strCurrentLine.IsEmpty())
			{
				outLines.Add(strCurrentLine);
				strCurrentLine = strW; 
			}
			else
			{
				outLines.Add(strW);
				strCurrentLine = _T("");
			}
		}
	}

	if (!strCurrentLine.IsEmpty())
	{
		outLines.Add(strCurrentLine);
	}
}

void CChatDlgB::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (GetSafeHwnd() != NULL)
	{
		LayoutControls();
	}
}