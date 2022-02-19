// 3DMeterCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "math.h"
//#include "MeterTestForm.h"
#include "3DMeterCtrl.h"
#include "MemDC.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// C3DMeterCtrl

C3DMeterCtrl::C3DMeterCtrl()
{
	m_dCurrentValue  =  0.0 ;
	m_dMaxValue      =  0.4 ;
	m_dMinValue      =	0.0 ;

	m_nScaleDecimals = 1 ;
	m_nValueDecimals = 3 ;

	m_strUnits.Format("Peak") ;

	m_colorNeedle = RGB(200, 0, 0) ;

}

C3DMeterCtrl::~C3DMeterCtrl()
{
	if ((m_pBitmapOldBackground) && 
		  (m_bitmapBackground.GetSafeHandle()) && 
			(m_dcBackground.GetSafeHdc()))
		m_dcBackground.SelectObject(m_pBitmapOldBackground);
}


BEGIN_MESSAGE_MAP(C3DMeterCtrl, CStatic)
	//{{AFX_MSG_MAP(C3DMeterCtrl)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C3DMeterCtrl message handlers

void C3DMeterCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
		
	// Find out how big we are
	GetClientRect (&m_rectCtrl) ;

	// make a memory dc
	CMemDC memDC(&dc, &m_rectCtrl);

	// set up a memory dc for the background stuff 
	// if one isn't being used
	if ((m_dcBackground.GetSafeHdc() == NULL) || (m_bitmapBackground.m_hObject == NULL))
	{
		m_dcBackground.CreateCompatibleDC(&dc) ;
		m_bitmapBackground.CreateCompatibleBitmap(&dc, m_rectCtrl.Width(), 
																							m_rectCtrl.Height()) ;
		m_pBitmapOldBackground = m_dcBackground.SelectObject(&m_bitmapBackground) ;

		// Fill this bitmap with the background.

		// Note: This requires some serious drawing and calculating, 
		// therefore it is drawn "once" to a bitmap and 
		// the bitmap is stored and blt'd when needed.
		DrawMeterBackground(&m_dcBackground, m_rectCtrl) ;
						
	}

	// drop in the background
	memDC.BitBlt(0, 0, m_rectCtrl.Width(), m_rectCtrl.Height(), 
			           &m_dcBackground, 0, 0, SRCCOPY) ;

	// add the needle to the background
	DrawNeedle(&memDC) ;

	// add the value to the background
	DrawValue(&memDC) ;

}

void C3DMeterCtrl::DrawValue(CDC *pDC)
{
	CFont *pFontOld ;
	CString strTemp ;

	// Pick up the font.
	// Note: the font was determined in the drawing
	// of the background
  pFontOld = pDC->SelectObject(&m_fontValue) ;

	// set the colors based on the system colors
	pDC->SetTextColor(m_colorText) ;
	pDC->SetBkColor(m_colorButton) ;

	// draw the text in the recessed rectangle
	pDC->SetTextAlign(TA_CENTER|TA_BASELINE) ;
	strTemp.Format("%.*f", m_nValueDecimals, m_dCurrentValue) ;
	pDC->TextOut(m_nValueCenter, m_nValueBaseline, strTemp) ;

	// restore the color and the font
	pDC->SetBkColor(m_colorWindow) ;
  pDC->SelectObject(pFontOld) ;

}

void C3DMeterCtrl::UpdateNeedle(double dValue)
{
	m_dCurrentValue = dValue ;
	Invalidate() ;

}

