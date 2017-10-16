// ControlPanel.cpp
//

//#include "rb_plugin.h"

#include <string.h>
#include <stdio.h>

#include "ControlPanelLib.h"

#include "SharedMemory.h"

//#include "TT's Plugin Support.h"

#include "IKCommon.h"
#include "IKString.h"
#include "IKUtil.h"
#include "IKFile.h"
#include "AppLib.h"

//#define DEBUG 1

#if TARGET_OS_WIN32

	#include "ExecImageVersion.h"

	struct SharedMemBlock ControlPanelChannel = {"6789",0,0,6789,0,0,"ControlPanelChannel"};
	struct SharedMemBlock EngineChannel       = {"8765",0,0,8765,0,0,"EngineChannel"};

	struct SharedMemBlock ControlPanelChannelVista = {"Global\\6789",0,0,6789,0,0,"Global\\ControlPanelChannel"};
	struct SharedMemBlock EngineChannelVista       = {"Global\\8765",0,0,8765,0,0,"Global\\EngineChannel"};

#else

//#include "callbundle.h"
#include <CFBundle.h>


static SharedMemBlock ControlPanelChannel  = {"6789",0,0,6789,(char*)-1};
static SharedMemBlock EngineChannel        = {"8765",0,0,8765,(char*)-1};
static SharedMemBlock BalloonChannel       = {"5432",0,0,5432,(char*)-1};
	
////	extern "C" CreateSharedMemoryPtr theCreateSharedMemoryPtr;
////	extern "C" LockSharedMemoryPtr theLockSharedMemoryPtr;
////	extern "C" UnlockSharedMemoryPtr theUnlockSharedMemoryPtr;
////	extern "C" WriteSharedMemoryPtr theWriteSharedMemoryPtr;
////	extern "C" WriteSharedMemoryBytePtr theWriteSharedMemoryBytePtr;
////	extern "C" ReadSharedMemoryPtr theReadSharedMemoryPtr;
////	extern "C" ReadSharedMemoryBytePtr theReadSharedMemoryBytePtr;
////	extern "C" DestroySharedMemoryPtr theDestroySharedMemoryPtr;
////	extern "C" ClearSharedMemoryPtr theClearSharedMemoryPtr;
#endif

////
static int add5func(int v)
{
	return v + 5;
}
//load bundle function
static void loadBundlefunc(void){
#if TARGET_OS_WIN32
//do nothing, this will load an instance of our dll
#else
////	DoLoadBundle();
#endif
}


extern "C" int addFunction( int a, int b ) 
{ 
    return a + b;
}
extern "C" int stringLength( char *str ) 
{ 
    return strlen(str);
}

extern "C" char *stringCat( const char *str ) 
{
    const char *catstr = "_kitty";
    char * newstr = (char*) malloc (strlen (str) + strlen (catstr) + 1);
    strcpy (newstr, str);
    strcat (newstr, catstr);
    return newstr;
}

extern "C" void freeMemoryBlock(void* ptr) 
{ 
    free (ptr);
}

extern "C" SharedMemBlock* ChooseMemoryBlock ( int channel )
{
#if TARGET_OS_WIN32

	if (IKUtil::IsWinVistaOrGreater())
	{
		if (channel==ENGINE_CHANNEL)
			return &EngineChannelVista;
		return &ControlPanelChannelVista;		
	}
	else
	{
		if (channel==ENGINE_CHANNEL)
			return &EngineChannel;
		return &ControlPanelChannel;
	}

#else

	if (channel==ENGINE_CHANNEL)
		return &EngineChannel;
	if (channel==BALLOON_CHANNEL)
		return &BalloonChannel;
	return &ControlPanelChannel;

#endif

}

//create shared memory
extern "C"  int doCreateSharedMemory (int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);
	
#if TARGET_OS_WIN32
	return CreateSharedMemory(tempBlock);
