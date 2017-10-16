/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		Message Client DLL.cpp
//
// Purpose:		global keyboard hook used for toggle state of Numlock, CapsLock, etc.
//
// 06/18/01 fwr initial implementation	
//
**************************************************************************************************************************/

#include <windows.h>

#include "messages.h"
#include "messageclient.h"

#define DLLHEADER
#include "message client dll.h"

#include "IKCommon.h"
#include "IKUtil.h"

static void _DllLog(TCHAR * lpszFormat, ...)
{
	IKUtil::Initialize();

	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vsnprintf(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), lpszFormat, args);

	szBuffer[nBuf] = TEXT('\n');
	nBuf++;
	szBuffer[nBuf] = 0;
	nBuf++;

	//IKUtil::LogString(szBuffer);
	OutputDebugString(szBuffer);

	va_end(args);
}

#if 1
#define DLLTRACE(_x_) \
if (DATAI(TEXT("log_dlls"),0)!=0) \
{ \
	_DllLog _x_; \
}
#else
#define DLLTRACE(_x_)
#endif



HINSTANCE hInstance = NULL;

BOOL WINAPI DllMain
	( HINSTANCE hinstDLL, DWORD fdwReason,LPVOID lpvReserved)
{

	hInstance = (HINSTANCE) hinstDLL;

	switch (fdwReason) 
	{

		case DLL_PROCESS_ATTACH:
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			break;

		default:
			break;
	}

	return TRUE;

	UNREFERENCED_PARAMETER(lpvReserved);
}




extern "C" __export int DllIKSendOverlay ( TCHAR *overlayname )
{
	int result = IKSendOverlay ( overlayname );

	DLLTRACE (("DllIKSendOverlay(%s) returns %d",overlayname,result));

	return result;
}

extern "C" __export int DllIKSendOverlayName ( TCHAR *name )
{
	int result = IKSendOverlayName ( name );

	DLLTRACE (("DllIKSendOverlayName(%s) returns %d",name,result));

	return result;
}

extern "C" __export int DllIKSendOverlayWithName ( TCHAR *overlayname, TCHAR *name, TCHAR *sender )
{
	int result = IKSendOverlayWithName ( overlayname, name, sender );

	DLLTRACE (("DllIKSendOverlayWithName(%s,%s,%s) returns %d",overlayname, name, sender,result));

	return result;
}

extern "C" __export int DllIKShowControlPanel ()
{
	int result = 0;

	DLLTRACE (("DllIKShowControlPanel() returns %d",result));

	return result;
}

extern "C" __export int DllIKStopServer () 
{
	int result = IKShowControlPanel ();

	DLLTRACE (("DllIKStopServer() returns %d",result));

	return result;
}

extern "C" __export int DllIKStopSystray () 
{
	int result = IKStopSystray ();

	DLLTRACE (("DllIKStopSystray() returns %d",result));

	return result;
}

extern "C" __export int DllIKIsIntellikeysConnected () 
{
	int result = IKIsIntellikeysConnected () ;

	DLLTRACE (("DllIKIsIntellikeysConnected() returns %d",result));

	return result;
}

extern "C" __export int DllIKSendSettings ( BYTE *set ) 
{
	int result = IKSendSettings ( set );

	DLLTRACE (("DllIKSendSettings(%d) returns %d",set,result));

	return result;
}

extern "C" __export int DllIKGetSettings ( BYTE *set ) 
{
	int result = IKGetSettings ( set );

	DLLTRACE (("DllIKGetSettings(%d) returns %d",set,result));

	return result;
}

extern "C" __export int DllIKIsStandardOverlayInPlace () 
{
	int result = IKIsStandardOverlayInPlace ()  ;

	DLLTRACE (("DllIKIsStandardOverlayInPlace() returns %d",result));

	return result;
}

extern "C" __export int DllIKIsIntellikeysOn () 
{
	int result = IKIsIntellikeysOn () ;

	DLLTRACE (("DllIKIsIntellikeysOn() returns %d",result));

	return result;
}

extern "C" __export int DllIKSetStudent ( TCHAR *group, TCHAR *student ) 
{
	int result = IKSetStudent ( group, student ) ;

	DLLTRACE (("DllIKSetStudent(%s,%s) returns %d",group, student,result));

	return result;
}

extern "C" __export bool DllIKIsEngineAlive () 
{
	bool result = IKIsEngineAlive () ;

	DLLTRACE (("DllIKIsEngineAlive() returns %d",result));

	return result;
}

extern "C" __export bool DllIKIsControlPanelAlive () 
{
	bool result = IKIsControlPanelAlive () ;

	DLLTRACE (("DllIKIsControlPanelAlive() returns %d",result));

	return result;
}

extern "C" __export TCHAR * DllIKGetMessageName ( int message ) 
{
	TCHAR * result = IKGetMessageName ( message ) ;

	DLLTRACE (("DllIKGetMessageName(%s) returns %s",message,result));

	return result;
}

extern "C" __export int DllIKResetKeyboard () 
{
	int result = IKResetKeyboard () ;

	DLLTRACE (("DllIKResetKeyboard() returns %d",result));

	return result;
}

extern "C" __export int DllIKBeep () 
{
	int result = IKBeep ();

	DLLTRACE (("DllIKBeep() returns %d",result));

	return result;
}

extern "C" __export int DllIKShortBeep () 
{
	int result = IKShortBeep ();

	DLLTRACE (("DllIKShortBeep() returns %d",result));

	return result;
}

extern "C" __export int DllIKStartDiagnosticMode () 
{
	int result = IKStartDiagnosticMode () ;

	DLLTRACE (("DllIKStartDiagnosticMode() returns %d",result));

	return result;
}

