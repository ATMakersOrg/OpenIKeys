//
//  System Headers
//

#if TARGET_API_MAC_CARBON

#if __MACH__

	#include <Carbon/Carbon.h>

#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_IMPORT
#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
    #pragma pack(2)
#endif

typedef struct {
	UInt32		tmp[6];
} MachOFPStorage;

typedef MachOFPStorage MachOFPStorage;

//extern void *MachOFunctionPointerForCFMFunctionPointer( void *cfmfp, MachOFPStorage *storage);

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
    #pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

	#define CFMGLUE(x,y) MachOFunctionPointerForCFMFunctionPointer(x,y)
	MachOFPStorage glue1, glue2, glue3, glue4;
	
//
//	This function allocates a block of CFM glue code which contains the instructions to call CFM routines
//
UInt32 templatex[6] = {0x3D800000, 0x618C0000, 0x800C0000, 0x804C0004, 0x7C0903A6, 0x4E800420};

static void *MachOFunctionPointerForCFMFunctionPointer( void *cfmfp, MachOFPStorage *storage)
{
    storage->tmp[0] = templatex[0] | ((UInt32)cfmfp >> 16);
    storage->tmp[1] = templatex[1] | ((UInt32)cfmfp & 0xFFFF);
    storage->tmp[2] = templatex[2];
    storage->tmp[3] = templatex[3];
    storage->tmp[4] = templatex[4];
    storage->tmp[5] = templatex[5];
    MakeDataExecutable( storage, sizeof(templatex) );
    return( storage );
}


#else
	#include <Carbon.h>
	#endif
#else
	#include <Dialogs.h>
	#include <Fonts.h>
	#include <MacWindows.h>
	#include <Menus.h>
	#include <QuickDraw.h>
	#include <TextEdit.h>
	#include <gestalt.h>
	#include <folders.h>
#endif
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if! TARGET_CPU_PPC
#include <dlfcn.h>
#endif

//#include "MacUtilities.h"

//
//  Our header
//

#include "IntelliKeys.h"

//
//  Stuff that relates to finding/calling the shared library
//

typedef bool (*IsIntelliKeysConnectedProcPtr) (void);
typedef bool (*IsSendableOverlayFileProcPtr)  (char * pFilePath );
typedef int  (*SendOverlayProcPtr)            (char * pFilePath, bool bReportErrors);
typedef int  (*SendOverlay2ProcPtr)           (char * pFilePath, bool bReportErrors, char * pMessage, bool bWait, bool bBannerOnly );

static IsIntelliKeysConnectedProcPtr pIsIntelliKeysConnected = nil;
static IsSendableOverlayFileProcPtr  pIsSendableOverlayFile  = nil;
static SendOverlayProcPtr            pSendOverlay            = nil;
static SendOverlay2ProcPtr           pSendOverlay2           = nil;

CFragConnectionID connID = kInvalidID;

#define kFragmentName "\pOverlay Sending Library"

#if TARGET_API_MAC_CARBON
  #define kSharedLibFile "\p:intellitools:Carbon Sending.shlb"
#else
  #define kSharedLibFile "\p:intellitools:Classic Sending.shlb"
#endif


//
//  Get the version of Mac OS we're running.
//

static unsigned int  MacGetOSVersion ()
{
	static   Boolean    sAlreadyChecked = false;
	static   SInt32     version = 0;

	if (!sAlreadyChecked) 
	{
		OSErr err = Gestalt (gestaltSystemVersion, &version);
		sAlreadyChecked = true;
	}
	return version;
}

//
//  Take a standard C string and convert it to Pascal form.
//

static void ConvertCToPStr255 ( const char *pSrc, unsigned char *pDest )
{
	int nLen = strlen ( pSrc );
	if ( nLen > 255 )
		nLen = 255;
	memcpy ( pDest+1, pSrc, nLen );
	*pDest = (unsigned char) nLen;
}

//
//  Figure out what library to use.
//