#else
#if defined (DEBUG_CONTROL_PANEL)|defined (DEBUG_IKUSB)
    NSLog (@"doCreateSharedMemory: channel is (%d); key is (%d); address is (%p);", channel, tempBlock->memoryKey, tempBlock->address);
#endif
	////return ::theCreateSharedMemoryPtr(tempBlock);
    int result = CreateSharedMemory(tempBlock);

#if defined (DEBUG_CONTROL_PANEL)|defined (DEBUG_IKUSB)
    NSLog (@"doCreateSharedMemory: channel is (%d); key is (%d); address is (%p);", channel, tempBlock->memoryKey, tempBlock->address);
#endif

	return result;
#endif	

}


//lock shared memory
extern "C"  int doLockSharedMemory (int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);
	
#if TARGET_OS_WIN32
	return LockSharedMemory(tempBlock);
#else
	////return ::theLockSharedMemoryPtr(tempBlock);	
	return LockSharedMemory(tempBlock);
#endif

}

//unlock shared memory
extern "C"  int doUnlockSharedMemory (int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

#if TARGET_OS_WIN32
	return UnlockSharedMemory(tempBlock);
#else
	////return ::theUnlockSharedMemoryPtr(tempBlock);
	return UnlockSharedMemory(tempBlock);
#endif	

}


//write to our shared memory
extern "C" int doWriteSharedMemory (int channel,int offset, CString rbstring)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);
	
	char *somestring = (char *)REALCString(rbstring);

#ifdef DEBUG_CONTROL_PANEL
    NSLog (@"doWriteSharedMemory: channel is (%d); key is (%d); address is (%p); string is (%s)", channel, tempBlock->memoryKey, tempBlock->address, somestring);
#endif

#if TARGET_OS_WIN32
	return WriteSharedMemory(tempBlock,offset,somestring);	
#else
	////return ::theWriteSharedMemoryPtr(tempBlock,offset,somestring);
	return WriteSharedMemory(tempBlock,offset,somestring);	
#endif	

}

//write a byte to our shared memory
extern "C" int doWriteSharedMemoryByte (int channel,int offset,int byteVal)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

#ifdef DEBUG_CONTROL_PANEL
    if (1 || byteVal) {
        NSLog (@"doWriteSharedMemoryByte: channel is (%d); offset is (%d); key is (%d); address is (%p); byteval is (%x, %c)", channel, offset, tempBlock->memoryKey, tempBlock->address, byteVal, byteVal);
    }
#endif

#if TARGET_OS_WIN32
	return WriteSharedMemoryByte(tempBlock,offset,byteVal);	
#else
	////return ::theWriteSharedMemoryBytePtr(tempBlock,offset,byteVal);
	return WriteSharedMemoryByte(tempBlock,offset,byteVal);	
#endif	
}

//read from our shared memory
extern "C" CString doReadSharedMemory (int channel,int offset)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);
    char readBuffer [1500];

#ifdef DEBUG_CONTROL_PANEL
    NSLog (@"doReadSharedMemory: channel is (%d); key is (%d); address is (%p)", channel, tempBlock->memoryKey, tempBlock->address);
#endif

#if TARGET_OS_WIN32
	 ReadSharedMemory(tempBlock,offset,(char *)myString);
#else	
    ////::theReadSharedMemoryPtr(tempBlock,offset,(char *)myString);
    ReadSharedMemory(tempBlock,offset,(char *)readBuffer);
#endif
    int dataLength = strlen (readBuffer);
    if (dataLength > sizeof (readBuffer)) {
        dataLength = sizeof (readBuffer);
    }
    char *result = (char *) malloc (dataLength + 1);
    strcpy (result, readBuffer);
////////return REALBuildString((char *)myString,myLength);
    return (CString) result;
}

//read a byte from our shared memory
extern "C" int doReadSharedMemoryByte (int channel,int offset)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);
    
#ifdef DEBUG_CONTROL_PANEL
    NSLog (@"doReadSharedMemoryByte: channel is (%d); offset is (%d); key is (%d); address is (%p)", channel, offset, tempBlock->memoryKey, tempBlock->address);
