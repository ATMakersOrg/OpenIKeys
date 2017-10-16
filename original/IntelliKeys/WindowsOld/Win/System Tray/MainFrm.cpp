// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "System Tray.h"

#include "SystemTrayIcon.h"

#include "IKUTil.h"
#include "IKString.h"
#include "IKFile.h"
#include "IKMessage.h"
#include "MessageClient.h"
#include "IKSettings.h"
#include "AppLib.h"

#include "MainFrm.h"

#include "IKFileDialog.h"
#include "IKCommon.h"

#ifdef PLAT_WINDOWS
  #define KEYEVENTF_UNICODE 4
  // #include <winable.h>
#endif

enum {IK_DOWN=1,IK_UP,IK_TOGGLE};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_TIMER()
	ON_WM_ENDSESSION()
	ON_WM_QUERYENDSESSION()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

class MySystemTrayIcon: public SystemTrayIcon
{
public:
	MySystemTrayIcon(bool bUnicode);
	void OnMouseMove();
	void DoMenu ();
	void OnLButtonDown();
	void OnLButtonUp();
	void OnRButtonDown();
	void OnRButtonUp();
	void OnLButtonDblClick();
	void SetToolTip(LPCTSTR tip);

	IKString m_strLastApp;
	HWND m_lastWindow;
	bool m_bHandlingClick;
	bool m_bForceSend;
};

static bool Launch()
{

	//  is it already sunning?
	int response = IKMessage::SendTO ( TEXT("control panel"), kQueryOKToSend, 0, 0, 0, 0, 250 );
	if (response != kResponseNoServer)
	{
		//  bring it forward
		response = IKMessage::Send ( TEXT("control panel"), kQueryForwardControlPanel, 0, 0, 0, 0);

		return true;
	}

	//  remember when we started for timeout purposes
	unsigned int then = IKUtil::GetCurrentTimeMS();

	//  launch it
	IKString app = IKUtil::GetRootFolder(); 
	app += DATAS("Control_Panel_Name","");

	SHELLEXECUTEINFO ShExecInfo = {0};

	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = (TCHAR *)app;		
	ShExecInfo.lpParameters = "";	
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;	

	ShellExecuteEx(&ShExecInfo);

	//  wait for it to become responsive
	bool bReady = false;
	while (true)
	{
		int response;
		response = IKMessage::SendTO ( TEXT("control panel"), kQueryOKToSend, 0, 0, 0, 0, 250 );
		if (response==kResponseUnsupported)
		{
			bReady = true;
			break;
		}
		unsigned int now = IKUtil::GetCurrentTimeMS();
		if (now-then > 10000)  //  10 seconds
			break;
		IKUtil::Sleep(250);
	}

	return bReady;
}

void MySystemTrayIcon::SetToolTip(LPCTSTR tip)
{
	//  see if we're really supposed to allow the menu
	bool bEnabled = !!DATAI(TEXT("menu_enabled"), 1);
	if (!bEnabled)
		return;

	SystemTrayIcon::SetToolTip(tip);
}

//  track when the mouse was last moved
static unsigned int lastMouseMove = 0;

void MySystemTrayIcon::OnLButtonDown()
{
}

void MySystemTrayIcon::OnLButtonUp()
{
	//  if we are already handling a click, don't recurse.
	if (m_bHandlingClick)
		return;

	//  specify that we're handling this click
	//  while DoMenu() is underway.
	m_bHandlingClick = true;
	DoMenu();
	m_bHandlingClick = false;
}

void MySystemTrayIcon::OnRButtonDown()
{
}

void MySystemTrayIcon::OnRButtonUp()
{
	//  if we are already handling a click, don't recurse.
	if (m_bHandlingClick)
		return;

	//  specify that we're handling this click
	//  while DoMenu() is underway.
	m_bHandlingClick = true;
	DoMenu();
	m_bHandlingClick = false;
}

void MySystemTrayIcon::OnLButtonDblClick()
{
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	//  call base class
	CFrameWnd::OnTimer(nIDEvent);

	//  call the slow timer every 100 times (500 msec)
	static int counter = 100;
	if (counter>=100)
	{
		DoTimer();
		counter = 0;
	}
	counter += 1;
	
	//  call the fast timer every time (5 msec)
	DoFastTimer();
}

//--------------------------------------------------------------------------

//  pointer to the system tray icon object, created dynamically.
static MySystemTrayIcon *pSystray = NULL;

//  settings for the current user
static IKString student;
static IKString group;
static IKSettings currentSettings;
static IKSettings sendingSettings;


//  load settings
static void LoadCurrentSettings (IKSettings *pTheSettings=NULL);
static void LoadCurrentSettings (IKSettings *pTheSettings)
{
	//  load global preferences
	IKString prefsFile = IKUtil::GetPrivateFolder();
	prefsFile += DATAS(TEXT("Preferences_File"),TEXT("preferences.txt"));
	IKPrefs prefs;
	prefs.Read(prefsFile);

	//  get group and student name
	group   = prefs.GetValueString(TEXT("Group"),  TEXT(""));
	student = prefs.GetValueString(TEXT("Student"),TEXT(""));
	if (group.IsEmpty() && student.IsEmpty())
	{
		group = DATAS(TEXT("Guest"),TEXT("Guest"));
	}

	//  make path to the user's settings file
	IKString path = IKUtil::MakeStudentPath ( group, student );
	IKString settingsFile = path;
	settingsFile += TEXT("settings.txt");

	//  read user settings
	IKSettings *pSettings = IKSettings::GetSettings();
	pSettings->Read(settingsFile);

	//  copy them locally.
	if (pTheSettings)
	{
		*pTheSettings = *pSettings;
	}
	else
	{
		currentSettings = *pSettings;
	}
}

//  save settings
static void SaveCurrentSettings ()
{
	//  make path to the settings file
	IKString path = IKUtil::MakeStudentPath ( group, student );
	IKString settingsFile = path;
	settingsFile += TEXT("settings.txt");

	//  write out the settings
	currentSettings.Write(settingsFile);
}

//  get switch setting name from the number
static IKString GetSwitchSettingName ( int iSwitchSetting )
{
	int nn = iSwitchSetting;
	if (nn==0)
		nn = 1;
	IKString key = TEXT("switch_file_");
	key += IKUtil::IntToString(nn);
	IKString switchSetting = DATAS(key,TEXT(""));
	IKUtil::StripFileName(switchSetting,true,true);

	return switchSetting;
}