void C3DMeterCtrl::DrawNeedle(CDC *pDC)
{
	int nResult ;
	
	double dAngleRad ;
	double dTemp ;
	CBrush brushFill, *pBrushOld ;
	CPen penDraw, *pPenOld ;
	CPoint pointNeedle[3] ;

	// This function draws a triangular needle.
	// The base of the needle is a horizontal line
	// that runs through the center point of the arcs
	// that make up the face of the meter.
	// The tip of the needle is at an angle that is 
	// calculated based on the current value and the scale.

	// The needle is constructed as a 3-point polygon
	// (i.e. triangle).  The triangle is drawn to the 
	// screen based on a clipping region that is derived
	// from the meter face.  See "DrawMeterBackground".

	// calculate the first and last points of the needle.
	pointNeedle[0].x = m_nBottomCX + m_nBottomRadius/20 ;
	pointNeedle[0].y = m_nBottomCY ;
	pointNeedle[2].x = m_nBottomCX - m_nBottomRadius/20 ;
	pointNeedle[2].y = m_nBottomCY ;

	// calculate the angle for the tip of the needle
	dAngleRad = (m_dCurrentValue-m_dMinValue)*(m_dRightAngleRad-m_dLeftAngleRad)/
		          (m_dMaxValue-m_dMinValue) + m_dLeftAngleRad ;

	// if the angle is beyond the meter, draw the needle
	// at the limit (as if it is "pegged")
	dAngleRad = max(dAngleRad, m_dRightAngleRad) ;
	dAngleRad = min(dAngleRad, m_dLeftAngleRad) ;

	// calculate the X position of the tip
	dTemp = m_nBottomCX + m_nTopRadius*cos(dAngleRad) ;
	pointNeedle[1].x = ROUND(dTemp) ;

	// calculate the Y position of the tip
	dTemp = m_nBottomCY - m_nTopRadius*sin(dAngleRad) ;
	pointNeedle[1].y = ROUND(dTemp) ;

	// select the clipping region based on the meter
	pDC->SelectClipRgn(&m_rgnBoundary) ;

	// create a pen and brush based on the needle color
	brushFill.CreateSolidBrush(m_colorNeedle) ;
	penDraw.CreatePen(PS_SOLID, 1, m_colorNeedle) ;

	// select the pen and brush
	pPenOld = pDC->SelectObject(&penDraw) ;
	pBrushOld = pDC->SelectObject(&brushFill) ;

	// draw the needle
	pDC->Polygon(pointNeedle, 3) ;

	// restore the clipping region
	nResult = pDC->SelectClipRgn(NULL) ;

	// restore the pen and brush
	pDC->SelectObject(pPenOld) ;
	pDC->SelectObject(pBrushOld) ;

	
}

