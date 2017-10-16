// testPlugin.cpp
//
//	This file just puts various features of the SDK through their paces.  It's used
//	mainly for internal testing, so the code isn't as pretty as some of the other
//	examples.

#include "rb_plugin.h"

#include <string.h>
#include <stdio.h>

#include "sharedMemory.h"

#include "TT's Plugin Support.h"

#include "IKCommon.h"
#include "IKString.h"
#include "IKUtil.h"
#include "IKFile.h"
#include "AppLib.h"


#if TARGET_OS_WIN32

	#include "ExecImageVersion.h"

	struct SharedMemBlock ControlPanelChannel = {"6789",0,0,6789,0,0,"ControlPanelChannel"};
	struct SharedMemBlock EngineChannel       = {"8765",0,0,8765,0,0,"EngineChannel"};

	struct SharedMemBlock ControlPanelChannelVista = {"Global\\6789",0,0,6789,0,0,"Global\\ControlPanelChannel"};
	struct SharedMemBlock EngineChannelVista       = {"Global\\8765",0,0,8765,0,0,"Global\\EngineChannel"};

#else

#include "callbundle.h"
#include <CFBundle.h>


	struct SharedMemBlock ControlPanelChannel = {"6789",0,0,6789,0};
	struct SharedMemBlock EngineChannel = {"8765",0,0,8765,0};
	struct SharedMemBlock BalloonChannel = {"5432",0,0,5432,0};
	
	extern "C" CreateSharedMemoryPtr theCreateSharedMemoryPtr;
	extern "C" LockSharedMemoryPtr theLockSharedMemoryPtr;
	extern "C" UnlockSharedMemoryPtr theUnlockSharedMemoryPtr;
	extern "C" WriteSharedMemoryPtr theWriteSharedMemoryPtr;
	extern "C" WriteSharedMemoryBytePtr theWriteSharedMemoryBytePtr;
	extern "C" ReadSharedMemoryPtr theReadSharedMemoryPtr;
	extern "C" ReadSharedMemoryBytePtr theReadSharedMemoryBytePtr;
	extern "C" DestroySharedMemoryPtr theDestroySharedMemoryPtr;
	extern "C" ClearSharedMemoryPtr theClearSharedMemoryPtr;
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
	DoLoadBundle();
#endif
}

SharedMemBlock* ChooseMemoryBlock ( int channel )
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
static int doCreateSharedMemoryfunc(int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);
	
#if TARGET_OS_WIN32
	return CreateSharedMemory(tempBlock);
#else
	return ::theCreateSharedMemoryPtr(tempBlock);
#endif	

}


//lock shared memory
static int doLockSharedMemoryfunc(int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);
	
#if TARGET_OS_WIN32
	return LockSharedMemory(tempBlock);
#else
	return ::theLockSharedMemoryPtr(tempBlock);	
#endif

}

//unlock shared memory
static int doUnlockSharedMemoryfunc(int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

#if TARGET_OS_WIN32
	return UnlockSharedMemory(tempBlock);
#else
	return ::theUnlockSharedMemoryPtr(tempBlock);
#endif	

}


//write to our shared memory
static int doWriteSharedMemoryfunc(int channel,int offset,REALstring rbstring)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);
	
	char *somestring = (char *)REALCString(rbstring);

#if TARGET_OS_WIN32
	return WriteSharedMemory(tempBlock,offset,somestring);	
#else
	return ::theWriteSharedMemoryPtr(tempBlock,offset,somestring);
#endif	

}

//write a byte to our shared memory
static int doWriteSharedMemoryBytefunc(int channel,int offset,int byteVal)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

#if TARGET_OS_WIN32
	return WriteSharedMemoryByte(tempBlock,offset,byteVal);	
#else
	return ::theWriteSharedMemoryBytePtr(tempBlock,offset,byteVal);
#endif	
}

//read from our shared memory
static REALstring doReadSharedMemoryfunc(int channel,int offset)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

char *myString="";
	int myLength;
	int i;
#if TARGET_OS_WIN32
	 ReadSharedMemory(tempBlock,offset,(char *)myString);
#else
	
	 ::theReadSharedMemoryPtr(tempBlock,offset,(char *)myString);

#endif
	 //i have a problem with <string.h> so I'm making my own strlen()
i=0;
while(myString[i] != '\0')
	i++;
//that should be it
myLength = i;
return REALBuildString((char *)myString,myLength);

}

//read a byte from our shared memory
static int doReadSharedMemoryBytefunc(int channel,int offset)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

#if TARGET_OS_WIN32
	return ReadSharedMemoryByte(tempBlock,offset);
