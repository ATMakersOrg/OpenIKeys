/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		AppLib DLL.h
//
// Purpose:
//
// 06/18/01 fwr initial implementation	
//
**************************************************************************************************************************/

#ifdef DLLHEADER
#define __export __declspec( dllexport )
#else
#define __export __declspec( dllimport )
#endif

//extern "C" __export int		DllIKSendOverlay				( TCHAR *overlayname );

extern "C" __export int		DllAppLibAddSystem						( TCHAR * SystemName, TCHAR * IdentifyingFileName );
extern "C" __export int		DllAppLibRemoveSystem					( TCHAR * SystemName );
extern "C" __export int		DllAppLibCountSystems					( );
extern "C" __export TCHAR *	DllAppLibGetNthSystem					( int n );
extern "C" __export int		DllAppLibAddApplication					( TCHAR * AppPath );
extern "C" __export int		DllAppLibRemoveApplication				( TCHAR * AppPath, TCHAR * SystemName );
extern "C" __export int		DllAppLibTakeOwnership					( TCHAR * AppPath, TCHAR * SystemName );
extern "C" __export int		DllAppLibHasOwnership					( TCHAR * AppPath, TCHAR * SystemName);
extern "C" __export TCHAR *	DllAppLibGetOwner						( TCHAR * AppPath );
extern "C" __export int		DllAppLibReleaseOwnership				( TCHAR * AppPath, TCHAR * SystemName);
extern "C" __export int		DllAppLibCountApplications				( );
extern "C" __export TCHAR *	DllAppLibGetNthApplication				( int n );
extern "C" __export bool	DllAppLibLock							( int timeout );
extern "C" __export bool	DllAppLibUnlock							( );
extern "C" __export TCHAR *	DllAppLibGetAppFriendlyName				( TCHAR * AppPath );
extern "C" __export int		DllAppLibMaintenance					( );
extern "C" __export TCHAR *	DllAppLibGetStudentAttachedOverlay		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
extern "C" __export int		DllAppLibCountAttachedOverlays			( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
extern "C" __export int		DllAppLibGetSelectedOverlay				( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
extern "C" __export TCHAR *	DllAppLibGetNumberedOverlay				( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, int OverlayNumber);
extern "C" __export int		DllAppLibAddApplicationForStudent		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
extern "C" __export int		DllAppLibAddApplicationForCurrentStudent		( TCHAR * AppPath );
extern "C" __export int		DllAppLibSetSelectedOverlay				( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, int OverlayNumber);
extern "C" __export int		DllAppLibAttachOverlayForStudent		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, TCHAR *OverlayPath );
extern "C" __export int		DllAppLibDetachOverlayForStudent		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, TCHAR *OverlayPath );
extern "C" __export bool	DllAppLibIsSystemPresent				( TCHAR * SystemName );
extern "C" __export void	DllAppLibAddPreinstalledOverlays		( TCHAR * GroupName, TCHAR * StudentName, bool bForce );
extern "C" __export void	DllAppLibFloatingMessage				( TCHAR * Message );
extern "C" __export bool	DllAppLibIsAppAllowed					( TCHAR * AppPath );
extern "C" __export void	DllAppLibSetAppAllowed					( TCHAR * AppPath, bool bAllowed );
extern "C" __export int		DllAppLibRemoveApplicationForStudent	( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
extern "C" __export int		DllAppLibCountStandardApps				( );
extern "C" __export TCHAR *	DllAppLibGetNthStandardApp				( int n );
extern "C" __export TCHAR * DllAppLibGetTruePath					( TCHAR * AppPath );
extern "C" __export void	DllAppLibSetSystemInterest				( TCHAR * AppPath, TCHAR * SystemName, bool bInterest );
extern "C" __export bool	DllAppLibHasSystemInterest				( TCHAR * AppPath, TCHAR * SystemName );
extern "C" __export void	DllAppLibAddDisallowedApps				( );
extern "C" __export void	DllAppLibDeferSaving					( bool bDefer );
extern "C" __export bool	DllAppLibIsOverlayPreinstalled			( TCHAR * OverlayPath );
extern "C" __export int		DllAppLibUpdateAppPath					( TCHAR * oldPath, TCHAR * newPath );
extern "C" __export bool	DllAppLibIsIntelliSwitchInstalled		( );
extern "C" __export bool	DllAppLibIsIntellikeysInstalled			( );
extern "C" __export bool	DllAppLibCanListApplication				( TCHAR * AppName );
extern "C" __export int		DllAppLibIsSystemActive					( TCHAR * SystemName );
extern "C" __export int		DllAppLibSetSystemActive				( TCHAR * SystemName );
extern "C" __export int		DllAppLibSetSystemInactive				( TCHAR * SystemName );