#include "IntelliKeys.h"

static int IKSendOverlayWithName ( TCHAR *overlayname, TCHAR *name, TCHAR *sender )
{
	BYTE data[1000];
	int ndata = 0;

	//  do we have unicode strings?
	data[ndata] = sizeof(TCHAR);
	ndata++;

	//  length of overlayname in chars
	data[ndata] = IKString::strlen(overlayname);
	ndata++;

	//  chars for overlayname
	IKString::strcpy((TCHAR *)&(data[ndata]),overlayname);
	ndata = ndata + IKString::strlen(overlayname) * sizeof(TCHAR);

	//  length of name in chars
	data[ndata] = IKString::strlen(name);
	ndata++;

	//  chars for name
	IKString::strcpy((TCHAR *)&(data[ndata]),name);
	ndata = ndata + IKString::strlen(name) * sizeof(TCHAR);

	//  length of sender in chars
	data[ndata] = IKString::strlen(sender);
	ndata++;

	//  chars for sender
	IKString::strcpy((TCHAR *)&(data[ndata]),sender);
	ndata = ndata + IKString::strlen(sender) * sizeof(TCHAR);

	int response = IKMessage::Send ( TEXT("engine"), kQuerySendOverlayWithName, data, ndata, 0, 0  );
	return response;

}

static void SendOverlayForApp ( IKString &filename, IKString &sender )
{
	//  do some mode checking first
	int Mode = sendingSettings.m_iMode;
	bool bButAllow = sendingSettings.m_bButAllowOverlays;
	bool bWarning = sendingSettings.m_bShowModeWarning;

	DebugLogToFile("SendOverlayForApp - Mode=[%d] ButAllow=[%d] Warn=[%d] File=[%s] Sender=[%s]", Mode, bButAllow, bWarning, (TCHAR*)filename, (TCHAR*)sender);

	IKString s;
	switch (Mode)
	{
	case 1:
#if 0  //  NO FLOATING MESSAGE 7/25/06
		if (bWarning)
		{
			s = DATAS("s_overlay_not_sent_1","");
			s.ToACP();
			AppLibFloatingMessage ( (TCHAR *)s );
		}
#endif
		return;
		break;

	case 2:
#if 0  //  NO FLOATING MESSAGE 7/25/06
		if (bWarning)
		{
			s = DATAS("s_overlay_not_sent_2","");
			s.ToACP();
			AppLibFloatingMessage ( (TCHAR *)s );
		}
#endif
		return;
		break;

	case 3:
		if (bButAllow)
		{
			//  allow the overlay to be sent anyway
			//SetRawMode(false);
		}
		else
		{
#if 0  //  NO FLOATING MESSAGE 7/25/06
			if (bWarning)
			{
				s = DATAS("s_overlay_not_sent_3","");
				s.ToACP();
				AppLibFloatingMessage ( (TCHAR *)s );
			}
#endif
			return;
		}
		break;

	default:
		break;
	}

	//  format a message and play the banner
	IKString simple = filename;
	IKUtil::StripFileName ( simple, true, true );
	
	IKString message;
	message += DATAS(TEXT("sending"),TEXT("sending"));
	message += TEXT(" ");
	message += simple;
	message += TEXT(" ");
	message += DATAS(TEXT("for"),TEXT("for"));
	message += TEXT(" ");
	message += sender;
	message += " ...";
	
	//  OK to call directly here, since it results in
	//  starting a separate program
	//  convert from UTF-8 to ACP first.

	int len = message.GetLength();
	WCHAR wide[1024];
	int res = MultiByteToWideChar ( CP_UTF8, 0, message, len, wide, 1024);
	wide[res] = 0;

	char message2[1024];
	int result2 = WideCharToMultiByte ( CP_ACP, 0, wide, res, message2, 1024, 0, 0);
	message2[result2] = 0;

	DebugLogToFile("SendOverlayForApp - call SendOverlay (Banner) [%s] Message: [%s]", (TCHAR*)filename, message2);

	//  send banner
	bool bReportErrors = false;
	bool bWait = false;
	bool bBannerOnly = true;
	int result = IntelliKeys::SendOverlay( filename, bReportErrors, message2, bWait, bBannerOnly );  // banner only

	//  send overlay back to Engine
	IKString newSender;
	newSender = DATAS(TEXT("intellikeys_usb"),TEXT("IntelliKeys USB"));
	newSender += TEXT(" (");
	newSender += DATAS(TEXT("for"),TEXT("for"));
	newSender += TEXT(" ");
	newSender += sender;
	newSender += TEXT(")");

	DebugLogToFile("SendOverlayForApp - IKSendOverlayWithName (Engine) Overlay [%s] Simple: [%s] Message: [%s]", (TCHAR*)filename, (TCHAR *)simple, (TCHAR *)newSender);

	IKSendOverlayWithName ( (TCHAR *)filename, (TCHAR *)simple, (TCHAR *)newSender );
}

