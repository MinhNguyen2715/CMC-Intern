// CChatDlgA.cpp : implementation file
//

#include "pch.h"
#include "ChatAppNamedPipe.h"
#include "CChatDlgA.h"
#include "CChatDlgB.h"
#include "afxdialogex.h"


// CChatDlgA dialog

IMPLEMENT_DYNAMIC(CChatDlgA, CDialogEx)

CChatDlgA::CChatDlgA(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_DLG_A, pParent),
	m_hPipe(INVALID_HANDLE_VALUE),
	m_hListenThread(NULL),
	m_bRunning(FALSE),
	m_hWndPartner(NULL)
{
}

CChatDlgA::~CChatDlgA()
{
	// Stop thread
	m_bRunning = FALSE;

	// Close Handle Pipe (Unblock ReadFile / ConnectNamedPipe)
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		::CancelIoEx(m_hPipe, NULL);
		::CloseHandle(m_hPipe);
		m_hPipe = INVALID_HANDLE_VALUE;
	}

	// Wait thread
	if (m_hListenThread != NULL)
	{
		::WaitForSingleObject(m_hListenThread, 100);
		::CloseHandle(m_hListenThread);
		m_hListenThread = NULL;
	}
}


void CChatDlgA::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChatDlgA, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SEND_A, &CChatDlgA::OnBnClickedBtnSendA)
	ON_MESSAGE(WM_PIPE_MSG_RECEIVED, &CChatDlgA::OnPipeMsgReceived)
END_MESSAGE_MAP()

// CChatDlgA message handlers

BOOL CChatDlgA::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText(_T("Chat Dialog A"));

	// Position and size
	CRect rectWorkArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkArea, 0);

	int nWidth = 900;
	int nHeight = 800;
	int x = rectWorkArea.left + 20;
	int y = rectWorkArea.top + (rectWorkArea.Height() - nHeight) / 2;

	SetWindowPos(NULL, x, y, nWidth, nHeight, SWP_NOZORDER);

	//m_hPipe = ::CreateNamedPipe(
	//	CHAT_PIPE_NAME,                                         // (1) Tên của Pipe
	//	PIPE_ACCESS_DUPLEX,                                     // (2) Chế độ truy cập 2 chiều
	//	PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,  // (3) Chế độ truyền/đọc/chờ dữ liệu
	//	1,                                                      // (4) Số lượng Instance tối đa
	//	4096,                                                   // (5) Kích thước Outbuffer (Gửi)
	//	4096,                                                   // (6) Kích thước Inbuffer (Nhận)
	//	0,                                                      // (7) Timeout mặc định
	//	NULL                                                    // (8) Thuộc tính bảo mật (Security Attributes)
	//);

	// Named pipe server (Duplex)
	m_hPipe = CreateNamedPipe(
		CHAT_PIPE_NAME,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1, 4096, 4096, 0, NULL
	);

	// Worker thread
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		m_bRunning = TRUE;
		m_hListenThread = CreateThread(NULL, 0, PipeServerThread, this, 0, NULL);
	}

	if (m_font.GetSafeHandle() == NULL)
	{
		m_font.CreateFont(
			24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Segoe UI")
		);
	}
	// List control 
	CListCtrl* pList = static_cast<CListCtrl*>(GetDlgItem(IDC_LIST_CHAT_A));
	if (pList != NULL && pList->GetSafeHwnd() != NULL)
	{
		pList->SetFont(&m_font);
		pList->ModifyStyle(0, LVS_NOCOLUMNHEADER);
		pList->SetExtendedStyle(LVS_EX_FULLROWSELECT);

		pList->InsertColumn(0, _T(""), LVCFMT_LEFT, 0);
		pList->InsertColumn(1, _T(""), LVCFMT_RIGHT, 0);
	}

	LayoutControls();

	// Edit box 
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
	int nColWidth = (nListWidth - 5) / 2;
	pListCtrl->SetColumnWidth(0, nColWidth);
	pListCtrl->SetColumnWidth(1, nColWidth);
}

