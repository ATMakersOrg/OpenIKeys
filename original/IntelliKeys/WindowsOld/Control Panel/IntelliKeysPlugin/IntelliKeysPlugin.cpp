#ifdef WIN32
#include "WinHeader++.h"
#include "Stdafx.h"
#endif
#include "rb_plugin.h"
#include <string.h>
#include <stdio.h>

#include "sharedMemory.h"

//#include "TT's Plugin Support.h"

#include "IKCommon.h"
#include "IKString.h"
#include "IKUtil.h"
#include "IKFile.h"
#include "AppLib.h"
#include "ExecImageVersion.h"

SharedMemBlock ControlPanelChannel = {"6789",0,0,6789,0,0,"ControlPanelChannel"};
SharedMemBlock EngineChannel       = {"8765",0,0,8765,0,0,"EngineChannel"};
//SharedMemBlock ControlPanelChannel = {"Global\\6789",0,0,6789,0,0,"Global\\ControlPanelChannel"};
//SharedMemBlock EngineChannel       = {"Global\\8765",0,0,8765,0,0,"Global\\EngineChannel"};

SharedMemBlock ControlPanelChannelVista = {"Global\\6789",0,0,6789,0,0,"Global\\ControlPanelChannel"};
SharedMemBlock EngineChannelVista       = {"Global\\8765",0,0,8765,0,0,"Global\\EngineChannel"};



//extern "C" int add5func(int v)
//{
//	return v + 5;
//}
//
//REALmethodDefinition add5defn = {
//	(REALproc) add5func,
//	REALnoImplementation,
//	"add5(v as integer) as integer"
//};

//load bundle function
extern "C" void loadBundlefunc(void){
	//do nothing, this will load an instance of our dll
}

SharedMemBlock* ChooseMemoryBlock ( int channel )
{
	// June 2012 - JR - can use the 'global' namespace on Win2K and above
	//  as it is safely ignored when it is not supported on Win2k and XP
	//if (IKUtil::IsWinVistaOrGreater())
	if (IKUtil::IsWin2KOrGreater())
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

}

//create shared memory
extern "C" int doCreateSharedMemoryfunc(int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

	return CreateSharedMemory(tempBlock);
}

//lock shared memory
extern "C" int doLockSharedMemoryfunc(int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

	return LockSharedMemory(tempBlock);
}

//unlock shared memory
extern "C" int doUnlockSharedMemoryfunc(int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

	return UnlockSharedMemory(tempBlock);
}

//write to our shared memory
extern "C" int doWriteSharedMemoryfunc(int channel,int offset,REALstring rbstring)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

	char *somestring = (char *)REALCString(rbstring);

	return WriteSharedMemory(tempBlock,offset,somestring);	
}

//write a byte to our shared memory
extern "C" int doWriteSharedMemoryBytefunc(int channel,int offset,int byteVal)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);
	return WriteSharedMemoryByte(tempBlock,offset,byteVal);	
}

//read from our shared memory
extern "C" REALstring doReadSharedMemoryfunc(int channel,int offset)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

	char *myString="";
	int myLength;
	int i;
	ReadSharedMemory(tempBlock,offset,(char *)myString);
	//i have a problem with <string.h> so I'm making my own strlen()
	i=0;
	while(myString[i] != '\0')
	{
		i++;
	}
	//that should be it
	myLength = i;
	return REALBuildString((char *)myString,myLength);
}

//read a byte from our shared memory
extern "C" int doReadSharedMemoryBytefunc(int channel,int offset)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

	return ReadSharedMemoryByte(tempBlock,offset);
}

//destroy our shared memory
extern "C" int doDestroySharedMemoryfunc(int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

	return DestroySharedMemory(tempBlock);	
}

//clear our shared memory
extern "C" int doClearSharedMemoryfunc(int channel)
{
	SharedMemBlock* tempBlock = ChooseMemoryBlock(channel);

	return ClearSharedMemory(tempBlock);	
}