static void SwitchApplication (IKString &app)
{
	//  get the application's friendly name
	TCHAR *pFriendly = ::AppLibGetAppFriendlyName((TCHAR *)app);
	IKString friendly(pFriendly);

	//  get the overlay to send for the current group/student
	LoadCurrentSettings(&sendingSettings);
	TCHAR *pOverlay = AppLibGetStudentAttachedOverlay ( (TCHAR *)app, (TCHAR *)group, (TCHAR *)student );

	//  get the app's current owner
	IKString owner = AppLibGetOwner((TCHAR *)app);

	DebugLogToFile("SwitchApplication- Overlay [%s] Friendly: [%s] Owner: [%s]", pOverlay, pFriendly, (TCHAR*)owner);

	//  IK USB 3.0.3
	//  if there is an owner who is not us, release it.
	//  in 3.0.3, only us and blank are valid owners.
	if (owner.GetLength()>0 && owner.CompareNoCase(TEXT("intellikeys usb"))!=0)
	{
		AppLibReleaseOwnership((TCHAR *)app,(TCHAR *)owner);
		owner = "";
	}

	//  what mode are we currently in?
	int Mode = sendingSettings.m_iMode;

	//  set Raw Mode based on the control panel
	if (Mode == 3)
	{
		IKStartRawMode();
	}
	else
	{
		IKStopRawMode();
	}

	if (/*CountOpenDevices()>0 && */ (strlen(pOverlay) > 0) && (owner.CompareNoCase(TEXT("intellikeys usb"))==0))
	{
		IKString overlay(pOverlay);
		if (IKFile::FileExists(overlay))
		{
			DebugLogToFile("SwitchApplication - call SendOverlayForApp(Overlay [%s] FriendlyName  [%s])", pOverlay, pFriendly);
			//  send it.
			SendOverlayForApp (overlay,friendly);
		}
		else
		{
#if 0  //  NO FLOATING MESSAGE 7/25/06
			//  overlay not found
			IKString message;
			
			IKString simple(overlay);
			IKUtil::StripFileName(simple,true,true);

			message += DATAS(TEXT("S_error_sending_overlay"),TEXT("Error Sending Overlay"));
			message += "|";

			message += DATAS(TEXT("S_overlay"),TEXT("overlay"));
			message += " ";
			message += "\\\"";
			message += simple;
			message += "\\\"";
			message += " ";

			message += DATAS(TEXT("S_attached_to_application"),TEXT("attached to application"));
			message += " ";
			message += "\\\"";
			message += friendly;
			message += "\\\"";
			message += " ";
			message += DATAS(TEXT("S_cannot_be_sent"),TEXT("cannot be sent.  It may have been moved or deleted."));

			bool bWarning = sendingSettings.m_bShowModeWarning;
			if (bWarning)
				AppLibFloatingMessage(message);
#endif
		}
	}
}

//  watch applications

static IKString lastApp = TEXT("");
static void WatchApplications ()
{
	DebugLogToFile("WatchApplications start - LastApp: [%s]", (TCHAR*)lastApp);
	//  have we changed apps?
	bool bChanged = false;
	IKString thisApp = IKUtil::GetCurrentApplicationPath();

	if (!thisApp.IsEmpty())
	{
		if (lastApp.CompareNoCase(thisApp) != 0)
		{
			bChanged = true;
		}
	}
	if (pSystray->m_bForceSend)
	{
		bChanged = true;
	}
	
	if (bChanged)
	{
		DebugLogToFile("WatchApplications - LastApp: [%s] CurrentApp: [%s]", (TCHAR*)lastApp, (TCHAR*)thisApp);
		SwitchApplication(thisApp);
		lastApp = thisApp;
	}

	pSystray->m_bForceSend = false;
}

#include "imm.h"
#define IMC_SETOPENSTATUS  0x0006

static void SetIMEOpen()
{
	HWND hwnd = ::GetForegroundWindow();

	HWND hDefIME = ImmGetDefaultIMEWnd(hwnd);

	SendMessage(hDefIME, WM_IME_CONTROL, IMC_SETOPENSTATUS, true);
}

//--------------------------------------------------------------------------

static void InjectUnicode( BYTE lead, BYTE trail, BYTE direction )
{
	//  force the IME open in Vista
	if (IKUtil::IsWinVistaOrGreater())
	{
		SetIMEOpen();
	}

	WORD wd = MAKEWORD ( trail, lead );

	INPUT in;
	in.type				= INPUT_KEYBOARD;
	in.ki.wVk			= 0;
	in.ki.time			= 0;
	in.ki.dwExtraInfo	= 0;
	in.ki.wScan			= wd;

	in.ki.dwFlags = KEYEVENTF_UNICODE;
	if (direction == IK_UP)
	{
		in.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
	}

	UINT result = ::SendInput( 1, &in, sizeof(INPUT));

}

static void HandleIncoming ()
{
	BYTE data[255]; 
	int datalen; 
	int command;
	int response = IKMessage::Receive ( TEXT("system tray"), &command, (void *)data, 255, &datalen );
	if (response != kResponseNoCommand)
	{
		switch (command)
		{
		case kQuerySendUnicode:
			IKMessage::Respond(TEXT("system tray"),kResponseNoError,0,0);
			{
				BYTE lead = data[0];
				BYTE trail = data[1];
				int direction = data[2];
				InjectUnicode ( lead, trail, direction );
			}
			break;

		case kQueryLaunchFile:
			IKMessage::Respond(TEXT("system tray"),kResponseNoError,0,0);
			{
				IKString file(data,datalen);
				IKUtil::LaunchFile ( file );
			}
			break;

		case kQueryShutdown:
			IKMessage::Respond(TEXT("system tray"),kResponseNoError,0,0);
			{
				if (pSystray)
				{
					delete pSystray;
				}
				exit(0);
			}
			break;

		case kQueryShowControlPanel:
			IKMessage::Respond(TEXT("system tray"),kResponseNoError,0,0);
			{
				IKString app = IKUtil::GetRootFolder();
				app += DATAS(TEXT("Control_Panel_Name"),TEXT("USB Control Panel.exe"));
				int nResult = (int) ShellExecute( ::GetDesktopWindow(), TEXT("Open"), (TCHAR *)app, TEXT(""), TEXT("C:\\"), SW_SHOWNORMAL); 
			}
			break;

		case kQuerySetStudent:
			IKMessage::Respond(TEXT("system tray"),kResponseNoError,0,0);
			LoadCurrentSettings();
			break;

		default:
			IKMessage::Respond(TEXT("system tray"),kResponseNoError,0,0);
			break;
		}
	}
}

//  function called when the timer fires.
void CMainFrame::DoFastTimer()
{
	HandleIncoming ();
}

//  indicates whether we are processing the menu
static bool bInMenu = false;

