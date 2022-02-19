// LedButton.cpp : implementation file
//

#include "stdafx.h"
#include "LedButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
  Copyright:
    Completely Free (as long as this header remains like this) !
    Please visit http://welcome.to/softbird for updates, other sources and more !
    (c)Benjamin Mayrargue 2000
*/

/////////////////////////////////////////////////////////////////////////////
// CLedButton

void CLedButton::Init( BOOL bReadOnly/*=TRUE*/, BOOL bDepressed/*=FALSE*/, BOOL bCenterAlign/*=TRUE*/ )
{
	m_bDepressed = bDepressed;
	m_bCenterAlign = bCenterAlign;
	m_nMargin = DEFAULT_MARGIN;
  m_bReadOnly = bReadOnly;
}


BEGIN_MESSAGE_MAP(CLedButton, CButton)
	//{{AFX_MSG_MAP(CLedButton)
	//}}AFX_MSG_MAP
	//ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLedButton message handlers
void CLedButton::PreSubclassWindow() 
{
	CButton::PreSubclassWindow();

	// make sure we are an owner draw button
	ModifyStyle(0, BS_OWNERDRAW);

	CRect rect;
	GetClientRect(rect);

	// setup the button metrics
	LOGFONT lf;
	GetFont()->GetLogFont(&lf);
	m_nRadius = lf.lfHeight;
	if( m_nRadius == 0 )
		m_nRadius = 15;
	if( m_nRadius < 0 )
		m_nRadius = (-1)*m_nRadius;
	m_nRadius  = (int)(rect.bottom*0.5)-5;

	// don't let the button get above this maximum radius
	// user can reset m_nRadius later, if desired
	if( m_nRadius > 6 )
		m_nRadius = 6;

	m_ptCenter.x = rect.left+m_nRadius+1;
	if( m_bCenterAlign )
		m_ptCenter.y = rect.top+(int)(rect.Height()*0.5+0.5);
	else
		m_ptCenter.y = rect.top+m_nRadius+1;		
}

void CLedButton::SetImage(UINT idBmp, UINT nWidthOfOneImage)
{
  m_imgList.Create( idBmp, nWidthOfOneImage );
  m_rcImage = m_imgList.GetImageDimension();
  m_ptCenter.x  = 1; //m_ptCenter.x - rcImage.cx/2;
  m_ptCenter.y  = m_ptCenter.y - m_rcImage.cy/2;

  if( m_nMargin<m_rcImage.cx )
    m_nMargin = 5 + m_rcImage.cx;
}


// draw the button and text
void CLedButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	ASSERT(lpDrawItemStruct != NULL);
  if( lpDrawItemStruct->itemAction != ODA_DRAWENTIRE )
    return;

	CDC* pDC   = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect = lpDrawItemStruct->rcItem;

	int nSavedDC = pDC->SaveDC();

	pDC->SelectStockObject(NULL_BRUSH);
	pDC->FillSolidRect(rect, ::GetSysColor(COLOR_BTNFACE));

  //if( GetStyle()&BS_AUTOCHECKBOX && lpDrawItemStruct->itemState&ODS_SELECTED)
//    SetCheck(GetCheck()?0:1);
  //if( !m_bReadOnly )
    //m_bDepressed = lpDrawItemStruct->itemState&ODS_SELECTED;
    //m_bDepressed = GetCheck();
    //Does not work as an auto checkbox !!! Why ???
  
  if( m_bDepressed ) //Button down
    m_imgList.Draw(pDC,1,m_ptCenter);
	else	//Button up
    m_imgList.Draw(pDC,0,m_ptCenter);

	// draw the text if there is any
	CString strText;
	GetWindowText(strText);
	CSize Extent;
	CPoint pt;

	CRect textRect, clientRect;
	GetClientRect( clientRect );
	textRect = clientRect;
	textRect.left += m_nMargin;
	textRect.right -= 2;
	if (!strText.IsEmpty())
	{

		pDC->SetBkMode(TRANSPARENT);

		/*if (state & ODS_DISABLED)
    {
		  Extent = pDC->GetTextExtent(strText);
		  pt.x = rect.left + m_nMargin;
		  pt.y = (int)((rect.Height() - Extent.cy)*0.5);
			pDC->DrawState(pt, Extent, strText, DSS_DISABLED, TRUE, 0, (HBRUSH)NULL);
      }
		else*/
		{
			pDC->DrawText( strText, textRect, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK|DT_CALCRECT );
			int h = textRect.Height();
			textRect.top = clientRect.top+(int)((clientRect.Height()-textRect.Height())*0.5+0.5);
			textRect.bottom = textRect.top + h;
			pDC->DrawText( strText, textRect, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK );
			m_textRect = textRect;
		}
	}
	textRect.right += 2;

	pDC->RestoreDC(nSavedDC);
}


// return the button's depression state
BOOL CLedButton::IsDepressed()
{
  if( !m_bReadOnly )
    m_bDepressed = GetCheck();
	return m_bDepressed;
}


// set the button's depression state
BOOL CLedButton::Depress(BOOL bDown)
{
  //if( !m_bReadOnly )
  //  m_bDepressed = GetCheck();
	if( bDown != m_bDepressed )
	{
		m_bDepressed = bDown;
    //SetCheck(bDown);
    //PostMessage(BM_SETCHECK, bDown, 0);
		RedrawWindow();
	}
	
	return m_bDepressed;
}
/*
BOOL CLedButton::OnClicked() 
{
  //Depress(!GetCheck()); //Does not work. It seems that SetCheck can not be called from inside a handler.
  return FALSE; //Continue routing
}
  */