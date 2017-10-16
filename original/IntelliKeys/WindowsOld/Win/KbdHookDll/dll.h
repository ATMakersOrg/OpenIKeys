/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		dll.h
//
// Purpose:		global keyboard hook used for toggle state of Numlock, CapsLock, etc.
//
// 06/18/01 fwr initial implementation	
//
**************************************************************************************************************************/

#ifdef DLLHEADER
#define __export __declspec( dllexport )
#else
#define __export __declspec( dllimport )
#endif

extern "C" __export void DllKeyEnable ( BOOL bEnable );
extern "C" __export void DllGetStates (SHORT *numlock, SHORT *capslock);
extern "C" __export bool DLLGetIMEOpenStatus();

extern "C" __export void DLLSaveIME();
extern "C" __export void DLLRestoreIME();

