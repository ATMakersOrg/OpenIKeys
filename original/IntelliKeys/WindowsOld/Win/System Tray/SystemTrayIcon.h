/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		SystemTrayIcon.h
//
// Purpose:		interface for the SystemTrayIcon class.
//
// 06/18/01 fwr initial implementation adapted from VC++ Developer sample
//
**************************************************************************************************************************/

#ifndef SYSTEM_TRAY_ICON_H_INCLUDED
#define SYSTEM_TRAY_ICON_H_INCLUDED

/////////////////////////////////////////////////////////////////////////////
// CNotifyTargetWnd window

#define WM_SYSTEM_TRAY_NOTIFY (WM_USER + 1234)

class SystemTrayIcon;					// forward ref for CNotifyTargetWnd

class CNotifyTargetWnd : public CWnd
{
	DECLARE_DYNAMIC(CNotifyTargetWnd)
// Construction
private:
	CNotifyTargetWnd();					// can't make one w/o parent
public:
	CNotifyTargetWnd(SystemTrayIcon* sti) { m_Sti = sti; ASSERT(sti); }

// Attributes
public:
private:
	SystemTrayIcon* m_Sti;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNotifyTargetWnd)
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNotifyTargetWnd();
	LRESULT OnSystemTrayNotify(WPARAM id,LPARAM event);

	// Generated message map functions
protected:
	//{{AFX_MSG(CNotifyTargetWnd)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class SystemTrayIcon {
	friend LRESULT CNotifyTargetWnd::OnSystemTrayNotify(WPARAM,LPARAM);
private:

	// this window gets activated on icon dblclick
	// this window also owns the popup menu
	CWnd* m_ActivateWnd;

	// this window exists to receive sys tray icon notifications
	CNotifyTargetWnd* m_NotifyTargetWnd;

	UINT m_Menu;
	NOTIFYICONDATAW m_Nidw;
	NOTIFYICONDATA m_Nida;
	int m_Enabled;
	bool m_bUnicode;

protected:
	CWnd* GetActivateWnd() const { return m_ActivateWnd; }
	CNotifyTargetWnd* GetNotifyTargetWnd() const { return m_NotifyTargetWnd; }

public:
	// construction and destruction
	SystemTrayIcon(bool bUnicode);
	virtual ~SystemTrayIcon();

	// setup
	void SetWindow(CWnd* wnd) { m_ActivateWnd = wnd; }
	void SetIcon(UINT iconid);
	void SetIcon(LPCTSTR iconresourcename);
	void SetStandardIcon(LPCTSTR iconname);
	void SetToolTip(LPCTSTR tip);
	void SetMenuID(UINT menuid) { m_Menu = menuid; }

	// query
	//LPCSTR GetToolTip() const { return m_Nid.szTip; }

	// on and off
	int Enable(BOOL bOn=TRUE); // TRUE enable FALSE disable returns old value
	int IsEnabled() const { return m_Enabled; } // returns 0 for not enabled

protected:
	virtual void OnRButtonDblClick() {}
	virtual void OnRButtonDown() {}
	virtual void OnRButtonUp();
	virtual void OnMButtonDblClick() {}
	virtual void OnMButtonDown() {}
	virtual void OnMButtonUp() {}
	virtual void OnLButtonDblClick();
	virtual void OnLButtonDown() {}
	virtual void OnLButtonUp() {}
	virtual void OnMouseMove() {}
	virtual void ModifyMenu(CMenu* menu);
private:
	LRESULT OnSystemTrayNotify(WPARAM id,LPARAM event);
	void internalSetIcon(HICON hicon);
	static CString m_class; // classname for CNotifyTargetWnd
};

/////////////////////////////////////////////////////////////////////////////
#endif //SYSTEM_TRAY_ICON_H_INCLUDED