#endif

#if TARGET_OS_WIN32
	return ReadSharedMemoryByte(tempBlock,offset);
#else
	 ////return ::theReadSharedMemoryBytePtr(tempBlock,offset);
	return ReadSharedMemoryByte(tempBlock,offset);
#endif

}

//destroy our shared memory
extern "C" int doDestroySharedMemory (int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

#if TARGET_OS_WIN32
	return DestroySharedMemory(tempBlock);	
#else
	////return ::theDestroySharedMemoryPtr(tempBlock);
	return DestroySharedMemory(tempBlock);	
#endif
	
}

//clear our shared memory
extern "C" int doClearSharedMemory (int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

#if TARGET_OS_WIN32
	return ClearSharedMemory(tempBlock);	
#else
	////return ::theClearSharedMemoryPtr(tempBlock);
	return ClearSharedMemory(tempBlock);	
#endif	

}

//see if a channel is alive on the win side
extern "C" int doIsOwnerAlive (int channel){

#if TARGET_OS_WIN32
char szMutexName[255];
//strcpy(szMutexName,"IKUSB Mutex - ");
if(channel == 0){
	strcpy(szMutexName,"IKUSB Mutex - control panel\0");
}else{
	strcpy(szMutexName,"IKUSB Mutex - engine\0");
	}		
    int bAlive = 0;
    HANDLE mutex = CreateMutex(NULL,FALSE,szMutexName);
    if (mutex){
	DWORD result = WaitForSingleObject(mutex,0);
	if(result==WAIT_TIMEOUT){
		bAlive = 1;
	}
	ReleaseMutex(mutex);
}
return bAlive;



#else// not needed on the mac side
	return 1;
#endif

}

//make a channel alive on the win side
extern "C" int doOwnChannel (int channel)
{

#if TARGET_OS_WIN32
    char szMutexName[255];
    //strcpy(szMutexName,"IKUSB Mutex - ");
    if(channel == 0){
        strcpy(szMutexName,"IKUSB Mutex - control panel\0");
    }else{
        strcpy(szMutexName,"IKUSB Mutex - engine\0");
        
    }	
    HANDLE mutex = CreateMutex(NULL,TRUE,szMutexName);
    return (int)mutex;
#else// not needed on the mac side
	return 1;
#endif

}


extern "C" CString doGetAppFriendlyName ( const CString rbAppName )
{	
	//  original path as an IK string
	IKString path((char *)REALCString(rbAppName));
	
	//  get the friendly name
	IKString friendly = IKUtil::GetAppFriendlyName(path);
	
	//  return in a REAL string
	////////return REALBuildString((char *)friendly,friendly.GetLength());
    char *result = (char *) malloc (friendly.GetLength () + 1);
    strcpy (result, (char *) friendly);
    return (CString) result;
}

extern "C" CString doGetFileName ( CString rbPath )
{	
	//  original path as an IK string
	IKString path((char *)REALCString(rbPath));
	
	//  get the friendly name
	IKString name = path;
	IKUtil::StripFileName(name,true,false);
	
	//  return in a REAL string
	////////return REALBuildString((char *)name,name.GetLength());
    char *result = (char *) malloc (name.GetLength () + 1);
    strcpy (result, (char *) name);
    return (CString) result;
}

extern "C" int doAppLibAddSystem ( CString SysName, CString SysIdentifier )
{
//	char *pSysName       = (char *)REALCString(SysName);
//	char *pSysIdentifier = (char *)REALCString(SysIdentifier);

	int result = AppLibAddSystem ( SysName,  SysIdentifier);
	return result;
}

extern "C" int doAppLibRemoveSystem( CString SystemName )
{
	return AppLibRemoveSystem(SystemName);
}

extern "C" bool doAppLibCanListApplication( CString AppName )
{
	return AppLibCanListApplication((char *)REALCString(AppName));
}

