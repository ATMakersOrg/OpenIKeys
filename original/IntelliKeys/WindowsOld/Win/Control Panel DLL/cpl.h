/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		cpl.h
//
// Purpose:		main header file for the CPL DLL
//
// 06/18/01 fwr initial implementation	
//
**************************************************************************************************************************/

#if !defined(AFX_CPL_H__7D0D8345_1232_11D5_BC01_00D0590D2899__INCLUDED_)
#define AFX_CPL_H__7D0D8345_1232_11D5_BC01_00D0590D2899__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include <cpl.h>

#include "IKUTil.h"
#include "IKString.h"
#include "IKMessage.h"
#include "MessageClient.h"

static bool bInited = false;

/////////////////////////////////////////////////////////////////////////////
// CCplApp
// See cpl.cpp for the implementation of this class
//

class CCplApp : public CWinApp
{
public:
	CCplApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCplApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CCplApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

__declspec(dllexport) LONG CALLBACK CPlApplet(HWND hwndCPL, UINT uMsg, LPARAM lParam1, LPARAM lParam2) 

{ 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    LPCPLINFO lpCPlInfo; 

    switch (uMsg) { 
        case CPL_INIT:      // first message, sent once
			// AfxMessageBox("IK USB CP CPL_INIT");
            return TRUE; 
 
        case CPL_GETCOUNT:  // second message, sent once 
			// AfxMessageBox("IK USB CP CPL_GETCOUNT");
            return 1; 
            break; 
 
        case CPL_INQUIRE: // third message, sent once per application 
			// AfxMessageBox("IK USB CP CPL_INQUIRE");
            lpCPlInfo = (LPCPLINFO) lParam2; 
            lpCPlInfo->lData = 0; 
            lpCPlInfo->idIcon = IDI_ICON1;
            lpCPlInfo->idName = IDS_STRING1;
            lpCPlInfo->idInfo = IDS_STRING2;
            break; 

        case CPL_DBLCLK:    // application icon double-clicked
			// AfxMessageBox("IK USB CP CPL_DBLCLK");

			try
			{
				if (!bInited)
				{
					IKUtil::Initialize();
					IKMessage::Initialize();
					bInited = true;
					// AfxMessageBox("IK USB IKUtil::Initialize");
				}

				if (!IKIsControlPanelAlive())
				{
					IKString app = IKUtil::GetRootFolder();
					app += DATAS("Control Panel Name","");
					//// AfxMessageBox((char *)app);

					int nResult = (int) ShellExecute ( GetDesktopWindow(), TEXT("Open"), (TCHAR *)app, TEXT(""), TEXT("C:\\"), SW_SHOWNORMAL); 
					//if ( nResult<=32 && nResult!=ERROR_FILE_NOT_FOUND );
				}

			}
			catch (...)
			{
				//AfxMessageBox("IK USB exception caught");
			}

            break; 
 
        case CPL_STOP:      // sent once per application before CPL_EXIT 
			// AfxMessageBox("IK USB CP CPL_STOP");
            break; 
 
        case CPL_EXIT:    // sent once before FreeLibrary is called 
			// AfxMessageBox("IK USB CP CPL_EXIT");
            break; 
 
        default: 
			// AfxMessageBox("IK USB CP default");
            break; 
    } 


	return 0;

}


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CPL_H__7D0D8345_1232_11D5_BC01_00D0590D2899__INCLUDED_)
