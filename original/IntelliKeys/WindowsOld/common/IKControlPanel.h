// IKControlPanel.h: interface for the IKControlPanel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKCONTROLPANEL_H__2D357E72_D97C_458D_8A80_46AB19DCB637__INCLUDED_)
#define AFX_IKCONTROLPANEL_H__2D357E72_D97C_458D_8A80_46AB19DCB637__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IKString.h"
#include "IKSettings.h"

class IKControlPanel  
{
public:
	static void Launch();
	static IKString ShiftKeyActionAsText ( int i );
	static IKString LightsAsText ( int i );
	static IKString VolToText ( int volume );
	static void Show();
	static void Help();
	static void Toggle();
	static void Refresh();
	static void ListFeatures();
	static IKString BoolToText ( bool b );
};

#endif // !defined(AFX_IKCONTROLPANEL_H__2D357E72_D97C_458D_8A80_46AB19DCB637__INCLUDED_)