//  function called when the timer fires.
void CMainFrame::DoTimer()
{
	//  not while we're in the menu
	if (bInMenu)
	{
		return;
	}

	//  initialize
	static bool bInitialized = false;
	if (!bInitialized)
	{
		bInitialized = true;

		//  initialize some APIs
		IKUtil::Initialize();
		IKMessage::Initialize();

		//  make the system tray icon
		if (pSystray == NULL)
		{
			bool b2kandup = IKUtil::IsWin2KOrGreater();
			pSystray = new MySystemTrayIcon(b2kandup);
		}

		//  attempt to create and hold a file in the root folder.
		//  if you can't, then there is another instance running
		//  try a few times before giving up

		CString filename = IKUtil::GetPrivateFolder();
		filename += DATAS("Tray_File","iksystray.dat");

		static CFile file;
		bool bAbleToOpen = false;
		int ntries = 0;
		while (!bAbleToOpen)
		{
			bAbleToOpen = !!file.Open((LPCSTR)filename, CFile::modeWrite | CFile::shareExclusive  | CFile::modeCreate );
			ntries++;
			if (ntries>10)
			{
				exit(0);
			}
			if (!bAbleToOpen)
			{
				Sleep(100);
			}
		}


		//  set up the tray icon
		pSystray->SetWindow(NULL);
		pSystray->SetIcon(IDR_TRAYICON);

		// default tool tip
		IKString tt = DATAS("Systray_Tip","tip");
		pSystray->SetToolTip(tt);

		pSystray->SetMenuID(NULL);

	}

	//  remember the last app seen
	bool bCaptureApp = true;
	if (pSystray->m_bHandlingClick)
	{
		bCaptureApp = false;
	}
	unsigned int now = IKUtil::GetCurrentTimeMS();
	if ((lastMouseMove != 0) && ((now - lastMouseMove) < 1000))
	{
		bCaptureApp = false;
	}
	IKString lastApp = IKUtil::GetCurrentApplicationPath();
	if (bCaptureApp)
	{
		bCaptureApp = AppLibCanListApplication ( lastApp );
	}
	if (pSystray->m_bForceSend)
	{
		bCaptureApp = true;
	}
	if (bCaptureApp)
	{
		pSystray->m_strLastApp = lastApp;
		pSystray->m_lastWindow = ::GetForegroundWindow();
	}

	//  enable system tray if need be
	if (!pSystray->IsEnabled())
	{
		pSystray->Enable(true);
	}

	//  get info about the devices
	int nIK = 0;
	int nIKOn = 0;
	int nIS = 0;
	int nISOn = 0;
	BYTE deviceArray[2000];
	int response = IKGetDeviceArray ( deviceArray );
	// JR - the number of connected devices is the 1st byte in the returned buffer
	int nDevices = deviceArray[0];

	if (response == kResponseUSBIntelliKeysArray)
	{
		int deviceArrayIdx = 0;
		deviceArrayIdx++;

		for (int iDev = 0; iDev < nDevices; iDev++)
		{
			int deviceIndex = deviceArray[deviceArrayIdx];  
			deviceArrayIdx++;
			int nOnOff =  deviceArray[deviceArrayIdx];  
			deviceArrayIdx++;
			deviceArrayIdx = deviceArrayIdx + 9;  //  skip sensors
			int level = deviceArray[deviceArrayIdx];  
			deviceArrayIdx++;
			int std = deviceArray[deviceArrayIdx];  
			deviceArrayIdx++;

			//  overlay
			IKString overlay;
			int len = deviceArray[deviceArrayIdx];  
			deviceArrayIdx++;
			int i;
			for (i = 0;i < len; i++)
			{
				overlay += char(deviceArray[deviceArrayIdx+i]);
			}
			deviceArrayIdx = deviceArrayIdx + len;

			//  serial number
			IKString serial;
			len = deviceArray[deviceArrayIdx];  
			deviceArrayIdx++;
			for (i = 0; i < len; i++)
			{
				serial += char(deviceArray[deviceArrayIdx+i]);
			}
			deviceArrayIdx = deviceArrayIdx + len;

			// skip switch status
			deviceArrayIdx = deviceArrayIdx + 2;

			//  skip membrane status
			len = deviceArray[deviceArrayIdx]; 
			deviceArrayIdx++;
			deviceArrayIdx = deviceArrayIdx + len*2;

			//  device type
			int devType = deviceArray[deviceArrayIdx]; 
			deviceArrayIdx++;

			//  skip overlay number
			deviceArrayIdx = deviceArrayIdx + 1;

			//  skip switch status for 3, 4, and 5
			deviceArrayIdx = deviceArrayIdx + 3;
			
			switch (devType)
			{
			case 1:
				nIK++;
				if (nOnOff)
				{
					nIKOn++;
				}
				break;
			case 2:
				nIS++;
				if (nOnOff)
				{
					nISOn++;
				}
				break;
			default:
				break;
			}
		}
	}


	//  set the icon based on whether devices are connected, and if they are on.
	static int lastIcon = 0;
	int newIcon = 0;

	if ((nIKOn > 0) || (nISOn > 0))
	{
		newIcon = IDR_TRAYICON;
	}
	else
	{
		newIcon = IDR_TRAYICON2;
	}

	if (newIcon != lastIcon)
	{
		lastIcon = newIcon;
		pSystray->SetIcon(newIcon);
	}

	//  set the tooltip
	//  don't do if we're not watching apps

	bool bWatch = !!DATAI(TEXT("watch_apps"), 0);
	if (bWatch)
	{
		IKString toolTip;
		if ((nIK > 0) && (nIS <= 0))
		{
			if (nIKOn > 0)
			{
				toolTip = DATAS(TEXT("S_TOOLTIP_IK_ON"),TEXT("S_TOOLTIP_IK_ON"));
			}
			else
			{
				toolTip = DATAS(TEXT("S_TOOLTIP_IK_OFF"),TEXT("S_TOOLTIP_IK_OFF"));
			}
		}
		else if ((nIK <= 0) && (nIS > 0))
		{
			if (nISOn > 0)
			{
				toolTip = DATAS(TEXT("S_TOOLTIP_IS_ON"),TEXT("S_TOOLTIP_IS_ON"));
			}
			else
			{
				toolTip = DATAS(TEXT("S_TOOLTIP_IS_OFF"),TEXT("S_TOOLTIP_IS_OFF"));
			}
		}
		else if ((nIK >= 0) && (nIS > 0))
		{
			toolTip = DATAS(TEXT("S_TOOLTIP_MULTIPLE_DEVICES"),TEXT("S_TOOLTIP_MULTIPLE_DEVICES"));
		}
		else
		{
			toolTip = DATAS(TEXT("S_TOOLTIP_NO_DEVICES"),TEXT("S_TOOLTIP_NO_DEVICES"));
		}

		pSystray->SetToolTip(toolTip);
	}

	//DebugLogToFile("MainFrm::DoTimer - Devices: [%d] WatchApps: [%d]", nDevices, bWatch);

	//  do app-watching
	if (nDevices > 0)
	{
		WatchApplications();
	}
}

