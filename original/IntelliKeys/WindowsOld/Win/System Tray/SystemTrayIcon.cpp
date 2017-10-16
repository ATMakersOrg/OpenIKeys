/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		SystemTrayIcon.cpp
//
// Purpose:		implementation for the SystemTrayIcon class.
//
// 06/18/01 fwr initial implementation adapted from VC++ Developer sample
//
**************************************************************************************************************************/

#include "stdafx.h"

#include "SystemTrayIcon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString SystemTrayIcon::m_class;

SystemTrayIcon::SystemTrayIcon(bool bUnicode)
	: m_ActivateWnd(NULL),
	m_NotifyTargetWnd(NULL),
	m_Menu(NULL),
	m_Enabled(0),
	m_bUnicode(bUnicode)
{
	if (m_bUnicode)
	{
		memset(&m_Nidw,0,sizeof(NOTIFYICONDATAW));
		m_Nidw.cbSize = sizeof(NOTIFYICONDATAW);
		m_Nidw.uID = (UINT)this;				// not really used
		m_Nidw.uCallbackMessage = WM_SYSTEM_TRAY_NOTIFY;
	}
	else
	{
		memset(&m_Nida,0,sizeof(NOTIFYICONDATA));
		m_Nida.cbSize = sizeof(NOTIFYICONDATA);
		m_Nida.uID = (UINT)this;				// not really used
		m_Nida.uCallbackMessage = WM_SYSTEM_TRAY_NOTIFY;
	}
}

SystemTrayIcon::~SystemTrayIcon() {
	/*
		remove icon if displayed
		kill external window if exists
	*/
	Enable(FALSE);
	if (m_NotifyTargetWnd) m_NotifyTargetWnd->SendMessage(WM_CLOSE);
}

void SystemTrayIcon::SetIcon(UINT iconid) {
	HICON hic = AfxGetApp()->LoadIcon(iconid);
	internalSetIcon(hic);
}

void SystemTrayIcon::SetIcon(LPCTSTR iconresourcename) {
	HICON hic = AfxGetApp()->LoadIcon(iconresourcename);
	internalSetIcon(hic);
}

void SystemTrayIcon::SetStandardIcon(LPCTSTR iconname) {
	HICON hic = AfxGetApp()->LoadStandardIcon(iconname);
	internalSetIcon(hic);
}

void SystemTrayIcon::SetToolTip(LPCTSTR tip) 
{
	int tip_was_NULL;
	if (m_bUnicode)
		tip_was_NULL = m_Nidw.szTip[0] ? 0 : 1;
	else
		tip_was_NULL = m_Nida.szTip[0] ? 0 : 1;

	int new_tip_is_NULL = (tip && tip[0]) ? 0 : 1;

	// don't bother replacing one blank with another
	if (! (tip_was_NULL && new_tip_is_NULL) ) 
	{ // OK there's data

#if 1
		if (m_bUnicode)
		{
			//  convert text to Unicode
			int len = strlen(tip);
			int result = MultiByteToWideChar (
								CP_UTF8,				// utf-8
								0,						// character-type options
								(const char *) tip, // string to map
								len,				// number of bytes in string
								m_Nidw.szTip,				// wide-character buffer
								64		// size of buffer
								);

			m_Nidw.szTip[result] = 0;
			m_Nidw.szTip[63] = 0;
		}
		else
		{
			strcpy(m_Nida.szTip,tip);
		}

#else
		// first replace the text
		int bufsiz = sizeof(m_Nid.szTip) / sizeof(m_Nid.szTip[0]);
		int len = lstrlen(tip);
		if (len>(bufsiz-1)) 
			len = bufsiz-1; // stay in bounds
		memcpy(m_Nid.szTip,tip,len);	// copy the text
		m_Nid.szTip[len] = 0;			// NULL terminate
#endif
		// now change the current text if we're active
		if (m_Enabled) 
		{
			if (m_bUnicode)
			{
				m_Nidw.uFlags = NIF_TIP;
				VERIFY(Shell_NotifyIconW(NIM_MODIFY,&m_Nidw));
			}
			else
			{
				m_Nida.uFlags = NIF_TIP;
				VERIFY(Shell_NotifyIcon(NIM_MODIFY,&m_Nida));
			}

		}
	}
}

