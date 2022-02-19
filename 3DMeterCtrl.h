#if !defined(AFX_3DMETERCTRL_H__C84783CA_4358_11D4_9F64_D07E4CC1E805__INCLUDED_)
#define AFX_3DMETERCTRL_H__C84783CA_4358_11D4_9F64_D07E4CC1E805__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// 3DMeterCtrl.h : header file
//

#ifndef ROUND
#define ROUND(x) (int)((x) + 0.5 - (double)((x) < 0))
#endif

#define BOUNDARY_POINTS 48

/////////////////////////////////////////////////////////////////////////////
// C3DMeterCtrl window

class C3DMeterCtrl : public CStatic
{
// Construction
public:
	C3DMeterCtrl();

// Attributes
protected:
	int m_nBottomCX ;
	int m_nBottomCY ;
	int m_nBottomRadius ;
	int m_nTopRadius ;
	int m_nValueFontHeight ;
	int m_nValueBaseline ;
	int m_nValueCenter ;

	double m_dLeftAngleRad ;
	double m_dRightAngleRad ;

	CRect m_rectCtrl ;
	CRect m_rectValue ;
	CPoint m_pointBoundary[BOUNDARY_POINTS] ;
	CRgn m_rgnBoundary ;

	CFont m_fontValue ;

	COLORREF m_colorWindow ;
	COLORREF m_colorHighlight ;
	COLORREF m_colorShadow ;
	COLORREF m_colorButton ;
	COLORREF m_colorText ;
	
	CDC m_dcBackground ;

	CBitmap *m_pBitmapOldBackground ;
	CBitmap m_bitmapBackground ;

public:
	int m_nScaleDecimals ;
	int m_nValueDecimals ;

	double m_dCurrentValue ;
	double m_dMaxValue ;
	double m_dMinValue ;

	COLORREF m_colorNeedle ;
	CString m_strUnits ;
	
// Operations
public:
	void DrawMeterBackground(CDC *pDC, CRect &rect) ;
	void DrawNeedle(CDC *pDC) ;
	void DrawValue(CDC *pDC) ;
	void UpdateNeedle(double dValue) ;
	void ReconstructControl() ;
	void SetRange(double dMin, double dMax) ;
	void SetScaleDecimals(int nDecimals) ;
	void SetValueDecimals(int nDecimals) ;
	void SetUnits(CString &strUnits) ;
	void SetNeedleColor (COLORREF colorNeedle) ;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(C3DMeterCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~C3DMeterCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(C3DMeterCtrl)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_3DMETERCTRL_H__C84783CA_4358_11D4_9F64_D07E4CC1E805__INCLUDED_)