//see if a channel is alive on the win side
extern "C" int doIsOwnerAlivefunc(int channel){

	char szMutexName[255];
	//strcpy(szMutexName,"IKUSB Mutex - ");
	if(channel == 0)
	{
		strcpy(szMutexName,"IKUSB Mutex - control panel\0");
	}
	else
	{
		strcpy(szMutexName,"IKUSB Mutex - engine\0");
	}		
	int bAlive = 0;
	HANDLE mutex = CreateMutex(NULL,FALSE,szMutexName);
	if (mutex)
	{
		DWORD result = WaitForSingleObject(mutex,0);
		if(result==WAIT_TIMEOUT)
		{
			bAlive = 1;
		}
		ReleaseMutex(mutex);
	}
	return bAlive;
}

//make a channel alive on the win side
extern "C" int doOwnChannelfunc(int channel)
{
	char szMutexName[255];
	//strcpy(szMutexName,"IKUSB Mutex - ");
	if(channel == 0)
	{
		strcpy(szMutexName,"IKUSB Mutex - control panel\0");
	}
	else
	{
		strcpy(szMutexName,"IKUSB Mutex - engine\0");

	}	
	HANDLE mutex = CreateMutex(NULL,TRUE,szMutexName);
	return (int)mutex;
}


extern "C" REALstring doGetAppFriendlyNamefunc( REALstring rbAppName )
{	
	//  original path as an IK string
	IKString path((char *)REALCString(rbAppName));

	//  get the friendly name
	IKString friendly = IKUtil::GetAppFriendlyName(path);

	//  return in a REAL string
	return REALBuildString((char *)friendly,friendly.GetLength());

}

extern "C" REALstring doGetFileNamefunc( REALstring rbPath )
{	
	//  original path as an IK string
	IKString path((char *)REALCString(rbPath));

	//  get the friendly name
	IKString name = path;
	IKUtil::StripFileName(name,true,false);

	//  return in a REAL string
	return REALBuildString((char *)name,name.GetLength());

}

extern "C" int doAppLibAddSystemfunc( REALstring SysName, REALstring SysIdentifier )
{
	char *pSysName       = (char *)REALCString(SysName);
	char *pSysIdentifier = (char *)REALCString(SysIdentifier);

	int result = AppLibAddSystem ( pSysName,  pSysIdentifier);
	return result;
}

extern "C" int doAppLibRemoveSystemfunc( REALstring SystemName )
{
	return AppLibRemoveSystem((char *)REALCString(SystemName));
}

extern "C" bool doAppLibCanListApplicationfunc( REALstring AppName )
{
	return AppLibCanListApplication((char *)REALCString(AppName));
}

extern "C" int doAppLibCountSystemsfunc()
{
	return AppLibCountSystems();
}

extern "C" int doIsDiscoverInstalledfunc()
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

extern "C" REALstring doAppLibGetNthSystemfunc( int n )
{
	IKString result = AppLibGetNthSystem(n);
	return REALBuildString ((char *) result, result.GetLength());
}

extern "C" int doAppLibAddApplicationfunc( REALstring AppPath )
{
	return AppLibAddApplication ((char *)REALCString(AppPath));
}