DWORD WINAPI CChatDlgA::PipeServerThread(LPVOID lpParam)
{
	CChatDlgA* pDlg = reinterpret_cast<CChatDlgA*>(lpParam);

	while (pDlg->m_bRunning)
	{
		// Wait client to connect
		BOOL bConnected = ::ConnectNamedPipe(pDlg->m_hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (bConnected)
		{
			TCHAR szBuffer[1024] = { 0 };
			DWORD dwRead = 0;

			CString strFullMessage = _T("");

			while (TRUE)
			{
				BOOL bSuccess = ::ReadFile(pDlg->m_hPipe, szBuffer, sizeof(szBuffer) - sizeof(TCHAR), &dwRead, NULL);
				DWORD dwError = ::GetLastError();

				if (bSuccess && dwRead > 0)
				{
					// Case 1: finish
					szBuffer[dwRead / sizeof(TCHAR)] = _T('\0');

					// Append to result string
					strFullMessage += szBuffer;

					if (!strFullMessage.IsEmpty())
					{
						CString* pMsg = new CString(strFullMessage);
						pDlg->PostMessage(WM_PIPE_MSG_RECEIVED, 0, reinterpret_cast<LPARAM>(pMsg));
					}

					break; 
				}
				else if (!bSuccess && dwError == ERROR_MORE_DATA)
				{
					// Case 2: more data
					szBuffer[dwRead / sizeof(TCHAR)] = _T('\0');

					// Append to result string
					strFullMessage += szBuffer;

					// Cleanup buffer for next ReadFile
					ZeroMemory(szBuffer, sizeof(szBuffer));
				}
				else
				{
					// break if error
					break;
				}
			}

			// Stop for next message
			::DisconnectNamedPipe(pDlg->m_hPipe);
		}
	}
	return 0;
}

LRESULT CChatDlgA::OnPipeMsgReceived(WPARAM wParam, LPARAM lParam)
{
	CString* pMsg = reinterpret_cast<CString*>(lParam);
	if (pMsg != NULL)
	{
		AppendMessage(_T("Dialog B"), *pMsg);
		delete pMsg;
	}
	return 0;
}


void CChatDlgA::OnBnClickedBtnSendA()
{
	CString strMsg;
	GetDlgItemText(IDC_EDIT_MSG_A, strMsg);
	if (strMsg.IsEmpty()) return;

	AppendMessage(_T("Me"), strMsg);

	HWND hWndB = ::FindWindow(NULL, _T("Chat Dialog B"));
	if (hWndB != NULL && ::IsWindow(hWndB))
	{
		CChatDlgB* pDlgB = static_cast<CChatDlgB*>(CWnd::FromHandle(hWndB));
		if (pDlgB != NULL)
		{
			pDlgB->AppendMessage(_T("Dialog A"), strMsg);
		}
	}

	SetDlgItemText(IDC_EDIT_MSG_A, _T(""));
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

	CTime time = CTime::GetCurrentTime();
	CString strHeader;
	strHeader.Format(_T("[%s] "), time.Format(_T("%H:%M:%S")).GetString());

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
		if (bIsFirstLine)
		{
			strLineToWrap = strHeader + strLineToWrap;
			bIsFirstLine = FALSE;
		}

		CStringArray lines;
		WrapMessageToLines(pDC, strLineToWrap, nMaxWidth, lines);

		for (INT_PTR i = 0; i < lines.GetCount(); i++)
		{
			CString strLine = lines.GetAt(i);
			int nRow = -1;

			if (bIsMe)
			{
				nRow = pList->InsertItem(pList->GetItemCount(), _T(""));
				pList->SetItemText(nRow, 1, strLine);
			}
			else
			{
				nRow = pList->InsertItem(pList->GetItemCount(), strLine);
			}

			if (nRow != -1)
			{
				pList->EnsureVisible(nRow, FALSE);
			}
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

	CString strCurrentLine = _T("");
	int nPos = 0;
	CString strWord;

	while (AfxExtractSubString(strWord, strInput, nPos, _T(' ')))
	{
		if (pDC->GetTextExtent(strWord).cx > nMaxWidth && !strCurrentLine.IsEmpty())
		{
			outLines.Add(strCurrentLine);
			strCurrentLine = _T("");

			for (int i = 0; i < strWord.GetLength(); i++)
			{
				CString strChar = strWord.Mid(i, 1);
				if (pDC->GetTextExtent(strCurrentLine + strChar).cx > nMaxWidth)
				{
					outLines.Add(strCurrentLine);
					strCurrentLine = strChar;
				}
				else
				{
					strCurrentLine += strChar;
				}
			}
		}
		else
		{
			CString strTestLine = strCurrentLine;
			if (!strTestLine.IsEmpty())
				strTestLine += _T(" ");
			strTestLine += strWord;

			if (pDC->GetTextExtent(strTestLine).cx > nMaxWidth && !strCurrentLine.IsEmpty())
			{
				outLines.Add(strCurrentLine);
				strCurrentLine = strWord;
			}
			else
			{
				strCurrentLine = strTestLine;
			}
		}

		nPos++;
	}

	if (!strCurrentLine.IsEmpty())
	{
		outLines.Add(strCurrentLine);
	}
}
