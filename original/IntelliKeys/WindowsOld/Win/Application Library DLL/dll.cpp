/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class: Message Client DLL.cpp
//
// Purpose: global keyboard hook used for toggle state of Numlock, CapsLock, etc.
//
// 06/18/01 fwr initial implementation 
//
**************************************************************************************************************************/

#include <windows.h>

#include "AppLib.h"

#define DLLHEADER
#include "Application Library dll.h"

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

BOOL WINAPI DllMain ( HINSTANCE hinstDLL, DWORD fdwReason,LPVOID lpvReserved)
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


extern "C" __export int DllAppLibAddSystem ( TCHAR * SystemName, TCHAR * IdentifyingFileName )
{
 	int result = AppLibAddSystem ( SystemName, IdentifyingFileName );

	DLLTRACE(("DllAppLibAddSystem(%s,%s) returns %d",SystemName,IdentifyingFileName,result));

	return result;
}

extern "C" __export int DllAppLibRemoveSystem ( TCHAR * SystemName )
{
 	int result = AppLibRemoveSystem ( SystemName );

	DLLTRACE(("DllAppLibRemoveSystem(%s) returns %d",SystemName,result));

	return result;
}

extern "C" __export bool DllAppLibCanListApplication ( TCHAR * AppName )
{
 	bool result = AppLibCanListApplication ( AppName );

	DLLTRACE(("DllAppLibCanListApplication(%s) returns %d",AppName,result));

	return result;
}

extern "C" __export int DllAppLibCountSystems ( )
{
 	int result = AppLibCountSystems ( );

	DLLTRACE(("DllAppLibCountSystems() returns %d",result));

	return result;
}

extern "C" __export TCHAR * DllAppLibGetNthSystem ( int n )
{
 	TCHAR * result = AppLibGetNthSystem ( n );

	DLLTRACE(("DllAppLibGetNthSystem(%d) returns %s",n,result));

	return result;
}

extern "C" __export int DllAppLibAddApplication ( TCHAR * AppPath )
{
	return 1;

 	int result = AppLibAddApplication ( AppPath );

	DLLTRACE(("DllAppLibAddApplication(%s) returns %d",AppPath,result));

	return result;
}

extern "C" __export int DllAppLibRemoveApplication ( TCHAR * AppPath, TCHAR * SystemName )
{
 	int result = AppLibRemoveApplication ( AppPath, SystemName );

	DLLTRACE(("DllAppLibRemoveApplication(%s,%s) returns %d",AppPath,SystemName,result));

	return result;
}

extern "C" __export int DllAppLibTakeOwnership ( TCHAR * AppPath, TCHAR * SystemName )
{
	if (strcmp(SystemName,"Discover")==0)
		return 1;

 	int result = AppLibTakeOwnership ( AppPath, SystemName );

	DLLTRACE(("DllAppLibTakeOwnership(%s,%s) returns %d",AppPath,SystemName,result));

	return result;
}

extern "C" __export int DllAppLibHasOwnership ( TCHAR * AppPath, TCHAR * SystemName)
{
	int result;

	//  IK USB 3.0.3
	//  bkwd-compatibility for Discover 2.0.2 and Envoy 0.9
	if (strcmp(SystemName,"Discover")==0)
		result = 1;
	else
 		result = AppLibHasOwnership ( AppPath, SystemName);

	DLLTRACE(("DllAppLibHasOwnership(%s,%s) returns %d",AppPath,SystemName,result));

	return result;
}

extern "C" __export TCHAR * DllAppLibGetOwner ( TCHAR * AppPath )
{
	//  IK USB 3.1
	//  bkwd-compatibility for Discover 2.0.2 and Envoy 0.9
	static TCHAR *result = "";

 	//TCHAR * result = AppLibGetOwner ( AppPath );

	DLLTRACE(("DllAppLibGetOwner(%s) returns %s",AppPath,result));

	return result;
}

extern "C" __export int DllAppLibReleaseOwnership ( TCHAR * AppPath, TCHAR * SystemName)
{
 	int result = AppLibReleaseOwnership ( AppPath, SystemName );

	DLLTRACE(("DllAppLibReleaseOwnership(%s,%s) returns %d",AppPath, SystemName ,result));

	return result;
}

extern "C" __export int DllAppLibCountApplications ( )
{
 	int result = AppLibCountApplications ( );

	DLLTRACE(("DllAppLibCountApplications() returns %d",result));

	return result;
}

extern "C" __export TCHAR * DllAppLibGetNthApplication ( int n )
{
 	TCHAR * result = AppLibGetNthApplication ( n );

	DLLTRACE(("DllAppLibGetNthApplication(%d) returns %s",n,result));

	return result;
}

extern "C" __export bool DllAppLibLock ( int timeout )
{
 	bool result = AppLibLock ( timeout );

	DLLTRACE(("DllAppLibLock(%d) returns %d",timeout,result));

	return result;
}

extern "C" __export bool DllAppLibUnlock ( )
{
 	bool result = AppLibUnlock ( );

	DLLTRACE(("DllAppLibUnlock() returns %d",result));

	return result;
}