extern "C" int doAppLibCountSystems()
{
	return AppLibCountSystems();
}

extern "C" int doIsDiscoverInstalled ()
{
	int nSys = AppLibCountSystems();
	for (int i=0;i<nSys;i++)
	{
		IKString result = AppLibGetNthSystem(i);
		if (result.CompareNoCase(TEXT("Discover"))==0)
			return 1;
	}
	
	return 0;
}

extern "C" CString	doAppLibGetNthSystem( int n )
{
	IKString appSystem = AppLibGetNthSystem(n);
	////////return REALBuildString ((char *) result, result.GetLength());
    char *result = (char *) malloc (appSystem.GetLength () + 1);
    strcpy (result, (char *) appSystem);
    return (CString) result;
}

extern "C" int doAppLibAddApplication( CString AppPath )
{
	return AppLibAddApplication ((char *)REALCString(AppPath));
}

extern "C" int doAppLibRemoveApplication( CString AppPath, CString SystemName )
{
	return AppLibRemoveApplication ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

extern "C" int doAppLibTakeOwnership( CString AppPath, CString SystemName )
{
	return AppLibTakeOwnership ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

extern "C" int doAppLibHasOwnership( CString AppPath, CString SystemName )
{
	return AppLibHasOwnership ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

extern "C" CString	doAppLibGetOwner( CString AppPath )
{
	char *pAppPath       = (char *)REALCString(AppPath);
	
	IKString result = AppLibGetOwner(pAppPath);
	////////return REALBuildString ((char *) result, result.GetLength());
    char *stringResult = (char *) malloc (result.GetLength () + 1);
    strcpy (stringResult, (char *) result);
    return (CString) stringResult;
}

extern "C" int doAppLibReleaseOwnership( CString AppPath, CString SystemName )
{
	return AppLibReleaseOwnership ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

extern "C" int doAppLibCountApplications()
{
	return AppLibCountApplications();
}

extern "C" CString	doAppLibGetNthApplication( int n )
{
	IKString result = AppLibGetNthApplication(n);
	////////return REALBuildString ((char *) result, result.GetLength());
    char *stringResult = (char *) malloc (result.GetLength () + 1);
    strcpy (stringResult, (char *) result);
    return (CString) stringResult;
}

extern "C" bool doAppLibLock( int timeout )
{
	return AppLibLock(timeout);
}

extern "C" bool doAppLibUnlock()
{
	return AppLibUnlock();
}

extern "C" bool doAppLibShowDiscoverAnyway()
{
	return AppLibShowDiscoverAnyway();
}

extern "C" CString	doAppLibGetAppFriendlyName( CString AppPath )
{
	char *pAppPath       = (char *)REALCString(AppPath);
	
	char * result = AppLibGetAppFriendlyName(pAppPath);
	////////return REALBuildString ( result, strlen(result));
    return (CString) result;
}

extern "C" int doAppLibMaintenance()
{
	return AppLibMaintenance();
}

//------------------------
//   added draft 3

extern "C" CString doAppLibGetStudentAttachedOverlay ( CString AppPath, CString GroupName, CString StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	IKString result = AppLibGetStudentAttachedOverlay( pAppPath, pGroupName, pStudentName );
	////////return REALBuildString ((char *) result, result.GetLength());
    char *stringResult = (char *) malloc (result.GetLength () + 1);
    strcpy (stringResult, (char *) result);
    return (CString) stringResult;
}

extern "C" int doAppLibCountAttachedOverlays ( CString AppPath, CString GroupName, CString StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	int result = AppLibCountAttachedOverlays ( pAppPath, pGroupName, pStudentName );
	return result;
}

extern "C" int doAppLibGetSelectedOverlay ( CString AppPath, CString GroupName, CString StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	int result = AppLibGetSelectedOverlay ( pAppPath, pGroupName, pStudentName );
	return result;
}

extern "C" CString doAppLibGetNumberedOverlay ( CString AppPath, CString GroupName, CString StudentName, int OverlayNumber)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	IKString result = AppLibGetNumberedOverlay(pAppPath, pGroupName, pStudentName, OverlayNumber );
	////////return REALBuildString ((char *) result, result.GetLength());
    char *stringResult = (char *) malloc (result.GetLength () + 1);
    strcpy (stringResult, (char *) result);
    return (CString) stringResult;
}

extern "C" int doAppLibAddApplicationForStudent ( CString AppPath, CString GroupName, CString StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	int result = AppLibAddApplicationForStudent ( pAppPath, pGroupName, pStudentName );
	return result;
}

extern "C" int doAppLibRemoveApplicationForStudent ( CString AppPath, CString GroupName, CString StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	int result = AppLibRemoveApplicationForStudent ( pAppPath, pGroupName, pStudentName );
	return result;
}

extern "C" int doAppLibSetSelectedOverlay ( CString AppPath, CString GroupName, CString StudentName, int OverlayNumber)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	int result = AppLibSetSelectedOverlay ( pAppPath, pGroupName, pStudentName, OverlayNumber );
	return result;
}

extern "C" int doAppLibAttachOverlayForStudent ( CString AppPath, CString GroupName, CString StudentName, CString OverlayPath)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	char *pOverlayPath = (char *)REALCString(OverlayPath);

	int result = AppLibAttachOverlayForStudent ( pAppPath, pGroupName, pStudentName, pOverlayPath );
	return result;
}

extern "C" int doAppLibDetachOverlayForStudent ( CString AppPath, CString GroupName, CString StudentName, CString OverlayPath)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	char *pOverlayPath = (char *)REALCString(OverlayPath);

	int result = AppLibDetachOverlayForStudent ( pAppPath, pGroupName, pStudentName, pOverlayPath );
	return result;
}