void C3DMeterCtrl::DrawMeterBackground(CDC *pDC, CRect &rect)
{
	int i, nAngleDeg, nRef ;
	int nHeight ;
	int nHalfPoints ;
	int nStartAngleDeg, nEndAngleDeg ;
	int nTickDeg ;
	int nAngleIncrementDeg ;

	double dTemp, dAngleRad, dX, dY ;
	double dRadPerDeg ;

	CString strTemp ;

	CPoint pointRecess[BOUNDARY_POINTS] ;

	CBrush brushFill, *pBrushOld ;

	CFont *pFontOld ;

	CPen penDraw, *pPenOld ;

	TEXTMETRIC tm ;

	// determine the centerpoint of the radii for
	// the meter face.
	m_nBottomCX = (rect.left+rect.right)/2 ;
	m_nBottomCY = rect.bottom - rect.Height()/16 ;

	// determine the radii for the top of the meter
	// and the bottom of the meter
	m_nTopRadius = rect.Height()*6/8 ;
	m_nBottomRadius = m_nTopRadius/2 ;

	// Radians per Degree
	// This helps me lay things out in degrees
	// which are more intuitive than radians.
	dRadPerDeg = 4.0*atan(1.0)/180.0 ;

	// set the left and right limits for the meter face
	nStartAngleDeg =  60 ;
	nEndAngleDeg   = 120 ;
	nTickDeg       =  15 ;

	// this is the density of points along the arcs
	nAngleIncrementDeg = 5 ;

	// convert these to radians 
	// for computer (rather than human) use!
	m_dLeftAngleRad = nEndAngleDeg*dRadPerDeg ;
	m_dRightAngleRad = nStartAngleDeg*dRadPerDeg ;


	// construct the meter face region
	// This is a polygon starting at the top right of
	// the meter face and moving across the top arc.
	nRef = 0 ;
	for (nAngleDeg=nStartAngleDeg; nAngleDeg<=nEndAngleDeg; nAngleDeg+=nAngleIncrementDeg)
	{
		// determine the current angle in radians
		dAngleRad = nAngleDeg*dRadPerDeg ;

		// determine the X position
		dTemp = m_nBottomCX + m_nTopRadius*cos(dAngleRad) ;
		m_pointBoundary[nRef].x = ROUND(dTemp) ;

		// determine the Y position
		dTemp = m_nBottomCY - m_nTopRadius*sin(dAngleRad) ;
		m_pointBoundary[nRef].y = ROUND(dTemp) ;
		nRef++ ;
	}

	// at this point we have constructed the entire
	// top arc of the meter face

	nHalfPoints = nRef ;   // hold onto this for later use


	// now add points to the polygon starting at the 
	// left side of the lower arc.
	for (nAngleDeg=nEndAngleDeg; nAngleDeg>=nStartAngleDeg; nAngleDeg-=nAngleIncrementDeg)
	{
		dAngleRad = nAngleDeg*dRadPerDeg ;
		dTemp = m_nBottomCX + m_nBottomRadius*cos(dAngleRad) ;
		m_pointBoundary[nRef].x = ROUND(dTemp) ;
		dTemp = m_nBottomCY - m_nBottomRadius*sin(dAngleRad) ;
		m_pointBoundary[nRef].y = ROUND(dTemp) ;
		nRef++ ;
	}

	
	// Now construct a polygon that is just outside 
	// the meter face to use in drawing a "recess"
	// around the meter face.
	for (i=0; i<nRef; i++)
	{
		pointRecess[i].x = m_pointBoundary[i].x ;
		pointRecess[i].y = m_pointBoundary[i].y-1 ;
	}
	pointRecess[0].x = pointRecess[0].x + 1 ;
	pointRecess[nRef-1].x = pointRecess[nRef-1].x - 1 ;
	for (i=nHalfPoints; i<nRef; i++)
	{
		pointRecess[i].x = m_pointBoundary[i].x ;
		pointRecess[i].y = m_pointBoundary[i].y+1 ;
	}
	pointRecess[nHalfPoints].x = pointRecess[nHalfPoints].x - 1 ;
	pointRecess[nRef-1].x = pointRecess[nRef-1].x + 1 ;

	// create a region based on the meter face
	// (to be used lated in clipping the needle)
	m_rgnBoundary.DeleteObject() ;
	m_rgnBoundary.CreatePolygonRgn(m_pointBoundary, nRef, ALTERNATE) ;

	// locate a rectangle below the meter face
	m_rectValue.left = rect.left + rect.Width()/20 ;
	m_rectValue.right = rect.right - rect.Width()/20 ;
	m_rectValue.bottom = rect.bottom - rect.Width()/20 ;
	m_rectValue.top = rect.bottom - (rect.bottom - pointRecess[nRef-1].y)*60/100 ;

	// get the relevant system colors
	m_colorWindow    = RGB(255,255,220);// GetSysColor(COLOR_WINDOW) ;
	m_colorButton    = GetSysColor(COLOR_BTNFACE) ;
	m_colorShadow    = GetSysColor(COLOR_BTNSHADOW) ;
	m_colorHighlight = GetSysColor(COLOR_BTNHIGHLIGHT) ;
	m_colorText			 = GetSysColor(COLOR_BTNTEXT) ;

	// fill the background with the button color
	brushFill.DeleteObject() ;
	brushFill.CreateSolidBrush(m_colorButton) ;
	pBrushOld = pDC->SelectObject(&brushFill) ;
	pDC->Rectangle(rect) ;
	pDC->SelectObject(pBrushOld) ;

	// Draw the meter recess.

	// This happens by first drawing the 
	// top and left sides in the shadow color.
	penDraw.DeleteObject() ;
	penDraw.CreatePen(PS_SOLID, 1, m_colorShadow) ;
	pPenOld = pDC->SelectObject(&penDraw) ;
	pDC->MoveTo(pointRecess[0]) ;
	pDC->PolylineTo(pointRecess, nHalfPoints+1) ;
	pDC->SelectObject(pPenOld) ;

	// and then drawing the 
	// left and bottom sides in the shadow color.
	penDraw.DeleteObject() ;
	penDraw.CreatePen(PS_SOLID, 1, m_colorHighlight) ;
	pPenOld = pDC->SelectObject(&penDraw) ;
	// draw the bottom arc
	pDC->PolylineTo(&pointRecess[nHalfPoints], nHalfPoints) ;
	pDC->LineTo(pointRecess[0]) ;  // connect it to the top
	pDC->SelectObject(pPenOld) ;

	// Draw the meter face

	// use the text color for the border
	penDraw.DeleteObject() ;
	penDraw.CreatePen(PS_SOLID, 1, m_colorText) ;
	pPenOld = pDC->SelectObject(&penDraw) ;

	// use the current window color for filling
	brushFill.DeleteObject() ;
	brushFill.CreateSolidBrush(m_colorWindow) ;
	pBrushOld = pDC->SelectObject(&brushFill) ;

	// draw the meter face
	pDC->Polygon(m_pointBoundary, nRef) ;

	// restore the brush (but keep the pen!)
	pDC->SelectObject(pBrushOld) ;

	// Draw the tick marks.
	// Make the tick marks extend down 10% of the face
	dTemp = m_nTopRadius - 0.1*(m_nTopRadius-m_nBottomRadius) ;

	// draw the right side tick marks
	for (nAngleDeg=90; nAngleDeg>nStartAngleDeg; nAngleDeg-=nTickDeg)
	{
		dAngleRad = nAngleDeg*dRadPerDeg ;

		// move to the top of the tick mark
		dX = m_nBottomCX + m_nTopRadius*cos(dAngleRad) ;
		dY = m_nBottomCY - m_nTopRadius*sin(dAngleRad) ;
		pDC->MoveTo(ROUND(dX), ROUND(dY)) ;

		// move to the bottom of the tick mark
		dX = m_nBottomCX + dTemp*cos(dAngleRad) ;
		dY = m_nBottomCY - dTemp*sin(dAngleRad) ;
		pDC->LineTo(ROUND(dX), ROUND(dY)) ;
	}

	// draw the left side tick marks
	for (nAngleDeg=90+nTickDeg; nAngleDeg<nEndAngleDeg; nAngleDeg+=nTickDeg)
	{
		dAngleRad = nAngleDeg*dRadPerDeg ;

		// move to the top of the tick mark
		dX = m_nBottomCX + m_nTopRadius*cos(dAngleRad) ;
		dY = m_nBottomCY - m_nTopRadius*sin(dAngleRad) ;
		pDC->MoveTo(ROUND(dX), ROUND(dY)) ;

		// move to the bottom of the tick mark
		dX = m_nBottomCX + dTemp*cos(dAngleRad) ;
		dY = m_nBottomCY - dTemp*sin(dAngleRad) ;
		pDC->LineTo(ROUND(dX), ROUND(dY)) ;
	}

	// now we're done with the pen!
	pDC->SelectObject(pPenOld) ;

	// Draw the recessed rectangle 
	// for the numerical value.

	// draw the left and top sides with the shadow
	penDraw.DeleteObject() ;
	penDraw.CreatePen(PS_SOLID, 1, m_colorShadow) ;
	pPenOld = pDC->SelectObject(&penDraw) ;
	pDC->MoveTo(m_rectValue.left, m_rectValue.bottom) ;
	pDC->LineTo(m_rectValue.left, m_rectValue.top) ;
	pDC->LineTo(m_rectValue.right, m_rectValue.top) ;
	pDC->SelectObject(pPenOld) ;

	// draw the right and bottom sides with the highlight
	penDraw.DeleteObject() ;
	penDraw.CreatePen(PS_SOLID, 1, m_colorHighlight) ;
	pPenOld = pDC->SelectObject(&penDraw) ;
	pDC->LineTo(m_rectValue.right, m_rectValue.bottom) ;
	pDC->LineTo(m_rectValue.left, m_rectValue.bottom) ;
	pDC->SelectObject(pPenOld) ;

	// determine the font size
	nHeight = m_rectValue.Height()*45/100 ;

  m_fontValue.DeleteObject() ;
	m_fontValue.CreateFont (nHeight, 0, 0, 0, 400,
												  FALSE, FALSE, 0, ANSI_CHARSET,
												  OUT_DEFAULT_PRECIS, 
												  CLIP_DEFAULT_PRECIS,
												  DEFAULT_QUALITY, 
												  DEFAULT_PITCH|FF_SWISS, "Arial") ;

	// Now select this font and calculate the 
	// actual height to see what Windows gave us.
	pFontOld = pDC->SelectObject(&m_fontValue) ;
  pDC->GetTextMetrics(&tm) ;
  m_nValueFontHeight = tm.tmHeight ;
 
	// determine the location for the value within
	// the rectangele based on the font height
	m_nValueBaseline = m_rectValue.bottom - 
		                 m_nValueFontHeight/4 ;
	m_nValueCenter   = m_rectValue.left + 
		                 m_rectValue.Width()/2 ;

	// now we need to use the font to draw some text
	// set the colors (based on system colors)
	pDC->SetTextColor(m_colorText) ;
	pDC->SetBkColor(m_colorButton) ;

	// draw the units below the meter face
	pDC->SetTextAlign(TA_CENTER|TA_BASELINE) ;
	pDC->TextOut(m_nValueCenter, 
		           m_rectValue.top - m_nValueFontHeight/4,
							 m_strUnits) ;


	// draw the value on the minimum side of the meter
	pDC->SetTextAlign(TA_LEFT|TA_BASELINE) ;
	strTemp.Format("%.*lf", m_nScaleDecimals, m_dMinValue) ;
	pDC->TextOut((rect.left+m_pointBoundary[nHalfPoints-1].x)/2, 
		           m_pointBoundary[nHalfPoints/2].y-m_nValueFontHeight*25/100 ,
							 strTemp) ;


	// draw the center
	pDC->SetTextAlign(TA_RIGHT|TA_BASELINE) ;
	if(m_dMaxValue > 0.1)
		strTemp.Format("%.*lf", m_nScaleDecimals, m_dMaxValue / 2) ;
	else
		strTemp.Format("%.*lf", 2, m_dMaxValue / 2) ;



	if(m_dMaxValue > 1)
		pDC->TextOut((rect.right + 40)/2 - 10 , 
		           m_pointBoundary[nHalfPoints/2].y-m_nValueFontHeight*25/100 - 20,
							 strTemp) ;
	else
		pDC->TextOut((rect.right + 40)/2 , 
		           m_pointBoundary[nHalfPoints/2].y-m_nValueFontHeight*25/100 - 20,
							 strTemp) ;




	// draw the value on the maximum side of the meter
	pDC->SetTextAlign(TA_RIGHT|TA_BASELINE) ;
	strTemp.Format("%.*lf", m_nScaleDecimals, m_dMaxValue) ;
	pDC->TextOut((rect.right+m_pointBoundary[0].x)/2, 
		           m_pointBoundary[nHalfPoints/2].y-m_nValueFontHeight*25/100 ,
							 strTemp) ;

	// restore the font and background color
  pDC->SelectObject(pFontOld) ;
	pDC->SetBkColor(m_colorWindow) ;
	
}


