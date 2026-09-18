
// HelloWorldMFCDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "HelloWorldMFC.h"
#include "HelloWorldMFCDlg.h"
#include "afxdialogex.h"
#include <Windows.h>
#include <string>
#include <vector>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CHelloWorldMFCDlg dialog

CHelloWorldMFCDlg::CHelloWorldMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HELLOWORLDMFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHelloWorldMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHelloWorldMFCDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT, &CHelloWorldMFCDlg::OnEnChangeEdit)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CHelloWorldMFCDlg::OnBnClickedBtnBrowse)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CHelloWorldMFCDlg::OnBnClickedBtnSave)
	ON_BN_CLICKED(IDC_BTN_LOAD, &CHelloWorldMFCDlg::OnBnClickedBtnLoad)
END_MESSAGE_MAP()

// CHelloWorldMFCDlg message handlers

BOOL CHelloWorldMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE; // return TRUE unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHelloWorldMFCDlg::OnPaint()
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
HCURSOR CHelloWorldMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHelloWorldMFCDlg::OnEnChangeEdit()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void CHelloWorldMFCDlg::OnBnClickedBtnBrowse()
{
	//explicit CFolderPickerDialog(
	//	LPCTSTR lpszFolder = NULL, //  Initial Folder Directory
	//	DWORD dwFlags = 0, // Flags: customize the behavior of the folder picker.
	//	CWnd * pParentWnd = NULL, // Pointer to the parent or owner window of the dialog box.
	//								// NULL: The main application window becomes the owner.
	//								// this: ensuring the folder picker stays centered over it and blocks interaction until closed
	//	DWORD dwSize = 0
	//);

	CFolderPickerDialog folderDlg(NULL, 0, this, 0);

	// Change title of folder picker
	folderDlg.m_ofn.lpstrTitle = _T("Select path");

	// IDOK: OK button
	// IDCANCEL: Cancel button
	if (folderDlg.DoModal() == IDOK) {
		CString strFolderPath = folderDlg.GetFolderPath();

		// Change static text
		SetDlgItemText(IDC_PATH, strFolderPath);

		// Store path to m_strFolderPath
		m_strFolderPath = strFolderPath;
	}
}


void CHelloWorldMFCDlg::OnBnClickedBtnSave()
{
	CString strFolderPath;
	GetDlgItemText(IDC_PATH, strFolderPath);

	// Check empty path
	if (strFolderPath.IsEmpty()) {
		AfxMessageBox(_T("Select valid path"));
		return;
	}

	CString strFileContent;
	GetDlgItemText(IDC_EDIT, strFileContent);

	// Check empty text file
	if (strFileContent.IsEmpty()) {
		AfxMessageBox(_T("Empty text field"));
		return;
	}

	CString strFilePath;

	// Add trailing \ to the path if not exist
	if (strFolderPath.Right(1) != _T("\\")) {
		strFolderPath += _T("\\");
	}

	strFilePath.Format(_T("%smydata.txt"), strFolderPath.GetString());

	// Convert CString (UTF-16) -> UTF-8 std::string
	CT2CA pszUtf8(strFileContent, CP_UTF8);
	string strUtf8Content(pszUtf8);

	// Alert before overwriting
	if (PathFileExists(strFilePath))
	{
		CString strPrompt;
		strPrompt.Format(_T("The file 'mydata.txt' already exists in:\n%s\n\nDo you want to overwrite it?"), strFolderPath.GetString());

		int nResponse = AfxMessageBox(strPrompt, MB_YESNO | MB_ICONQUESTION);

		// No -> cancel
		if (nResponse == IDNO)
		{
			return;
		}
	}

	CStdioFile file;
	CFileException ex;

	if (file.Open(strFilePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText, &ex))
	{
		// Write UTF-8 BOM (Byte Order Mark) -> other apps know the text file is encoded in UTF-8
		unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
		file.Write(bom, sizeof(bom));

		// Write the edit control content into the file
		file.Write(strUtf8Content.c_str(), static_cast<UINT>(strUtf8Content.length()));
		file.Close();

		CString strSuccessMsg;
		strSuccessMsg.Format(_T("File successfully saved to:\n%s"), strFilePath.GetString());
		AfxMessageBox(strSuccessMsg, MB_ICONINFORMATION);
	}
	else
	{
		TCHAR szError[1024];
		ex.GetErrorMessage(szError, 1024);
		CString strErrMsg;
		strErrMsg.Format(_T("Failed to create file. Error: %s"), szError);
		AfxMessageBox(strErrMsg, MB_ICONERROR);
	}
}


void CHelloWorldMFCDlg::OnBnClickedBtnLoad()
{
	// Filter format: "Description|*.ext|Description2|*.ext2||"
	TCHAR szFilter[] = _T("Text Documents (*.txt)|*.txt|All Files (*.*)|*.*||");

	//CFileDialog(
	//	BOOL bOpenFileDialog = TRUE, // Sets the mode to File Open dialog. FALSE: create a Save As dialog
	//	LPCTSTR lpszDefExt = NULL, // Default file name extension
	//	LPCTSTR lpszFileName = NULL, // Initial file name that appears in the Filename box
	//	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, , // Flag
	//	LPCTSTR lpszFilter = NULL, // Filter
	//	CWnd * pParentWnd = NULL // Pointer to the parent or owner window
	//);

	CFileDialog fileDlg(TRUE, _T("txt"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, this);
	fileDlg.m_ofn.lpstrTitle = _T("Select text file to load");

	if (fileDlg.DoModal() == IDOK) {
		CString strSelectedFilePath = fileDlg.GetPathName();

		CStdioFile file;
		CFileException ex;

		if (file.Open(strSelectedFilePath, CFile::modeRead | CFile::typeText, &ex))
		{
			ULONGLONG dwFileLength = file.GetLength();
			if (dwFileLength == 0) {
				SetDlgItemText(IDC_EDIT, _T(""));
				file.Close();
				return;
			}

			// Read file
			vector<char> buffer(static_cast<size_t>(dwFileLength) + 1, 0);
			file.Read(buffer.data(), static_cast<UINT>(dwFileLength));
			file.Close();

			char* pData = buffer.data();
			size_t dataSize = static_cast<size_t>(dwFileLength);

			// Check and skip UTF-8 BOM if present (0xEF, 0xBB, 0xBF)
			if (dataSize >= 3 &&
				(unsigned char)pData[0] == 0xEF &&
				(unsigned char)pData[1] == 0xBB &&
				(unsigned char)pData[2] == 0xBF)
			{
				pData += 3;
			}

			// Convert UTF-8 buffer to CString (Unicode)
			CA2CT pszUnicode(pData, CP_UTF8);
			CString strFullContent(pszUnicode);

			SetDlgItemText(IDC_EDIT, strFullContent);

			//CString strSuccessMsg;
			//strSuccessMsg.Format(_T("Text file loaded successfully from:\n%s"), strSelectedFilePath.GetString());
			//AfxMessageBox(strSuccessMsg, MB_ICONINFORMATION);
		}
		else {
			// Handle file opening errors (e.g., Access Denied)
			TCHAR szError[1024];
			ex.GetErrorMessage(szError, 1024);
			CString strErrMsg;
			strErrMsg.Format(_T("Failed to open text file. Error: %s"), szError);
			AfxMessageBox(strErrMsg, MB_ICONERROR);
		}
	}
}
