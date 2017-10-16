// IntelliKeysPlugin.h

#pragma once

#include "WinHeader++.h"
#include "rb_plugin.h"

extern "C" void loadBundlefunc(void);
//SharedMemBlock* ChooseMemoryBlock ( int channel );
extern "C" int doCreateSharedMemoryfunc(int channel);
extern "C" int doLockSharedMemoryfunc(int channel);
extern "C" int doUnlockSharedMemoryfunc(int channel);
extern "C" int doWriteSharedMemoryfunc(int channel,int offset,REALstring rbstring);
extern "C" int doWriteSharedMemoryBytefunc(int channel,int offset,int byteVal);
extern "C" REALstring doReadSharedMemoryfunc(int channel,int offset);
extern "C" int doReadSharedMemoryBytefunc(int channel,int offset);
extern "C" int doDestroySharedMemoryfunc(int channel);
extern "C" int doClearSharedMemoryfunc(int channel);
extern "C" int doIsOwnerAlivefunc(int channel);
extern "C" int doOwnChannelfunc(int channel);
extern "C" REALstring doGetAppFriendlyNamefunc( REALstring rbAppName );
extern "C" REALstring doGetFileNamefunc( REALstring rbPath );
extern "C" int doAppLibAddSystemfunc( REALstring SysName, REALstring SysIdentifier );
extern "C" int doAppLibRemoveSystemfunc( REALstring SystemName );
extern "C" bool doAppLibCanListApplicationfunc( REALstring AppName );
extern "C" int doAppLibCountSystemsfunc();
extern "C" int doIsDiscoverInstalledfunc();
extern "C" REALstring doAppLibGetNthSystemfunc( int n );
extern "C" int doAppLibAddApplicationfunc( REALstring AppPath );
extern "C" int doAppLibRemoveApplicationfunc( REALstring AppPath, REALstring SystemName );
extern "C" int doAppLibTakeOwnershipfunc( REALstring AppPath, REALstring SystemName );
extern "C" int doAppLibHasOwnershipfunc( REALstring AppPath, REALstring SystemName );
extern "C" REALstring doAppLibGetOwnerfunc( REALstring AppPath );
extern "C" int doAppLibReleaseOwnershipfunc( REALstring AppPath, REALstring SystemName );
extern "C" int doAppLibCountApplicationsfunc();
extern "C" REALstring doAppLibGetNthApplicationfunc( int n );
extern "C" bool doAppLibLockfunc( int timeout );
extern "C" bool doAppLibUnlockfunc ();
extern "C" bool doAppLibShowDiscoverAnywayfunc();
extern "C" REALstring doAppLibGetAppFriendlyNamefunc( REALstring AppPath );
extern "C" int doAppLibMaintenancefunc();
extern "C" REALstring doAppLibGetStudentAttachedOverlayfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName );
extern "C" int doAppLibCountAttachedOverlaysfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName );
extern "C" int doAppLibGetSelectedOverlayfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName );
extern "C" REALstring doAppLibGetNumberedOverlayfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName, int OverlayNumber);
extern "C" int doAppLibAddApplicationForStudentfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName );
extern "C" int doAppLibRemoveApplicationForStudentfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName );
extern "C" int doAppLibSetSelectedOverlayfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName, int OverlayNumber);
extern "C" int doAppLibAttachOverlayForStudentfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName, REALstring OverlayPath);
extern "C" int doAppLibDetachOverlayForStudentfunc( REALstring AppPath, REALstring GroupName, REALstring StudentName, REALstring OverlayPath);
extern "C" int doAppLibAddPreinstalledOverlaysfunc( REALstring GroupName, REALstring StudentName, bool bForce);
extern "C" Boolean doAppLibIsAppAllowedfunc( REALstring AppPath );
extern "C" void doAppLibSetAppAllowedfunc(  REALstring AppPath, Boolean bAllowed);
extern "C" int doAppLibCountStandardAppsfunc();
extern "C" REALstring doAppLibGetNthStandardAppfunc( int n );
extern "C" REALstring doAppLibGetTruePathfunc(  REALstring AppPath );
extern "C" Boolean doAppLibHasSystemInterestfunc( REALstring SysName, REALstring SysIdentifier );
extern "C" void doAppLibSetSystemInterestfunc( REALstring SysName, REALstring SysIdentifier, Boolean bInterest );
extern "C" bool doAppLibIsOverlayPreinstalledfunc( REALstring OverlayPath );
extern "C" bool doIsOverlayMakerInstalledfunc();
extern "C" int doAppLibUpdateAppPathfunc( REALstring oldPath, REALstring newPath );
extern "C" bool doAppLibIsIntelliSwitchInstalledfunc();
extern "C" bool doAppLibIsIntellikeysInstalledfunc();
extern "C" bool doFileExistsfunc( REALstring path );
extern "C" REALstring doHFSToUnixfunc( REALstring path );
extern "C" int doGetIntDatafunc( REALstring key, int defaultVal );
extern "C" REALstring doGetOMOverlaysFolderfunc( );
extern "C" int doAppLibIsSystemActivefunc( REALstring SystemName );
extern "C" bool doAppLibIsSystemPresentfunc( REALstring SystemName );
extern "C" int doAppLibSetSystemActivefunc( REALstring SystemName );
extern "C" int doAppLibSetSystemInactivefunc( REALstring SystemName );
extern "C" bool IsControlPanelRunningfunc( );

extern "C" bool IsVistaOrHigherfunc( );