//  meta-data about each menu item
class menuItem
{
public:
	int type;
	IKString sdata;
	int idata;
};

void MyAppendMenu ( CMenu *pMenu, CMenu *pSubMenu, UINT nFlags, LPCTSTR lpszNewItem )
{
	if (DATAI(TEXT("Unicode_File_Names"),0) != 0)
	{
		//  convert string to Unicode
		int len = IKString::strlen(lpszNewItem);
		WCHAR wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,				// utf-8
							0,						// character-type options
							(const char *) lpszNewItem, // string to map
							len,				// number of bytes in string
							wide,				// wide-character buffer
							255		// size of buffer
							);
		wide[result] = 0;

		//  get the menu
		HMENU h = pMenu->GetSafeHmenu();

		//  insert using the Unicode string
		::AppendMenuW( h, nFlags, (UINT)pSubMenu->m_hMenu, wide );
	}
	else
	{
		pMenu->AppendMenu(nFlags, (UINT)pSubMenu->m_hMenu, lpszNewItem);
	}
}

//  insert an item in a menu.  Handles Japanese.
void MyInsertMenu (CMenu *pMenu, UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem)
{
	//  see if we're really supposed to allow the menu
	bool bEnabled = !!DATAI(TEXT("menu_enabled"), 1);
	UINT enable = MF_ENABLED;
	if (!bEnabled)
			enable = MF_GRAYED;

	if (DATAI(TEXT("Unicode_File_Names"),0) != 0)
	{
		//  get the menu
		HMENU h = pMenu->GetSafeHmenu();

		//  convert string to Unicode
		int len = IKString::strlen(lpszNewItem);
		WCHAR wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,				// utf-8
							0,						// character-type options
							(const char *) lpszNewItem, // string to map
							len,				// number of bytes in string
							wide,				// wide-character buffer
							255		// size of buffer
							);
		wide[result] = 0;

		//  insert using the Unicode string
		InsertMenuW( h, nPosition, nFlags, nIDNewItem, wide );

		::EnableMenuItem(h, nIDNewItem, enable);
	}
	else
	{
		pMenu->InsertMenu(nPosition, nFlags, nIDNewItem, lpszNewItem);

		pMenu->EnableMenuItem(nIDNewItem, enable);
	}
}



