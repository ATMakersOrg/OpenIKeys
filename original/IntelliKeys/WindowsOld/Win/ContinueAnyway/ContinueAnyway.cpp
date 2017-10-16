// ContinueAnyway.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
// #include <winable.h>

#include "IKUtil.h"


static void TypeChar ( char c )
{
	UINT result;
	INPUT in;
	in.type = INPUT_KEYBOARD;
	in.ki.time = 0;
	in.ki.dwExtraInfo = 0;
	in.ki.wScan = 0;

	in.ki.wVk = c;

	in.ki.dwFlags = 0;
	result = ::SendInput( 1, &in, sizeof(INPUT));
	Sleep(10);

	in.ki.dwFlags = KEYEVENTF_KEYUP;
	result = ::SendInput( 1, &in, sizeof(INPUT));
	Sleep(250);

}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	IKUtil::Initialize();

	DWORD then = ::GetTickCount();
	while (true)
	{
		HWND hwnd = ::GetForegroundWindow();
		if (hwnd != NULL)
		{
			char wname[1024];
			GetWindowText ( hwnd, wname, 1024 );
			IKString s = wname;

			if (s.CompareNoCase("Hardware Installation")==0 ||
			    s.CompareNoCase("Software Installation")==0)
			{
				//  coupla left-arrows
				TypeChar ( VK_LEFT );
				TypeChar ( VK_LEFT );

				//  return
				TypeChar ( VK_RETURN );

				break;
			}

			if (IKUtil::IsWinVistaOrGreater())
			{
				if (s.CompareNoCase("Windows Security")==0)
				{
					TypeChar ( VK_TAB );
					TypeChar ( VK_TAB );
					TypeChar ( VK_TAB );
					TypeChar ( VK_RETURN );
					break;
				}
			}
		}
		Sleep(250);

		DWORD now = ::GetTickCount();
		if (now>then+60000)
			break;
	}

	return 0;
}