static OSErr GetLibrarySpec ( FSSpec *spec )
{
	//  find the application support folder
	short vRefNum;
	long dirID;
	OSErr err = FindFolder ( kOnSystemDisk, kApplicationSupportFolderType, kDontCreateFolder, &vRefNum, &dirID );
	if (err != noErr)
		return err;
		
	//  find the shared library in there.
	err =  FSMakeFSSpec ( vRefNum, dirID, kSharedLibFile, spec ) ;
	return err;
}


//
//  Called once to load and set up the shared library.
//

static void Initialize()
{
	static bool bInitialized = false;
	if (!bInitialized)
	{
		bInitialized = true;
		
#if! TARGET_CPU_PPC
		unsigned int  MacGetOSVersion ();
		if (MacGetOSVersion() >= 0x1040)
		{
			//  use universal library for 10.4 and above
			void* lib_handle = dlopen("/library/application support/intellitools/universal sending.dylib", RTLD_LOCAL|RTLD_LAZY);
			if (lib_handle)
			{
				pIsIntelliKeysConnected = (IsIntelliKeysConnectedProcPtr) dlsym(lib_handle, "IsIntelliKeysConnected");
				pIsSendableOverlayFile  = (IsSendableOverlayFileProcPtr)  dlsym(lib_handle, "IsSendableOverlayFile");
				pSendOverlay			= (SendOverlayProcPtr)            dlsym(lib_handle, "SendOverlay");
				pSendOverlay2			= (SendOverlay2ProcPtr)           dlsym(lib_handle, "SendOverlay2");
			}
		}
		else
#endif
		{
			FSSpec fs;
			OSErr err = GetLibrarySpec ( &fs );
			if (err == noErr)
			{
				//  load it
				err = GetDiskFragment(&fs, 0, kCFragGoesToEOF, kFragmentName, kPrivateCFragCopy, &connID, nil, nil);
				
				if (err == noErr)
				{
					//  find symbols
					err = FindSymbol( connID, "\pIsIntelliKeysConnected", (Ptr *)  &pIsIntelliKeysConnected, NULL );
					err = FindSymbol( connID, "\pIsSendableOverlayFile",   (Ptr *) &pIsSendableOverlayFile,  NULL );
					err = FindSymbol( connID, "\pSendOverlay", (Ptr *)             &pSendOverlay,            NULL );
					err = FindSymbol( connID, "\pSendOverlay2", (Ptr *)            &pSendOverlay2,           NULL );

					#if __MACH__
					
						// convert the CFM function pointers to Mach-O function pointers
						pIsIntelliKeysConnected  = (IsIntelliKeysConnectedProcPtr) CFMGLUE((void *)pIsIntelliKeysConnected, &glue1);
						pIsSendableOverlayFile   = (IsSendableOverlayFileProcPtr)  CFMGLUE((void *)pIsSendableOverlayFile,  &glue2);
						pSendOverlay             = (SendOverlayProcPtr)            CFMGLUE((void *)pSendOverlay,            &glue3);
						pSendOverlay2            = (SendOverlay2ProcPtr)           CFMGLUE((void *)pSendOverlay2,           &glue4);
						
					#endif
				}
			}
		}

	}
}

// ---------------------------------------

bool IntelliKeys :: IsIntelliKeysConnected ( void )
{
	Initialize();
	
	if (pIsIntelliKeysConnected==nil)
		return false;
		
	return (*pIsIntelliKeysConnected)();
}

bool IntelliKeys :: IsSendableOverlayFile ( char * pFilePath )
{
	Initialize();
	
	if (pIsSendableOverlayFile==nil)
		return false;
		
	return (*pIsSendableOverlayFile)(pFilePath);
}

int  IntelliKeys :: SendOverlay ( char * pFilePath, bool bReportErrors /*= false*/, char * pMessage/*=NULL*/, bool bWait/*=true*/, bool bBannerOnly /* =false */ )
{
	Initialize();
	
	int result = false;
	
	if (pSendOverlay2)
	{
		result = (*pSendOverlay2)(pFilePath,bReportErrors,pMessage,bWait,bBannerOnly);
	}
	else if (pSendOverlay)
	{
		result = (*pSendOverlay)(pFilePath,bReportErrors);
	}
	else
	{
	}
	
	return result;
}
