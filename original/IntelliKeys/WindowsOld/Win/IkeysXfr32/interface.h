/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		interface.h
//
// Purpose:		messaging primarily involved with overlay sending and (Windows) sleep / wake
//
// 06/18/01 fwr initial implementation	
//
**************************************************************************************************************************/
#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 1  //  ndef NO_ENUMS

// Commands are all > 0
enum
{
	// Daemon commands
#if OPT_MACOS
	kQueryv1ShouldCallOldInTT = 1000,
#endif
	kQueryv1IsUSBIntelliKeysConnected = 1001,
	kQueryv1ShowControlPanel = 1002,
	kQueryv1StopServer = 1003,
	kQueryv1SendOverlay = 1005,
	kQueryv1IsIntellikeysConnected = 1006,
	kQueryv1SimulateInput = 1007,
	kQueryv1SendSettings = 1008,
	kQueryv1IsStandardOverlayInPlace = 1009,
	kQueryv1SendOverlayName = 1010,
	kQueryv1IsIntellikeysOn = 1011,
	kQueryv1SendOverlayWithName = 1012,
};

// Responses are all <= 0
enum
{
#if OPT_MACOS
#endif
	
// Daemon responses
#if OPT_MACOS
// Special value allowing us to call GetResource('InTT',128) to get the real sending package
	kDisableGetResourcePatchOneTime = -999,
	kResponsev1CallOldInTT = -1002,
	kResponsev1DoNotCallOldInTT = -1003,
#endif
	kResponsev1Handshake = -1000,
	kResponsev1Timeout = -1001,
	kResponsev1Error = -1004,
	kResponsev1NoServer = -1005,
	kResponsev1NotEnoughtData = -1006,
	kResponsev1FileError = -1007,
	kResponsev1NotConnected = -1008,
	kResponsev1Connected = -1009,
	kResponsev1Registered = -1010,
	kResponsev1AlreadyRegistered = -1011,
	kResponsev1RegistrationError = -1012,
	kResponsev1NoError = -1013,
	kResponsev1StandardOverlayIsInPlace = -1014,
	kResponsev1StandardOverlayIsNotInPlace = -1015,
	kResponsev1NotOn = -1016,
	kResponsev1On = -1017,

	kResponsev1NoCommand = 0
};

#endif

typedef void (CALLBACK SERVERPROC)(int,int);

typedef void (CALLBACK WINMSGPROC)(UINT,WPARAM,LPARAM);

typedef LRESULT (CALLBACK WINMSGPROCWITHRESULT)(UINT,WPARAM,LPARAM);

enum {
	kServerCallbackTerminating=0,
	kServerCallbackCommand
};

//  server stuff
int  IKUSBReceiveMessage ( int *command, void *data, int maxdata, int *dataread );
int  IKUSBRegisterServer (SERVERPROC *pProc, HWND *window);
int  IKUSBRespondToMessage ( int response, void *data, int datalength );
void IKUSBFlushMessages();
int  IKUSBIsServerRegistered();


//  client stuff

int IKUSBSendOverlay ( char *overlayname );
int IKUSBSendOverlayName ( char *name );
int IKUSBSendOverlayWithName ( char *overlayname, char *name );
int IKUSBShowControlPanel ();
int IKUSBStopServer ();
int IKUSBIsIntellikeysConnected ();
int IKUSBSimulateInput ();
int IKUSBSendSettings ( BYTE *set );
int IKUSBGetSettings ( BYTE *set );
int IKUSBIsStandardOverlayInPlace();
int IKUSBRegisterWinMsgProc ( WINMSGPROC *pProc );
int IKUSBRegisterWinMsgProcWithResult ( WINMSGPROCWITHRESULT *pProc );
int IKUSBIsIntellikeysOn ();
int IKUSBIsServerRunning ();

void IKUSBGetCommFile(char *filename);


#ifdef __cplusplus
}
#endif

#endif
