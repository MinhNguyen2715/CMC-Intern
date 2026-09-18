// LoginDlg.cpp : implementation file
//

#include "pch.h"          
#include "ChatApp.h"      
#include "LoginDlg.h"    
#include "resource.h"    
#include "RegisterDlg.h"

using namespace std;

// CLoginDlg dialog

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_LOGIN, pParent)
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_LOG_REG, &CLoginDlg::OnBnClickedBtnLogReg)
	ON_BN_CLICKED(IDC_BTN_LOG_LOG, &CLoginDlg::OnBnClickedBtnLogLog)
END_MESSAGE_MAP()


// CLoginDlg message handlers

void CLoginDlg::OnBnClickedBtnLogReg()
{
	CRegisterDlg regDlg;
	INT_PTR nResult = regDlg.DoModal();
}


void CLoginDlg::OnBnClickedBtnLogLog()
{
	GetDlgItemText(IDC_EDIT_LOGIN_USERNAME, strUser);
	GetDlgItemText(IDC_EDIT_LOGIN_PASSWORD, strPass);

	string username = CT2CA(strUser);
	string password = CT2CA(strPass);

	if (username.empty() || password.empty())
	{
		AfxMessageBox(_T("Please enter both username and password!"), MB_ICONWARNING);
		return;
	}

	// Load user database
	ifstream inFile("users.json");
	if (!inFile.is_open())
	{
		AfxMessageBox(_T("No registered users found. Please register first."), MB_ICONERROR);
		return;
	}

	json userDb;
	try {
		inFile >> userDb;
		inFile.close();
	}
	catch (...) {
		inFile.close();
		AfxMessageBox(_T("Failed to read user database."), MB_ICONERROR);
		return;
	}

	// Hash the entered password
	string hashedPassword = HashPasswordSHA256(password);
	bool bAuthenticated = false;

	// Search for matching username & hashed password
	for (const auto& user : userDb)
	{
		if (user.contains("username") && user["username"] == username &&
			user.contains("password") && user["password"] == hashedPassword)
		{
			bAuthenticated = true;
			break;
		}
	}

	if (bAuthenticated)
	{
		// Signal IDOK back to CChatAppApp::InitInstance()
		EndDialog(IDOK);
	}
	else
	{
		AfxMessageBox(_T("Invalid username or password!"), MB_ICONERROR);
	}
}