extern "C" int doAppLibAddPreinstalledOverlays ( CString GroupName, CString StudentName, bool bForce)
{
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	AppLibAddPreinstalledOverlays ( pGroupName, pStudentName, bForce );
	
	return 1;
}


extern "C" Boolean doAppLibIsAppAllowed ( CString AppPath )
{
	char *pAppPath   = (char *)REALCString(AppPath);
	
	return AppLibIsAppAllowed ( pAppPath );
}


extern "C" void doAppLibSetAppAllowed (  CString AppPath, Boolean bAllowed)
{
	char *pAppPath   = (char *)REALCString(AppPath);
	
	AppLibSetAppAllowed ( pAppPath, bAllowed );

}


extern "C" int doAppLibCountStandardApps ()
{
	int nApps = AppLibCountStandardApps();
	
	return nApps;
}

extern "C" CString	doAppLibGetNthStandardApp		( int n )
{
	IKString result = AppLibGetNthStandardApp(n);
	////return REALBuildString ((char *) result, result.GetLength());
    char *stringResult = (char *) malloc (result.GetLength () + 1);
    strcpy (stringResult, (char *) result);
    return (CString) stringResult;
}


extern "C" CString	doAppLibGetTruePath		(  CString AppPath )
{
	char *pAppPath = (char *)REALCString(AppPath);
	
	char *result = AppLibGetTruePath ( pAppPath );
	
	////return REALBuildString (result, strlen(result));
    char *stringResult = (char *) malloc (strlen (result) + 1);
    strcpy (stringResult, (char *) result);
    return (CString) stringResult;
}

extern "C" Boolean doAppLibHasSystemInterest ( CString SysName, CString SysIdentifier )
{
	char *pSysName       = (char *)REALCString(SysName);
	char *pSysIdentifier = (char *)REALCString(SysIdentifier);

	bool result = AppLibHasSystemInterest ( pSysName,  pSysIdentifier);
	return result;
}