void C3DMeterCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
	
	ReconstructControl() ;

}

void C3DMeterCtrl::ReconstructControl() 
{
	// if we've got a stored background - remove it!
	if ((m_pBitmapOldBackground) && 
		  (m_bitmapBackground.GetSafeHandle()) && 
			(m_dcBackground.GetSafeHdc()))
	{
			m_dcBackground.SelectObject(m_pBitmapOldBackground);
			m_dcBackground.DeleteDC() ;
			m_bitmapBackground.DeleteObject();
	}

	Invalidate () ;

}

void C3DMeterCtrl::SetRange(double dMin, double dMax) 
{
	m_dMaxValue = dMax ;
	m_dMinValue = dMin ;
	ReconstructControl() ;
}

void C3DMeterCtrl::SetScaleDecimals(int nDecimals) 
{
	m_nScaleDecimals = nDecimals ;
	ReconstructControl() ;
}

void C3DMeterCtrl::SetValueDecimals(int nDecimals) 
{
	m_nValueDecimals = nDecimals ;
	ReconstructControl() ;
}

void C3DMeterCtrl::SetUnits(CString &strUnits) 
{
	m_strUnits = strUnits ;
	ReconstructControl() ;
}

void C3DMeterCtrl::SetNeedleColor (COLORREF colorNeedle)
{
	m_colorNeedle = colorNeedle ;
	ReconstructControl() ;
}
