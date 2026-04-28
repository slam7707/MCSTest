
// MCSTestDlg.h : header file
//

#pragma once
#include "EMotionMcsDevice.h"


// CMCSTestDlg dialog
class CMCSTestDlg : public CDialogEx
{
// Construction
public:
	CMCSTestDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MCSTEST_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	CWinThread			* m_thd_mcst;
	BOOL				m_b_thd_flag;
	EMotionMcsDevice	* m_mcst;
	int					m_n_lAddress;
	int					m_array_size;
	int					m_array_data[10];

	static UINT		MCSThread(LPVOID param);

	afx_msg void	OnClickedButton(UINT resID);


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