extern "C" void doAppLibSetSystemInterest ( CString SysName, CString SysIdentifier, Boolean bInterest )
{
	char *pSysName       = (char *)REALCString(SysName);
	char *pSysIdentifier = (char *)REALCString(SysIdentifier);

	AppLibSetSystemInterest ( pSysName,  pSysIdentifier, bInterest);
}

extern "C" bool doAppLibIsOverlayPreinstalled ( CString OverlayPath )
{
	char *pOverlayPath       = (char *)REALCString(OverlayPath);
	
	return AppLibIsOverlayPreinstalled ( pOverlayPath );
}

extern "C" bool doIsOverlayMakerInstalled ()
{	
	return IKUtil::IsOverlayMakerInstalled ();
}

extern "C" int doAppLibUpdateAppPath ( CString oldPath, CString newPath )
{
	char *poldPath       = (char *)REALCString(oldPath);
	char *pnewPath		 = (char *)REALCString(newPath);
	
	int result = AppLibUpdateAppPath ( poldPath, pnewPath );
	return result;
}

extern "C" bool doAppLibIsIntelliSwitchInstalled ()
{	
	return AppLibIsIntelliSwitchInstalled();
}

extern "C" bool doAppLibIsIntellikeysInstalled ()
{	
	return AppLibIsIntellikeysInstalled();
}

extern "C" bool doFileExists ( CString path )
{
	char *pPath = (char *)REALCString(path);
	
	return IKFile::FileExists(pPath);
}

extern "C" CString doHFSToUnix ( CString path )
{
	char *pPath       = (char *)REALCString(path);
	IKString strPath = pPath;

#if TARGET_OS_WIN32
	////return REALBuildString ((char *) strPath, strPath.GetLength());
#else
	IKString result = IKFile::HFSToUnix(strPath);
	////return REALBuildString ((char *) result, result.GetLength());
    char *stringResult = (char *) malloc (result.GetLength () + 1);
    strcpy (stringResult, (char *) result);
    return (CString) stringResult;
#endif
}

extern "C" int doGetIntData ( CString key, int defaultVal )
{
	char *pKey = (char *)REALCString(key);
	int val = DATAI ( pKey, defaultVal );
	return val;
}

	
extern "C" CString doGetOMOverlaysFolder ( )
{
	IKString result = IKUtil::GetOMOverlaysFolder();
	
	////return REALBuildString ((char *) result, result.GetLength());
    char *stringResult = (char *) malloc (result.GetLength () + 1);
    strcpy (stringResult, (char *) result);
    return (CString) stringResult;
}

extern "C" int doAppLibIsSystemActive			( CString SystemName )
{
	return  AppLibIsSystemActive((char *)REALCString(SystemName));
}

extern "C" bool doAppLibIsSystemPresent			( CString SystemName )
{
	return  AppLibIsSystemPresent((char *)REALCString(SystemName));
}

extern "C" int doAppLibSetSystemActive			( CString SystemName )
{
	return AppLibSetSystemActive ((char *)REALCString(SystemName));
}

extern "C" int doAppLibSetSystemInactive			( CString SystemName )
{
	return AppLibSetSystemInactive ((char *)REALCString(SystemName));
}

extern "C" bool IsControlPanelRunning			( )
{

	IKString name;
	name = DATAS(TEXT("control_panel_name_1"),TEXT(""));
	if (!name.IsEmpty())
		if (IKUtil::IsAppRunning(name))
			return true;
	name = DATAS(TEXT("control_panel_name_2"),TEXT(""));
	if (!name.IsEmpty())
		if (IKUtil::IsAppRunning(name))
			return true;
	name = DATAS(TEXT("control_panel_name_3"),TEXT(""));
	if (!name.IsEmpty())
		if (IKUtil::IsAppRunning(name))
			return true;
	return false;

}


//------------------------

	
void PluginExit (void)
{
	/*
	 * This function gets called when the app that used this plugin terminates.
	 * You can do your cleanup here in case you did open persistent resources
	 * (like having a Timer Task or asynchronous functions pending).
	 */
}

void PluginEntry(void)
{	

}