//  called each time the system tray icon is clicked.
void MySystemTrayIcon::DoMenu()
{
	//  see if we're really supposed to allow the menu
	bool bEnabled = !!DATAI(TEXT("menu_enabled"), 1);
	if (!bEnabled)
	{
		return;
	}

	//  declare that menu processing is is underway.
	bInMenu = true;

	//  create the menu
	CMenu menu;
	BOOL bCreated = menu.CreatePopupMenu();
	if(!bCreated)
	{
		return;
	}

	//  load preferences for the current user
	LoadCurrentSettings();

	//  create data for items.
	menuItem items[60];
	int nItems = 0;

	//  create an item for launching the CP
	items[nItems].type = 1;	
	if (AppLibIsIntelliSwitchInstalled())
	{
		MyInsertMenu ( &menu, -1, MF_BYPOSITION, 1000+nItems, DATAS("Systray_Menu_Switch","menu") );
	}
	else
	{
		MyInsertMenu ( &menu, -1, MF_BYPOSITION, 1000+nItems, DATAS("Systray_Menu","menu") );
	}
	nItems++;


	//  create items for the four modes
	IKString message;
	MyInsertMenu ( &menu, -1, MF_BYPOSITION|MF_SEPARATOR, 998, "-" );
	MyInsertMenu ( &menu, -1, MF_BYPOSITION|MF_DISABLED, 999, DATAS(TEXT("Mode:"),TEXT("Mode:")) );

	items[nItems].type = 8;
	message = "   ";
	message += DATAS(TEXT("Overlays"),TEXT("Overlays"));
	MyInsertMenu ( &menu, -1, MF_BYPOSITION, 1000+nItems, message );
	if (currentSettings.m_iMode == 0)
	{
		menu.CheckMenuItem(1000 + nItems, MF_CHECKED);
	}
	nItems++;

	items[nItems].type = 11;
	message = "   ";
	message += DATAS(TEXT("One_Overlay_Only"),TEXT("One Overlay Only"));
	MyInsertMenu ( &menu, -1, MF_BYPOSITION, 1000+nItems, message );
	if (currentSettings.m_iMode==1)
	{
		menu.CheckMenuItem(1000 + nItems, MF_CHECKED);
	}
	nItems++;

	items[nItems].type = 9;
	message = "   ";
	message += DATAS(TEXT("Switch_Presets"),TEXT("Switch Presets"));
	MyInsertMenu ( &menu, -1, MF_BYPOSITION, 1000+nItems, message );
	if (currentSettings.m_iMode==2)
	{
		menu.CheckMenuItem(1000+nItems,MF_CHECKED);
	}
	nItems++;

	if (AppLibIsSystemPresent(TEXT("Discover")) || AppLibShowDiscoverAnyway())  //  don't show Discover if it's not installed.
	{
		items[nItems].type = 10;
		message = "   ";
		message += DATAS(TEXT("DiscoverName"),TEXT("Discover"));
		MyInsertMenu ( &menu, -1, MF_BYPOSITION, 1000+nItems, message );
		if (currentSettings.m_iMode==3)
		{
			menu.CheckMenuItem(1000+nItems,MF_CHECKED);
		}
		nItems++;
	}

	IKString owner = "";

	// create items for overlay mode.
	if (currentSettings.m_iMode == kSettingsModeLastSentOverlay)
	{
		MyInsertMenu ( &menu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, "-" );
		//MyInsertMenu ( &menu, -1, MF_BYPOSITION|MF_DISABLED, 0, DATAS(TEXT("In Overlay Mode:"),TEXT("In Overlays Mode:")) );

		//  remember the number of which item to check.
		//  if it's zero, then don't check anything.
		int itemToCheck = 0;

		// item for the current app
		bool bItemForApp = true;  //  AppLibIsAppAllowed(pSystray->m_strLastApp);

		//  not if it's the control panel itself
		IKString s = pSystray->m_strLastApp;
		IKUtil::StripFileName(s, true, false);
		if (s.CompareNoCase("IntelliKeys USB.exe")==0)
		{
			bItemForApp = false;
		}

		//  not if it's no longer running
		{
			IKString s2 = pSystray->m_strLastApp;
			IKUtil::StripFileName(s2, true, true);

			//  at this point app is in UTF-8.  IsAppRunning
			//  requires ANSI.  Let:s convert

			int len = s2.GetLength();
			WCHAR wide[1024];
			int res = MultiByteToWideChar(CP_UTF8, 0, s2, len, wide, 1024);
			wide[res] = 0;

			char narrow[1024];
			int result2 = WideCharToMultiByte(CP_ACP, 0, wide, res, narrow, 1024, 0, 0);
			narrow[result2] = 0;

			s2 = IKString(narrow);

			if (!IKUtil::IsAppRunning(s2))
			{
				bItemForApp = false;
			}
		}

		if (bItemForApp) 
		{
			IKString strItem;
			strItem = "";
			//strItem += "   ";
			strItem += DATAS("attach_edit_overlays_for","Attach/Edit overlays for");
			strItem += " ";
			IKString friendly = AppLibGetAppFriendlyName(pSystray->m_strLastApp);
			IKString strName = friendly;
			// July 2012 - JR - the call to AppLibGetAppFriendlyName already calls StripFileName, if required,  
			//   so calling it here again just results in strange text being displayed
			//IKUtil::StripFileName(strName, true, true);
			strItem += "\"";
			strItem += strName;
			strItem += "\"";
			strItem += "...";
			items[nItems].type = 2; 
			items[nItems].sdata = pSystray->m_strLastApp;
			MyInsertMenu(&menu, -1, MF_BYPOSITION, 1000 + nItems, strItem );
			nItems++;
		}

		//  get a bunch of information about apps, overlays and systems

		int nSelected = AppLibGetSelectedOverlay(pSystray->m_strLastApp, group, student);// 1-based
		int nOverlays = AppLibCountAttachedOverlays(pSystray->m_strLastApp, group, student);
		owner = AppLibGetOwner(pSystray->m_strLastApp);
		int nSys = AppLibCountSystems();
		int nInterest = 0;
		for(int i=0; i < nSys; i++)
		{
			IKString sys = AppLibGetNthSystem(i);
			if (sys.CompareNoCase(TEXT("IntelliKeys USB"))!=0)
			{
				if (AppLibHasSystemInterest(pSystray->m_strLastApp,sys))
				{
					nInterest++;
				}
			}
		}

		//  optional separator
		if (bItemForApp && (nOverlays > 0))
		{
			MyInsertMenu(&menu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, "-" );
		}
		//  items for attached overlays
		unsigned int itemNone = 0;
		if (nOverlays > 0)
		{
			for (int i=0; i< nOverlays; i++)
			{
				TCHAR *pOverlay = AppLibGetNumberedOverlay(pSystray->m_strLastApp, group, student, i+1);  // 1-based
				IKString strPath(pOverlay);
				IKString strName = strPath;
				IKUtil::StripFileName(strName,true,true);

				IKString message;
				message = "";
				//message += "   ";
				message += DATAS(TEXT("use_attached_overlay"),TEXT("use attached overlay"));
				message += " ";
				message += "\"";
				message += strName;
				message += "\"";

				items[nItems].type = 4;  items[nItems].sdata = strPath;  items[nItems].idata = i+1;
				MyInsertMenu(&menu, -1, MF_BYPOSITION, 1000 + nItems, message);

				if ((nSelected == (i+1)) && (owner.CompareNoCase(TEXT("intellikeys usb")) == 0))
				{
					ASSERT(itemToCheck==0);
					itemToCheck = 1000 + nItems;
				}
				nItems++;
			}
		}

		if (nOverlays > 0)
		{
			//  item for "None"
			items[nItems].type = 3;
			itemNone = 1000 + nItems;
			IKString message;
			message = TEXT("");
			//message += TEXT("   ");

			//  use a different message if the app might have a clickit overlay attached
			bool bClickit = IKUtil::HasClickitAdaptation(pSystray->m_strLastApp);
			if (bClickit)
			{
				message += DATAS(TEXT("app_will_choose"),TEXT("app_will_choose"));
			}
			else
			{
				message += DATAS(TEXT("app_will_choose_clickit"),TEXT("app_will_choose_clickit"));
			}

			MyInsertMenu(&menu, -1, MF_BYPOSITION, itemNone, message);
			nItems++;
		}

		//  if nobody checked yet, check none.
		if (itemToCheck == 0)
		{
			if (itemNone != 0)
			{
				itemToCheck = itemNone;
			}
		}
		//  now really check an item
		if (itemToCheck != 0)
		{
			menu.CheckMenuItem(itemToCheck,MF_CHECKED);
		}
	}

	//  items for One Overlay mode
	if (currentSettings.m_iMode == kSettingsModeThisOverlay)
	{
		MyInsertMenu(&menu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, "-");
		//MyInsertMenu ( &menu, -1, MF_BYPOSITION|MF_DISABLED, 0, DATAS(TEXT("In One Overlay Only Mode:"),TEXT("In One Overlay Only Mode:")) );

		IKString overlayName = currentSettings.m_sUseThisOverlay;
		IKUtil::StripFileName(overlayName, true, false);

		message = "";
		//message += "   ";
		message += DATAS(TEXT("using_overlay"), TEXT("Using overlay"));
		message += " ";
		message += "\"";
		message += overlayName;
		message += "\"";

		MyInsertMenu(&menu, -1, MF_BYPOSITION | MF_DISABLED, 0, message);
	}

	//  items for switch preset mode
	if (currentSettings.m_iMode == kSettingsModeSwitch)
	{
		MyInsertMenu(&menu, -1, MF_BYPOSITION|MF_SEPARATOR, 0, "-");
		//MyInsertMenu ( &menu, -1, MF_BYPOSITION|MF_DISABLED, 0, DATAS(TEXT("In Switch Preset Mode:"),TEXT("In Switch Preset Mode:")) );

		message = "";
		//message += "   ";
		message += DATAS(TEXT("using_switch_setting"), TEXT("Using switch setting"));
		message += " ";
		message += "\"";
		message += GetSwitchSettingName(currentSettings.m_iUseThisSwitchSetting);
		message += "\"";

		MyInsertMenu(&menu, -1, MF_BYPOSITION|MF_DISABLED, 0, message);


		//  create a submenu for the switch settings
		CMenu theSub;
		theSub.CreatePopupMenu();

		//  number of settings
		int numSettings = DATAI(TEXT("num_switch_files"), 0);

		for (int i=0; i < numSettings; i++)
		{
			IKString s = TEXT("switch_file_"); 
			s += IKUtil::IntToString(i + 1);
			IKString setting = DATAS(s, TEXT("error"));
			IKUtil::StripFileName(setting, true, true);

			items[nItems].type = 13;  items[nItems].idata = i+1;
			MyInsertMenu(&theSub, -1, MF_BYPOSITION, 1000 + nItems, (LPCSTR)setting);

			if (currentSettings.m_iUseThisSwitchSetting == (i + 1))
			{
				theSub.CheckMenuItem(1000 + nItems, MF_CHECKED);
			}

			nItems++;
		}

		message = "";
		//message += "   ";
		message += DATAS(TEXT("choose_a_switch_setting"),TEXT("Choose a switch setting"));
		//menu.AppendMenu ( MF_POPUP, (UINT)theSub.m_hMenu, message );
		MyAppendMenu(&menu, &theSub, MF_POPUP, message);
		theSub.Detach();
	}

	//  items for Discover mode
	if (AppLibIsSystemPresent(TEXT("Discover")) || AppLibShowDiscoverAnyway())  //  but not if Discover is not installed.
	{
		if (currentSettings.m_iMode == kSettingsModeDiscover)
		{
			MyInsertMenu(&menu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, "-");
			//MyInsertMenu ( &menu, -1, MF_BYPOSITION|MF_DISABLED, 0, DATAS(TEXT("In Discover Mode:"),TEXT("In Discover Mode:")) );


			items[nItems].type = 12;  
			items[nItems].sdata = "";
			message = "";
			//message += "   ";
			message += DATAS(TEXT("allow_applications_to_send_overlays"), TEXT("allow applications to send overlays"));
			MyInsertMenu(&menu, -1, MF_BYPOSITION, 1000 + nItems, message);
			if (currentSettings.m_bButAllowOverlays)
			{
				menu.CheckMenuItem(1000 + nItems, MF_CHECKED);
			}
			nItems++;
		}
	}

	//  item for troubleshooting
	MyInsertMenu(&menu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, "-");
	items[nItems].type = 6;
	MyInsertMenu(&menu, -1, MF_BYPOSITION, 1000 + nItems, DATAS(TEXT("S_TROUBLESHOOTING"), TEXT("Troubleshooting...")) );
	nItems++;


	//--------------

	//  force foreground first
	//  this interferes with WordQ (I think).  Removed 7/25/2005.
	SetForegroundWindow(GetNotifyTargetWnd()->m_hWnd);

	//  track the result of the user using the menu
	CPoint currloc;
	GetCursorPos(&currloc);
	UINT flags = TPM_RIGHTALIGN | TPM_LEFTBUTTON | TPM_LEFTBUTTON | TPM_RETURNCMD ;
	int result = menu.TrackPopupMenu(flags, currloc.x, currloc.y, GetNotifyTargetWnd(), 0);

	// See MS KB article Q135788
	::PostMessage(GetNotifyTargetWnd()->m_hWnd, WM_NULL, 0, 0);

	//  destroy the menu
	menu.DestroyMenu();

	//  handle the result
	bool bReady = false;

	menuItem *pItem=NULL;
	if (result >= 1000)
	{
		pItem = &(items[result - 1000]);
	}

	if (pItem)
	{
		switch (pItem->type)
		{
		case 1:
			m_lastWindow = false;
			bReady = Launch();
			//  Tell control panel to show itself
			if (bReady)
			{
				IKMessage::Send(TEXT("control panel"), kQueryForwardControlPanel, 0, 0, 0, 0 );
			}
			break;
		case 2:
			m_lastWindow = false;
			if (!AppLibIsAppAllowed(pSystray->m_strLastApp))
			{
				IKString title = DATAS(TEXT("intellikeys_usb"), TEXT("intellikeys_usb"));
				IKString message = DATAS(TEXT("app_not_allowed"), TEXT("app_not_allowed"));
				IKUtil::DisplayAlert(title,message);
			}
			else
			{
				AppLibAddApplication(pSystray->m_strLastApp);
				AppLibAddApplicationForStudent(pSystray->m_strLastApp, group, student);

				IKMessage::Send(TEXT("engine"), kQueryReloadStudent, 0, 0, 0, 0);

				//  launch CP and wait for it
				bReady = Launch();

				//  Tell control panel to switch to the apps tab
				//  and tell it which app to select

				if (bReady)
				{
					unsigned char data[2000];
					int ndata = 0;
					data[ndata] = strlen((char*)pSystray->m_strLastApp);
					ndata++;
					strcpy((char*)&(data[ndata]), (char*)pSystray->m_strLastApp);
					ndata = ndata + strlen((char*)pSystray->m_strLastApp);

					IKMessage::Send(TEXT("control panel"), kQueryApplication, data, ndata, 0, 0 );
				}
			}
			break;
		case 3:
			lastApp = TEXT("");
			AppLibReleaseOwnership(pSystray->m_strLastApp,owner);
			//AppLibSetSelectedOverlay(pSystray->m_strLastApp, group, student, 0);
			currentSettings.m_iMode = kSettingsModeLastSentOverlay;
			SaveCurrentSettings();
			result = IKMessage::Send(TEXT("engine"),kQueryReloadStudent, 0, 0, 0, 0);
			break;
		case 4:
			lastApp = TEXT("");
			AppLibTakeOwnership(pSystray->m_strLastApp, TEXT("IntelliKeys USB"));
			AppLibSetSelectedOverlay(pSystray->m_strLastApp, group, student, pItem->idata);
			currentSettings.m_iMode = kSettingsModeLastSentOverlay;
			SaveCurrentSettings();
			result= IKMessage::Send(TEXT("engine"), kQueryReloadStudent, 0, 0, 0, 0);
			break;
		case 5:
			lastApp = TEXT("");
			AppLibSetSelectedOverlay(pSystray->m_strLastApp, group, student, 0);
			AppLibTakeOwnership(pSystray->m_strLastApp,pItem->sdata);
			currentSettings.m_iMode = kSettingsModeLastSentOverlay;
			SaveCurrentSettings();
			result= IKMessage::Send(TEXT("engine"), kQueryReloadStudent, 0, 0, 0, 0);
			break;
		case 6:
			m_lastWindow = false;
			//  launch CP and wait for it
			bReady = Launch();
			//  Tell control panel to switch to the troubleshooting tab
			if (bReady)
				IKMessage::Send(TEXT("control panel"), kQueryTroubleshooting, 0, 0, 0, 0 );
			break;
		case 7:
			//  do nothing
			break;
		case 8:
			//  Overlay Mode
			currentSettings.m_iMode = 0;
			SaveCurrentSettings();
			result = IKMessage::Send(TEXT("engine"), kQueryReloadStudent, 0, 0, 0, 0);
			m_bForceSend = true;
			break;
		case 9:
			//  Switch Mode
			currentSettings.m_iMode = 2;
			if (currentSettings.m_iUseThisSwitchSetting == 0)
			{
				currentSettings.m_iUseThisSwitchSetting = 1;
			}
			SaveCurrentSettings();
			result = IKMessage::Send(TEXT("engine"), kQueryReloadStudent, 0, 0, 0, 0);
			m_bForceSend = true;
			break;
		case 10:
			//  Discover mode
			currentSettings.m_iMode = 3;
			SaveCurrentSettings();
			result = IKMessage::Send(TEXT("engine"), kQueryReloadStudent, 0, 0, 0, 0);
			m_bForceSend = true;
			break;
		case 11:
			//  one overlay mode mode
			if (currentSettings.m_sUseThisOverlay.IsEmpty())
			{
				//  tell them they must specify an overlay
				IKString title = DATAS(TEXT("intellikeys_usb"), TEXT("intellikeys_usb"));
				IKString message = DATAS(TEXT("must_specify_overlay"), TEXT("must_specify_overlay"));
				IKUtil::DisplayAlert(title, message);
				//  ask for the file
				IKString filename;
				IKString strFilter("");
				strFilter += DATAS(TEXT("overlay_files"), TEXT("Overlay Files"));
				strFilter += "|";
				strFilter += "*.oms;*.omt;*.omc;*.omg";

				if (IKFile::GetOneFile(DATAS(TEXT("choose_overlay"), TEXT("choose_overlay")), filename, strFilter))
				{
					if (IntelliKeys::IsSendableOverlayFile(filename))
					{
						currentSettings.m_sUseThisOverlay = filename;
						currentSettings.m_iMode = 1;
						SaveCurrentSettings();
						result = IKMessage::Send(TEXT("engine"), kQueryReloadStudent, 0, 0, 0, 0);
						m_bForceSend = true;
					}
					else
					{
						title = DATAS(TEXT("intellikeys_usb"), TEXT("intellikeys_usb"));
						message = DATAS(TEXT("not_valid_overlay"), TEXT("not_valid_overlay"));
						IKUtil::DisplayAlert(title, message);
					}
				}
				else
				{
					title = DATAS(TEXT("intellikeys_usb"), TEXT("intellikeys_usb"));
					message = DATAS(TEXT("no_overlay_chosen"), TEXT("no_overlay_chosen"));
					IKUtil::DisplayAlert(title, message);
				}
			}
			else
			{
				currentSettings.m_iMode = 1;
				SaveCurrentSettings();
				result = IKMessage::Send(TEXT("engine"), kQueryReloadStudent, 0, 0, 0, 0);
				m_bForceSend = true;
			}
			break;
		case 12:
			//  one overlay mode mode
			currentSettings.m_bButAllowOverlays = !currentSettings.m_bButAllowOverlays;
			SaveCurrentSettings();
			result = IKMessage::Send(TEXT("engine"), kQueryReloadStudent, 0, 0, 0, 0);
			m_bForceSend = true;
			break;
		case 13:
			//  one overlay mode mode
			currentSettings.m_iUseThisSwitchSetting = pItem->idata;
			SaveCurrentSettings();
			result = IKMessage::Send(TEXT("engine"), kQueryReloadStudent, 0, 0, 0, 0);
			m_bForceSend = true;
			break;
		default:
			break;
		}
	}

	if (m_lastWindow)
	{
		::SetForegroundWindow (m_lastWindow);
	}

	bInMenu = false;
	lastMouseMove = 0;
}

