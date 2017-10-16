/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		dll.cpp
//
// Purpose:		global keyboard hook used for toggle state of Numlock, CapsLock, etc.
//
// 06/18/01 fwr initial implementation	
//
**************************************************************************************************************************/

#include <windows.h>
#include <memory.h>
#include <assert.h>
#include <string.h>

#include "Aclapi.h"

#include "imm.h"

#define DLLHEADER
#include "dll.h"

HINSTANCE hInstance = NULL;
HANDLE g_hMap = NULL;

typedef struct HookRec {
int		hookType;
FARPROC func;
HHOOK 	hhook;
} HookRec;

typedef struct
{
	HookRec kbdHook;
	SHORT numlock_keystate;
	SHORT capslock_keystate;
	BOOL bStatus;
	int command;  //  0 = nothing, 1 = save, 2 = restore
} GlobalsRec;
GlobalsRec *pGlobals = NULL;
#define globals (*pGlobals)


static void UpdateKeyStates()
{
	BYTE keys[256];
	BOOL bResult;

	SHORT nl = GetKeyState(VK_NUMLOCK);
	BYTE bnl = (nl & 0x0001);

	bResult = GetKeyboardState(keys);

	keys[VK_NUMLOCK] = bnl;
	bResult = SetKeyboardState(keys);

	if (nl != globals.numlock_keystate)
	{
		globals.numlock_keystate = nl;
	}

	SHORT cl = GetKeyState(VK_CAPITAL);
	BYTE bcl = (cl & 0x0001);

	bResult = GetKeyboardState(keys);
	keys[VK_CAPITAL] = bcl;

	bResult = SetKeyboardState(keys);

	if (cl != globals.capslock_keystate)
	{
		globals.capslock_keystate = cl;
	}
}


static void UpdateIMEStates()
{
	//  get the context
	HWND activeWin = ::GetFocus();
	HIMC hContext = ImmGetContext( activeWin );

	if (globals.command==1)  //  save
	{
		if (hContext!=NULL)
		{
			globals.bStatus = ImmGetOpenStatus(hContext);
			ImmSetOpenStatus(hContext,false);
		}
	}

	else if (globals.command==2)  //  restore
	{
		if (hContext!=NULL)
		{
			ImmSetOpenStatus(hContext, globals.bStatus);
		}
	}

	globals.command = 0;

}

// Appears to be unreferenced.
/*
typedef struct {
	DWORD vkCode;
	DWORD scanCode;
	DWORD flags;
	DWORD time;
	unsigned long * dwExtraInfo;
} KBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;
*/

LRESULT CALLBACK KeyboardProc ( int nCode, WPARAM wParam, LPARAM lParam)
{
	if ( nCode == HC_ACTION )
	{
		UpdateKeyStates();
		UpdateIMEStates();
	}

	return (int)CallNextHookEx(globals.kbdHook.hhook, nCode, wParam, lParam);
}

LPVOID CreateMemFile ( const char *name, DWORD size, HANDLE &hMap, BOOL *pbInit )
{

	//  set up security descriptors for full access

	PSECURITY_DESCRIPTOR pSD = NULL;
	EXPLICIT_ACCESS ea;
	PACL pACL = NULL;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof (SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;

	PSID pEveryoneSID = NULL;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

	if(AllocateAndInitializeSid(&SIDAuthWorld, 1,
					 SECURITY_WORLD_RID,
					 0, 0, 0, 0, 0, 0, 0,
					 &pEveryoneSID))
	{
		ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
		ea.grfAccessPermissions = KEY_ALL_ACCESS;
		ea.grfAccessMode = SET_ACCESS;
		ea.grfInheritance= NO_INHERITANCE;
		ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea.Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

		DWORD dwRes = SetEntriesInAcl(1, &ea, NULL, &pACL);
		if (ERROR_SUCCESS == dwRes) 
		{
			pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH); 
			if (pSD) 
			{ 
				if (InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) 
				{  
					if (SetSecurityDescriptorDacl(pSD, 
							TRUE,     // bDaclPresent flag   
							pACL, 
							FALSE))   // not a default DACL 
					{  
						sa.lpSecurityDescriptor = pSD;
					} 
				} 
			} 
		}
	}

	//  make the mapping.

	LPVOID ptr = NULL;
	hMap = CreateFileMapping(
		(HANDLE) 0xFFFFFFFF, /* use paging file */
		&sa,				/* no security attr.  */
		PAGE_READWRITE,   /* read/write access	*/
		0,				   /* size: high 32-bits */
		sizeof(globals),   /* size: low 32-bits  */
		name);	/* name of map object */

	//  clean up security descriptors

	if (pSD) 
		LocalFree(pSD);

	if (pACL) 
		LocalFree(pACL);

	if (pEveryoneSID) 
		FreeSid(pEveryoneSID);

	//  return an error if not created

	if (hMap == NULL)
		return NULL;

	//SetNamedSecurityInfo((char *)name, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, (PACL) NULL, NULL);

	/* The first process to attach initializes memory. */

	BOOL fInit = (GetLastError() != ERROR_ALREADY_EXISTS);

	/* Get a pointer to the file-mapped shared memory. */

	ptr = MapViewOfFile(
		hMap,	 /* object to map view of	*/
		FILE_MAP_WRITE, /* read/write access		*/
		0,			  /* high offset:	map from  */
		0,			  /* low offset:	beginning */
		0); 		 /* default: map entire file */
	if (ptr == NULL)
		return NULL;

	/* Initialize memory if this is the first process. */

	*pbInit = fInit;

	return ptr;
}


BOOL WINAPI DllMain
	( HINSTANCE hinstDLL, DWORD fdwReason,LPVOID lpvReserved)
{

	hInstance = (HINSTANCE) hinstDLL;

	switch (fdwReason) 
	{

		case DLL_PROCESS_ATTACH:
			{
				BOOL bInit=false;

				pGlobals = (GlobalsRec *) CreateMemFile ( "ikusbglobaldata",
						sizeof(GlobalsRec), g_hMap, &bInit );

				if ( pGlobals == NULL )
					return FALSE;

				if ( bInit )
				{
					globals.kbdHook.hookType = WH_KEYBOARD;
					globals.kbdHook.func = (FARPROC) KeyboardProc;
					globals.kbdHook.hhook = 0;
					globals.numlock_keystate = -1;
					globals.capslock_keystate = -1;
				}

			}
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			UnmapViewOfFile(pGlobals);
			/* Close the process's handle to the file-mapping object. */
			CloseHandle(g_hMap);

			break;

		default:
			break;
	}

	return TRUE;

	UNREFERENCED_PARAMETER(lpvReserved);
}



extern "C" __export void DllKeyEnable ( BOOL enable )
{
	if ( enable )
	{
		globals.kbdHook.hhook = SetWindowsHookEx(globals.kbdHook.hookType, (HOOKPROC) globals.kbdHook.func, hInstance,NULL);
	}
	else
	{
		UnhookWindowsHookEx ( globals.kbdHook.hhook );
		globals.kbdHook.hhook = 0;
	}

}


extern "C" __export void DllGetStates (SHORT *numlock, SHORT *capslock)
{
	*numlock = globals.numlock_keystate;
	*capslock = globals.capslock_keystate;
}



extern "C" __export void DLLSaveIME()
{
	globals.command = 1;
}


extern "C" __export void DLLRestoreIME()
{
	globals.command = 2;
}