extern "C" __export TCHAR * DllAppLibGetAppFriendlyName ( TCHAR * AppPath )
{
 	TCHAR * result = AppLibGetAppFriendlyName ( AppPath );

	DLLTRACE(("DllAppLibGetAppFriendlyName(%s) returns %s",AppPath,result));

	return result;
}

extern "C" __export int DllAppLibMaintenance ( )
{
 	int result = AppLibMaintenance ( );

	DLLTRACE(("DllAppLibMaintenance() returns %d",result));

	return result;
}

extern "C" __export TCHAR * DllAppLibGetStudentAttachedOverlay ( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName )
{
 	TCHAR * result = AppLibGetStudentAttachedOverlay ( AppPath, GroupName, StudentName );

	DLLTRACE(("DllAppLibGetStudentAttachedOverlay(%s,%s,%s) returns %s",AppPath, GroupName, StudentName,result));

	return result;
}

extern "C" __export int DllAppLibCountAttachedOverlays ( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName )
{
 	int result = AppLibCountAttachedOverlays ( AppPath, GroupName, StudentName );

	DLLTRACE(("DllAppLibCountAttachedOverlays(%s,%s,%s) returns %d",AppPath, GroupName, StudentName,result));

	return result;
}

extern "C" __export int DllAppLibGetSelectedOverlay ( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName )
{
 	int result = AppLibGetSelectedOverlay ( AppPath, GroupName, StudentName );

	DLLTRACE(("DllAppLibGetSelectedOverlay(%s,%s,%s) returns %d",AppPath, GroupName, StudentName,result));

	return result;
}

extern "C" __export TCHAR * DllAppLibGetNumberedOverlay ( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, int OverlayNumber)
{
 	TCHAR * result = AppLibGetNumberedOverlay ( AppPath, GroupName, StudentName, OverlayNumber);

	DLLTRACE(("DllAppLibGetNumberedOverlay(%s,%s,%s,%d) returns %s",AppPath, GroupName, StudentName, OverlayNumber,result));

	return result;
}

extern "C" __export int DllAppLibAddApplicationForStudent ( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName )
{
 	int result = AppLibAddApplicationForStudent ( AppPath, GroupName, StudentName );

	DLLTRACE(("DllAppLibAddApplicationForStudent(%s,%s,%s) returns %d",AppPath, GroupName, StudentName,result));

	return result;
}

extern "C" __export int DllAppLibAddApplicationForCurrentStudent ( TCHAR * AppPath )
{
 	int result = AppLibAddApplicationForCurrentStudent ( AppPath );

	DLLTRACE(("DllAppLibAddApplicationForCurrentStudent(%s) returns %d",AppPath,result));

	return result;
}

extern "C" __export int DllAppLibSetSelectedOverlay ( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, int OverlayNumber)
{
 	int result = AppLibSetSelectedOverlay ( AppPath, GroupName, StudentName, OverlayNumber);

	DLLTRACE(("DllAppLibSetSelectedOverlay(%s,%s,%s,%d) returns %d",AppPath, GroupName, StudentName, OverlayNumber,result));

	return result;
}

extern "C" __export int DllAppLibAttachOverlayForStudent ( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, TCHAR *OverlayPath )
{
 	int result = AppLibAttachOverlayForStudent ( AppPath, GroupName, StudentName, OverlayPath );

	DLLTRACE(("DllAppLibAttachOverlayForStudent(%s,%s,%s,%s) returns %d",AppPath, GroupName, StudentName, OverlayPath ,result));

	return result;
}

extern "C" __export int DllAppLibDetachOverlayForStudent ( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, TCHAR *OverlayPath )
{
 	int result = AppLibDetachOverlayForStudent ( AppPath, GroupName, StudentName, OverlayPath );

	DLLTRACE(("DllAppLibDetachOverlayForStudent(%s,%s,%s,%s) returns %d",AppPath, GroupName, StudentName, OverlayPath,result));

	return result;
}

extern "C" __export bool DllAppLibIsSystemPresent ( TCHAR * SystemName )
{
 	bool result = AppLibIsSystemPresent ( SystemName );

	DLLTRACE(("DllAppLibIsSystemPresent(%s) returns %d",SystemName,result));

	return result;
}

extern "C" __export void DllAppLibAddPreinstalledOverlays ( TCHAR * GroupName, TCHAR * StudentName, bool bForce )
{
	AppLibAddPreinstalledOverlays ( GroupName, StudentName, bForce );

	DLLTRACE(("DllAppLibAddPreinstalledOverlays(%s,%s,%d)",GroupName, StudentName, bForce));

 	return;
}

extern "C" __export void DllAppLibFloatingMessage ( TCHAR * Message )
{
	AppLibFloatingMessage ( Message );
 
	DLLTRACE(("DllAppLibAddSystem(%s)",Message));

	return;
}

