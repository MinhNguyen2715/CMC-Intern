// CChatDlgA.cpp : implementation file
//

#include "pch.h"
#include "resource.h"
#include "ChatServer.h"
#include "CChatDlgA.h"
#include "afxdialogex.h"


// CChatDlgA dialog

IMPLEMENT_DYNAMIC(CChatDlgA, CDialogEx)

CChatDlgA::CChatDlgA(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_DLG_A, pParent),
	m_listenSocket(INVALID_SOCKET),
	m_clientSocket(INVALID_SOCKET),
	m_hListenThread(NULL),
	m_bRunning(FALSE)
{
}

CChatDlgA::~CChatDlgA()
{
	// Stop background thread
	m_bRunning = FALSE;

	// Close listening socket to unblock accept()
	if (m_listenSocket != INVALID_SOCKET)
	{
		::closesocket(m_listenSocket);
		m_listenSocket = INVALID_SOCKET;
	}

	// Close active client socket to unblock recv()
	if (m_clientSocket != INVALID_SOCKET)
	{
		::shutdown(m_clientSocket, SD_BOTH);
		::closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
	}

	// Wait for worker thread to exit safely
	if (m_hListenThread != NULL)
	{
		::WaitForSingleObject(m_hListenThread, 200);
		::CloseHandle(m_hListenThread);
		m_hListenThread = NULL;
	}

	// Cleanup Winsock
	::WSACleanup();
}

void CChatDlgA::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChatDlgA, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SEND_A, &CChatDlgA::OnBnClickedBtnSendA)
	ON_MESSAGE(WM_SOCKET_MSG_RECEIVED, &CChatDlgA::OnSocketMsgReceived) 
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CChatDlgA message handlers

BOOL CChatDlgA::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText(_T("Chat TCP Server (Dialog A)"));

	// Set window position and size
	CRect rectWorkArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkArea, 0);

	int nWidth = 900;
	int nHeight = 800;
	int x = rectWorkArea.left + 20;
	int y = rectWorkArea.top + (rectWorkArea.Height() - nHeight) / 2;

	SetWindowPos(NULL, x, y, nWidth, nHeight, SWP_NOZORDER);

	// Winsock DLL
	WSADATA wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		AfxMessageBox(_T("WSAStartup failed!"));
		return FALSE;
	}

	// TCP Server Listening Socket
	m_listenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listenSocket == INVALID_SOCKET)
	{
		::WSACleanup();
		AfxMessageBox(_T("Failed to create listening socket!"));
		return FALSE;
	}

	// Bind socket to port 
	sockaddr_in serverAddr = { 0 };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT_CHAT); // Default 2715
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (::bind(m_listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		::closesocket(m_listenSocket);
		::WSACleanup();
		AfxMessageBox(_T("Bind failed on port 2715!"));
		return FALSE;
	}

	// Start listening for incoming client connections (Backlog queue: 5)
	if (::listen(m_listenSocket, 5) == SOCKET_ERROR)
	{
		::closesocket(m_listenSocket);
		::WSACleanup();
		AfxMessageBox(_T("Listen failed!"));
		return FALSE;
	}

	// Create background worker thread for TCP communication
	m_bRunning = TRUE;
	m_hListenThread = ::CreateThread(NULL, 0, SocketServerThread, this, 0, NULL);

	// Setup font, ListControl and EditBox UI 
	if (m_font.GetSafeHandle() == NULL)
	{
		m_font.CreateFont(
			24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Segoe UI")
		);
	}

	CListCtrl* pList = static_cast<CListCtrl*>(GetDlgItem(IDC_LIST_CHAT_A));
	if (pList != NULL && pList->GetSafeHwnd() != NULL)
	{
		pList->SetFont(&m_font);
		pList->ModifyStyle(0, LVS_NOCOLUMNHEADER);
		pList->SetExtendedStyle(LVS_EX_FULLROWSELECT);
		pList->ModifyStyle(WS_HSCROLL, 0);

		CRect rectList;
		pList->GetClientRect(&rectList);
		int nColumnWidth = (rectList.Width() > 100) ? (rectList.Width() - 5) / 2 : 400;

		pList->InsertColumn(0, _T("B"), LVCFMT_LEFT, nColumnWidth);
		pList->InsertColumn(1, _T("Me"), LVCFMT_RIGHT, nColumnWidth);
	}

	LayoutControls();

	CWnd* pEdit = GetDlgItem(IDC_EDIT_MSG_A);
	if (pEdit != NULL && pEdit->GetSafeHwnd() != NULL)
	{
		pEdit->SetFont(&m_font);
		pEdit->ModifyStyle(0, ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL);
	}

	return TRUE;
}