#else
	 return ::theReadSharedMemoryBytePtr(tempBlock,offset);
#endif

}

//destroy our shared memory
static int doDestroySharedMemoryfunc(int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

#if TARGET_OS_WIN32
	return DestroySharedMemory(tempBlock);	
#else
	return ::theDestroySharedMemoryPtr(tempBlock);
#endif
	
}

//clear our shared memory
static int doClearSharedMemoryfunc(int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

#if TARGET_OS_WIN32
	return ClearSharedMemory(tempBlock);	
#else
	return ::theClearSharedMemoryPtr(tempBlock);
#endif	

}

//see if a channel is alive on the win side
static int doIsOwnerAlivefunc(int channel){

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
static int doOwnChannelfunc(int channel)
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


static REALstring doGetAppFriendlyNamefunc ( REALstring rbAppName )
{	
	//  original path as an IK string
	IKString path((char *)REALCString(rbAppName));
	
	//  get the friendly name
	IKString friendly = IKUtil::GetAppFriendlyName(path);
	
	//  return in a REAL string
	return REALBuildString((char *)friendly,friendly.GetLength());

}

static REALstring doGetFileNamefunc ( REALstring rbPath )
{	
	//  original path as an IK string
	IKString path((char *)REALCString(rbPath));
	
	//  get the friendly name
	IKString name = path;
	IKUtil::StripFileName(name,true,false);
	
	//  return in a REAL string
	return REALBuildString((char *)name,name.GetLength());

}

static int doAppLibAddSystemfunc ( REALstring SysName, REALstring SysIdentifier )
{
	char *pSysName       = (char *)REALCString(SysName);
	char *pSysIdentifier = (char *)REALCString(SysIdentifier);

	int result = AppLibAddSystem ( pSysName,  pSysIdentifier);
	return result;
}

static int		doAppLibRemoveSystemfunc			( REALstring SystemName )
{
	return AppLibRemoveSystem((char *)REALCString(SystemName));
}

static bool		doAppLibCanListApplicationfunc			( REALstring AppName )
{
	return AppLibCanListApplication((char *)REALCString(AppName));
}

static int		doAppLibCountSystemsfunc			()
{
	return AppLibCountSystems();
}

static int		doIsDiscoverInstalledfunc ()
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

static REALstring	doAppLibGetNthSystemfunc			( int n )
{
	IKString result = AppLibGetNthSystem(n);
	return REALBuildString ((char *) result, result.GetLength());
}

static int		doAppLibAddApplicationfunc			( REALstring AppPath )
{
	return AppLibAddApplication ((char *)REALCString(AppPath));
}

static int		doAppLibRemoveApplicationfunc		( REALstring AppPath, REALstring SystemName )
{
	return AppLibRemoveApplication ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

static int		doAppLibTakeOwnershipfunc			( REALstring AppPath, REALstring SystemName )
{
	return AppLibTakeOwnership ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

static int		doAppLibHasOwnershipfunc			( REALstring AppPath, REALstring SystemName )
{
	return AppLibHasOwnership ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

static REALstring	doAppLibGetOwnerfunc				( REALstring AppPath )
{
	char *pAppPath       = (char *)REALCString(AppPath);
	
	IKString result = AppLibGetOwner(pAppPath);
	return REALBuildString ((char *) result, result.GetLength());
}

static int		doAppLibReleaseOwnershipfunc		( REALstring AppPath, REALstring SystemName )
{
	return AppLibReleaseOwnership ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

static int		doAppLibCountApplicationsfunc		()
{
	return AppLibCountApplications();
}

static REALstring	doAppLibGetNthApplicationfunc		( int n )
{
	IKString result = AppLibGetNthApplication(n);
	return REALBuildString ((char *) result, result.GetLength());
}

static bool		doAppLibLockfunc					( int timeout )
{
	return AppLibLock(timeout);
}

static bool		doAppLibUnlockfunc					()
{
	return AppLibUnlock();
}

static bool		doAppLibShowDiscoverAnywayfunc					()
{
	return AppLibShowDiscoverAnyway();
}

static REALstring	doAppLibGetAppFriendlyNamefunc		( REALstring AppPath )
{
	char *pAppPath       = (char *)REALCString(AppPath);
	
	char * result = AppLibGetAppFriendlyName(pAppPath);
	return REALBuildString ( result, strlen(result));
}

static int		doAppLibMaintenancefunc				()
{
	return AppLibMaintenance();
}

//------------------------
//   added draft 3

static REALstring doAppLibGetStudentAttachedOverlayfunc ( REALstring AppPath, REALstring GroupName, REALstring StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	IKString result = AppLibGetStudentAttachedOverlay( pAppPath, pGroupName, pStudentName );
	return REALBuildString ((char *) result, result.GetLength());
}

static int doAppLibCountAttachedOverlaysfunc ( REALstring AppPath, REALstring GroupName, REALstring StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	int result = AppLibCountAttachedOverlays ( pAppPath, pGroupName, pStudentName );
	return result;
}

static int doAppLibGetSelectedOverlayfunc ( REALstring AppPath, REALstring GroupName, REALstring StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	int result = AppLibGetSelectedOverlay ( pAppPath, pGroupName, pStudentName );
	return result;
}

static REALstring doAppLibGetNumberedOverlayfunc ( REALstring AppPath, REALstring GroupName, REALstring StudentName, int OverlayNumber)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	IKString result = AppLibGetNumberedOverlay(pAppPath, pGroupName, pStudentName, OverlayNumber );
	return REALBuildString ((char *) result, result.GetLength());
}

static int doAppLibAddApplicationForStudentfunc ( REALstring AppPath, REALstring GroupName, REALstring StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	int result = AppLibAddApplicationForStudent ( pAppPath, pGroupName, pStudentName );
	return result;
}

static int doAppLibRemoveApplicationForStudentfunc ( REALstring AppPath, REALstring GroupName, REALstring StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	int result = AppLibRemoveApplicationForStudent ( pAppPath, pGroupName, pStudentName );
	return result;
}

static int doAppLibSetSelectedOverlayfunc ( REALstring AppPath, REALstring GroupName, REALstring StudentName, int OverlayNumber)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	int result = AppLibSetSelectedOverlay ( pAppPath, pGroupName, pStudentName, OverlayNumber );
	return result;
}

static int doAppLibAttachOverlayForStudentfunc ( REALstring AppPath, REALstring GroupName, REALstring StudentName, REALstring OverlayPath)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	char *pOverlayPath = (char *)REALCString(OverlayPath);

	int result = AppLibAttachOverlayForStudent ( pAppPath, pGroupName, pStudentName, pOverlayPath );
	return result;
}

static int doAppLibDetachOverlayForStudentfunc ( REALstring AppPath, REALstring GroupName, REALstring StudentName, REALstring OverlayPath)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	char *pOverlayPath = (char *)REALCString(OverlayPath);

	int result = AppLibDetachOverlayForStudent ( pAppPath, pGroupName, pStudentName, pOverlayPath );
	return result;
}

static int doAppLibAddPreinstalledOverlaysfunc ( REALstring GroupName, REALstring StudentName, bool bForce)
{
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	
	AppLibAddPreinstalledOverlays ( pGroupName, pStudentName, bForce );
	
	return 1;
}


static Boolean doAppLibIsAppAllowedfunc ( REALstring AppPath )
{
	char *pAppPath   = (char *)REALCString(AppPath);
	
	return AppLibIsAppAllowed ( pAppPath );
}


static void doAppLibSetAppAllowedfunc (  REALstring AppPath, Boolean bAllowed)
{
	char *pAppPath   = (char *)REALCString(AppPath);
	
	AppLibSetAppAllowed ( pAppPath, bAllowed );

}


static int doAppLibCountStandardAppsfunc ()
{
	int nApps = AppLibCountStandardApps();
	
	return nApps;
}

static REALstring	doAppLibGetNthStandardAppfunc		( int n )
{
	IKString result = AppLibGetNthStandardApp(n);
	return REALBuildString ((char *) result, result.GetLength());
}


static REALstring	doAppLibGetTruePathfunc		(  REALstring AppPath )
{
	char *pAppPath   = (char *)REALCString(AppPath);
	
	char *result = AppLibGetTruePath ( pAppPath );
	
	return REALBuildString (result, strlen(result));
}

static Boolean doAppLibHasSystemInterestfunc ( REALstring SysName, REALstring SysIdentifier )
{
	char *pSysName       = (char *)REALCString(SysName);
	char *pSysIdentifier = (char *)REALCString(SysIdentifier);

	bool result = AppLibHasSystemInterest ( pSysName,  pSysIdentifier);
	return result;
}

static void doAppLibSetSystemInterestfunc ( REALstring SysName, REALstring SysIdentifier, Boolean bInterest )
{
	char *pSysName       = (char *)REALCString(SysName);
	char *pSysIdentifier = (char *)REALCString(SysIdentifier);

	AppLibSetSystemInterest ( pSysName,  pSysIdentifier, bInterest);
}

static bool doAppLibIsOverlayPreinstalledfunc ( REALstring OverlayPath )
{
	char *pOverlayPath       = (char *)REALCString(OverlayPath);
	
	return AppLibIsOverlayPreinstalled ( pOverlayPath );
}

static bool doIsOverlayMakerInstalledfunc ()
{	
	return IKUtil::IsOverlayMakerInstalled ();
}

static int doAppLibUpdateAppPathfunc ( REALstring oldPath, REALstring newPath )
{
	char *poldPath       = (char *)REALCString(oldPath);
	char *pnewPath		 = (char *)REALCString(newPath);
	
	int result = AppLibUpdateAppPath ( poldPath, pnewPath );
	return result;
}

static bool doAppLibIsIntelliSwitchInstalledfunc ()
{	
	return AppLibIsIntelliSwitchInstalled();
}

static bool doAppLibIsIntellikeysInstalledfunc ()
{	
	return AppLibIsIntellikeysInstalled();
}

static bool doFileExistsfunc ( REALstring path )
{
	char *pPath       = (char *)REALCString(path);
	
	return IKFile::FileExists(pPath);
}

static REALstring doHFSToUnixfunc ( REALstring path )
{
	char *pPath       = (char *)REALCString(path);
	IKString strPath = pPath;

#if TARGET_OS_WIN32
	return REALBuildString ((char *) strPath, strPath.GetLength());
#else
	IKString result = IKFile::HFSToUnix(strPath);
	return REALBuildString ((char *) result, result.GetLength());
#endif
}

static int doGetIntDatafunc ( REALstring key, int defaultVal )
{
	char *pKey = (char *)REALCString(key);
	int val = DATAI ( pKey, defaultVal );
	return val;
}

	
static REALstring doGetOMOverlaysFolderfunc ( )
{
	IKString result = IKUtil::GetOMOverlaysFolder();
	
	return REALBuildString ((char *) result, result.GetLength());
}

static int		doAppLibIsSystemActivefunc			( REALstring SystemName )
{
	return  AppLibIsSystemActive((char *)REALCString(SystemName));
}

static bool		doAppLibIsSystemPresentfunc			( REALstring SystemName )
{
	return  AppLibIsSystemPresent((char *)REALCString(SystemName));
}

static int		doAppLibSetSystemActivefunc			( REALstring SystemName )
{
	return AppLibSetSystemActive ((char *)REALCString(SystemName));
}

static int		doAppLibSetSystemInactivefunc			( REALstring SystemName )
{
	return AppLibSetSystemInactive ((char *)REALCString(SystemName));
}

static bool		IsControlPanelRunningfunc			( )
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




REALmethodDefinition add5defn = {
	(REALproc) add5func,
	REALnoImplementation,
	"add5(v as integer) as integer"
};
//loadBundle def
REALmethodDefinition loadBundledefn = {
	(REALproc) loadBundlefunc,
	REALnoImplementation,
	"InitializeSharedMemPlugin"
};

//create shared mem def
REALmethodDefinition doCreateSharedMemorydefn = {
	(REALproc) doCreateSharedMemoryfunc,
	REALnoImplementation,
	"doCreateSharedMemory(channel as integer) as integer"
};
//lock shared mem def
REALmethodDefinition doLockSharedMemorydefn = {
	(REALproc) doLockSharedMemoryfunc,
	REALnoImplementation,
	"doLockSharedMemory(channel as integer) as integer"
};
//unlock shared mem def
REALmethodDefinition doUnlockSharedMemorydefn = {
	(REALproc) doUnlockSharedMemoryfunc,
	REALnoImplementation,
	"doUnlockSharedMemory(channel as integer) as integer"
};

//write shared mem def
REALmethodDefinition doWriteSharedMemorydefn = {
	(REALproc) doWriteSharedMemoryfunc,
	REALnoImplementation,
	"doWriteSharedMemory(channel as integer, offset as integer, rbstring as string) as integer"
};
//write shared mem byte def
REALmethodDefinition doWriteSharedMemoryBytedefn = {
	(REALproc) doWriteSharedMemoryBytefunc,
	REALnoImplementation,
	"doWriteSharedMemoryByte(channel as integer, offset as integer, byteValue as integer) as integer"
};
//read shared mem def
REALmethodDefinition doReadSharedMemorydefn = {
	(REALproc) doReadSharedMemoryfunc,
	REALnoImplementation,
	"doReadSharedMemory(channel as integer, offset as integer) as string"
};
//read shared mem byte def
REALmethodDefinition doReadSharedMemoryBytedefn = {
	(REALproc) doReadSharedMemoryBytefunc,
	REALnoImplementation,
	"doReadSharedMemoryByte(channel as integer, offset as integer) as integer"
};
//destroy shared mem def
REALmethodDefinition doDestroySharedMemorydefn = {
	(REALproc) doDestroySharedMemoryfunc,
	REALnoImplementation,
	"doDestroySharedMemory(channel as integer) as integer"
};
//destroy shared mem def
REALmethodDefinition doClearSharedMemorydefn = {
	(REALproc) doClearSharedMemoryfunc,
	REALnoImplementation,
	"doClearSharedMemory(channel as integer) as integer"
};
//is owner alive def
REALmethodDefinition doIsOwnerAlivedefn = {
	(REALproc) doIsOwnerAlivefunc,
	REALnoImplementation,
	"doIsOwnerAlive(channel as integer) as integer"
};
//i own the channel def
REALmethodDefinition doOwnChanneldefn = {
	(REALproc) doOwnChannelfunc,
	REALnoImplementation,
	"doOwnChannel(channel as integer) as integer"
};
//get an app's friendly name
REALmethodDefinition doGetAppFriendlyNamedefn = {
	(REALproc) doGetAppFriendlyNamefunc,
	REALnoImplementation,
	"doGetAppFriendlyName(name as String) as String"
};

//get a file's name
REALmethodDefinition doGetFileNamedefn = {
	(REALproc) doGetFileNamefunc,
	REALnoImplementation,
	"doGetFileName(name as String) as String"
};

REALmethodDefinition doFileExistsdefn = { (REALproc) doFileExistsfunc, REALnoImplementation,
	"doFileExists (path as String) as Boolean" };
	
REALmethodDefinition doHFSToUnixdefn = { (REALproc) doHFSToUnixfunc, REALnoImplementation,
	"doHFSToUnix (path as String) as String" };

REALmethodDefinition doGetIntDatadefn = { (REALproc) doGetIntDatafunc, REALnoImplementation,
	"doGetIntData (key as String, defaultVal as Integer) as Integer" };

REALmethodDefinition doIsDiscoverInstalleddefn = { (REALproc) doIsDiscoverInstalledfunc, REALnoImplementation,
	"doIsDiscoverInstalled () as Integer" };
	
REALmethodDefinition doGetOMOverlaysFolderdefn = { (REALproc) doGetOMOverlaysFolderfunc, REALnoImplementation,
	"doGetOMOverlaysFolder () as String" };
	

//  applib functions

REALmethodDefinition doAppLibAddSystemdefn = { (REALproc) doAppLibAddSystemfunc, REALnoImplementation,
	"doAppLibAddSystem(SysName as String, SysIdentifier as String) as Integer" };
	
REALmethodDefinition doAppLibRemoveSystemdefn = { (REALproc) doAppLibRemoveSystemfunc, REALnoImplementation,
	"doAppLibRemoveSystem(SystemName as String) as Integer" };

REALmethodDefinition doAppLibCanListApplicationdefn = { (REALproc) doAppLibCanListApplicationfunc, REALnoImplementation,
	"doAppLibCanListApplication(AppName as String) as Boolean" };
	
REALmethodDefinition doAppLibCountSystemsdefn = { (REALproc) doAppLibCountSystemsfunc, REALnoImplementation,
	"doAppLibCountSystems() as Integer" };
	
REALmethodDefinition doAppLibGetNthSystemdefn = { (REALproc) doAppLibGetNthSystemfunc, REALnoImplementation,
	"doAppLibGetNthSystem(n as Integer) as String" };
	
REALmethodDefinition doAppLibAddApplicationdefn = { (REALproc) doAppLibAddApplicationfunc, REALnoImplementation,
	"doAppLibAddApplication(AppPath as String) as Integer" };
	
REALmethodDefinition doAppLibRemoveApplicationdefn = { (REALproc) doAppLibRemoveApplicationfunc, REALnoImplementation,
	"doAppLibRemoveApplication(AppPath as String, SystemName as String) as Integer" };
	
REALmethodDefinition doAppLibTakeOwnershipdefn = { (REALproc) doAppLibTakeOwnershipfunc, REALnoImplementation,
	"doAppLibTakeOwnership(AppPath as String, SystemName as String) as Integer" };
	
REALmethodDefinition doAppLibHasOwnershipdefn = { (REALproc) doAppLibHasOwnershipfunc, REALnoImplementation,
	"doAppLibHasOwnership(AppPath as String, SystemName as String) as Integer" };
	
REALmethodDefinition doAppLibGetOwnerdefn = { (REALproc) doAppLibGetOwnerfunc, REALnoImplementation,
	"doAppLibGetOwner(AppPath as String) as String" };
	
REALmethodDefinition doAppLibReleaseOwnershipdefn = { (REALproc) doAppLibReleaseOwnershipfunc, REALnoImplementation,
	"doAppLibReleaseOwnership(AppPath as String, SystemName as String) as Integer" };
	
REALmethodDefinition doAppLibCountApplicationsdefn = { (REALproc) doAppLibCountApplicationsfunc, REALnoImplementation,
	"doAppLibCountApplications() as Integer" };
	
REALmethodDefinition doAppLibGetNthApplicationdefn = { (REALproc) doAppLibGetNthApplicationfunc, REALnoImplementation,
	"doAppLibGetNthApplication(n as Integer) as String" };
	
REALmethodDefinition doAppLibLockdefn = { (REALproc) doAppLibLockfunc, REALnoImplementation,
	"doAppLibLock(timeout as Integer) as Boolean" };
	
REALmethodDefinition doAppLibUnlockdefn = { (REALproc) doAppLibUnlockfunc, REALnoImplementation,
	"doAppLibUnlock() as Boolean" };
	
REALmethodDefinition doAppLibShowDiscoverAnywaydefn = { (REALproc) doAppLibShowDiscoverAnywayfunc, REALnoImplementation,
	"doAppLibShowDiscoverAnyway() as Boolean" };
	
REALmethodDefinition doAppLibGetAppFriendlyNamedefn = { (REALproc) doAppLibGetAppFriendlyNamefunc, REALnoImplementation,
	"doAppLibGetAppFriendlyName(AppPath as String) as String" };
	
REALmethodDefinition doAppLibMaintenancedefn = { (REALproc) doAppLibMaintenancefunc, REALnoImplementation,
	"doAppLibMaintenance() as Integer" };
	
	//  spec draft 3
	
REALmethodDefinition doAppLibGetStudentAttachedOverlaydefn = { (REALproc) doAppLibGetStudentAttachedOverlayfunc, REALnoImplementation,
	"doAppLibGetStudentAttachedOverlay(AppPath as String, GroupName as String, StudentName as String) as String" };
	
REALmethodDefinition doAppLibCountAttachedOverlaysdefn = { (REALproc) doAppLibCountAttachedOverlaysfunc, REALnoImplementation,
	"doAppLibCountAttachedOverlays(AppPath as String, GroupName as String, StudentName as String) as integer" };
	
REALmethodDefinition doAppLibGetSelectedOverlaydefn = { (REALproc) doAppLibGetSelectedOverlayfunc, REALnoImplementation,
	"doAppLibGetSelectedOverlay(AppPath as String, GroupName as String, StudentName as String) as integer" };
	
REALmethodDefinition doAppLibGetNumberedOverlaydefn = { (REALproc) doAppLibGetNumberedOverlayfunc, REALnoImplementation,
	"doAppLibGetNumberedOverlay(AppPath as String, GroupName as String, StudentName as String, OverlayNumber as Integer) as String" };
	
REALmethodDefinition doAppLibAddApplicationForStudentdefn = { (REALproc) doAppLibAddApplicationForStudentfunc, REALnoImplementation,
	"doAppLibAddApplicationForStudent(AppPath as String, GroupName as String, StudentName as String) as Integer" };
	
REALmethodDefinition doAppLibSetSelectedOverlaydefn = { (REALproc) doAppLibSetSelectedOverlayfunc, REALnoImplementation,
	"doAppLibSetSelectedOverlay(AppPath as String, GroupName as String, StudentName as String, OverlayNumber as Integer) as Integer" };

REALmethodDefinition doAppLibAttachOverlayForStudentdefn = { (REALproc) doAppLibAttachOverlayForStudentfunc, REALnoImplementation,
	"doAppLibAttachOverlayForStudent(AppPath as String, GroupName as String, StudentName as String, OverlayPath as String) as Integer" };
	
REALmethodDefinition doAppLibDetachOverlayForStudentdefn = { (REALproc) doAppLibDetachOverlayForStudentfunc, REALnoImplementation,
	"doAppLibDetachOverlayForStudent(AppPath as String, GroupName as String, StudentName as String, OverlayPath as String) as Integer" };
	
REALmethodDefinition doAppLibAddPreinstalledOverlaysdefn = { (REALproc) doAppLibAddPreinstalledOverlaysfunc, REALnoImplementation,
	"doAppLibAddPreinstalledOverlays(GroupName as String, StudentName as String, bForce as Boolean) as integer" };
	
REALmethodDefinition doAppLibIsAppAlloweddefn = { (REALproc) doAppLibIsAppAllowedfunc, REALnoImplementation,
	"doAppLibIsAppAllowed (AppPath as String) as boolean" };
	
REALmethodDefinition doAppLibSetAppAlloweddefn = { (REALproc) doAppLibSetAppAllowedfunc, REALnoImplementation,
	"doAppLibSetAppAllowed (AppPath as String, bAllowed as boolean)" };
	
REALmethodDefinition doAppLibRemoveApplicationForStudentdefn = { (REALproc) doAppLibRemoveApplicationForStudentfunc, REALnoImplementation,
	"doAppLibRemoveApplicationForStudent (AppPath as String, GroupName as String, StudentName as String) as integer" };
	
REALmethodDefinition doAppLibCountStandardAppsdefn = { (REALproc) doAppLibCountStandardAppsfunc, REALnoImplementation,
	"doAppLibCountStandardApps () as integer" };
	
REALmethodDefinition doAppLibGetNthStandardAppdefn = { (REALproc) doAppLibGetNthStandardAppfunc, REALnoImplementation,
	"doAppLibGetNthStandardApp (n as integer) as String" };
	
REALmethodDefinition doAppLibGetTruePathdefn = { (REALproc) doAppLibGetTruePathfunc, REALnoImplementation,
	"doAppLibGetTruePath (AppPath as String) as String" };
	
REALmethodDefinition doAppLibSetSystemInterestdefn = { (REALproc) doAppLibSetSystemInterestfunc, REALnoImplementation,
	"doAppLibSetSystemInterest (AppPath as String, SystemName as String, bInterest as Boolean)" };
	
REALmethodDefinition doAppLibHasSystemInterestdefn = { (REALproc) doAppLibHasSystemInterestfunc, REALnoImplementation,
	"doAppLibHasSystemInterest (AppPath as String, SystemName as String) as Boolean" };
	
REALmethodDefinition doAppLibIsOverlayPreinstalleddefn = { (REALproc) doAppLibIsOverlayPreinstalledfunc, REALnoImplementation,
	"doAppLibIsOverlayPreinstalled (OverlayPath as String) as Boolean" };
	
REALmethodDefinition doIsOverlayMakerInstalleddefn = { (REALproc) doIsOverlayMakerInstalledfunc, REALnoImplementation,
	"doIsOverlayMakerInstalled () as Boolean" };
	
REALmethodDefinition doAppLibUpdateAppPathdefn = { (REALproc) doAppLibUpdateAppPathfunc, REALnoImplementation,
	"doAppLibUpdateAppPath (oldPath as String, newPath as String) as Integer" };

REALmethodDefinition doAppLibIsIntelliSwitchInstalleddefn = { (REALproc) doAppLibIsIntelliSwitchInstalledfunc, REALnoImplementation,
	"doAppLibIsIntelliSwitchInstalled () as Boolean" };
	
REALmethodDefinition doAppLibIsIntellikeysInstalleddefn = { (REALproc) doAppLibIsIntellikeysInstalledfunc, REALnoImplementation,
	"doAppLibIsIntellikeysInstalled () as Boolean" };
	
REALmethodDefinition doAppLibIsSystemActivedefn = { (REALproc) doAppLibIsSystemActivefunc, REALnoImplementation,
	"doAppLibIsSystemActive(SystemName as String) as Integer" };
	
REALmethodDefinition doAppLibIsSystemPresentdefn = { (REALproc) doAppLibIsSystemPresentfunc, REALnoImplementation,
	"doAppLibIsSystemPresent(SystemName as String) as Boolean" };

REALmethodDefinition doAppLibSetSystemActivedefn = { (REALproc) doAppLibSetSystemActivefunc, REALnoImplementation,
	"doAppLibSetSystemActive(SystemName as String) as Integer" };

REALmethodDefinition doAppLibSetSystemInactivedefn = { (REALproc) doAppLibSetSystemInactivefunc, REALnoImplementation,
	"doAppLibSetSystemInactive(SystemName as String) as Integer" };

REALmethodDefinition IsControlPanelRunningdefn = { (REALproc) IsControlPanelRunningfunc, REALnoImplementation,
	"IsControlPanelRunning() as Boolean" };

	
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
	InitTTsPluginSupport ();
	
	IKUtil::Initialize();

	REALRegisterMethod(&add5defn);
	REALRegisterMethod(&loadBundledefn);
	REALRegisterMethod(&doCreateSharedMemorydefn);
	REALRegisterMethod(&doLockSharedMemorydefn);
	REALRegisterMethod(&doUnlockSharedMemorydefn);
	REALRegisterMethod(&doWriteSharedMemorydefn);
	REALRegisterMethod(&doWriteSharedMemoryBytedefn);
	REALRegisterMethod(&doReadSharedMemorydefn);
	REALRegisterMethod(&doReadSharedMemoryBytedefn);
	REALRegisterMethod(&doDestroySharedMemorydefn);
	REALRegisterMethod(&doClearSharedMemorydefn);
	REALRegisterMethod(&doIsOwnerAlivedefn);
	REALRegisterMethod(&doOwnChanneldefn);
	REALRegisterMethod(&doGetAppFriendlyNamedefn);
	REALRegisterMethod(&doIsOverlayMakerInstalleddefn);
	REALRegisterMethod(&doFileExistsdefn);
	REALRegisterMethod(&doHFSToUnixdefn);
	
	REALRegisterMethod(&doGetFileNamedefn);
	
	
	//  AppLib draft 2 spec
	REALRegisterMethod(&doAppLibAddSystemdefn);
	REALRegisterMethod(&doAppLibRemoveSystemdefn);
	REALRegisterMethod(&doAppLibCountSystemsdefn);
	REALRegisterMethod(&doAppLibGetNthSystemdefn);
	REALRegisterMethod(&doAppLibAddApplicationdefn);
	REALRegisterMethod(&doAppLibRemoveApplicationdefn);
	REALRegisterMethod(&doAppLibTakeOwnershipdefn);
	REALRegisterMethod(&doAppLibHasOwnershipdefn);
	REALRegisterMethod(&doAppLibGetOwnerdefn);
	REALRegisterMethod(&doAppLibReleaseOwnershipdefn);
	REALRegisterMethod(&doAppLibCountApplicationsdefn);
	REALRegisterMethod(&doAppLibGetNthApplicationdefn);
	REALRegisterMethod(&doAppLibLockdefn);
	REALRegisterMethod(&doAppLibUnlockdefn);
	REALRegisterMethod(&doAppLibGetAppFriendlyNamedefn);
	REALRegisterMethod(&doAppLibMaintenancedefn);
	
	//  AppLib draft 3 spec
	REALRegisterMethod(&doAppLibGetStudentAttachedOverlaydefn);
	REALRegisterMethod(&doAppLibCountAttachedOverlaysdefn);
	REALRegisterMethod(&doAppLibGetSelectedOverlaydefn);
	REALRegisterMethod(&doAppLibGetNumberedOverlaydefn);
	REALRegisterMethod(&doAppLibAddApplicationForStudentdefn);
	REALRegisterMethod(&doAppLibSetSelectedOverlaydefn);
	REALRegisterMethod(&doAppLibAttachOverlayForStudentdefn);
	REALRegisterMethod(&doAppLibDetachOverlayForStudentdefn);
	REALRegisterMethod(&doAppLibAddPreinstalledOverlaysdefn);
	REALRegisterMethod(&doAppLibIsAppAlloweddefn);
	REALRegisterMethod(&doAppLibSetAppAlloweddefn);
	REALRegisterMethod(&doAppLibRemoveApplicationForStudentdefn);
	REALRegisterMethod(&doAppLibCountStandardAppsdefn);
	REALRegisterMethod(&doAppLibGetNthStandardAppdefn);
	REALRegisterMethod(&doAppLibGetTruePathdefn);
	REALRegisterMethod(&doAppLibSetSystemInterestdefn);
	REALRegisterMethod(&doAppLibHasSystemInterestdefn);
	REALRegisterMethod(&doAppLibIsOverlayPreinstalleddefn);
	REALRegisterMethod(&doAppLibUpdateAppPathdefn);
	
	REALRegisterMethod(&doAppLibIsIntelliSwitchInstalleddefn);
	REALRegisterMethod(&doAppLibIsIntellikeysInstalleddefn);
	
	REALRegisterMethod(&doGetIntDatadefn);
	REALRegisterMethod(&doIsDiscoverInstalleddefn);
	
	REALRegisterMethod(&doGetOMOverlaysFolderdefn);
	REALRegisterMethod(&doAppLibCanListApplicationdefn);

	REALRegisterMethod(&doAppLibIsSystemActivedefn);
	REALRegisterMethod(&doAppLibIsSystemPresentdefn);
	REALRegisterMethod(&doAppLibSetSystemActivedefn);
	REALRegisterMethod(&doAppLibSetSystemInactivedefn);
	REALRegisterMethod(&doAppLibShowDiscoverAnywaydefn);
	
	REALRegisterMethod(&IsControlPanelRunningdefn);



}
