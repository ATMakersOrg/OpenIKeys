
#ifndef _Messages_H_
#define _Messages_H_

//  Commands are all >0
enum
{
	kQueryShouldCallOldInTT 				= 1000,
	kQueryIsUSBIntelliKeysConnected 		= 1001,
	kQueryShowControlPanel 					= 1002,
	kQueryShutdown 							= 1003,
	kQueryStartRecording 					= 1004,
	kQueryStopRecording 					= 1005,
	kQueryStartPlayback 					= 1006,
	kQueryStopPlayback 						= 1007,
	kQueryOKToSend 							= 1008,
	kQueryIsConnectedAndSwitchedOn 			= 1009,
						
	kQuerySendOverlay						= 1010,
	kQueryIsIntellikeysConnected			= 1011,
	kQuerySendSettings						= 1012,
	kQueryIsStandardOverlayInPlace			= 1013,
	kQuerySendOverlayName					= 1014,
	kQueryIsIntellikeysOn					= 1015,
	kQuerySendOverlayWithName				= 1016,
	kQueryGetSettings						= 1017,
	kQuerySetStudent						= 1018,
	
	kQueryResetKeyboard						= 1050,
	kQueryUSBIntelliKeysArray				= 1051,
	kQueryUSBIntelliKeysArrayChanged		= 1052,
	kQuerySoftwareVersion					= 1053,
	kQueryFirmwareVersion					= 1054,
	kQueryStartDiagnosticMode				= 1055,
	kQueryStopDiagnosticMode				= 1056,
	kQueryDiagnosticDataBlock				= 1057,
	kQueryLastSentOverlay					= 1058,
	kQueryGlobalData						= 1059,
	kQueryBeep								= 1060,
	kQueryLaunchControlPanel				= 1061,
    kQueryLockKeys						    = 1062,
	kQuerySendOverlayForApp					= 1063,
	kQueryReloadStudent						= 1064,
	kQueryApplication						= 1065,
	kQuerySendOverlayProxy					= 1066,
	kQueryTroubleshooting					= 1067,
	kQueryStartRawMode						= 1068,
	kQueryStopRawMode						= 1069,
	kQueryGetRawEvent						= 1070,
	kQueryForwardControlPanel 				= 1071,
	kQueryShortBeep							= 1072,
	kQueryGetRawMode						= 1073,
	kQueryGetRawModeUsable					= 1074,
	kQuerySendAttachedOverlay				= 1075,
	kQueryGetRawEvents						= 1076,
	kQueryGetCurrentApp						= 1077,
	kQueryLaunchFile						= 1078,
	kQuerySetSystemActive					= 1079,
	kQuerySetSystemInactive					= 1080,
	kQueryIsSystemActive					= 1081,
	kQueryKillBalloon						= 1082,
	kQueryPostKey							= 1083,
	kQueryPostDelay							= 1084,
	kQueryPostMouseButtonToggle				= 1085,
	kQueryPostMouseButtonClick				= 1086,
	kQueryPostMouseButtonDoubleClick		= 1087,
	kQueryPostMouseButton					= 1088,
	kQueryPostMouseMove						= 1089,
	kQueryResetMouseInterface				= 1090,
	kQueryResetKeyboardInterface			= 1091,
	kQueryRawNotify							= 1092,
	kQuerySetNotifyModeOn					= 1093,
	kQuerySetNotifyModeOff					= 1094,
	kQuerySendUnicode						= 1095
};

// Responses are all <= 0
enum
{
	// Special value allowing us to call GetResource('InTT',128) to get the real sending package
	kDisableGetResourcePatchOneTime 		= -999,
	
	kResponseHandshake 						= -1000,
	kResponseDaemonTimeout					= -1001,
	kResponseCallOldInTT 					= -1002,
	kResponseDoNotCallOldInTT 				= -1003,
	kResponseDoSend 						= -1004,
	kResponseDoNotSend 						= -1005,
	kResponseIsConnectedAndSwitchedOn 		= -1006,
	kResponseNotConnectedOrNotSwitchedOn 	= -1007,
	
	kResponseError							= -1008,
	kResponseNoServer						= -1009,
	kResponseNotEnoughtData					= -1010,
	kResponseFileError						= -1011,
	kResponseNotConnected					= -1012,
	kResponseConnected						= -1013,
	kResponseRegistered						= -1014,
	kResponseAlreadyRegistered				= -1015,
	kResponseRegistrationError				= -1016,
	kResponseNoError						= -1017,
	kResponseStandardOverlayIsInPlace		= -1018,
	kResponseStandardOverlayIsNotInPlace	= -1019,
	kResponseNotOn							= -1020,
	kResponseOn								= -1021,
	kResponseTimeout						= -1022,

	kResponseUnsupported					= -1030,
	
	kRespondResetKeyboard					= -1050,
	kResponseUSBIntelliKeysArray			= -1051,
	kResponseUSBIntelliKeysArrayChanged		= -1052,
	kResponseSoftwareVersion				= -1053,
	kResponseFirmwareVersion				= -1054,
	kResponseStartDiagnosticMode			= -1055,
	kResponseStopDiagnosticMode				= -1056,
	kResponseDiagnosticDataBlock			= -1057,
	kResponseLastSentOverlay				= -1058,
	kResponseGlobalData						= -1059,
	kResponseRawEvent						= -1060,
	kResponseNoRawEvents					= -1061,
	kResponseRawModeOn						= -1062,
	kResponseRawModeOff						= -1063,
	kResponseRawModeUsable					= -1064,
	kResponseRawModeUnusable				= -1065,
	kResponseRawEvents						= -1066,
	kResponseGetCurrentApp					= -1067,
	kResponseSystemActive					= -1068,
	kResponseSystemInactive					= -1069,

	kResponseNoCommand						= 0
};

typedef struct 
{
	int deviceNumber;	//  which intelikeys/intelliswitch
	int eventType;		//  1=membrane, 2=switch, 3=std overlay change
	int row;			//  zero-based, top-bottom, only applies if eventType=1
	int col;			//  zero-based, left-right, only applies if eventType=1
	int switchNumber;	//  zero-based, only applies if eventType=2
	bool press;			//  true if the event is a press, false if itﾒs a release, applies for eventType 1 and 2
	unsigned int time;  //  100-msec units
	int overlay;			//  0=no overlay, 1-8=std ovl number, applies when eventType==3
} RawEvent;

#endif  //  #ifndef _Messages_H_
