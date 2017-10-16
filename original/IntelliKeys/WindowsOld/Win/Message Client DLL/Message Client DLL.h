/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		Message Client DLL.h
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

extern "C" __export int		DllIKSendOverlay				( TCHAR *overlayname );
extern "C" __export int		DllIKSendOverlayName			( TCHAR *name );
extern "C" __export int		DllIKSendOverlayWithName		( TCHAR *overlayname, TCHAR *name, TCHAR *sender );
extern "C" __export int		DllIKShowControlPanel			();
extern "C" __export int		DllIKStopServer				();
extern "C" __export int		DllIKStopSystray				();
extern "C" __export int		DllIKIsIntellikeysConnected	();
extern "C" __export int		DllIKSendSettings				( BYTE *set );
extern "C" __export int		DllIKGetSettings				( BYTE *set );
extern "C" __export int		DllIKIsStandardOverlayInPlace	();
extern "C" __export int		DllIKIsIntellikeysOn			();
extern "C" __export int		DllIKSetStudent				( TCHAR *group, TCHAR *student );
extern "C" __export bool	DllIKIsEngineAlive				();
extern "C" __export bool	DllIKIsControlPanelAlive		();
extern "C" __export TCHAR * DllIKGetMessageName			( int message );

extern "C" __export int		DllIKResetKeyboard				();
extern "C" __export int		DllIKBeep						();
extern "C" __export int		DllIKShortBeep						();
extern "C" __export int		DllIKStartDiagnosticMode		();
extern "C" __export int		DllIKStopDiagnosticMode		();
extern "C" __export int		DllIKGetDeviceArray			( BYTE *array );
extern "C" __export int		DllIKGetSoftwareVersion		( TCHAR *version );
extern "C" __export int		DllIKGetFirmwareVersion		( TCHAR *version );
extern "C" __export int		DllIKGetLastSentOverlay		( TCHAR *overlay );

extern "C" __export int		DllIKReceive					( TCHAR *channel, int *command, void *data, int maxdata, int *datalen );
extern "C" __export int		DllIKRespond					( TCHAR *channel, int command, void *data, int datalen );

extern "C" __export void	DllIKMessageInitialize			();
extern "C" __export bool	DllIKDebugSender();
extern "C" __export void	DllIKMakeWriteable				( TCHAR *filename );

extern "C" __export int		DllIKStartRawMode		();
extern "C" __export int		DllIKStopRawMode		();
extern "C" __export bool	DllIKIsRawModeOn		();
extern "C" __export bool	DllIKIsRawModeUsable	();

extern "C" __export int		DllIKGetRawEvent ( RawEvent *event );
extern "C" __export int		DllIKGetRawEvents ( RawEvent *events, int nEventsIn, int *nEventsOut );
