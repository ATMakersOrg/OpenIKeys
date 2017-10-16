// System Tray.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "System Tray.h"

#include "MainFrm.h"

#include "IKUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSystemTrayApp

BEGIN_MESSAGE_MAP(CSystemTrayApp, CWinApp)
	//{{AFX_MSG_MAP(CSystemTrayApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSystemTrayApp construction

CSystemTrayApp::CSystemTrayApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSystemTrayApp object

CSystemTrayApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSystemTrayApp initialization

BOOL CSystemTrayApp::InitInstance()
{
	//  one processor, please.
	IKUtil::SetOneProcessor();

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create and load the frame with its resources

	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	//  set a timer
	m_pMainWnd->SetTimer(1,5,0);
	pFrame->DoTimer() ;

	// show the window off-screen
	
	RECT rect;
	pFrame->GetWindowRect(&rect);
	rect.top += 4000;
	rect.bottom += 4000;
	rect.left += 4000;
	rect.right += 4000;
	pFrame->MoveWindow(&rect,false);

	// The one and only window has been initialized, so show and update it.
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	pFrame->ShowWindow(SW_HIDE);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTrayApp message handlers





/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CSystemTrayApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTrayApp message handlers