extern "C" int doAppLibRemoveApplicationfunc( REALstring AppPath, REALstring SystemName )
{
	return AppLibRemoveApplication ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

extern "C" int doAppLibTakeOwnershipfunc( REALstring AppPath, REALstring SystemName )
{
	return AppLibTakeOwnership ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

extern "C" int doAppLibHasOwnershipfunc( REALstring AppPath, REALstring SystemName )
{
	return AppLibHasOwnership ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

extern "C" REALstring doAppLibGetOwnerfunc( REALstring AppPath )
{
	char *pAppPath       = (char *)REALCString(AppPath);

	IKString result = AppLibGetOwner(pAppPath);
	return REALBuildString ((char *) result, result.GetLength());
}

extern "C" int doAppLibReleaseOwnershipfunc( REALstring AppPath, REALstring SystemName )
{
	return AppLibReleaseOwnership ((char *)REALCString(AppPath), (char *)REALCString(SystemName));
}

extern "C" int doAppLibCountApplicationsfunc()
{
	return AppLibCountApplications();
}

extern "C" REALstring doAppLibGetNthApplicationfunc( int n )
{
	IKString result = AppLibGetNthApplication(n);
	return REALBuildString ((char *) result, result.GetLength());
}

extern "C" bool doAppLibLockfunc( int timeout )
{
	return AppLibLock(timeout);
}

extern "C" bool doAppLibUnlockfunc ()
{
	return AppLibUnlock();
}

extern "C" bool doAppLibShowDiscoverAnywayfunc()
{
	return AppLibShowDiscoverAnyway();
}

extern "C" REALstring doAppLibGetAppFriendlyNamefunc( REALstring AppPath )
{
	char *pAppPath       = (char *)REALCString(AppPath);

	char * result = AppLibGetAppFriendlyName(pAppPath);
	return REALBuildString ( result, strlen(result));
}

extern "C" int doAppLibMaintenancefunc()
{
	return AppLibMaintenance();
}

//------------------------
//   added draft 3
extern "C" REALstring doAppLibGetStudentAttachedOverlayfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	IKString result = AppLibGetStudentAttachedOverlay( pAppPath, pGroupName, pStudentName );
	return REALBuildString ((char *) result, result.GetLength());
}

extern "C" int doAppLibCountAttachedOverlaysfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	int result = AppLibCountAttachedOverlays ( pAppPath, pGroupName, pStudentName );
	return result;
}

extern "C" int doAppLibGetSelectedOverlayfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	int result = AppLibGetSelectedOverlay ( pAppPath, pGroupName, pStudentName );
	return result;
}

extern "C" REALstring doAppLibGetNumberedOverlayfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName, int OverlayNumber)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	IKString result = AppLibGetNumberedOverlay(pAppPath, pGroupName, pStudentName, OverlayNumber );
	return REALBuildString ((char *) result, result.GetLength());
}

extern "C" int doAppLibAddApplicationForStudentfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	int result = AppLibAddApplicationForStudent ( pAppPath, pGroupName, pStudentName );
	return result;
}

extern "C" int doAppLibRemoveApplicationForStudentfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName )
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	int result = AppLibRemoveApplicationForStudent ( pAppPath, pGroupName, pStudentName );
	return result;
}

extern "C" int doAppLibSetSelectedOverlayfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName, int OverlayNumber)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	int result = AppLibSetSelectedOverlay ( pAppPath, pGroupName, pStudentName, OverlayNumber );
	return result;
}

extern "C" int doAppLibAttachOverlayForStudentfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName, REALstring OverlayPath)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	char *pOverlayPath = (char *)REALCString(OverlayPath);

	int result = AppLibAttachOverlayForStudent ( pAppPath, pGroupName, pStudentName, pOverlayPath );
	return result;
}

extern "C" int doAppLibDetachOverlayForStudentfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName, REALstring OverlayPath)
{
	char *pAppPath     = (char *)REALCString(AppPath);
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);
	char *pOverlayPath = (char *)REALCString(OverlayPath);

	int result = AppLibDetachOverlayForStudent ( pAppPath, pGroupName, pStudentName, pOverlayPath );
	return result;
}

extern "C" int doAppLibAddPreinstalledOverlaysfunc( REALstring GroupName, REALstring StudentName, bool bForce)
{
	char *pGroupName   = (char *)REALCString(GroupName);
	char *pStudentName = (char *)REALCString(StudentName);

	AppLibAddPreinstalledOverlays ( pGroupName, pStudentName, bForce );

	return 1;
}

extern "C" Boolean doAppLibIsAppAllowedfunc( REALstring AppPath )
{
	char *pAppPath   = (char *)REALCString(AppPath);

	return AppLibIsAppAllowed ( pAppPath );
}

extern "C" void doAppLibSetAppAllowedfunc(  REALstring AppPath, Boolean bAllowed)
{
	char *pAppPath   = (char *)REALCString(AppPath);

	AppLibSetAppAllowed ( pAppPath, bAllowed );

}

extern "C" int doAppLibCountStandardAppsfunc()
{
	int nApps = AppLibCountStandardApps();

	return nApps;
}

extern "C" REALstring doAppLibGetNthStandardAppfunc( int n )
{
	IKString result = AppLibGetNthStandardApp(n);
	return REALBuildString ((char *) result, result.GetLength());
}

