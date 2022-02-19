// wfguiDlg.h : header file
//

#if !defined(AFX_WFGUIDLG_H__8E92E6B6_2231_440B_AB03_989937F74ECB__INCLUDED_)
#define AFX_WFGUIDLG_H__8E92E6B6_2231_440B_AB03_989937F74ECB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma once
#include <MMSystem.h>
#define MAX_BUFFERS	2

#include "OScopeCtrl.h"
#include "SpectrumGraph.h"
#include "3DMeterCtrl.h"
#include "LedButton.h"
#include "afxwin.h"



struct DataHolder
{
	void* pData;
	void* pData2;
};


/////////////////////////////////////////////////////////////////////////////
// CWfguiDlg dialog

class CWfguiDlg : public CDialog
{
// Construction
public:
	void DoFFT();
	CWfguiDlg(CWnd* pParent = NULL);	// standard constructor

	
// Dialog Data
	//{{AFX_DATA(CWfguiDlg)
	enum { IDD = IDD_WFGUI_DIALOG };
	C3DMeterCtrl m_3DMeterCtrl;
	CProgressCtrl	m_LevelCtrl;
	CComboBox	m_filter_combo;
	CProgressCtrl	m_metercontrol;
	CButton	m_radio10;
	CButton	m_radio01;
	CButton	m_radio05;
	CString	m_status;
	CString	m_freq;
	CString	m_rms;
	CString	m_filter_type;
	CString	m_peak;
	BOOL	m_savefile;
	CString m_max_10_sec;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWfguiDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	COScopeCtrl m_OScopeCtrl;
	CFrequencyGraph m_graph;
	DataHolder m_data;
	BOOL m_weighted;
	BOOL m_bRun;
	HWAVEIN m_hWaveIn;
	CString m_csErrorText;
	WAVEHDR m_stWHDR[MAX_BUFFERS];

	// Generated message map functions
	//{{AFX_MSG(CWfguiDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg LONG CWfguiDlg::OnWaveMessage(UINT wParam, LONG lParam);
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void PrepareBuffers();
	void ProcessHeader(WAVEHDR * pHdr);
	WAVEFORMATEX m_stWFEX;
	CString StoreError(MMRESULT mRes,BOOL bDisplay,LPCTSTR lpszFormat, ...);
	void OpenDevice();
	void OnCbnSelchangeDevices();
	int FillDevices(void);
	void UnPrepareBuffers(void);
	afx_msg void OnBnClickedRadio4();
	bool m_radio3;
	CButton m_radio30;
	afx_msg void OnBnClickedRadio5();
	CButton m_3000;
	CEdit m_freq_display;
	CEdit m_RMS_box;
	//CButton
	CLedButton m_OK_LED;
	bool m_log;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WFGUIDLG_H__8E92E6B6_2231_440B_AB03_989937F74ECB__INCLUDED_)