extern "C" __export int DllIKStopDiagnosticMode () 
{
	int result = IKStopDiagnosticMode ();

	DLLTRACE (("DllIKStopDiagnosticMode() returns %d",result));

	return result;
}

extern "C" __export int DllIKGetDeviceArray ( BYTE *array ) 
{
	int result = IKGetDeviceArray ( array );

	DLLTRACE (("DllIKGetDeviceArray(%d) returns %d",array,result));

	return result;
}

extern "C" __export int DllIKGetSoftwareVersion ( TCHAR *version ) 
{
	int result = IKGetSoftwareVersion ( version ) ;

	DLLTRACE (("DllIKGetSoftwareVersion(%s) returns %d",version,result));

	return result;
}

extern "C" __export int DllIKGetFirmwareVersion ( TCHAR *version ) 
{
	int result = IKGetFirmwareVersion ( version );

	DLLTRACE (("DllIKGetFirmwareVersion(%s) returns %d",version,result));

	return result;
}

extern "C" __export int DllIKGetLastSentOverlay ( TCHAR *overlay ) 
{
	int result = IKGetLastSentOverlay ( overlay );

	DLLTRACE (("DllIKGetLastSentOverlay(%s) returns %d",overlay,result));

	return result;
}

extern "C" __export int DllIKReceive ( TCHAR *channel, int *command, void *data, int maxdata, int *datalen ) 
{
	int result = IKReceive ( channel, command, data, maxdata, datalen ) ;

	DLLTRACE (("DllIKReceive(%d,%d,%d,%d,%d) returns %d",channel, command, data, maxdata, datalen,result));

	return result;
}

extern "C" __export int DllIKRespond ( TCHAR *channel, int command, void *data, int datalen ) 
{
	int result = IKRespond ( channel, command, data, datalen );

	DLLTRACE (("DllIKSendOverlay(%s,%d,%d,%d) returns %d",channel, command, data, datalen,result));

	return result;
}

extern "C" __export void DllIKMessageInitialize () 
{
	IKMessageInitialize();

	DLLTRACE (("DllIKMessageInitialize()"));

}

extern "C" __export bool DllIKDebugSender() 
{
	bool result = IKDebugSender() ;

	DLLTRACE (("DllIKDebugSender() returns %d",result));

	return result;
}

extern "C" __export void DllIKMakeWriteable ( TCHAR *filename ) 
{
	IKMakeWriteable ( filename ) ;

	DLLTRACE (("DllIKMakeWriteable(%s)",filename));
}

extern "C" __export int DllIKStartRawMode () 
{
	int result;

	//  IK USB 3.0.3
	//  bkwd-compatibility for Discover 2.0.2 and Envoy 0.9
	result = kResponseNoError;

	//result = IKStartRawMode () ;

	DLLTRACE (("DllIKStartRawMode() returns %d",result));

	return result;
}

extern "C" __export int DllIKStopRawMode () 
{
	int result;

	//  IK USB 3.0.3
	//  bkwd-compatibility for Discover 2.0.2 and Envoy 0.9
	result = kResponseNoError;

	//result = IKStopRawMode ();


	DLLTRACE (("DllIKStopRawMode() returns %d",result));

	return result;
}

extern "C" __export int DllIKGetRawEvent ( RawEvent *event )
{
	int result = IKGetRawEvent ( event ) ;

	DLLTRACE (("DllIKGetRawEvent(%d) returns %d",event,result));

	return result;
}

extern "C" __export int DllIKGetRawEvents ( RawEvent *events, int nEventsIn, int *nEventsOut )
{
	int result = IKGetRawEvents ( events, nEventsIn, nEventsOut) ;

	DLLTRACE (("DllIKSendOverlay(%d,%d,%d) returns %d",events, nEventsIn, *nEventsOut,result));

	return result;
}

extern "C" __export bool DllIKIsRawModeOn ( ) 
{
	bool result = IKIsRawModeOn ( ) ;

	DLLTRACE (("DllIKIsRawModeOn() returns %d",result));

	return result;
}

extern "C" __export bool DllIKIsRawModeUsable ( ) 
{
	bool result;

	//  IK USB 3.0.3
	//  bkwd-compatibility for Discover 2.0.2 and Envoy 0.9
	//result = true;

	result = IKIsRawModeUsable ( ) ;

	DLLTRACE (("DllIKIsRawModeUsable() returns %d",result));

	return result;
}

extern "C"  int		DllIKPostKey						(int code, int direction)
{
	return IKPostKey (code, direction);
}

extern "C"  int		DllIKPostDelay						(int delay)
{
	return IKPostDelay (delay);
}

extern "C"  int		DllIKPostMouseButtonToggle			(int whichButton)
{
	return IKPostMouseButtonToggle (whichButton);
}

extern "C"  int		DllIKPostMouseButtonClick			(int whichButton)
{
	return IKPostMouseButtonClick (whichButton);
}

extern "C"  int		DllIKPostMouseButtonDoubleClick	(int whichButton)
{
	return IKPostMouseButtonDoubleClick (whichButton);
}

extern "C"  int		DllIKPostMouseButton				(int whichButton, int direction)
{
	return IKPostMouseButton (whichButton, direction);
}

extern "C"  int		DllIKPostMouseMove					(int x, int y)
{
	return IKPostMouseMove (x, y);
}

extern "C"  int		DllIKResetMouseInterface			()
{
	return IKResetMouseInterface ();
}

extern "C"  int		DllIKResetKeyboardInterface			()
{
	return IKResetKeyboardInterface	();
}

