
#ifndef _MessageClient_H_
#define _MessageClient_H_

#include "Messages.h"

#ifdef __cplusplus
extern "C" {
#endif

//  client routines

int		IKSendOverlay				( TCHAR *overlayname );
int		IKSendOverlayName			( TCHAR *name );
int		IKSendOverlayWithName		( TCHAR *overlayname, TCHAR *name, TCHAR *sender );
int		IKShowControlPanel			();
int		IKStopServer				();
int		IKStopSystray				();
int		IKIsIntellikeysConnected	();
int		IKSendSettings				( BYTE *set );
int		IKGetSettings				( BYTE *set );
int		IKIsStandardOverlayInPlace	();
int		IKIsIntellikeysOn			();
int		IKSetStudent				( TCHAR *group, TCHAR *student );
bool	IKIsEngineAlive				();
bool	IKIsControlPanelAlive		();
TCHAR * IKGetMessageName			( int message );

int		IKResetKeyboard				();
int		IKBeep						();
int		IKShortBeep						();
int		IKStartDiagnosticMode		();
int		IKStopDiagnosticMode		();
int		IKGetDeviceArray			( BYTE *array );
int		IKGetSoftwareVersion		( TCHAR *version );
int		IKGetFirmwareVersion		( TCHAR *version );
int		IKGetLastSentOverlay		( TCHAR *overlay );

int		IKReceive					( TCHAR *channel, int *command, void *data, int maxdata, int *datalen );
int		IKRespond					( TCHAR *channel, int command, void *data, int datalen );

void	IKMessageInitialize			();
bool	IKDebugSender();
void	IKMakeWriteable				( TCHAR *filename );

int		IKStartRawMode		();
int		IKStopRawMode		();
bool	IKIsRawModeOn		();
bool	IKIsRawModeUsable	();

int		IKGetRawEvent ( RawEvent *event );
int		IKGetRawEvents ( RawEvent *events, int nEventsIn, int *nEventsOut );

int		IKPostKey						(int code, int direction);
int		IKPostDelay						(int delay);
int		IKPostMouseButtonToggle			(int whichButton);
int		IKPostMouseButtonClick			(int whichButton);
int		IKPostMouseButtonDoubleClick	(int whichButton);
int		IKPostMouseButton				(int whichButton, int direction);
int		IKPostMouseMove					(int x, int y);
int		IKResetMouseInterface			();
int		IKResetKeyboardInterface		();

int		IKSetNotifyModeOn		();
int		IKSetNotifyModeOff		();




// mmmmmmmmmmmmmmmmmmmmmmmmmmm

int		IK2SendOverlay				( TCHAR *overlayname );
int		IK2SendOverlayName			( TCHAR *name );
int		IK2SendOverlayWithName		( TCHAR *overlayname, TCHAR *name, TCHAR *sender );
int		IK2ShowControlPanel			();
int		IK2StopServer				();
int		IK2StopSystray				();
int		IK2IsIntellikeysConnected	();
int		IK2SendSettings				( BYTE *set );
int		IK2GetSettings				( BYTE *set );
int		IK2IsStandardOverlayInPlace	();
int		IK2IsIntellikeysOn			();
int		IK2SetStudent				( TCHAR *group, TCHAR *student );
bool	IK2IsEngineAlive				();
bool	IK2IsControlPanelAlive		();
TCHAR * IK2GetMessageName			( int message );

int		IK2ResetKeyboard				();
int		IK2Beep						();
int		IK2StartDiagnosticMode		();
int		IK2StopDiagnosticMode		();
int		IK2GetDeviceArray			( BYTE *array );
int		IK2GetSoftwareVersion		( TCHAR *version );
int		IK2GetFirmwareVersion		( TCHAR *version );
int		IK2GetLastSentOverlay		( TCHAR *overlay );

int		IK2Receive					( TCHAR *channel, int *command, void *data, int maxdata, int *datalen );
int		IK2Respond					( TCHAR *channel, int command, void *data, int datalen );


#ifdef __cplusplus
}
#endif

#endif  //  #ifndef _MessageClient_H_