void MySystemTrayIcon::OnMouseMove()
{
	lastMouseMove = IKUtil::GetCurrentTimeMS();
}

//------------------------------
//
//  shutdown stuff.
//

static bool bShutdown = false;  //  gets set to true if we're about to shut down

//  first we are asked if it's ok to end the session.
//  we always say yes.

BOOL CMainFrame::OnQueryEndSession() 
{
	if (!CFrameWnd::OnQueryEndSession())
		return FALSE;
	
	return TRUE;
}

//  the window proc is the only place we can 
//  tell logoff from shutdown.  just remember it for now.

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	//  remember if we're logging off or shutting down.

	if (message == WM_QUERYENDSESSION)
	{
		bShutdown = !(lParam&ENDSESSION_LOGOFF);
	}
	
	return CFrameWnd::WindowProc(message, wParam, lParam);
}

//  finally, we actually get told to end the session.  
//  If it's shutdown, we tell the engine to stop.

void CMainFrame::OnEndSession(BOOL bEnding) 
{
	if (bShutdown)
	{
		IKMessage::Send ( TEXT("engine"), kQueryShutdown, 0, 0, 0, 0 );
		::Sleep(2000);
	}

	if (pSystray)
		delete pSystray;
	
	CFrameWnd::OnEndSession(bEnding);

}

MySystemTrayIcon :: MySystemTrayIcon(bool bUnicode)
: SystemTrayIcon(bUnicode)
{
	m_strLastApp = "";
	m_lastWindow = NULL;
	m_bHandlingClick = false;

}
