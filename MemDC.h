#ifndef _MEMDC_H_
#define _MEMDC_H_

//////////////////////////////////////////////////
// CMemDC - memory DC
//
// Author: Keith Rule
// Email:  keithr@europa.com
// Copyright 1996-1999, Keith Rule
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
// History - 10/3/97 Fixed scrolling bug.
//                   Added print support. - KR
//
//           11/3/99 Fixed most common complaint. Added
//                   background color fill. - KR
//
//           11/3/99 Added support for mapping modes other than
//                   MM_TEXT as suggested by Lee Sang Hun. - KR
//
// Modified by Mark Malburg  March 12, 1998
// Email:  mcmalburg@sytech.cummins.com
//  (added new construction and clipboard handling)
//
//	Construction :
//	|
//	|	CMemDC pDC (dc, &drawRect, toMemDC) ;
//	|	
//	|	where:
//	|		"dc"		- pointer to the CDC that is an argument to OnDraw
//	|		"drawRect"	- pointer to the rectangle to draw in
//	|		"boolToMemory"	- TRUE: to the client, FALSE: to clipboard or printer
//	|
//
// This class implements a memory Device Context which allows
// flicker free drawing.

class CMemDC : public CDC 
{
private:	
	CBitmap		m_bitmap;		// Offscreen bitmap
	CBitmap*	m_oldBitmap;	// bitmap originally found in CMemDC
	CDC*		m_pDC;			// Saves CDC passed in constructor
	CRect		m_rect;			// Rectangle of drawing area.
	BOOL		m_bMemDC;		// TRUE if CDC really is a Memory DC.
public:
	
	CMemDC(CDC* pDC, const CRect* pRect = NULL, bool boolToMemory = TRUE) : CDC()
	{
		ASSERT(pDC != NULL); 

		// Some initialization
		m_pDC = pDC;
		m_oldBitmap = NULL;
		if (boolToMemory)
			m_bMemDC = !pDC->IsPrinting();
		else
			m_bMemDC = FALSE ;

		// Get the rectangle to draw
		if (pRect == NULL) 
		{
			pDC->GetClipBox(&m_rect);
		} 
		else 
		{
			m_rect = *pRect;
		}
		
		if (m_bMemDC) 
		{
			// Create a Memory DC
			CreateCompatibleDC(pDC);
			pDC->LPtoDP(&m_rect);

			m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
			m_oldBitmap = SelectObject(&m_bitmap);
			
			SetMapMode(pDC->GetMapMode());
			pDC->DPtoLP(&m_rect);
			SetWindowOrg(m_rect.left, m_rect.top);
		} 
		else 
		{
			// Make a copy of the relevent parts of the current DC for printing
			if (pDC->IsPrinting())
				m_bPrinting = pDC->m_bPrinting;
			m_hDC       = pDC->m_hDC;
			m_hAttribDC = pDC->m_hAttribDC;
		}

		// Fill background 
		FillSolidRect(m_rect, pDC->GetBkColor());
	}

	
	~CMemDC()	
	{		
		if (m_bMemDC) 
		{
			// Copy the offscreen bitmap onto the screen.
			m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
				            this, m_rect.left, m_rect.top, SRCCOPY);			
			
			//Swap back the original bitmap.
			SelectObject(m_oldBitmap);
		} 
		else 
		{
			// All we need to do is replace the DC with an illegal value,
			// this keeps us from accidently deleting the handles associated with
			// the CDC that was passed to the constructor.			
			m_hDC = m_hAttribDC = NULL;
		}	
	}
	
	// Allow usage as a pointer	
	CMemDC* operator->() 
	{
		return this;
	}	

	// Allow usage as a pointer	
	operator CMemDC*() 
	{
		return this;
	}
};

#endif