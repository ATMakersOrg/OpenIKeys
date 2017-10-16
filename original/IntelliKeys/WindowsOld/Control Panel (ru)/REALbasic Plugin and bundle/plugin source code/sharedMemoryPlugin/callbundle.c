
#include "sharedMemory.h"
#include <CFBundle.h>
#include "callbundle.h"
#include "string.h"
SayHelloPtr theSayHelloPtr;
CreateSharedMemoryPtr theCreateSharedMemoryPtr;
LockSharedMemoryPtr theLockSharedMemoryPtr;
UnlockSharedMemoryPtr theUnlockSharedMemoryPtr;
WriteSharedMemoryPtr theWriteSharedMemoryPtr;
WriteSharedMemoryBytePtr theWriteSharedMemoryBytePtr;
ReadSharedMemoryPtr theReadSharedMemoryPtr;
ReadSharedMemoryBytePtr theReadSharedMemoryBytePtr;
DestroySharedMemoryPtr theDestroySharedMemoryPtr;
ClearSharedMemoryPtr theClearSharedMemoryPtr;
static OSErr FindApplicationDirectory(short *theVRefNum, long *theParID)
{
	//  this is where it is
	char *path = "/applications/intellitools/intellikeys usb/IntelliKeys USB";
	char cstring[256];
	FSSpec spec;
	OSErr err;
	
	//  convert to HFS.
	CFStringRef stringRef = CFStringCreateWithCString( kCFAllocatorDefault, path, kCFStringEncodingMacHFS ); 
	CFURLRef urlRef = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, stringRef, kCFURLPOSIXPathStyle, false ); 
	stringRef = CFURLCopyFileSystemPath( urlRef, kCFURLHFSPathStyle ); 
	CFStringGetCString( stringRef, &(cstring[1]), 255, kCFStringEncodingMacHFS ); 
	cstring[0] = strlen(&(cstring[1]));
	
 	err = FSMakeFSSpec ( (short)0, (long)0, (const unsigned char *)cstring, &spec );
	*theVRefNum = spec.vRefNum;
	*theParID = spec.parID;
	
	return err;
	
#if 0
	OSErr theErr;
	ProcessSerialNumber thePSN;
	ProcessInfoRec theInfo;
	FSSpec theSpec;
	
	thePSN.highLongOfPSN = 0;
	thePSN.lowLongOfPSN = kCurrentProcess;
	
	theInfo.processInfoLength = sizeof(theInfo);
	theInfo.processName = NULL;
	theInfo.processAppSpec = &theSpec;
	
	/* Find the application FSSpec */
	theErr = GetProcessInformation(&thePSN, &theInfo);
	
	if (theErr == noErr)
	{
		/* Return the folder which contains the application */
		*theVRefNum = theSpec.vRefNum;
		*theParID = theSpec.parID;
	}
	
	return theErr;
#endif
}
static OSErr CreateBundleFromFSSpec(FSSpec *theSpec, CFBundleRef *theBundle)
{
	OSErr theErr;
	FSRef theRef;
	CFURLRef theBundleURL;
	
	/* Turn the FSSpec pointing to the Bundle into a FSRef */
	theErr = FSpMakeFSRef(theSpec, &theRef);
	
	/* Turn the FSRef into a CFURL */
	theBundleURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &theRef);
	
	if (theBundleURL != NULL)
	{
		/* Turn the CFURL into a bundle reference */
		*theBundle = CFBundleCreate(kCFAllocatorSystemDefault, theBundleURL);
		
		CFRelease(theBundleURL);
	}
	
	return theErr;
}
void DoLoadBundle(void)
{
	OSErr theErr;
	Boolean isLoaded;
	short theVRefNum;
	long theParID;
	FSSpec theSpec;
	CFBundleRef theBundle;
	
	
	/* Start with no bundle */
	theBundle = NULL;
	
	/* This returns the directory which contains the application */
	theErr = FindApplicationDirectory(&theVRefNum, &theParID);
	
	/* Create the FSSpec pointing to the Bundle */
	if (theErr == noErr)
		theErr = FSMakeFSSpec(theVRefNum, theParID, "\p:private:IKUSBcontrolPanel.bundle", &theSpec);
	
	/* Create a bundle reference based on a FSSpec */
	if (theErr == noErr)
		theErr = CreateBundleFromFSSpec(&theSpec, &theBundle);
	
	if ((theErr == noErr) && (theBundle != NULL))
	{
		isLoaded = CFBundleLoadExecutable(theBundle);
		
		if (isLoaded)
		{
			/* Lookup the functions in the bundle by name */
			theSayHelloPtr 				= (SayHelloPtr)				(void *) CFBundleGetFunctionPointerForName(theBundle, CFSTR("SayHello"));
			theCreateSharedMemoryPtr 	= (CreateSharedMemoryPtr)	(void *) CFBundleGetFunctionPointerForName(theBundle, CFSTR("CreateSharedMemory"));
			theLockSharedMemoryPtr 		= (LockSharedMemoryPtr)		(void *) CFBundleGetFunctionPointerForName(theBundle, CFSTR("LockSharedMemory"));
			theUnlockSharedMemoryPtr 	= (UnlockSharedMemoryPtr)	(void *) CFBundleGetFunctionPointerForName(theBundle, CFSTR("UnlockSharedMemory"));
			theWriteSharedMemoryPtr 	= (WriteSharedMemoryPtr)	(void *) CFBundleGetFunctionPointerForName(theBundle, CFSTR("WriteSharedMemory"));
			theWriteSharedMemoryBytePtr = (WriteSharedMemoryBytePtr)(void *) CFBundleGetFunctionPointerForName(theBundle, CFSTR("WriteSharedMemoryByte"));
			theReadSharedMemoryPtr 		= (ReadSharedMemoryPtr)		(void *) CFBundleGetFunctionPointerForName(theBundle, CFSTR("ReadSharedMemory"));
			theReadSharedMemoryBytePtr 	= (ReadSharedMemoryBytePtr)	(void *) CFBundleGetFunctionPointerForName(theBundle, CFSTR("ReadSharedMemoryByte"));
			theDestroySharedMemoryPtr 	= (DestroySharedMemoryPtr)	(void *) CFBundleGetFunctionPointerForName(theBundle, CFSTR("DestroySharedMemory"));
			theClearSharedMemoryPtr 	= (ClearSharedMemoryPtr)	(void *) CFBundleGetFunctionPointerForName(theBundle, CFSTR("ClearSharedMemory"));
		}
		
		
	}
}
