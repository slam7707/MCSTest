
// MCSTestDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MCSTest.h"
#include "MCSTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)

END_MESSAGE_MAP()

CMCSTestDlg::CMCSTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MCSTEST_DIALOG, pParent)
{
	m_hIcon			= AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_thd_mcst		= NULL;
	m_b_thd_flag	= FALSE;
	m_mcst			= NULL;
	m_array_size	= 1;
	m_n_lAddress	= 0;
	memset(m_array_data, 0, sizeof(m_array_data));
}

void CMCSTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

UINT CMCSTestDlg::MCSThread(LPVOID param)
{
	CMCSTestDlg*		pDlg = (CMCSTestDlg*)param;
	EMotionMcsDevice*	pMcs = pDlg->m_mcst;
	CString				str, erStr;

	int					nPos = 0;							// 축 위치 
	int					nAddress = pDlg->m_n_lAddress;		// l변수 주소 시작
	int					nECount = 0;						// 에러 카운트
	int					nResult = 0;						// mcst 리턴값
	int					nReadCount = 0;						

	// nAddress = 0, size = 5
	// nAddress = 6, size = 3

	while (pDlg->m_b_thd_flag) {
		if (pMcs->CheckConnection() == MCS_ERROR_SUCCESS) {
			str.Format(_T("connect ok -- array size[%d]\r\n"), pDlg->m_array_size);
			str.AppendFormat(_T("read count : %d\r\n"), nReadCount);

			for (int i = 0; i < 4; i++) {
				nResult = pMcs->GetPosition(i, &nPos);
				if (nResult == MCS_ERROR_SUCCESS) {
					str.AppendFormat(_T("[axis %d - %0.3lf]\r\n"), i, (double)nPos / 1000);
				}
				else {
					nECount++;
					erStr.AppendFormat(_T("%d-fail) GetPosition : axis %d"), nECount, i);
					pDlg->SetDlgItemText(IDC_CEDIT_OUTPUT_FAIL, erStr);
				}
			}

			nResult = pMcs->GetLVariables(nAddress, pDlg->m_array_size, pDlg->m_array_data);
			if (nResult == MCS_ERROR_SUCCESS) {
				for (int i = 0; i < pDlg->m_array_size; i++) {
					str.AppendFormat(_T("l var address %d = %d\r\n"), nAddress + i,  pDlg->m_array_data[i]);
				}
			}
			else {
				nECount++;
				erStr.AppendFormat(_T("%d) GetLVar [size %d]/[address %d] - return %d \r\n"), nECount, pDlg->m_array_size, nAddress, nResult);
				pDlg->SetDlgItemText(IDC_CEDIT_OUTPUT_FAIL, erStr);
			}

			pDlg->SetDlgItemText(IDC_CEDIT_OUTPUT_DATA, str);

			nAddress += 10;

			nReadCount++;
			if (nReadCount % 4 == 0) {
				nAddress = pDlg->m_n_lAddress;
			}
		}
		else {
			pDlg->OnClickedButton(IDC_CBUTTON_CONNECT);
		}
		Sleep(200);
	}

	return 0;
}

void CMCSTestDlg::OnClickedButton(UINT resID)
{
	switch (resID) {
	case IDC_CBUTTON_CONNECT:
		if (m_b_thd_flag) {
			m_b_thd_flag = FALSE;

			DWORD dwCode = 0;
			if (m_thd_mcst != NULL) {
				dwCode = WaitForSingleObject(m_thd_mcst->m_hThread, 200);
				if (dwCode == WAIT_OBJECT_0) OutputDebugString(L"---------------- m_thd_mcst wait_ob");
				else if (dwCode == WAIT_TIMEOUT) OutputDebugString(L"---------------- m_thd_mcst wait_timeout");
				else OutputDebugString(L"---------------- m_thd_mcst terminate"), TerminateThread(m_thd_mcst->m_hThread, 0);
				m_thd_mcst = NULL;
			}

			DestroyMcsDevice(m_mcst);
			m_mcst = NULL;

			SetDlgItemText(IDC_CEDIT_OUTPUT_DATA, _T("disconnected"));

		}else{
			m_mcst = CreateMcsDevice();
			if (m_mcst != NULL && m_mcst->Connect(GetDlgItemInt(IDC_CEDIT_PORT), CBR_38400) == MCS_ERROR_SUCCESS){
				m_b_thd_flag = TRUE;

				SetDlgItemInt(IDC_CEDIT_L_ADDRESS, m_n_lAddress);
				SetDlgItemInt(IDC_CEDIT_ARRAY_SIZE, m_array_size);

				m_thd_mcst = ::AfxBeginThread(MCSThread, this);
			}
			else {
				DestroyMcsDevice(m_mcst);
				m_mcst = NULL;
			}
		}
		break;

	case IDC_CBUTTON_SUBMIT:
		if (m_b_thd_flag && (GetDlgItemInt(IDC_CEDIT_ARRAY_SIZE) != m_array_size || GetDlgItemInt(IDC_CEDIT_L_ADDRESS) != m_n_lAddress)) {
			if (SuspendThread(m_thd_mcst->m_hThread) != 0xFFFFFFFF) {

				int nAdd = GetDlgItemInt(IDC_CEDIT_L_ADDRESS);
				if (nAdd < 0) nAdd = 0;
				m_n_lAddress = nAdd;

				int nSize = GetDlgItemInt(IDC_CEDIT_ARRAY_SIZE);
				if (nSize < 1) nSize = 1;
				if (nSize > 10) nSize = 10;
				m_array_size = nSize;

				ResumeThread(m_thd_mcst->m_hThread);
				Sleep(100);
			}
		}
		break;
	}
}

