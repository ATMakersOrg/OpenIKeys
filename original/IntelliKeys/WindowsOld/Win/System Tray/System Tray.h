// System Tray.h : main header file for the SYSTEM TRAY application
//

#if !defined(AFX_SYSTEMTRAY_H__2FF1AC56_3500_4F43_8EBB_B1A9D9AD987D__INCLUDED_)
#define AFX_SYSTEMTRAY_H__2FF1AC56_3500_4F43_8EBB_B1A9D9AD987D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSystemTrayApp:
// See System Tray.cpp for the implementation of this class
//

class CSystemTrayApp : public CWinApp
{
public:
	CSystemTrayApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSystemTrayApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CSystemTrayApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSTEMTRAY_H__2FF1AC56_3500_4F43_8EBB_B1A9D9AD987D__INCLUDED_)