extern "C" __export bool DllAppLibIsAppAllowed ( TCHAR * AppPath )
{
	bool bResult;

	//  IK USB 3.0.3
	//  bkwd-compatibility for Discover 2.0.2 and Envoy 0.9
	bResult = true;
 	//bResult = AppLibIsAppAllowed ( AppPath );

	DLLTRACE(("DllAppLibIsAppAllowed(%s) returns %d",AppPath,bResult));

	return bResult;
}

extern "C" __export void DllAppLibSetAppAllowed ( TCHAR * AppPath, bool bAllowed )
{
	AppLibSetAppAllowed ( AppPath, bAllowed );

	DLLTRACE(("DllAppLibSetAppAllowed(%s,%d)",AppPath, bAllowed));

 	return;
}

extern "C" __export int DllAppLibRemoveApplicationForStudent ( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName )
{
 	int result = AppLibRemoveApplicationForStudent ( AppPath, GroupName, StudentName );

	DLLTRACE(("DllAppLibRemoveApplicationForStudent(%s,%s,%s) returns %d",AppPath, GroupName, StudentName,result));

	return result;
}

extern "C" __export int DllAppLibCountStandardApps ( )
{
 	int result = AppLibCountStandardApps ( );

	DLLTRACE(("DllAppLibCountStandardApps() returns %d",result));

	return result;
}

extern "C" __export TCHAR * DllAppLibGetNthStandardApp ( int n )
{
 	TCHAR * result = AppLibGetNthStandardApp ( n );

	DLLTRACE(("DllAppLibGetNthStandardApp(%d) returns %s",n,result));

	return result;
}

extern "C" __export TCHAR * DllAppLibGetTruePath ( TCHAR * AppPath )
{
 	TCHAR * result = AppLibGetTruePath ( AppPath );

	DLLTRACE(("DllAppLibGetTruePath(%s) returns %s",AppPath,result));

	return result;
}

extern "C" __export void DllAppLibSetSystemInterest ( TCHAR * AppPath, TCHAR * SystemName, bool bInterest )
{
	AppLibSetSystemInterest (AppPath, SystemName, bInterest );

	DLLTRACE(("DllAppLibSetSystemInterest(%s,%s,%d)",AppPath, SystemName, bInterest));

 	return;
}

extern "C" __export bool DllAppLibHasSystemInterest ( TCHAR * AppPath, TCHAR * SystemName )
{
	bool bResult;

	//  IK USB 3.0.3
	//  bkwd-compatibility for Discover 2.0.2 and Envoy 0.9
	if (strcmp(SystemName,"Discover")==0)
		bResult = true;
	else
 		bResult = AppLibHasSystemInterest ( AppPath, SystemName );

	DLLTRACE(("DllAppLibHasSystemInterest(%s,%s) returns %d",AppPath, SystemName,bResult));

	return bResult;
}

extern "C" __export void DllAppLibAddDisallowedApps ( )
{
	AppLibAddDisallowedApps ( );

	DLLTRACE(("DllAppLibAddDisallowedApps()"));

 	return;
}

extern "C" __export void DllAppLibDeferSaving ( bool bDefer )
{
	AppLibDeferSaving ( bDefer );

	DLLTRACE(("DllAppLibDeferSaving(%d)",bDefer));

 	return;
}

extern "C" __export bool DllAppLibIsOverlayPreinstalled ( TCHAR * OverlayPath )
{
 	bool result = AppLibIsOverlayPreinstalled ( OverlayPath );

	DLLTRACE(("DllAppLibIsOverlayPreinstalled(%s) returns %d",OverlayPath,result));

	return result;
}

extern "C" __export int DllAppLibUpdateAppPath ( TCHAR * oldPath, TCHAR * newPath )
{
 	int result = AppLibUpdateAppPath ( oldPath, newPath );

	DLLTRACE(("DllAppLibUpdateAppPath(%s,%s) returns %d",oldPath, newPath,result));

	return result;
}

extern "C" __export bool DllAppLibIsIntelliSwitchInstalled ( )
{
 	bool result = AppLibIsIntelliSwitchInstalled ( );

	DLLTRACE(("DllAppLibIsIntelliSwitchInstalled() returns %d",result));

	return result;
}

extern "C" __export bool DllAppLibIsIntellikeysInstalled ( )
{
 	bool result = AppLibIsIntellikeysInstalled ( );

	DLLTRACE(("DllAppLibIsIntellikeysInstalled() returns %d",result));

	return result;
}

extern "C" __export int DllAppLibIsSystemActive ( TCHAR * SystemName )
{
 	int result = AppLibIsSystemActive ( SystemName );

	DLLTRACE(("DllAppLibIsSystemActive(%s) returns %d",SystemName,result));

	return result;
}

extern "C" __export int DllAppLibSetSystemActive ( TCHAR * SystemName )
{
 	int result = AppLibSetSystemActive ( SystemName );

	DLLTRACE(("DllAppLibSetSystemActive(%s) returns %d",SystemName,result));

	return result;
}

extern "C" __export int DllAppLibSetSystemInactive ( TCHAR * SystemName )
{
 	int result = AppLibSetSystemInactive ( SystemName );

	DLLTRACE(("DllAppLibSetSystemInactive(%s) returns %d",SystemName,result));

	return result;
}