void CChatDlgA::LayoutControls()
{
	CWnd* pList = GetDlgItem(IDC_LIST_CHAT_A);
	CWnd* pEdit = GetDlgItem(IDC_EDIT_MSG_A);
	CWnd* pBtn = GetDlgItem(IDC_BTN_SEND_A);

	if (pList == NULL || pList->GetSafeHwnd() == NULL ||
		pEdit == NULL || pEdit->GetSafeHwnd() == NULL ||
		pBtn == NULL || pBtn->GetSafeHwnd() == NULL)
	{
		return;
	}

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

DWORD WINAPI CChatDlgA::SocketServerThread(LPVOID lpParam)
{
	CChatDlgA* pDlg = reinterpret_cast<CChatDlgA*>(lpParam);

	while (pDlg->m_bRunning)
	{
		// Accept incoming client connection
		SOCKET clientSock = ::accept(pDlg->m_listenSocket, NULL, NULL);
		if (clientSock == INVALID_SOCKET) break;

		pDlg->m_clientSocket = clientSock;
		TCHAR szBuffer[1024] = { 0 };
		CString strFullMessage = _T("");

		// Loop to receive TCP stream data
		while (pDlg->m_bRunning)
		{
			int nBytesRecv = ::recv(clientSock, (char*)szBuffer, sizeof(szBuffer) - sizeof(TCHAR), 0);
			if (nBytesRecv > 0)
			{
				szBuffer[nBytesRecv / sizeof(TCHAR)] = _T('\0');
				strFullMessage += szBuffer;

				// Post message payload to UI thread
				CString* pMsg = new CString(strFullMessage);
				pDlg->PostMessage(WM_SOCKET_MSG_RECEIVED, 0, reinterpret_cast<LPARAM>(pMsg));

				strFullMessage = _T("");
				ZeroMemory(szBuffer, sizeof(szBuffer));
			}
			else
			{
				// Client disconnected or error occurred
				break;
			}
		}

		::closesocket(clientSock);
		pDlg->m_clientSocket = INVALID_SOCKET;
	}
	return 0;
}

LRESULT CChatDlgA::OnSocketMsgReceived(WPARAM wParam, LPARAM lParam)
{
	CString* pMsg = reinterpret_cast<CString*>(lParam);
	if (pMsg != NULL)
	{
		AppendMessage(_T("Client"), *pMsg);
		delete pMsg;
	}
	return 0;
}


void CChatDlgA::OnBnClickedBtnSendA()
{
	CString strMsg;
	GetDlgItemText(IDC_EDIT_MSG_A, strMsg);
	if (strMsg.IsEmpty()) return;

	if (m_clientSocket != INVALID_SOCKET)
	{
		// Timestamp when sent
		CTime time = CTime::GetCurrentTime();
		CString strPacket;
		strPacket.Format(_T("[%s] %s"), time.Format(_T("%H:%M:%S")).GetString(), strMsg.GetString());

		// Send
		DWORD cbToWrite = (strPacket.GetLength() + 1) * sizeof(TCHAR);
		::send(m_clientSocket, (const char*)strPacket.GetString(), cbToWrite, 0);

		// UI
		AppendMessage(_T("Me"), strPacket);
		SetDlgItemText(IDC_EDIT_MSG_A, _T(""));
	}
}

void CChatDlgA::AppendMessage(const CString& strSender, const CString& strTextOriginal)
{
	CListCtrl* pList = static_cast<CListCtrl*>(GetDlgItem(IDC_LIST_CHAT_A));
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

		// Wrapping function
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

void CChatDlgA::WrapMessageToLines(CDC* pDC, const CString& strInput, int nMaxWidth, CStringArray& outLines) 
{
	outLines.RemoveAll();
	if (strInput.IsEmpty() || nMaxWidth <= 0 || pDC == NULL)
	{
		outLines.Add(strInput);
		return;
	}

	// Split using space
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

	// If word > maxWidth -> next line
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
				// A word with no space -> next line
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

void CChatDlgA::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (GetSafeHwnd() != NULL)
	{
		LayoutControls();
	}
}