extern "C" REALstring doAppLibGetTruePathfunc(  REALstring AppPath )
{
	char *pAppPath   = (char *)REALCString(AppPath);

	char *result = AppLibGetTruePath ( pAppPath );

	return REALBuildString (result, strlen(result));
}

extern "C" Boolean doAppLibHasSystemInterestfunc( REALstring SysName, REALstring SysIdentifier )
{
	char *pSysName       = (char *)REALCString(SysName);
	char *pSysIdentifier = (char *)REALCString(SysIdentifier);

	bool result = AppLibHasSystemInterest ( pSysName,  pSysIdentifier);
	return result;
}

extern "C" void doAppLibSetSystemInterestfunc( REALstring SysName, REALstring SysIdentifier, Boolean bInterest )
{
	char *pSysName       = (char *)REALCString(SysName);
	char *pSysIdentifier = (char *)REALCString(SysIdentifier);

	AppLibSetSystemInterest ( pSysName,  pSysIdentifier, bInterest);
}

extern "C" bool doAppLibIsOverlayPreinstalledfunc( REALstring OverlayPath )
{
	char *pOverlayPath       = (char *)REALCString(OverlayPath);

	return AppLibIsOverlayPreinstalled ( pOverlayPath );
}

extern "C" bool doIsOverlayMakerInstalledfunc()
{	
	return IKUtil::IsOverlayMakerInstalled ();
}

extern "C" int doAppLibUpdateAppPathfunc( REALstring oldPath, REALstring newPath )
{
	char *poldPath       = (char *)REALCString(oldPath);
	char *pnewPath		 = (char *)REALCString(newPath);

	int result = AppLibUpdateAppPath ( poldPath, pnewPath );
	return result;
}

extern "C" bool doAppLibIsIntelliSwitchInstalledfunc()
{	
	return AppLibIsIntelliSwitchInstalled();
}

extern "C" bool doAppLibIsIntellikeysInstalledfunc()
{	
	return AppLibIsIntellikeysInstalled();
}

extern "C" bool doFileExistsfunc( REALstring path )
{
	char *pPath       = (char *)REALCString(path);

	return IKFile::FileExists(pPath);
}

extern "C" REALstring doHFSToUnixfunc( REALstring path )
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

extern "C" int doGetIntDatafunc( REALstring key, int defaultVal )
{
	char *pKey = (char *)REALCString(key);
	int val = DATAI ( pKey, defaultVal );
	return val;
}

extern "C" REALstring doGetOMOverlaysFolderfunc ( )
{
	IKString result = IKUtil::GetOMOverlaysFolder();

	return REALBuildString ((char *) result, result.GetLength());
}

extern "C" int doAppLibIsSystemActivefunc( REALstring SystemName )
{
	return  AppLibIsSystemActive((char *)REALCString(SystemName));
}

extern "C" bool doAppLibIsSystemPresentfunc( REALstring SystemName )
{
	return  AppLibIsSystemPresent((char *)REALCString(SystemName));
}

extern "C" int doAppLibSetSystemActivefunc( REALstring SystemName )
{
	return AppLibSetSystemActive ((char *)REALCString(SystemName));
}

extern "C" int doAppLibSetSystemInactivefunc( REALstring SystemName )
{
	return AppLibSetSystemInactive ((char *)REALCString(SystemName));
}

extern "C" bool IsControlPanelRunningfunc( )
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

extern "C" bool IsVistaOrHigherfunc( )
{
	return IKUtil::IsWinVistaOrGreater();
}

//------------------------
//------------------------
// REAL Method Definitions
//------------------------
//------------------------



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

//--------------------------------------------------------
//--------------------------------------------------------
//  applib functions
//--------------------------------------------------------
//--------------------------------------------------------

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

//--------------------------------------------------------
//--------------------------------------------------------
//  spec draft 3
//--------------------------------------------------------
//--------------------------------------------------------

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

REALmethodDefinition IsVistaOrHigherdefn = { (REALproc) IsVistaOrHigherfunc, REALnoImplementation,
	"doIsVistaOrHigher() as Boolean" };