BEGIN_MESSAGE_MAP(CMCSTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
	ON_CONTROL_RANGE(BN_CLICKED, IDC_CBUTTON_CONNECT, IDC_CBUTTON_SUBMIT, OnClickedButton)
END_MESSAGE_MAP()

BOOL CMCSTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMCSTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMCSTestDlg::OnPaint()
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
HCURSOR CMCSTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CMCSTestDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	CFont* pDefF = new CFont;
	pDefF->CreateStockObject(DEFAULT_GUI_FONT);

	CStatic* pPort = new CStatic;
	pPort->Create(_T("port : "), WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, CRect(CPoint(5, 5), CSize(70, 20)), this, 10000);
	pPort->SetFont(pDefF);

	CEdit* pPortEdit = new CEdit;
	pPortEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | SS_RIGHT | SS_CENTERIMAGE, CRect(CPoint(75, 5), CSize(50, 20)), this, IDC_CEDIT_PORT);
	pPortEdit->SetFont(pDefF);

	CButton* pConnBtn = new CButton;
	pConnBtn->Create(_T("connect"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(CPoint(130, 5), CSize(80, 20)), this, IDC_CBUTTON_CONNECT);
	pConnBtn->SetFont(pDefF);

	CStatic* pLAdd = new CStatic;
	pLAdd->Create(_T("l address : "), WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, CRect(CPoint(5, 35), CSize(70, 20)), this, 10000);
	pLAdd->SetFont(pDefF);

	CEdit* pLAddress = new CEdit;
	pLAddress->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | SS_RIGHT | SS_CENTERIMAGE, CRect(CPoint(75, 35), CSize(50, 20)), this, IDC_CEDIT_L_ADDRESS);
	pLAddress->SetFont(pDefF);

	CStatic* pLSize = new CStatic;
	pLSize->Create(_T("l size : "), WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, CRect(CPoint(135, 35), CSize(70, 20)), this, 10000);
	pLSize->SetFont(pDefF);

	CEdit* pLSizeEdit = new CEdit;
	pLSizeEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | SS_RIGHT | SS_CENTERIMAGE, CRect(CPoint(210, 35), CSize(50, 20)), this, IDC_CEDIT_ARRAY_SIZE);
	pLSizeEdit->SetFont(pDefF);

	CButton* pSubBtn = new CButton;
	pSubBtn->Create(_T("submit"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(CPoint(265, 35), CSize(80, 20)), this, IDC_CBUTTON_SUBMIT);
	pSubBtn->SetFont(pDefF);

	CEdit* pOutput = new CEdit;
	pOutput->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | SS_LEFT | ES_MULTILINE, CRect(CPoint(5, 65), CSize(200, 250)), this, IDC_CEDIT_OUTPUT_DATA);
	pOutput->SetFont(pDefF);
	pOutput->SetWindowText(_T("disconnected"));

	CEdit* pOutputFail = new CEdit;
	pOutputFail->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | SS_LEFT | ES_MULTILINE, CRect(CPoint(210, 65), CSize(300, 250)), this, IDC_CEDIT_OUTPUT_FAIL);
	pOutputFail->SetFont(pDefF);
	pOutputFail->SetWindowText(_T("return abnormal"));

	pDefF->DeleteObject();
	delete pDefF;

	return 0;
}
