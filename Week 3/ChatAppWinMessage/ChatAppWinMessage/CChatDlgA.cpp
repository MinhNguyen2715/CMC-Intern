// CChatDlgA.cpp : implementation file
//

#include "pch.h"
#include "ChatAppWinMessage.h"
#include "CChatDlgA.h"
#include "afxdialogex.h"


// CChatDlgA dialog

IMPLEMENT_DYNAMIC(CChatDlgA, CDialogEx)

CChatDlgA::CChatDlgA(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_DLG_A, pParent)
{

}

CChatDlgA::~CChatDlgA()
{
}

void CChatDlgA::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChatDlgA, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SEND_A, &CChatDlgA::OnBnClickedBtnSendA)
	ON_WM_COPYDATA()
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

		CRect rectList;
		pList->GetClientRect(&rectList);
		int nColumnWidth = (rectList.Width() - 5) / 2;

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

	// Check handle safety
	if (pList == NULL || pList->GetSafeHwnd() == NULL ||
		pEdit == NULL || pEdit->GetSafeHwnd() == NULL ||
		pBtn == NULL || pBtn->GetSafeHwnd() == NULL)
	{
		return;
	}

	// Get area inside dialog
	CRect rectClient;
	GetClientRect(&rectClient);
	int cx = rectClient.Width();
	int cy = rectClient.Height();

	// Margin, distance
	int nMargin = 15;      
	int nBtnWidth = 100;  
	int nBtnHeight = 35;   
	int nEditHeight = 35;  
	int nSpacing = 10;    

	// Send button (Middle bottom)
	int nBtnX = (cx - nBtnWidth) / 2;
	int nBtnY = cy - nMargin - nBtnHeight;
	pBtn->MoveWindow(nBtnX, nBtnY, nBtnWidth, nBtnHeight, TRUE);

	// Edit box (Above Send button, full width)
	int nEditX = nMargin;
	int nEditY = nBtnY - nSpacing - nEditHeight;
	int nEditWidth = cx - (nMargin * 2);
	pEdit->MoveWindow(nEditX, nEditY, nEditWidth, nEditHeight, TRUE);

	// List control (All of the remaining area)
	int nListX = nMargin;
	int nListY = nMargin;
	int nListWidth = cx - (nMargin * 2);
	int nListHeight = nEditY - nSpacing - nListY;
	pList->MoveWindow(nListX, nListY, nListWidth, nListHeight, TRUE);

	// Spit list control
	CListCtrl* pListCtrl = static_cast<CListCtrl*>(pList);
	int nColWidth = (nListWidth - 5) / 2;
	pListCtrl->SetColumnWidth(0, nColWidth);
	pListCtrl->SetColumnWidth(1, nColWidth);
}

void CChatDlgA::OnBnClickedBtnSendA()
{
	CString strMsg;
	GetDlgItemText(IDC_EDIT_MSG_A, strMsg);
	if (strMsg.IsEmpty()) return;

	// Find HWND of Dialog B 
	if (m_hWndPartner == NULL || !::IsWindow(m_hWndPartner))
	{
		m_hWndPartner = ::FindWindow(NULL, _T("Chat Dialog B"));
	}

	if (m_hWndPartner != NULL && ::IsWindow(m_hWndPartner))
	{
		COPYDATASTRUCT cds;
		cds.dwData = WM_CHAT_MESSAGE; // Use WM_CHAT_MESSAGE defined in pch.h
		cds.cbData = (strMsg.GetLength() + 1) * sizeof(TCHAR);
		cds.lpData = const_cast<LPTSTR>(strMsg.GetString());

		::SendMessage(m_hWndPartner, WM_COPYDATA, reinterpret_cast<WPARAM>(this->GetSafeHwnd()), reinterpret_cast<LPARAM>(&cds));

		AppendMessage(_T("Me"), strMsg);
		SetDlgItemText(IDC_EDIT_MSG_A, _T(""));
	}
	else
	{
		AfxMessageBox(_T("Cannot find Chat Dialog B Windows"));
	}
}

BOOL CChatDlgA::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	if (pCopyDataStruct != NULL && pCopyDataStruct->dwData == WM_CHAT_MESSAGE)
	{
		if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
		{
			m_hWndPartner = pWnd->GetSafeHwnd();
		}

		LPCTSTR lpszText = reinterpret_cast<LPCTSTR>(pCopyDataStruct->lpData);
		CString strReceived(lpszText);

		AppendMessage(_T("Dialog B"), strReceived);
		return TRUE;
	}

	return CDialogEx::OnCopyData(pWnd, pCopyDataStruct);
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

	// Time
	CTime time = CTime::GetCurrentTime();
	CString strHeader;
	strHeader.Format(_T("[%s] "), time.Format(_T("%H:%M:%S")).GetString());

	// Split \n
	CStringArray rawLines;
	int nPos = 0;
	CString strSub;

	// Change \r\n to \n
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

	// Calculate spacing and new line
	BOOL bIsFirstLine = TRUE;
	for (INT_PTR k = 0; k < rawLines.GetCount(); k++)
	{
		CString strLineToWrap = rawLines.GetAt(k);
		if (bIsFirstLine)
		{
			strLineToWrap = strHeader + strLineToWrap; // Only append Timestamp [HH:MM:SS] in the first line
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
		// If current word longer than column width -> split according to each charecter
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
			// Split according to space
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

