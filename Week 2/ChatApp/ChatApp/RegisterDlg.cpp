// RegisterDlg.cpp : implementation file
//

#include "pch.h"          
#include "ChatApp.h"
#include "RegisterDlg.h"
#include "resource.h"

using namespace std;

// CRegisterDlg dialog

IMPLEMENT_DYNAMIC(CRegisterDlg, CDialogEx)

CRegisterDlg::CRegisterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_REGISTER, pParent)
{
}

CRegisterDlg::~CRegisterDlg()
{
}

void CRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRegisterDlg, CDialogEx)
	ON_BN_CLICKED(ID_BTN_REG_REG, &CRegisterDlg::OnBnClickedBtnRegReg)
END_MESSAGE_MAP()


// CRegisterDlg message handlers

// Uses the Windows Cryptography API Next Generation -> compute SHA-256 cryptographic hash
string HashPasswordSHA256(const string& password)
{
	BCRYPT_ALG_HANDLE hAlg = NULL;
	BCRYPT_HASH_HANDLE hHash = NULL;
	DWORD cbHash = 0, cbHashObject = 0;
	DWORD cbData = 0;

	// Open SHA-256 algorithm provider
	NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0);
	if (!BCRYPT_SUCCESS(status))
		return "";

	// Asks Windows how much internal working memory the hashing engine needs (pbHashObject)
	status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0);
	if (!BCRYPT_SUCCESS(status))
	{
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return "";
	}

	// Asks for the exact byte size of a SHA-256 output digest (32 bytes / 256 bits).
	status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&cbHash, sizeof(DWORD), &cbData, 0);
	if (!BCRYPT_SUCCESS(status))
	{
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return "";
	}

	vector<BYTE> pbHashObject(cbHashObject);
	vector<BYTE> pbHash(cbHash);

	// Create and hash
	status = BCryptCreateHash(hAlg, &hHash, pbHashObject.data(), cbHashObject, NULL, 0, 0);
	if (BCRYPT_SUCCESS(status))
	{
		status = BCryptHashData(hHash, (PBYTE)password.c_str(), (ULONG)password.length(), 0);
		if (BCRYPT_SUCCESS(status))
		{
			status = BCryptFinishHash(hHash, pbHash.data(), cbHash, 0);
		}
		BCryptDestroyHash(hHash);
	}

	// Closes algorithm handle to prevent memory / resource leaks
	BCryptCloseAlgorithmProvider(hAlg, 0);

	// Convert binary hash output -> Hex string
	string hexHash = "";
	char hexBuffer[3]; //hold a 2-digit hexadecimal + 1 byte for the null-terminator ('\0')
	for (BYTE b : pbHash)
	{
		sprintf_s(hexBuffer, "%02x", b);
		hexHash += hexBuffer;
	}

	return hexHash;
}

void CRegisterDlg::OnBnClickedBtnRegReg()
{
	GetDlgItemText(IDC_EDIT_REG_USERNAME, strUser);
	GetDlgItemText(IDC_EDIT_REG_PASSWORD, strPass);
	GetDlgItemText(IDC_EDIT_REG_CONFIRM, strConfirm);
	
	// Convert CString (UTF-16) to std::string (UTF-8)
	string username = CT2CA(strUser);
	string password = CT2CA(strPass);
	string confirm = CT2CA(strConfirm);

	if (password != confirm) {
		AfxMessageBox(_T("Passwords do not match!"), MB_ICONERROR);
		return;
	}

	// Username validation (Alphanumeric + underscore, 5 - 20 chars) 
	regex userRegex("^[a-zA-Z0-9_]{5,20}$");
	if (!regex_match(username, userRegex))
	{
		AfxMessageBox(_T("Username must be 5-20 characters long and contain only letters, numbers, or underscores."), MB_ICONWARNING);
		return;
	}

	// Min 8 chars: 1 uppercase, 1 lowercase, 1 digit, 1 special character
	regex passRegex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&#]).{8,}$");
	if (!regex_match(password, passRegex))
	{
		AfxMessageBox(_T("Password must be at least 8 characters long and include an uppercase letter, lowercase letter, and a number."), MB_ICONWARNING);
		return;
	}

	string hashedPassword = HashPasswordSHA256(password);

	// Load JSON database
	json userDb = json::array();
	ifstream inFile("users.json");
	if (inFile.is_open())
	{
		try {
			inFile >> userDb;
		}
		catch (...) {
			userDb = json::array(); // Fallback if file is empty or corrupted
		}
		inFile.close();
	}

	// Check if username exists
	for (const auto& user : userDb)
	{
		if (user.contains("username") && user["username"] == username)
		{
			AfxMessageBox(_T("Username already taken. Please choose another."), MB_ICONWARNING);
			return;
		}
	}

	// Create new user
	json newUser = {
		{"username", username},
		{"password", hashedPassword},
		{"created_at", time(nullptr)}
	};
	userDb.push_back(newUser);

	// Write to users.json
	ofstream outFile("users.json");
	if (outFile.is_open())
	{
		outFile << userDb.dump(4); // Indent 4 spaces for readability
		outFile.close();

		AfxMessageBox(_T("Registration successful!"), MB_ICONINFORMATION);
		EndDialog(IDOK);
	}
	else
	{
		AfxMessageBox(_T("Failed to save user data to file."), MB_ICONERROR);
	}
}