REALmethodDefinition methodsDefs[] = 
{
	loadBundledefn,
	doCreateSharedMemorydefn,
	doLockSharedMemorydefn,
	doUnlockSharedMemorydefn,
	doWriteSharedMemorydefn,
	doWriteSharedMemoryBytedefn,
	doReadSharedMemorydefn,
	doReadSharedMemoryBytedefn,
	doDestroySharedMemorydefn,
	doClearSharedMemorydefn,
	doIsOwnerAlivedefn,
	doOwnChanneldefn,
	doGetAppFriendlyNamedefn,
	doIsOverlayMakerInstalleddefn,
	doFileExistsdefn,
	doHFSToUnixdefn,
	doGetFileNamedefn,
	//  AppLib draft 2 spec
	doAppLibAddSystemdefn,
	doAppLibRemoveSystemdefn,
	doAppLibCountSystemsdefn,
	doAppLibGetNthSystemdefn,
	doAppLibAddApplicationdefn,
	doAppLibRemoveApplicationdefn,
	doAppLibTakeOwnershipdefn,
	doAppLibHasOwnershipdefn,
	doAppLibGetOwnerdefn,
	doAppLibReleaseOwnershipdefn,
	doAppLibCountApplicationsdefn,
	doAppLibGetNthApplicationdefn,
	doAppLibLockdefn,
	doAppLibUnlockdefn,
	doAppLibGetAppFriendlyNamedefn,
	doAppLibMaintenancedefn,
	//  AppLib draft 3 spec
	doAppLibGetStudentAttachedOverlaydefn,
	doAppLibCountAttachedOverlaysdefn,
	doAppLibGetSelectedOverlaydefn,
	doAppLibGetNumberedOverlaydefn,
	doAppLibAddApplicationForStudentdefn,
	doAppLibSetSelectedOverlaydefn,
	doAppLibAttachOverlayForStudentdefn,
	doAppLibDetachOverlayForStudentdefn,
	doAppLibAddPreinstalledOverlaysdefn,
	doAppLibIsAppAlloweddefn,
	doAppLibSetAppAlloweddefn,
	doAppLibRemoveApplicationForStudentdefn,
	doAppLibCountStandardAppsdefn,
	doAppLibGetNthStandardAppdefn,
	doAppLibGetTruePathdefn,
	doAppLibSetSystemInterestdefn,
	doAppLibHasSystemInterestdefn,
	doAppLibIsOverlayPreinstalleddefn,
	doAppLibUpdateAppPathdefn,
	doAppLibIsIntelliSwitchInstalleddefn,
	doAppLibIsIntellikeysInstalleddefn,
	doGetIntDatadefn,
	doIsDiscoverInstalleddefn,
	doGetOMOverlaysFolderdefn,
	doAppLibCanListApplicationdefn,
	doAppLibIsSystemActivedefn,
	doAppLibIsSystemPresentdefn,
	doAppLibSetSystemActivedefn,
	doAppLibSetSystemInactivedefn,
	doAppLibShowDiscoverAnywaydefn,
	IsControlPanelRunningdefn,
	IsVistaOrHigherdefn
};