int SystemTrayIcon::Enable(BOOL bOn) {
	int retval = IsEnabled();
	DWORD msg;
	BOOL success;
	if ( !!bOn == !!m_Enabled ) return retval; // already in desired state
	if (bOn) {
		if (!m_NotifyTargetWnd) {

			// register a class name if necessary
			if (m_class.IsEmpty()) {
				UINT style = CS_NOCLOSE;
				m_class = AfxRegisterWndClass(style);
				if (m_class.IsEmpty()) return retval;	// failed to register
			}

			m_NotifyTargetWnd = new CNotifyTargetWnd(this);
			ASSERT(m_NotifyTargetWnd);
			if (!m_NotifyTargetWnd) return retval;		// failed to allocate

			success=m_NotifyTargetWnd->CreateEx(
				WS_EX_NOPARENTNOTIFY,	// ext style
				m_class,				// class name
				NULL,					// window name
				WS_OVERLAPPEDWINDOW,	// style
				0,0,					// x and y
				50,50,					// width and height totally arbitrary
				NULL,					// parent not wanted
				NULL,					// no menu
				NULL);					// no params

			ASSERT(success);
			if (!success) {
				delete m_NotifyTargetWnd;
				m_NotifyTargetWnd = NULL;
			}

			if (m_bUnicode)
				m_Nidw.hWnd = m_NotifyTargetWnd->m_hWnd;
			else
				m_Nida.hWnd = m_NotifyTargetWnd->m_hWnd;
		}
		ASSERT(m_NotifyTargetWnd);

		if (m_bUnicode)
		{
			ASSERT(m_Nidw.hWnd);
			ASSERT(m_Nidw.hIcon);
			ASSERT(m_Nidw.uCallbackMessage);
			if (!m_NotifyTargetWnd || !m_Nidw.hIcon) return retval; // can't proceed

			msg = NIM_ADD;					// adding icon
			m_Nidw.uFlags = NIF_ICON | NIF_MESSAGE;
			if ( m_Nidw.szTip[0] ) 
				m_Nidw.uFlags |= NIF_TIP; // tip?
		}
		else
		{
			ASSERT(m_Nida.hWnd);
			ASSERT(m_Nida.hIcon);
			ASSERT(m_Nida.uCallbackMessage);
			if (!m_NotifyTargetWnd || !m_Nida.hIcon) return retval; // can't proceed

			msg = NIM_ADD;					// adding icon
			m_Nida.uFlags = NIF_ICON | NIF_MESSAGE;
			if ( m_Nida.szTip[0] ) 
				m_Nida.uFlags |= NIF_TIP; // tip?
		}


	}
	else 
	{
		msg = NIM_DELETE;				// deleting icon
		if (m_bUnicode)
			m_Nidw.uFlags = 0;
		else
			m_Nida.uFlags = 0;

	}

	if (m_bUnicode)
		success = Shell_NotifyIconW(msg,&m_Nidw);
	else
		success = Shell_NotifyIcon(msg,&m_Nida);

	ASSERT(success);
	if (success) m_Enabled = bOn ? 1 : 0;
	return retval;
}

void SystemTrayIcon::OnRButtonUp() {
	CMenu menu;
	BOOL success;

	ASSERT(m_ActivateWnd);				// check window that will own menu
	if (!m_ActivateWnd) return;

	ASSERT(m_Menu);						// check menu identifier
	if (!m_Menu) return;

	success = menu.LoadMenu(m_Menu);	// load the menu
	ASSERT(success);					// check success of menu load
	if (!success) return;

	CMenu* submenu = menu.GetSubMenu(0); // extract submenu
	ASSERT(submenu);					// check success of submenu
	if (!submenu) return;

	// here allow derived classes to set a default menu item
	ModifyMenu(submenu);

	/*
		There's a bug that requires a workaround here...
		Thanks to Paul DiLascia for pointing it out!
		Check MSDN for KnowledgeBase article Q135788 for more details.
	*/
	CPoint currloc;
	GetCursorPos(&currloc);
	m_ActivateWnd->SetForegroundWindow();
	submenu->TrackPopupMenu(0,currloc.x,currloc.y,m_ActivateWnd);
	m_ActivateWnd->PostMessage(WM_GETTEXTLENGTH,0,0);
}

