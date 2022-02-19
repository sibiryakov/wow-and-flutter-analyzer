// wfgui.h : main header file for the WFGUI application
//

#if !defined(AFX_WFGUI_H__0DF72EC8_BC42_4BF2_8477_4ACB8BFDC5D2__INCLUDED_)
#define AFX_WFGUI_H__0DF72EC8_BC42_4BF2_8477_4ACB8BFDC5D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CWfguiApp:
// See wfgui.cpp for the implementation of this class
//

class CWfguiApp : public CWinApp
{
public:
	CWfguiApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWfguiApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWfguiApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WFGUI_H__0DF72EC8_BC42_4BF2_8477_4ACB8BFDC5D2__INCLUDED_)