REALmoduleDefinition moduledefn = {
	kCurrentREALControlVersion,
	"IKPluginModule",
	methodsDefs,
	sizeof(methodsDefs) / sizeof(REALmethodDefinition)
};

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
	//InitTTsPluginSupport ();

	IKUtil::Initialize();

	//REALRegisterMethod(&add5defn);

	REALRegisterModule(&moduledefn);

	//REALRegisterMethod(&loadBundledefn);
	//REALRegisterMethod(&doCreateSharedMemorydefn);
	//REALRegisterMethod(&doLockSharedMemorydefn);
	//REALRegisterMethod(&doUnlockSharedMemorydefn);
	//REALRegisterMethod(&doWriteSharedMemorydefn);
	//REALRegisterMethod(&doWriteSharedMemoryBytedefn);
	//REALRegisterMethod(&doReadSharedMemorydefn);
	//REALRegisterMethod(&doReadSharedMemoryBytedefn);
	//REALRegisterMethod(&doDestroySharedMemorydefn);
	//REALRegisterMethod(&doClearSharedMemorydefn);
	//REALRegisterMethod(&doIsOwnerAlivedefn);
	//REALRegisterMethod(&doOwnChanneldefn);
	//REALRegisterMethod(&doGetAppFriendlyNamedefn);
	//REALRegisterMethod(&doIsOverlayMakerInstalleddefn);
	//REALRegisterMethod(&doFileExistsdefn);
	//REALRegisterMethod(&doHFSToUnixdefn);

	//REALRegisterMethod(&doGetFileNamedefn);


	////  AppLib draft 2 spec
	//REALRegisterMethod(&doAppLibAddSystemdefn);
	//REALRegisterMethod(&doAppLibRemoveSystemdefn);
	//REALRegisterMethod(&doAppLibCountSystemsdefn);
	//REALRegisterMethod(&doAppLibGetNthSystemdefn);
	//REALRegisterMethod(&doAppLibAddApplicationdefn);
	//REALRegisterMethod(&doAppLibRemoveApplicationdefn);
	//REALRegisterMethod(&doAppLibTakeOwnershipdefn);
	//REALRegisterMethod(&doAppLibHasOwnershipdefn);
	//REALRegisterMethod(&doAppLibGetOwnerdefn);
	//REALRegisterMethod(&doAppLibReleaseOwnershipdefn);
	//REALRegisterMethod(&doAppLibCountApplicationsdefn);
	//REALRegisterMethod(&doAppLibGetNthApplicationdefn);
	//REALRegisterMethod(&doAppLibLockdefn);
	//REALRegisterMethod(&doAppLibUnlockdefn);
	//REALRegisterMethod(&doAppLibGetAppFriendlyNamedefn);
	//REALRegisterMethod(&doAppLibMaintenancedefn);

	////  AppLib draft 3 spec
	//REALRegisterMethod(&doAppLibGetStudentAttachedOverlaydefn);
	//REALRegisterMethod(&doAppLibCountAttachedOverlaysdefn);
	//REALRegisterMethod(&doAppLibGetSelectedOverlaydefn);
	//REALRegisterMethod(&doAppLibGetNumberedOverlaydefn);
	//REALRegisterMethod(&doAppLibAddApplicationForStudentdefn);
	//REALRegisterMethod(&doAppLibSetSelectedOverlaydefn);
	//REALRegisterMethod(&doAppLibAttachOverlayForStudentdefn);
	//REALRegisterMethod(&doAppLibDetachOverlayForStudentdefn);
	//REALRegisterMethod(&doAppLibAddPreinstalledOverlaysdefn);
	//REALRegisterMethod(&doAppLibIsAppAlloweddefn);
	//REALRegisterMethod(&doAppLibSetAppAlloweddefn);
	//REALRegisterMethod(&doAppLibRemoveApplicationForStudentdefn);
	//REALRegisterMethod(&doAppLibCountStandardAppsdefn);
	//REALRegisterMethod(&doAppLibGetNthStandardAppdefn);
	//REALRegisterMethod(&doAppLibGetTruePathdefn);
	//REALRegisterMethod(&doAppLibSetSystemInterestdefn);
	//REALRegisterMethod(&doAppLibHasSystemInterestdefn);
	//REALRegisterMethod(&doAppLibIsOverlayPreinstalleddefn);
	//REALRegisterMethod(&doAppLibUpdateAppPathdefn);

	//REALRegisterMethod(&doAppLibIsIntelliSwitchInstalleddefn);
	//REALRegisterMethod(&doAppLibIsIntellikeysInstalleddefn);

	//REALRegisterMethod(&doGetIntDatadefn);
	//REALRegisterMethod(&doIsDiscoverInstalleddefn);

	//REALRegisterMethod(&doGetOMOverlaysFolderdefn);
	//REALRegisterMethod(&doAppLibCanListApplicationdefn);

	//REALRegisterMethod(&doAppLibIsSystemActivedefn);
	//REALRegisterMethod(&doAppLibIsSystemPresentdefn);
	//REALRegisterMethod(&doAppLibSetSystemActivedefn);
	//REALRegisterMethod(&doAppLibSetSystemInactivedefn);
	//REALRegisterMethod(&doAppLibShowDiscoverAnywaydefn);

	//REALRegisterMethod(&IsControlPanelRunningdefn);



}