void SystemTrayIcon::OnLButtonDblClick() {
	ASSERT(m_ActivateWnd);
	if (m_ActivateWnd) {

		CMenu menu;
		BOOL success;

		ASSERT(m_ActivateWnd);			// check window that will own menu
		if (!m_ActivateWnd) return;

		ASSERT(m_Menu);					// check menu identifier
		if (!m_Menu) return;

		success = menu.LoadMenu(m_Menu);// load the menu
		ASSERT(success);				// check success of menu load
		if (!success) return;

		CMenu* submenu = menu.GetSubMenu(0); // extract submenu
		ASSERT(submenu);				// check success of submenu
		if (!submenu) return;

		// here allow derived classes to set a default menu item
		ModifyMenu(submenu);

		// now the default is set, suck it out
		UINT item = ::GetMenuDefaultItem(submenu->m_hMenu,FALSE,0);
		if ( item == -1) {				// if no default, activate window
			m_ActivateWnd->ShowWindow(SW_NORMAL);
			m_ActivateWnd->SetForegroundWindow();
		}
		else {
			m_ActivateWnd->SendMessage(WM_COMMAND,item,0);
		}
	}
}

void SystemTrayIcon::ModifyMenu(CMenu* menu) {
	ASSERT(menu);
	if (!menu) return;
	ASSERT(menu->m_hMenu);
	if (!menu->m_hMenu) return;

	BOOL result = ::SetMenuDefaultItem(menu->m_hMenu,0,TRUE);
	ASSERT(result);
}

void SystemTrayIcon::internalSetIcon(HICON hicon) 
{
	if (!hicon || !m_Enabled) 
	{
		Enable(FALSE);					// disabled or disabling
		if (m_bUnicode)
			m_Nidw.hIcon = hicon;
		else
			m_Nida.hIcon = hicon;
	}
	else 
	{								// enabled and valid new icon

		if (m_bUnicode)
		{
			m_Nidw.uFlags = NIF_ICON;
			m_Nidw.hIcon = hicon;
			VERIFY(Shell_NotifyIconW(NIM_MODIFY,&m_Nidw));
		}
		else
		{
			m_Nida.uFlags = NIF_ICON;
			m_Nida.hIcon = hicon;
			VERIFY(Shell_NotifyIcon(NIM_MODIFY,&m_Nida));
		}

	}
}

LRESULT SystemTrayIcon::OnSystemTrayNotify(WPARAM id,LPARAM event) 
{
	ASSERT(id==(UINT)this);				// data unexpectedly corrupted
	LRESULT handled = 1;				// assume handled
	switch (event) {
	case WM_MOUSEMOVE:		OnMouseMove();			break;

	case WM_LBUTTONDBLCLK:	OnLButtonDblClick();	break;
	case WM_LBUTTONDOWN:	OnLButtonDown();		break;
	case WM_LBUTTONUP:		OnLButtonUp();			break;

	case WM_MBUTTONDBLCLK:	OnMButtonDblClick();	break;
	case WM_MBUTTONDOWN:	OnMButtonDown();		break;
	case WM_MBUTTONUP:		OnMButtonUp();			break;

	case WM_RBUTTONDBLCLK:	OnRButtonDblClick();	break;
	case WM_RBUTTONDOWN:	OnRButtonDown();		break;
	case WM_RBUTTONUP:		OnRButtonUp();			break;

	default:				ASSERT(0); handled = 0;	break;
	}

	return handled;
}

//	CWnd SetForegroundWindow TrackPopupWindow Shell_NotifyIcon
//		friend CMenu


/////////////////////////////////////////////////////////////////////////////
// CNotifyTargetWnd

IMPLEMENT_DYNAMIC(CNotifyTargetWnd, CWnd)

CNotifyTargetWnd::CNotifyTargetWnd()
{
}

CNotifyTargetWnd::~CNotifyTargetWnd()
{
}


BEGIN_MESSAGE_MAP(CNotifyTargetWnd, CWnd)
	ON_MESSAGE(WM_SYSTEM_TRAY_NOTIFY,OnSystemTrayNotify)
	//{{AFX_MSG_MAP(CNotifyTargetWnd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNotifyTargetWnd message handlers
LRESULT CNotifyTargetWnd::OnSystemTrayNotify(WPARAM id,LPARAM event) {
	ASSERT(m_Sti);
	if (m_Sti) return m_Sti->OnSystemTrayNotify(id,event);
	else return 0;						// didn't handle
}

void CNotifyTargetWnd::PostNcDestroy() 
{
	CWnd::PostNcDestroy();

	delete this;
}
