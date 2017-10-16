//
// IKEngine.cpp: implementation of the IKEngine class.
//
//////////////////////////////////////////////////////////////////////

#define DEBUG_IKUSB 1

#include "IKCommon.h"
#include "IKUtil.h"
#include "IKEngine.h"
#include "IKMessage.h"
#include "IKMsg.h"
#include "IKControlPanel.h"
#include "IKFile.h"
#ifdef PLAT_MACINTOSH
#include "IKMsg.h"
#endif
#include "SharedMemory.h"
#include "LegacyIK.h"

#ifdef PLAT_MACINTOSH
#include <stdio.h>
#endif

#ifdef PLAT_WINDOWS
  #include "pnp.h"
  #include "dll.h"
#endif

#define _M(a) DATAS((TCHAR *)TEXT(#a),(TCHAR *)TEXT(#a))

#include "intellikeys.h"
#include "AppLib.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static IKEngine * theEngine;

IKEngine::IKEngine()
{
	m_bForceSendAttachedOverlay = false;
	
	theEngine = this;

#ifdef PLAT_MACINTOSH
	IKMsg::CleanSendingChannel();
#endif

	m_bRefreshCP = false;
}

IKEngine::~IKEngine()
{
	//  in case instantiator forgot
	Terminate();
}

//
//  Called once at the beginning of the session.
//

#ifdef PLAT_MACINTOSH_X

static void MakeAllFolders(char *strpath)
{
    IKString path(strpath);
    for (int i=0;i<path.GetLength();i++)
    {
	if (path.GetAt(i)==':')
	{
	    IKString part = path.Left(i+1);
	    //printf("Root Locator: creating folder %s\n",(char *)part);
	    IKFile::NewFolder(part);
	}
    }
}

static void MyPtoCstr2 ( unsigned char *str )
{
    int i, l;
    
    l = str[0];
    for (i=0;i<l;i++)
	str[i] = str[i+1];
    str[l] = 0;
}

static void MyCtoPstr2 ( unsigned char *str)
{
    int i, l, j;
    
    l = 0;
    while (str[l]!=0)
	l++;
		
    for (i=0;i<l;i++)
    {
	j = l-i-1;
	str[j+1] = str[j];
    }
    str[0] = l;
}

static void GetFullPath2 (long DirID, short vRefNum, char *path)
{
	CInfoPBRec myPB;// parameter block for PBGetCatInfo
	Str255 dirNameBuf; //  a directory name
	char fullPath[255];//  full pathname being constructed
	OSErr myErr;

	myPB.dirInfo.ioNamePtr = dirNameBuf;
	myPB.dirInfo.ioVRefNum = vRefNum;    //indicate target volume
	myPB.dirInfo.ioDrParID = DirID;      //initialize parent directory ID
	myPB.dirInfo.ioFDirIndex = -1;       //get info about a directory
	
	strcpy(fullPath,"");
	strcpy(path,"");

	do
	{
		myPB.dirInfo.ioDrDirID = myPB.dirInfo.ioDrParID;
		myErr = PBGetCatInfo(&myPB, FALSE);
		if ( myErr )
			return;

		MyPtoCstr2 ( dirNameBuf );
		
		char temp[255];
		strcpy(temp,fullPath);
		strcpy(fullPath,(char *)dirNameBuf);
		strcat(fullPath,":");
		strcat(fullPath,temp);

	} while ( myPB.dirInfo.ioDrDirID != fsRtDirID );

	strcpy(path,fullPath);
	return;

}

#endif

void IKEngine::Initialize()
{
	m_bDiagnosticMode = false;
	m_lastDiag = 0;
	
	m_bRawMode = false;
	m_bNotifyMode = false;

    //initialize the messaging systems

	IKMessage::Initialize();
	IKMsg::Initialize();

	//  initialize the Utilities package
	//  this will cause the global data items to get loaded

	IKUtil::Initialize();

	//  create some folders
	//  order is not important here

#ifdef PLAT_MACINTOSH
	IKUtil::MakeAllFolders(IKUtil::GetChannelsFolder());
	IKUtil::MakeAllFolders(IKUtil::GetEngineFolder());
#endif

	IKUtil::MakeAllFolders(IKUtil::GetGuestFolder());


	//  insure that everything is writeable
	MakeAllFilesWriteable(IKUtil::GetRootFolder());

#ifdef PLAT_MACINTOSH_X

        //  make sure some things are set up properly in the classic environment.
        //  we do this in the installer, too, but it might get disturbed by a
        //  subsequent IK USB 1.x installation.
        
        //  first, build an HFS-style path to the IK USB root.
        //  Do this by finding one of the OS X system folders, then finding the disk name from that, and then
        //  adding on the IK USB path.
        
        short vRefNum;
        long dirID;
        OSErr err;
        char root_path[255];
        err = FindFolder ( kOnSystemDisk, kApplicationSupportFolderType, kDontCreateFolder, &vRefNum, &dirID );
        GetFullPath2 ( dirID, vRefNum, root_path );
        int i = 0;
        while (root_path[i]!=':')
            i++;
        root_path[i+1] = '\0';
        strcat ( root_path, "Applications:IntelliTools:IntelliKeys USB:" );
        
        //  record the path we just found in Classic's app support folder so
        //  the sending INIT can find it.
        
        err = FindFolder ( kClassicDomain, kApplicationSupportFolderType, 
                                 kDontCreateFolder, &vRefNum, &dirID );
        if (err == noErr)
        {
            char classic_path[255];
            GetFullPath2 ( dirID, vRefNum, classic_path );
            strcat(classic_path,"IntelliTools:");
            MakeAllFolders(classic_path);  //  make sure the folder exists
            strcat(classic_path,"IKUSBRoot.txt");
            IKFile f;
            if (f.Open(IKString(classic_path),IKFile::modeCreate|IKFile::modeReadWrite))
            {
                IKString line;
                line += (const char *) "Root_Location     ";
                line += root_path;
                f.WriteLine(line);
                f.Close();
                printf("Root Locator: created classic file\n");
            }
        }
        
        //  install the sending INIT in the Classic extensions folder
        
        err = FindFolder ( kClassicDomain, kExtensionFolderType, 
                                 kDontCreateFolder, &vRefNum, &dirID );
        if (err==noErr)
        {
            char extensions_path[255];
            GetFullPath2 ( dirID, vRefNum, extensions_path );
            
            IKString src = IKUtil::GetPrivateFolder();
            src += DATAS(TEXT("classic_sending_extension"),TEXT("Switch InTT Release"));
             
            IKString dst(extensions_path);
            dst += DATAS(TEXT("classic_sending_extension"),TEXT("Switch InTT Release"));
            
            IKFile::MakeWritable (dst);
            IKFile::Remove (dst);
            IKFile::Copy ( src, dst );
        }
        
        //  install the Classic control panel into the Classic control panels folder
        
        err = FindFolder ( kClassicDomain, kControlPanelFolderType, 
                                 kDontCreateFolder, &vRefNum, &dirID );
        if (err==noErr)
        {
            char cp_path[255];
            GetFullPath2 ( dirID, vRefNum, cp_path );
            
            IKString src = IKUtil::GetPrivateFolder();
            src += DATAS(TEXT("classic_control_panel"),TEXT("IntelliKeys USB"));
             
            IKString dst(cp_path);
            dst += DATAS(TEXT("classic_control_panel"),TEXT("IntelliKeys USB"));
            
            IKFile::MakeWritable (dst);
            IKFile::Remove (dst);
            IKFile::Copy ( src, dst );
        }
        
        //  install the firewall into the Classic startup folder,
        //  but only if it's already there.
        
        err = FindFolder ( kClassicDomain, kStartupFolderType, 
                                 kDontCreateFolder, &vRefNum, &dirID );
        if (err==noErr)
        {
            char startup_path[255];
            GetFullPath2 ( dirID, vRefNum, startup_path );
             
            IKString src = IKUtil::GetPrivateFolder();
            src += DATAS(TEXT("classic_startup_item"),TEXT("IntelliKeys USB App"));
            
            IKString dst(startup_path);
            dst += DATAS(TEXT("classic_startup_item"),TEXT("IntelliKeys USB App"));
            
            char macfilename[255];
            strcpy ( &(macfilename[1]), (TCHAR *) dst);
            macfilename[0] = strlen(macfilename);
            FSSpec spec;
            err = FSMakeFSSpec ( (short)0, (long)0, (unsigned TCHAR *)macfilename, &spec );
            if (err == noErr)
            {
                IKFile::MakeWritable (dst);
                IKFile::Remove (dst);
                IKFile::Copy ( src, dst );
            }
        }

#endif

	//  trigger processing of external commands immediately
	m_nextExternalCommandTime = 0;
	m_nextExternalCommandTime2 = 0;
	m_nextAppWatchTime = 0;

#ifdef PLAT_WINDOWS

	if (IKUtil::IsWin2KOrGreater())
	{
		//  Service does not use Dll in W2K andup
	}
	else
	{
		//  enable keyboard hook DLL for Win98
		DllKeyEnable ( true );
	}
	
#endif

	//  load standard overlays
	LoadStandardOverlays();

	//  load preferences
	IKString prefsFile = IKUtil::GetPrivateFolder();
	prefsFile += DATAS(TEXT("Preferences_File"),TEXT("preferences.txt"));
	m_preferences.Read(prefsFile);

	//  load names of switch overlays
	m_nSwitchOverlays = DATAI(TEXT("num_switch_files"), 0);
	for (int i=0;i<m_nSwitchOverlays;i++)
	{
		IKString s = TEXT("switch_file_");
		s += IKUtil::IntToString(i+1);
		m_switchOverlays[i] = DATAS(s,TEXT(""));
	}
	
	//  establish AppLib "systems" for IKUSB
	//  but only if "watch apps" is true.

	bool bWatch = !!DATAI(TEXT("watch_apps"), 0);
	if (bWatch)
	{	
		//  for us, it's a file relative to ROOT.
		IKString s = DATAS(TEXT("intellikeys_file"),TEXT(""));
		if (!s.IsEmpty())
		{
			IKString idFilePath = IKUtil::GetRootFolder();
			idFilePath += s;
			
			if (IKFile::FileExists(idFilePath))
				AppLibAddSystem ( DATAS(TEXT("intellikeys_name"),TEXT("IntelliKeys USB")), (TCHAR *)idFilePath);
		}
		
	}
	
#ifdef PLAT_WINDOWS
	//  make sure we're using the best ikeysxfr
	FixIkeysXfr ();
#endif

	//  check to see if anything needsto be done
	//  because this is the first time we ran since an install
	CheckNewInstall();

	//  get current group and user
	IKString group, student;
	GetCurrentGroupAndStudent(group,student);
	m_preferences.Write();
	OnChangeStudent(group,student);

	//  we're going!
	m_bRunning = true;
}

//
//  Called by the hosting application very frequently.
//
void IKEngine::Periodic()
{
	unsigned int now;
	
	static int extPeriod = 250;
	static int extPeriod2 = 250;
	static int watchAppPeriod = 500;
	static bool bWatchApps = false;
	static int loadPeriod = 2000;
	
	static int once = 0;
	if (once==0)
	{
		once++;
		
		extPeriod		= DATAI(TEXT("External_Command_Period"),250);
		extPeriod2		= DATAI(TEXT("External_Command_Period_2"),250);
		watchAppPeriod  = DATAI(TEXT("App_Watch_Period"),500);
		bWatchApps		= !!DATAI(TEXT("watch_apps"), 0);
		loadPeriod		= DATAI(TEXT("Load_Student_Data"),2000);
	}

	//  Process External Commands
	now = IKUtil::GetCurrentTimeMS();
	if (now>=m_nextExternalCommandTime)
	{
		ProcessExternalCommands();
		m_nextExternalCommandTime = now + extPeriod;
	}

	if (now>=m_nextExternalCommandTime2)
	{
		ProcessExternalCommands2();
		m_nextExternalCommandTime2 = now + extPeriod2;
	}

	//  watch applications

	if (bWatchApps)
	{
		if (now>=m_nextAppWatchTime)
		{
			WatchApplications();
			m_nextAppWatchTime = now + watchAppPeriod;
		}
	}

	//  poke the devices
	for (int i=0;i<MAX_INTELLIKEYS;i++)
		m_devices[i].Periodic();

	//  send periodic updates to the control panel if we're in
	//  diagnostic mode

	if (now>=m_lastDiag)
	{
		//  check up on channel status
		IKMessage::CheckChannels();

	    //  get us out of diag mode if the control panel is not there
		if (GetDiagnosticMode())
			if (!IKMessage::IsOwnerAlive(TEXT("control panel")))
				SetDiagnosticMode(false);

	    m_lastDiag = now + DATAI(TEXT("Diagnostic_Period"),1000);
	}

	//  refresh the contrl panel if need be
	if (m_bRefreshCP)
	{
		IKControlPanel::Refresh();
		m_bRefreshCP = false;
	}
		
	//  do Utility periodics
	IKUtil::Periodic();
}

static IKString UnpackString ( BYTE *data, int ndata )
{
	int len = data[0];
	IKString strResult(&data[1],len);
	return strResult;
}

static IKStringArray activeSystems;

static int GetPackedInt ( BYTE *data, int ndata, int index )
{	
	int n1 = 0;
	int ncommas = 0;
	for (int i=0;i<ndata;i++)
	{
		if (data[i]==',')
		{
			ncommas++;
			if (ncommas==index)
			{
				IKString s(data,ndata);
				IKString s2 = s.Mid(n1,i-n1);
				int val = atoi((TCHAR *)s2);
				return val;
			}
			n1 = i+1;
		}
	}
	
	return -1;
}

void IKEngine::ProcessExternalCommands()
{
	BYTE data[2000];
	int datalen;
	int command;
	int response;

	const TCHAR *channel = TEXT("engine");
	
	response = IKMessage::Receive ( TEXT(channel), &command, (void *)data, 2000, &datalen );
	if (response == kResponseNoCommand)
		return;

#if defined (DEBUG_CONTROL_PANEL)|defined (DEBUG_IKUSB)
    if (command != 0 && command != 1051 && command != 1015 && command != -1067) {
        NSLog (@"IKEngine::ProcessExternalCommands: command (%d); datalen (%d);", 
               command, datalen);
    }
#endif

	switch (command)
	{
	case kQueryPostKey:
		if (NumDevicesConnected()<=0)
			IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
		else
		{
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			int code = GetPackedInt ( data, datalen, 1 );
			int direction = GetPackedInt ( data, datalen, 2 );
			PostKey ( code, direction );
		}
		break;
		
	case kQueryPostDelay:
		if (NumDevicesConnected()<=0)
			IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
		else
		{
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			int delay = GetPackedInt ( data, datalen, 1 );
			PostDelay ( delay );
		}
		break;

	case kQueryPostMouseButtonToggle:
		if (NumDevicesConnected()<=0)
			IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
		else
		{
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			int whichButton = GetPackedInt ( data, datalen, 1 );
			PostMouseButtonToggle ( whichButton );
		}
		break;
		
	case kQueryPostMouseButtonClick:
		if (NumDevicesConnected()<=0)
			IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
		else
		{
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			int whichButton = GetPackedInt ( data, datalen, 1 );
			PostMouseButtonClick ( whichButton );
		}
		break;
		
	case kQueryPostMouseButtonDoubleClick:
		if (NumDevicesConnected()<=0)
			IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
		else
		{
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			int whichButton = GetPackedInt ( data, datalen, 1 );
			PostMouseButtonDoubleClick ( whichButton );
		}
		break;

	case kQueryPostMouseButton:
		if (NumDevicesConnected()<=0)
			IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
		else
		{
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			int whichButton = GetPackedInt ( data, datalen, 1 );
			int direction = GetPackedInt ( data, datalen, 2 );
			PostMouseButton ( whichButton, direction );
		}
		break;
		
	case kQueryPostMouseMove:
		if (NumDevicesConnected()<=0)
			IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
		else
		{
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			int x = GetPackedInt ( data, datalen, 1 );
			int y = GetPackedInt ( data, datalen, 2 );
			PostMouseMove ( x, y );
		}
		break;
		
	case kQueryResetMouseInterface:
		if (NumDevicesConnected()<=0)
			IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
		else
		{
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			ResetMouseInterface ();
		}
		break;
		
	case kQueryResetKeyboardInterface:
		if (NumDevicesConnected()<=0)
			IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
		else
		{
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			ResetKeyboardInterface ();
		}
		break;
			
	case kQuerySetSystemActive:
		{
			//  add to the list if it's not there.
			IKString system = UnpackString ( data, datalen );
			activeSystems.AddNoDup(system);
			IKMessage::Respond ( TEXT(channel),  kResponseSystemActive, 0, 0 );
		}
		break;

	case kQuerySetSystemInactive:
		{
			IKString system = UnpackString ( data, datalen );
			activeSystems.Remove(system);
			IKMessage::Respond ( TEXT(channel),  kResponseSystemInactive, 0, 0 );
		}
		break;

	case kQueryIsSystemActive:
		{
			IKString system = UnpackString ( data, datalen );
			int i = activeSystems.Find(system);
			if (i == -1)
				IKMessage::Respond ( TEXT(channel),  kResponseSystemInactive, 0, 0 );
			else
				IKMessage::Respond ( TEXT(channel),  kResponseSystemActive, 0, 0 );
		}
		break;

		case kResponseGetCurrentApp:
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			{
				IKString app ( data, datalen );
				m_currentApp = app;
			}
			break;

		case kQuerySendAttachedOverlay:
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			m_bForceSendAttachedOverlay = true;
			break;
			
	case kQueryLaunchControlPanel:
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		if (!IKMessage::IsOwnerAlive(TEXT("control panel")))
		{

#ifdef PLAT_WINDOWS
                    IKString app = IKUtil::GetRootFolder();
                    app += DATAS(TEXT("Control_Panel_Name"),"");
    
                    int nResult = (int) ShellExecute ( GetDesktopWindow(), 
                            TEXT("Open"), TEXT("Control"), TEXT("ikusb.cpl"), TEXT("C:\\"), 
                            SW_SHOWNORMAL );
#endif
#ifdef PLAT_MACINTOSH
                    IKString filename= IKUtil::GetRootFolder();
                    filename += DATAS(TEXT("Control_Panel_Name"),TEXT("USB Control Panel.exe"));
                    IKUtil::LaunchFile(filename);
#endif
		}
		else
		{
			IKControlPanel::Show();
		}
		break;

	case kQueryBeep:
		{
			if (NumDevicesConnected()<=0)
				IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
			else
			{
				IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
				for (int i=0;i<MAX_INTELLIKEYS;i++)
				{
					m_devices[i].KeySound(500);
				}
			}
		}
		break;
	
	case kQueryShortBeep:
		{
			if (NumDevicesConnected()<=0)
				IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
			else
			{
				IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
				for (int i=0;i<MAX_INTELLIKEYS;i++)
				{
					m_devices[i].ShortKeySound();
				}
			}
		}
		break;
	
	case kQuerySendSettings:
	case DNLD_SET:
	    IKMessage::Respond ( TEXT(channel),  NO_ERR, 0, 0 );
		OnNewCustomOverlaySettings(data,datalen);
	    break;
	    
	case DNLD_OVL:
	    IKMessage::Respond ( TEXT(channel),  NO_ERR, 0, 0 );
		OnNewCustomOverlayData(data,datalen);
		{
			BYTE data[2000];
			int ndata = 0;
			IKEngine::GetEngine()->MakeGlobalData(data,&ndata);
			IKMessage::Send(TEXT("control panel"),kQueryGlobalData,data,ndata,0,0);
		}
	    break;
	    
	case UPLD_SET:
            //  TODO: return the real settings.
            //  although, we don't do that for IKUSB 1.x.
            datalen = sizeof ( LegacyIKSettings );
            memset ( data, 0, datalen );
	    IKMessage::Respond ( TEXT(channel),  NO_ERR, data, datalen );
	    break;

	case kQueryIsUSBIntelliKeysConnected:
	case kQueryIsIntellikeysConnected:
		{
			if (NumDevicesConnected()>0)
				IKMessage::Respond ( TEXT(channel),  kResponseConnected, 0, 0 );
			else
				IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
		}
		break;

	case kQueryShutdown:
		IKMessage::Respond(TEXT(channel),kResponseNoError,0,0);
		Stop();
		break;

	case kQueryShowControlPanel:
		IKMessage::Respond(TEXT(channel),kResponseNoError,0,0);
		IKControlPanel::Show();
		break;

	case kQueryIsStandardOverlayInPlace:
		{
			if (NumDevicesWithStandardOverlay()>0)
				IKMessage::Respond ( TEXT(channel),  kResponseStandardOverlayIsInPlace, 0, 0 );
			else
				IKMessage::Respond ( TEXT(channel),  kResponseStandardOverlayIsNotInPlace, 0, 0 );
		}
		break;

	case kQueryIsIntellikeysOn:
		{
			if (NumDevicesOn()>0)
				IKMessage::Respond ( TEXT(channel),  kResponseOn, 0, 0 );
			else
				IKMessage::Respond ( TEXT(channel),  kResponseNotOn, 0, 0 );
		}
		break;

	case kQuerySetStudent:
		{
			IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );

			MakeAllFilesWriteable ( IKUtil::GetUsersFolder() );

			int n = 0;
			int len;

			int size = data[n];
			n++;

			len = data[n];
			n++;
			IKString group ( (BYTE *)&(data[n]), len*size );
			n = n + len*size;

			len = data[n];
			n++;
			IKString student ( (BYTE *)&(data[n]), len*size );
			n = n + len*size;

			OnChangeStudent(group,student);
		}
		break;

	case kQuerySendOverlay:
		{
			IKString nameIn(data,datalen);  //  could be unicode string
			OnNewCustomOverlayFile (nameIn);
		}
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		{
			BYTE data[2000];
			int ndata = 0;
			IKEngine::GetEngine()->MakeGlobalData(data,&ndata);
			IKMessage::Send(TEXT("control panel"),kQueryGlobalData,data,ndata,0,0);
		}
		break;

	case kQuerySendOverlayName:
		{
			IKString nameIn(data,datalen);  //  could be unicode string
			OnNewCustomOverlayName (nameIn);
		}
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		{
			BYTE data[2000];
			int ndata = 0;
			IKEngine::GetEngine()->MakeGlobalData(data,&ndata);
			IKMessage::Send(TEXT("control panel"),kQueryGlobalData,data,ndata,0,0);
		}
		break;

	case kQuerySendOverlayWithName:
		{
			int n = 0;
			int len;

			int size = data[n];
			n++;

			len = data[n];
			n++;
			IKString file ( (BYTE *)&(data[n]), len*size );
			n = n + len*size;

			len = data[n];
			n++;
			IKString name ( (BYTE *)&(data[n]), len*size );
			n = n + len*size;

			len = data[n];
			n++;
			IKString sender ( (BYTE *)&(data[n]), len*size );
			n = n + len*size;

			OnNewCustomOverlayFile (file);
			OnNewCustomOverlayName (name);
			OnNewCustomOverlaySender ( name, sender );
		}
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		{
			BYTE data[2000];
			int ndata = 0;
			IKEngine::GetEngine()->MakeGlobalData(data,&ndata);
			IKMessage::Send(TEXT("control panel"),kQueryGlobalData,data,ndata,0,0);
		}
		break;

	case kQueryResetKeyboard:
		
		{
			if (NumDevicesConnected()<=0)
				IKMessage::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
			else
			{
				IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
				for (int i=0;i<MAX_INTELLIKEYS;i++)
				{
					m_devices[i].UserReset(true);
				}
			}
		}
		break;

	case kQueryStartDiagnosticMode:
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetDiagnosticMode (true);
		break;

	case kQueryStopDiagnosticMode:
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetDiagnosticMode (false);
		break;

	case kQueryStartRawMode:
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetRawMode (true);
		break;

	case kQueryStopRawMode:
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetRawMode (false);
		break;

	case kQuerySetNotifyModeOn:
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetNotifyMode (true);
		break;
		
	case kQuerySetNotifyModeOff:
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetNotifyMode (false);
		break;

	case kQueryGetRawMode:
		{
			if (GetRawMode())
				IKMessage::Respond ( TEXT(channel),  kResponseRawModeOn, 0, 0 );
			else
				IKMessage::Respond ( TEXT(channel),  kResponseRawModeOff, 0, 0 );
		}
		break;

	case kQueryGetRawModeUsable:
		{
			if (GetRawMode() && NumDevicesConnected()>0 && NumDevicesConnected()>0)
				IKMessage::Respond ( TEXT(channel),  kResponseRawModeUsable, 0, 0 );
			else
				IKMessage::Respond ( TEXT(channel),  kResponseRawModeUnusable, 0, 0 );
		}
		break;

	case kQuerySoftwareVersion:
		{
			IKString version = DATAS(TEXT("Software_Version"),TEXT(""));
			BYTE data[255];
			IKString::strcpy((TCHAR *)data,(TCHAR *)version);
			int datalen = version.GetByteCount();
			IKMessage::Respond ( TEXT(channel),  kResponseSoftwareVersion, data, datalen );
		}
		break;

	case kQueryFirmwareVersion:
		{
			IKString version = DATAS(TEXT("Firmware_Version"),TEXT(""));
			BYTE data[255];
			IKString::strcpy((TCHAR *)data,(TCHAR *)version);
			int datalen = version.GetByteCount();
			IKMessage::Respond ( TEXT(channel),  kResponseFirmwareVersion, data, datalen );
		}
		break;

	case kQueryLastSentOverlay:
		{
			//IKString overlay = m_customOverlay.GetName();
			IKString overlay = IKSettings::GetSettings()->m_sLastSent;
			//overlay.ToUTF8();
			BYTE data[255];
			IKString::strcpy((TCHAR *)data,(TCHAR *)overlay);
			int datalen = overlay.GetByteCount();
			IKMessage::Respond ( TEXT(channel),  kResponseLastSentOverlay, data, datalen );
		}
		break;

	case kQueryUSBIntelliKeysArrayChanged:
		{
			BYTE data[1000];
			int datalen = 0;
			MakeIntellikeysArray ( data, &datalen );
			IKMessage::Respond ( TEXT(channel),  kResponseUSBIntelliKeysArrayChanged, data, datalen );
		}
		break;

	case kQueryGlobalData:
		{
			BYTE data[1000];
			int datalen = 0;
			MakeGlobalData ( data, &datalen );
			IKMessage::Respond ( TEXT(channel),  kResponseGlobalData, data, datalen );
		}
		break;

	case kQueryUSBIntelliKeysArray:
		{
			BYTE data[1000];
			int datalen = 0;
			MakeIntellikeysArray ( data, &datalen );
			IKMessage::Respond ( TEXT(channel),  kResponseUSBIntelliKeysArray, data, datalen );
		}
		break;

	case kQueryDiagnosticDataBlock:
		{
			BYTE data[1000];
			int datalen = 0;
			MakeDiagnosticBlock ( data, &datalen );
			IKMessage::Respond ( TEXT(channel),  kResponseDiagnosticDataBlock, data, datalen );
		}
		break;

	case kQueryGetSettings:
	case kQueryShouldCallOldInTT:
		//  unsupported commands
		IKMessage::Respond ( TEXT(channel),  kResponseUnsupported, 0, 0 );
		break;

	case kQueryReloadStudent:
		LoadStudentData();
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
        IKControlPanel::Refresh();
		break;

	case kQueryGetRawEvents:
		{
			//  how many are we asked for?
			int nMaxEvents = data[0];

			//  how many are there?
			int nEvents = 0;
			int i;
			for (i=0;i<MAX_INTELLIKEYS;i++)
				nEvents += m_devices[i].CountRawEvents();

			//  anything?
			if (nEvents<=0 || nMaxEvents<=0)
				IKMessage::Respond ( TEXT(channel),  kResponseNoRawEvents, 0, 0 );
			else
			{
				//  ok, return events up to the maximum.
				if (nEvents>nMaxEvents)
					nEvents = nMaxEvents;

				//  allocate enough memory for the events.
				BYTE *pEvents = new BYTE[nEvents*9+1];
				
				//  look at the devices round-robin picking up events as we go.
				
				int ne = 0;
				int ndev = 0;
				int nfound = 0;
				
				while (true)
				{
					if (m_devices[ndev].GetRawEvent(&(pEvents[1+ne*9+1])))
					{
						pEvents[1+ne*9] = ndev;  //  device #
						ne++;
						
						//  if we run out of space.
						if (ne>=nEvents)
							break;
						
						nfound++;
					}
					
					//  round-robin for the devices.
					ndev++;
					if (ndev>=MAX_INTELLIKEYS)
					{
						//  if we went around once and found no events, break;
						if (nfound==0)
							break;
							
						ndev = 0;
						nfound = 0;
					}
					
				}
				
				//  put in the #of events actually found
				pEvents[0] = ne;

				//  send it back to the caller
				IKMessage::Respond ( TEXT(channel),  kResponseRawEvents, pEvents, nEvents*9+1 );

				//  delete the memory
				delete []pEvents;
			}

		}
		break;

	case kQueryGetRawEvent:
		{
			BYTE buffer[8];
			int nDevEvent = -1;
			int i;
			for (i=0;i<MAX_INTELLIKEYS;i++)
			{
				if (m_devices[i].GetRawEvent(buffer))
				{
					nDevEvent = i;
					break;
				}	
			}
			if (nDevEvent>=0)
			{
				BYTE data[9];
				data[0] = nDevEvent;
				for (i=0;i<8;i++)
					data[i+1] = buffer[i];

				IKMessage::Respond ( TEXT(channel),  kResponseRawEvent, data, 9 );
			}
			else
			{
				IKMessage::Respond ( TEXT(channel),  kResponseNoRawEvents, 0, 0 );
			}

		}
		break;

	default:
		//  unknown commands
		IKMessage::Respond ( TEXT(channel),  kResponseError, 0, 0 );
		break;
	}
}

IKEngine * IKEngine::GetEngine()
{
	return theEngine;
}

void IKEngine::LoadStandardOverlays()
{
	//  load 'em up
	for (int i=0;i<MAX_STANDARD_OVERLAYS;i++)
	{
		//  get the name of the file
		IKString key = TEXT("Standard_Overlay ");
		TCHAR snumber[10];
		MySprintf(snumber,TEXT("%d"),i);
		key += snumber;
		IKString name;
		name = IKUtil::GetOverlaysFolder();
		name += DATAS(key,TEXT("none.oms"));

		//  load it
		m_standardOverlay[i].LoadFromFile(name);

		//  get the new name
		IKString key2 = TEXT("Standard_Overlay ");
		//TCHAR snumber[10];
		MySprintf(snumber,TEXT("%d"),i);
		key2 += snumber;
		key2 += (const char *) TEXT(" Name");
		IKString newname = DATAS(key2,TEXT(""));
		if (!newname.IsEmpty())
		{
			//  change the name
			m_standardOverlay[i].SetName(newname);
		}

	}

	//  identify the setup overlay
	int nSetup = DATAI(TEXT("Standard_Setup_Overlay"),-1);
	if (nSetup!=-1)
		m_standardOverlay[nSetup].MakeSetupOverlay();

	//  load the default intelliswitch overlay
	IKString path = IKUtil::GetPrivateFolder();
	path += TEXT("default intelliswitch.oms");
	m_defaultIntelliSwitchOverlay.LoadFromFile(path);
}

IKOverlay * IKEngine::GetStandardOverlay(int index)
{
	if (index>=0 && index <=MAX_STANDARD_OVERLAYS)
		return &(m_standardOverlay[index]);
	return NULL;
}


void IKEngine::Terminate()
{
	Stop();

#ifdef PLAT_WINDOWS

	//  disable keyboard hook DLL
	if (!IKUtil::IsWin2KOrGreater())
		DllKeyEnable ( false );

#endif

}

int IKEngine::NumDevicesConnected()
{
	int n = 0;
	for (int i=0;i<MAX_INTELLIKEYS;i++)
	{
		if (m_devices[i].IsOpen())
			n++;
	}

	return n;
}

int IKEngine::NumDevicesOn()
{
	int n = 0;
	for (int i=0;i<MAX_INTELLIKEYS;i++)
	{
		if (m_devices[i].IsOpen() && m_devices[i].IsSwitchedOn())
			n++;
	}

	return n;
}

int IKEngine::NumDevicesWithStandardOverlay()
{
	int n = 0;
	for (int i=0;i<MAX_INTELLIKEYS;i++)
	{
		if (m_devices[i].IsOpen() && m_devices[i].HasStandardOverlay())
			n++;
	}

	return n;
}

void IKEngine::ProcessSentSettings(BYTE *settings)
{
	typedef struct
	{
		BYTE	OverlayLock;		// 0 - 1 (0)
		BYTE	PhotoLevelChange;	// 0
		BYTE	ResponseRate;		// 0 - 14 (14)
		BYTE	LiftOffFlag;		// 0 - 1 (0)
		BYTE	Keysound;			// 0 - 1 (1)
		BYTE	IBMRepeatFlag;		// 0 - 1 (1)
		BYTE	IBMRepeatRate;		// 0 - 14 (14)
		BYTE	RepeatLatching;		// 0 - 1 (0)
		BYTE	ShiftLatching;		// 0 - 2 (1)
		BYTE	LEDPairing;			// 0 - 1 (1)
		BYTE	MouseArrows;		// 0 - 1 (0)
		BYTE	MouseSpeed;			// 0 - 14 (5)
		BYTE	SmartTyping;		// 0 - 1 (0)
		BYTE	CustomLevel;		// 0 - 14 (0)
		BYTE	DataSendRate;		// 0 - 14 (14)
		BYTE	IBMStandardKeyboard;// 0 - 1 (1)
		BYTE	IBMCableOverride;	// 0 - 3 (0)
		BYTE	IBMOldATArrows;		// 0 - 1 (1)
		BYTE	CoordMode;			// 0 - 1 (0)
		BYTE	Reserved[5];
	} keyboardSettingsRec;

	keyboardSettingsRec *s = (keyboardSettingsRec *) settings;

	IKSettings *pSettings = IKSettings::GetSettings();

	//  this function takes the legacy keyboard settings
	//  and sets values in an IKSettings object to reflect their values.

	//  start with defaults values
	//m_settings.SetToDefault();

	//  skip custom overlay lock

	//  skip photo level change

	//  response rate

	if (s->ResponseRate!=0xFF)
		pSettings->m_iResponseRate = s->ResponseRate+1;

	//  lift off

	if (s->LiftOffFlag!=0xFF)
		pSettings->m_bRequiredLiftOff = (s->LiftOffFlag==1);

	//  key sound

	if (s->Keysound!=0xFF)
		if (s->Keysound==1)
			pSettings->m_iKeySoundVolume = kSettingsKeysound4;
		else
			pSettings->m_iKeySoundVolume = 0;

	//  repeat

	if (s->IBMRepeatFlag!=0xFF)
		pSettings->m_bRepeat = (s->IBMRepeatFlag==1);

	//  repeat rate

	if (s->IBMRepeatRate!=0xFF)
		pSettings->m_iRepeatRate = s->IBMRepeatRate+1;

	//  repeat latching

	if (s->RepeatLatching!=0xFF)
		pSettings->m_bRepeatLatching = (s->RepeatLatching==1);

	//  repeat latching

	if (s->ShiftLatching!=0xFF)
		if (s->ShiftLatching==0)
			pSettings->m_iShiftKeyAction = kSettingsShiftNoLatch;
		else if (s->ShiftLatching==1)
			pSettings->m_iShiftKeyAction = kSettingsShiftLatching;
		else if (s->ShiftLatching==2)
			pSettings->m_iShiftKeyAction = kSettingsShiftLocking;
		else if (s->ShiftLatching==3)
			pSettings->m_iShiftKeyAction = kSettingsShiftLocking;

	//  indicator lights

	if (s->LEDPairing!=0xFF)
		if (s->LEDPairing==0)
			pSettings->m_iIndicatorLights = kSettings6lights;
		else if (s->ShiftLatching==1)
			pSettings->m_iIndicatorLights = kSettings3lights;

	//  skip mouse arrows

	//  mouse speed

	if (s->MouseSpeed!=0xFF)
		pSettings->m_iMouseSpeed = s->MouseSpeed+1;

	//  smart typing

	if (s->SmartTyping!=0xFF)
		pSettings->m_bSmartTyping = (s->SmartTyping==1);

	//  skip custom overlay level
	
	//  data send rate
	if (s->DataSendRate!=0xFF)
		pSettings->m_iDataSendRate = s->DataSendRate+1;

	//  skip IBMStandardKeyboard
	//  skip IBMCableOverride
	//  skip IBMOldATArrows
	//  skip CoordMode
	//  skip 5 reserved values

	IKSettings::GetSettings()->Write();
	m_bRefreshCP = true;
	//IKControlPanel::Refresh();

}

void IKEngine::OnChangeStudent(IKString group, IKString student)
{
	//  save and persist the new values
	m_group   = group;
	m_student = student;
	m_preferences.SetValue(TEXT("Group"),group);
	m_preferences.SetValue(TEXT("Student"),student);

	//  make paths to the settings and applications files
	IKString path = IKUtil::MakeStudentPath ( group, student );
	IKString settingsFile = path;
	settingsFile += TEXT("settings.txt");
	//IKString appsFile = path;
	//appsFile += TEXT("applications.txt");

	//  create a setings file if one does not exist.
	if (!IKFile::FileExists(settingsFile))
	{
		IKSettings::GetSettings()->Write(settingsFile);
		IKFile::MakeWritable(settingsFile);
	}

	//  convert this user if it's older
	IKString oldfile = path;
	oldfile += TEXT("settings");
	ConvertUser ( oldfile, settingsFile );

	//  save a copy of the old settings
	IKSettings oldSettings = *(IKSettings::GetSettings());

	//  read new settings
	IKSettings::GetSettings()->Read(settingsFile);
	
	//  load the custom overlay
	IKString overlay = IKUtil::MakeStudentPath ( group, student );
	overlay += DATAS(TEXT("Custom_Overlay"),TEXT("custom.oms"));
	m_customOverlay.Unload();
	m_customOverlay.LoadFromFile(overlay);

	//  load the "use this" overlay
	overlay = IKSettings::GetSettings()->m_sUseThisOverlay;
	m_UseThisOverlay.Unload();
	m_UseThisOverlay.LoadFromFile(overlay);

	//  Load the switch settings overlay
	ReloadSwitchOverlay();
	
	//  if new settings constitutes a change in overlay data,
	//  reset devices.
	bool bReset = false;
	if (oldSettings.m_iMode != IKSettings::GetSettings()->m_iMode)
	{
		bReset = true;
	}
	else if (IKSettings::GetSettings()->m_iMode == kSettingsModeThisOverlay)
	{
		if (oldSettings.m_sUseThisOverlay != IKSettings::GetSettings()->m_sUseThisOverlay)
			bReset = true;
	}
	else if (IKSettings::GetSettings()->m_iMode == kSettingsModeSwitch)
	{
		if (oldSettings.m_iUseThisSwitchSetting != IKSettings::GetSettings()->m_iUseThisSwitchSetting)
			bReset = true;
	}
	if (bReset)
		OnNewOverlayData();

	//  make sure the current student has got pre-installed overlays
	//  add pre-disallowed apps

	bool bWatch = !!DATAI(TEXT("watch_apps"), 0);
	bool bAddPreinstalls = !!DATAI(TEXT("addpreinstalls"), 1);
	if (bWatch && bAddPreinstalls)
	{
		AppLibAddPreinstalledOverlays ( m_group, m_student, false );
		AppLibAddDisallowedApps ();
	}

	//  tell others who are interested
#ifdef PLAT_WINDOWS
	IKMessage::SendTO("system tray",kQuerySetStudent,0,0,0,0,250);
#endif

}

void IKEngine::OnNewCustomOverlayFile(IKString filename)
{
	//  make a path to where the file should be stored
	IKString newName = IKUtil::MakeStudentPath ( m_group, m_student );
	newName += DATAS(TEXT("Custom_Overlay"),TEXT("custom.oms"));

	//  make destination file writeable so it can be replaced.
	IKFile::MakeWritable(newName,2);
	
	//  delete the file first
	IKFile::Remove(newName);
	
	//  copy the file
	bool bCopied = IKFile::Copy(filename,newName);
	
	//make destination file writeable again just in case
	IKFile::MakeWritable(newName,2);

	//  load it
	if(bCopied)
	{
		//  stop Raw Mode.
		SetRawMode(false,DATAI(TEXT("raw_mode_immunity"),2000));

		ReloadCustomOverlayFile(newName);
		OnNewOverlayData();

		//  also apply settings from the file
		IKOverlay o;
        o.LoadFromFile(newName);
		BYTE *settings = o.GetRawSettings();
		OnNewCustomOverlaySettings(settings, 24);
	}
}

void IKEngine::OnNewCustomOverlayName(IKString name)
{
	m_customOverlay.SetName(name);
}

void IKEngine::AddDevice(int index, WriteDataProc proc, int devType)
{
//	SetRawMode(false);

	m_devices[index].Reset();
	m_devices[index].SetIndex(index);
	m_devices[index].SetWriteProc(proc);
	m_devices[index].SetOpen(true);
	m_devices[index].SetDevType(devType);

	m_devices[index].Start();

	m_bRefreshCP = true;
	//IKControlPanel::Refresh();
}

void IKEngine::RemoveDevice(int index)
{
//	SetRawMode(false);

	m_devices[index].Reset();
	m_devices[index].SetDevType(0);

	m_bRefreshCP = true;
	//IKControlPanel::Refresh();
}

void IKEngine::OnDataReceived(int index, BYTE *data)
{	
	m_devices[index].OnDataReceived ( data );
}

void IKEngine::SetDiagnosticMode(bool mode)
{
	m_bDiagnosticMode = mode;
}

bool IKEngine::GetDiagnosticMode()
{
	return m_bDiagnosticMode;
}

unsigned int g_immunityTime = 0;

void IKEngine::SetNotifyMode(bool mode)
{
	m_bNotifyMode = mode;
}


void IKEngine::SetRawMode(bool mode, int immunityTime /* =0 */)
{
#if 1
	//  calculate new immunity time if asked to do so
	unsigned int now = IKUtil::GetCurrentTimeMS();
	if (immunityTime>0)
		g_immunityTime = now + immunityTime;
	if (immunityTime<0)
		g_immunityTime = 0;

	//  don't set true if we're immune
	if (now<g_immunityTime && mode)
		return;
#endif

#if 0
	if (mode)
	{
		IKTRACE((TEXT("raw mode on")));
		for (int i=0;i<MAX_INTELLIKEYS;i++)
			m_devices[i].KeySound2(50,247);
	}
	else
	{
		IKTRACE((TEXT("raw mode OFF")));
		for (int i=0;i<MAX_INTELLIKEYS;i++)
			m_devices[i].KeySound2(50,200);
	}
#endif

	m_bRawMode = mode;

	for (int i=0;i<MAX_INTELLIKEYS;i++)
		m_devices[i].PurgeRawQueue();

}

bool IKEngine::GetRawMode()
{
	return m_bRawMode;
}

bool IKEngine::UseRawMode()
{
	if (!GetRawMode())
		return false;

	//  must be in overlay or discover mode for
	//  raw mode to work.

	IKSettings *pSettings = IKSettings::GetSettings();
	if (pSettings->m_iMode == kSettingsModeLastSentOverlay)
		return true;
	if (pSettings->m_iMode == kSettingsModeDiscover)
		return true;

	return false;
}

void IKEngine::MakeDiagnosticBlock ( BYTE *data, int *datalen )
{
	*datalen = 0;

	//  number of devices
	data[*datalen] = NumDevicesConnected();
	(*datalen)++;

	for (int i=0;i<MAX_INTELLIKEYS;i++)
	{
		if (m_devices[i].IsOpen())
		{
			//  device index
			data[*datalen] = i;
			(*datalen)++;

			//  on/off
			data[*datalen] = m_devices[i].IsSwitchedOn();
			(*datalen)++;

			//  sensor array
			m_devices[i].GetSensorArray (&(data[*datalen]));
			*datalen = *datalen + 9;

			//  level
			data[*datalen] = m_devices[i].GetLevel();
			(*datalen)++;

			//  is there a standard overlay?
			data[*datalen] = !!m_devices[i].HasStandardOverlay();
			(*datalen)++;

			IKString s;

			//  current overlay name
			IKOverlay *po = m_devices[i].GetCurrentOverlay();
			if(po != NULL)
				s = po->GetName();
			else
				s = TEXT("");

			data[*datalen] = s.GetLength();
			(*datalen)++;
			IKString::strcpy((TCHAR *)(&(data[*datalen])),(TCHAR *)s);
			*datalen = *datalen + s.GetByteCount();

			//  switch status
			m_devices[i].GetSwitchStatus (&(data[*datalen]), 1);
			*datalen = *datalen + 1;
			m_devices[i].GetSwitchStatus (&(data[*datalen]), 2);
			*datalen = *datalen + 1;

			//  membrane status
			int n;
			m_devices[i].GetMembraneStatus(&(data[*datalen]), &n);
			*datalen = *datalen + n;

		}
	}
}

void IKEngine::MakeIntellikeysArray(BYTE *data, int *datalen)
{
	*datalen = 0;

	//  number of devices
	data[*datalen] = NumDevicesConnected();
	(*datalen)++;

	for (int i=0;i<MAX_INTELLIKEYS;i++)
	{
		if (m_devices[i].IsOpen())
		{
			//  device index
			data[*datalen] = i;
			(*datalen)++;

			//  on/off
			data[*datalen] = m_devices[i].IsSwitchedOn();
			(*datalen)++;

			//  sensor array
			m_devices[i].GetSensorArray (&(data[*datalen]));
			*datalen = *datalen + 9;

			//  level
			data[*datalen] = m_devices[i].GetLevel();
			(*datalen)++;

			//  is there a standard overlay?
			data[*datalen] = !!m_devices[i].HasStandardOverlay();
			(*datalen)++;

			IKString s;

			//  current overlay name
			IKSettings *pSettings = IKSettings::GetSettings();
			if (!!m_devices[i].HasStandardOverlay() && m_devices[i].GetDevType()==1)
			{
				IKOverlay *po = m_devices[i].GetCurrentOverlay();
				if(po != NULL)
					s = po->GetName();
				else
					s = TEXT("");
			}
			else if (pSettings->m_iMode==kSettingsModeSwitch)
			{
				if (pSettings->m_iUseThisSwitchSetting > 0)
				{
					IKString key = TEXT("switch_file_");
					key += IKUtil::IntToString(pSettings->m_iUseThisSwitchSetting);
					s = DATAS(key,TEXT(""));
				}
				else
				{
					s = TEXT("");
				}
			}
			else 
			{
				IKOverlay *po = m_devices[i].GetCurrentOverlay();
				if(po != NULL)
					s = po->GetName();
				else
					s = TEXT("");
			}

			s.ToUTF8();

			data[*datalen] = s.GetLength();
			(*datalen)++;
			IKString::strcpy((TCHAR *)(&(data[*datalen])),(TCHAR *)s);
			*datalen = *datalen + s.GetByteCount();

			//  serial number
			s = m_devices[i].GetSerialNumber();
			data[*datalen] = s.GetLength();
			(*datalen)++;
			IKString::strcpy((TCHAR *)(&(data[*datalen])),(TCHAR *)s);
			*datalen = *datalen + s.GetByteCount();

			//  switch status
			m_devices[i].GetSwitchStatus (&(data[*datalen]), 1);
			*datalen = *datalen + 1;
			m_devices[i].GetSwitchStatus (&(data[*datalen]), 2);
			*datalen = *datalen + 1;

			//  membrane status
			int n;
			m_devices[i].GetMembraneStatus(&(data[*datalen]), &n);
			*datalen = *datalen + n;

			//  device type
			data[*datalen] = m_devices[i].GetDevType();
			*datalen = *datalen + 1;

			//  current overlay number
			data[*datalen] = m_devices[i].GetCurrentOverlayNumber();
			*datalen = *datalen + 1;

			//  more switch status (IK USB 3.1)
			m_devices[i].GetSwitchStatus (&(data[*datalen]), 3);
			*datalen = *datalen + 1;
			m_devices[i].GetSwitchStatus (&(data[*datalen]), 4);
			*datalen = *datalen + 1;
			m_devices[i].GetSwitchStatus (&(data[*datalen]), 5);
			*datalen = *datalen + 1;
		}
	}

}

IKString IKEngine::GetGroup()
{
	return m_group;
}

IKString IKEngine::GetStudent()
{
	return m_student;
}

IKDevice * IKEngine::GetFirstDevice()
{
	for (int i=0;i<MAX_INTELLIKEYS;i++)
	{
		if (m_devices[i].GetIndex()!=-1)
			return &(m_devices[i]);
	}

	return NULL;
}

IKDevice * IKEngine::GetNumberedDevice(int ndev)
{
	if (m_devices[ndev].GetIndex()!=-1)
		return &(m_devices[ndev]);

	return NULL;
}

void IKEngine::MakeGlobalData ( BYTE *data, int *datalen )
{
	IKString string;

	//  software version

    // JR - Dec 2012 - used the version macro in IKCommon.h
    string = INTELLIKEYS_USB_PRODUCT_VERSION;
	//string = DATAS(TEXT("Software_Version"),DATAS(TEXT("Unknown"),TEXT("Unknown")));
	data[*datalen] = string.GetLength();
	(*datalen)++;
	IKString::strcpy((TCHAR *)&(data[*datalen]),string);
	*datalen = *datalen + string.GetByteCount();

	//  firmware version

	//string = DATAS(TEXT("Firmware_Version"), DATAS(TEXT("Unknown"),TEXT("Unknown")));
    IKDevice* firstDevice = GetFirstDevice();
    if (firstDevice != NULL) {
        IKString s (GetFirstDevice()->GetFirmwareVersionString());
        string = s;
    }
    else {
        string = "Not Connected";
        //string = DATAS(TEXT("S_NOT_CONNECTED"), TEXT("Not Connected"));
    }
	data[*datalen] = string.GetLength();
	(*datalen)++;
	IKString::strcpy((TCHAR *)&(data[*datalen]),string);
	*datalen = *datalen + string.GetByteCount();

	//  last sent overlay

	string = IKSettings::GetSettings()->m_sLastSent;

	string.ToUTF8();
	data[*datalen] = string.GetLength();
	(*datalen)++;
	IKString::strcpy((TCHAR *)&(data[*datalen]),string);
	*datalen = *datalen + string.GetByteCount();
	
	//  sent by
	
	string = IKSettings::GetSettings()->m_sLastSentBy;

	string.ToUTF8();
	data[*datalen] = string.GetLength();
	(*datalen)++;
	IKString::strcpy((TCHAR *)&(data[*datalen]),string);
	*datalen = *datalen + string.GetByteCount();
}


void IKEngine::OnNewCustomOverlayData(BYTE *data, int datalen)
{
	//  stop Raw Mode.
		SetRawMode(false,DATAI(TEXT("raw_mode_immunity"),2000));

	//  make a path to where the overlay file should be stored
	IKString newName = IKUtil::MakeStudentPath ( m_group, m_student );
	newName += DATAS(TEXT("Custom_Overlay"),TEXT("custom.oms"));

	//  load the file into a temporary object
	IKOverlay ovl;
	ovl.LoadFromFile(newName);

	//  apply the given data as content
	ovl.StoreData(data,datalen);
	
	//  Get rid of old file here
	IKFile::MakeWritable(newName,2);
	IKFile::Remove(newName);

	//  save the resulting overlay file
	ovl.SaveToFile(newName);

	IKFile::MakeWritable(newName,2);

	//  ask engine to re-read the file
	ReloadCustomOverlayFile(newName);

	OnNewOverlayData();
        
	m_bRefreshCP = true;
	//IKControlPanel::Refresh();
}

void IKEngine::OnNewCustomOverlaySettings(BYTE *data, int datalen)
{
	//  make a path to where the overlay file should be stored
	IKString newName = IKUtil::MakeStudentPath ( m_group, m_student );
	newName += DATAS(TEXT("Custom_Overlay"),TEXT("custom.oms"));

	//  load the file into a temporary object
	IKOverlay ovl;
    ovl.LoadFromFile(newName);
        
	//  apply the given data to the overlay object as settings
	ovl.StoreSettings ( data );
        
    //  swap offset bytes.  Why?  I do not know.
    ovl.SwapOffsetBytes();

	//  save the resulting overlay file
	ovl.SaveToFile(newName);

	//  ask engine to re-read the file
	ReloadCustomOverlayFile(newName);
        
	//   also apply to the running engine
	ProcessSentSettings(data);

	m_bRefreshCP = true;
	//IKControlPanel::Refresh();

}

void IKEngine::ReloadCustomOverlayFile(IKString filename)
{
	bool bLoaded = m_customOverlay.LoadFromFile(filename);
	if (bLoaded)
	{
		for (int i=0;i<MAX_INTELLIKEYS;i++)
		{
			m_devices[i].OverlaySendingFeedback();
			m_devices[i].SetLevel(1);
		}
	}
	else
	{
		//  file copied but could not be loaded.
		//  delete and unload.
		IKFile::Remove(filename);
		m_customOverlay.Unload();
	}
}

IKOverlay * IKEngine::GetUseThisOverlay()
{
	return &m_UseThisOverlay;
}

IKOverlay * IKEngine::GetSwitchOverlay(int number)
{
	return &m_SwitchOverlay;
}

void IKEngine::DeleteLastSentOverlay()
{
	//  make a path to where the file is  stored
	IKString newName = IKUtil::MakeStudentPath ( m_group, m_student );
	newName += DATAS(TEXT("Custom_Overlay"),TEXT("custom.oms"));

	//  delete the file
	IKFile::MakeWritable(newName);
	IKFile::Remove(newName);

	//  Unload it
	m_customOverlay.Unload();

	//  forget settings
	IKSettings::GetSettings()->m_sLastSent = "";
	IKSettings::GetSettings()->m_sLastSentBy = "";
	IKSettings::GetSettings()->Write ();
	m_bRefreshCP = true;
}

void IKEngine::ConvertUser(IKString oldfile, IKString newfile)
{
	//  see if old file exists
	IKFile f;
	bool bOpened = f.Open ( oldfile, IKFile::modeRead);
	if (!bOpened)
		return;

	//  make a prefs object to hold the data we'll read
	IKPrefs settings;

	//  read past number of items
	BYTE b1, b2;
	f.Read(&b1,1); f.Read(&b2,1);
	int nItems = b1 + 256*b2;
	int nsaved = 0;

	//  read the items

	bool bDone = false;
	while (!bDone)
	{
		char name[100], val[100];
		BYTE lname, lval;

		int nb = f.Read(&lname,1);
		if (nb>0)
		{
			f.Read(name,lname);
			name[lname] = '\0';

			nb = f.Read(&lval,1);
			if ( nb>0 )
			{
				if (lval>0)
					f.Read(val,lval);
				val[lval] = '\0';

				//  add the item
				if (IKString::strcmp(name,"Group")!=0 && IKString::strcmp(name,"Student")!=0)
				{
					settings.SetValue(name,val);
					nsaved++;
				}
			}
			else
				bDone = true;

		}
		else
			bDone = true;

	}

	//  write the settings
	settings.Write(newfile);


	//  close and delete old file
	f.Close();
	IKFile::Remove(oldfile);

}

void IKEngine::ProcessExternalCommands2 ()
{
	BYTE data[2000];
	int datalen;
	int command;
	int response;

	const TCHAR *channel = TEXT("sending");
	
	response = IKMsg::Receive ( TEXT(channel), &command, (void *)data, 2000, &datalen );
	if (response == kResponseNoCommand)
		return;

	switch (command)
	{

	case kQuerySetSystemActive:
		{
			//  add to the list if it's not there.
			IKString system = UnpackString ( data, datalen );
			activeSystems.AddNoDup(system);
			IKMsg::Respond ( TEXT(channel),  kResponseSystemActive, 0, 0 );
		}
		break;

	case kQuerySetSystemInactive:
		{
			IKString system = UnpackString ( data, datalen );
			activeSystems.Remove(system);
			IKMsg::Respond ( TEXT(channel),  kResponseSystemInactive, 0, 0 );
		}
		break;

	case kQueryIsSystemActive:
		{
			IKString system = UnpackString ( data, datalen );
			int i = activeSystems.Find(system);
			if (i == -1)
				IKMsg::Respond ( TEXT(channel),  kResponseSystemInactive, 0, 0 );
			else
				IKMsg::Respond ( TEXT(channel),  kResponseSystemActive, 0, 0 );
		}
		break;


	case kQueryLaunchControlPanel:
		IKMsg::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		if (!IKMessage::IsOwnerAlive(TEXT("control panel")))
		{

#ifdef PLAT_WINDOWS
                    IKString app = IKUtil::GetRootFolder();
                    app += DATAS(TEXT("Control_Panel_Name"),"");
    
                    int nResult = (int) ShellExecute ( GetDesktopWindow(), 
                            TEXT("Open"), TEXT("Control"), TEXT("ikusb.cpl"), TEXT("C:\\"), 
                            SW_SHOWNORMAL );
#endif
#ifdef PLAT_MACINTOSH
                    IKString filename= IKUtil::GetRootFolder();
                    filename += DATAS(TEXT("Control_Panel_Name"),TEXT("IntelliKeys USB.app"));
                    IKUtil::LaunchFile(filename);
#endif
		}
		else
		{
			IKControlPanel::Show();
		}
		break;

	case kQueryBeep:
		{
			if (NumDevicesConnected()<=0)
				IKMsg::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
			else
			{
				IKMsg::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
				for (int i=0;i<MAX_INTELLIKEYS;i++)
				{
					m_devices[i].KeySound(500);
				}
			}
		}
		break;
	
	case kQueryShortBeep:
		{
			if (NumDevicesConnected()<=0)
				IKMsg::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
			else
			{
				IKMsg::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
				for (int i=0;i<MAX_INTELLIKEYS;i++)
				{
					m_devices[i].ShortKeySound();
				}
			}
		}
		break;
	
	case kQuerySendSettings:
	case DNLD_SET:
	    IKMsg::Respond ( TEXT(channel),  NO_ERR, 0, 0 );
		OnNewCustomOverlaySettings(data,datalen);
	    break;
	    
	case DNLD_OVL:
	    IKMsg::Respond ( TEXT(channel),  NO_ERR, 0, 0 );
		OnNewCustomOverlayData(data,datalen);
		{
			BYTE data[2000];
			int ndata = 0;
			IKEngine::GetEngine()->MakeGlobalData(data,&ndata);
			IKMessage::Send(TEXT("control panel"),kQueryGlobalData,data,ndata,0,0);
		}
	    break;
	    
	case UPLD_SET:
            //  TODO: return the real settings.
            //  although, we don't do that for IKUSB 1.x.
            datalen = sizeof ( LegacyIKSettings );
            memset ( data, 0, datalen );
	    IKMsg::Respond ( TEXT(channel),  NO_ERR, data, datalen );
	    break;

	case kQueryIsUSBIntelliKeysConnected:
	case kQueryIsIntellikeysConnected:
		{
			if (NumDevicesConnected()>0)
				IKMsg::Respond ( TEXT(channel),  kResponseConnected, 0, 0 );
			else
				IKMsg::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
		}
		break;

	case kQueryShutdown:
		IKMsg::Respond(TEXT(channel),kResponseNoError,0,0);
		Stop();
		break;

	case kQueryShowControlPanel:
		IKMsg::Respond(TEXT(channel),kResponseNoError,0,0);
		IKControlPanel::Show();
		break;

	case kQueryIsStandardOverlayInPlace:
		{
			if (NumDevicesWithStandardOverlay()>0)
				IKMsg::Respond ( TEXT(channel),  kResponseStandardOverlayIsInPlace, 0, 0 );
			else
				IKMsg::Respond ( TEXT(channel),  kResponseStandardOverlayIsNotInPlace, 0, 0 );
		}
		break;

	case kQueryIsIntellikeysOn:
		{
			if (NumDevicesOn()>0)
				IKMsg::Respond ( TEXT(channel),  kResponseOn, 0, 0 );
			else
				IKMsg::Respond ( TEXT(channel),  kResponseNotOn, 0, 0 );
		}
		break;

	case kQuerySetStudent:
		{
			IKMsg::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
			MakeAllFilesWriteable ( IKUtil::GetUsersFolder() );

			int n = 0;
			int len;

			int size = data[n];
			n++;

			len = data[n];
			n++;
			IKString group ( (BYTE *)&(data[n]), len*size );
			n = n + len*size;

			len = data[n];
			n++;
			IKString student ( (BYTE *)&(data[n]), len*size );
			n = n + len*size;

			OnChangeStudent(group,student);
		}
		break;

	case kQuerySendOverlay:
		{
			IKString nameIn(data,datalen);  //  could be unicode string
			OnNewCustomOverlayFile (nameIn);
		}
		IKMsg::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		break;

	case kQuerySendOverlayName:
		{
			IKString nameIn(data,datalen);  //  could be unicode string
			OnNewCustomOverlayName (nameIn);
		}
		IKMsg::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		break;

	case kQuerySendOverlayWithName:
		{
			int n = 0;
			int len;

			int size = data[n];
			n++;

			len = data[n];
			n++;
			IKString file ( (BYTE *)&(data[n]), len*size );
			n = n + len*size;

			len = data[n];
			n++;
			IKString name ( (BYTE *)&(data[n]), len*size );
			n = n + len*size;

			len = data[n];
			n++;
			IKString sender ( (BYTE *)&(data[n]), len*size );
			n = n + len*size;

			OnNewCustomOverlayFile (file);
			OnNewCustomOverlayName (name);
			OnNewCustomOverlaySender ( name, sender );

			IKMsg::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		}
		break;

	case kQueryResetKeyboard:
		
		{
			if (NumDevicesConnected()<=0)
				IKMsg::Respond ( TEXT(channel),  kResponseNotConnected, 0, 0 );
			else
			{
				IKMsg::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
				for (int i=0;i<MAX_INTELLIKEYS;i++)
				{
					m_devices[i].UserReset(true);
				}
			}
		}
		break;

	case kQueryStartDiagnosticMode:
		IKMsg::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetDiagnosticMode (true);
		break;

	case kQueryStopDiagnosticMode:
		IKMsg::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetDiagnosticMode (false);
		break;

	case kQueryStartRawMode:
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetRawMode (true);
		break;

	case kQueryStopRawMode:
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetRawMode (false);
		break;

	case kQuerySetNotifyModeOn:
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetNotifyMode (true);
		break;
		
	case kQuerySetNotifyModeOff:
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		SetNotifyMode (false);
		break;

	case kQueryGetRawMode:
		{
			if (GetRawMode())
				IKMessage::Respond ( TEXT(channel),  kResponseRawModeOn, 0, 0 );
			else
				IKMessage::Respond ( TEXT(channel),  kResponseRawModeOff, 0, 0 );
		}
		break;

	case kQueryGetRawModeUsable:
		{
			if (GetRawMode() && NumDevicesConnected()>0 && NumDevicesConnected()>0)
				IKMessage::Respond ( TEXT(channel),  kResponseRawModeUsable, 0, 0 );
			else
				IKMessage::Respond ( TEXT(channel),  kResponseRawModeUnusable, 0, 0 );
		}
		break;

	case kQuerySoftwareVersion:
		{
			IKString version = DATAS(TEXT("Software_Version"),TEXT(""));
			BYTE data[255];
			IKString::strcpy((TCHAR *)data,(TCHAR *)version);
			int datalen = version.GetByteCount();
			IKMsg::Respond ( TEXT(channel),  kResponseSoftwareVersion, data, datalen );
		}
		break;

	case kQueryFirmwareVersion:
		{
            IKString version = DATAS(TEXT("Firmware_Version"),DATAS(TEXT("Unknown"),TEXT("Unknown")));
            IKDevice* firstDevice = GetFirstDevice();
            if (firstDevice != NULL) {
                IKString s (GetFirstDevice()->GetFirmwareVersionString());
                version = s;
            }
			BYTE data[255];
			IKString::strcpy((TCHAR *)data,(TCHAR *)version);
			int datalen = version.GetByteCount();
			IKMsg::Respond ( TEXT(channel),  kResponseFirmwareVersion, data, datalen );
		}
		break;

	case kQueryLastSentOverlay:
		{
			//IKString overlay = m_customOverlay.GetName();
			IKString overlay = IKSettings::GetSettings()->m_sLastSent;
			//overlay.ToUTF8();
			BYTE data[255];
			IKString::strcpy((TCHAR *)data,(TCHAR *)overlay);
			int datalen = overlay.GetByteCount();
			IKMsg::Respond ( TEXT(channel),  kResponseLastSentOverlay, data, datalen );
		}
		break;

	case kQueryUSBIntelliKeysArrayChanged:
		{
			BYTE data[1000];
			int datalen = 0;
			MakeIntellikeysArray ( data, &datalen );
			IKMsg::Respond ( TEXT(channel),  kResponseUSBIntelliKeysArrayChanged, data, datalen );
		}
		break;

	case kQueryGlobalData:
		{
			BYTE data[1000];
			int datalen = 0;
			MakeGlobalData ( data, &datalen );
			IKMsg::Respond ( TEXT(channel),  kResponseGlobalData, data, datalen );
		}
		break;

	case kQueryUSBIntelliKeysArray:
		{
			BYTE data[1000];
			int datalen = 0;
			MakeIntellikeysArray ( data, &datalen );
			IKMsg::Respond ( TEXT(channel),  kResponseUSBIntelliKeysArray, data, datalen );
		}
		break;

	case kQueryDiagnosticDataBlock:
		{
			BYTE data[1000];
			int datalen = 0;
			MakeDiagnosticBlock ( data, &datalen );
			IKMsg::Respond ( TEXT(channel),  kResponseDiagnosticDataBlock, data, datalen );
		}
		break;

	case kQueryGetSettings:
	case kQueryShouldCallOldInTT:
		//  unsupported commands
		IKMsg::Respond ( TEXT(channel),  kResponseUnsupported, 0, 0 );
		break;

	case kQueryReloadStudent:
		LoadStudentData();
		IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
		break;

	default:
		//  unknown commands
		IKMsg::Respond ( TEXT(channel),  kResponseError, 0, 0 );
		break;
	}
}



void IKEngine::ReloadSwitchOverlay()
{
	m_SwitchOverlay.Unload();

	int nsw = IKSettings::GetSettings()->m_iUseThisSwitchSetting;
	if (nsw>0 && nsw<=m_nSwitchOverlays)
	{
		IKString overlay = IKUtil::GetSwitchSettingsFolder();
		overlay += m_switchOverlays[nsw-1];
		m_SwitchOverlay.LoadFromFile(overlay);
	}

}

void IKEngine::OnNewOverlayData()
{
	for (int i=0;i<MAX_INTELLIKEYS;i++)
	{
		m_devices[i].ResetKeyboard();
		m_devices[i].ResetMouse();
		m_devices[i].SetLevel(1);
	}
}

void IKEngine::MakeAllFilesWriteable(IKString startingFolder)
{
	IKString strType = "*.*";

#ifdef PLAT_WINDOWS

	// windows implementation

	IKString strPath = startingFolder;

	if ( ! strPath.GetLength() )
		return;

	WIN32_FIND_DATA findFileData;

	if ( strPath.GetAt ( strPath.GetLength() - 1 ) != '\\' )
		strPath += '\\';

	IKString s = strPath;
	s += strType;
	HANDLE h = ::FindFirstFile( s, &findFileData );
	
	if ( h == INVALID_HANDLE_VALUE )
		return;

	while (TRUE )
	{
		// sss: filter out "." and ".."
		if ( ( IKString::strcmp ( findFileData.cFileName, TEXT(".") ) != 0 ) &&
			 ( IKString::strcmp ( findFileData.cFileName, TEXT("..") ) != 0 ) )
		{

			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				//  this is a dir
				IKString s = startingFolder;
				s += findFileData.cFileName;
				s += "\\";
				MakeAllFilesWriteable(s);
			}
			else
			{
				//  it's not
				IKString s = startingFolder;
				s += findFileData.cFileName;
				IKFile::MakeWritable(s);
			}
		}

		if ( ! ::FindNextFile( h, &findFileData ) )
			break;
	}

	::FindClose ( h );

#else

	//  NADA

#endif

}


#ifdef PLAT_WINDOWS


void GetVersionOfFile (char * pszAppName, // file
                       char * pszVerBuff, // receives version
                       int     iVerBuffLen, // size of buffer
                       char * pszLangBuff, // receives language
                       int iLangBuffLen) // size of buffer
{
   DWORD dwScratch;
   DWORD * pdwLangChar;
   DWORD dwInfSize ;
   UINT uSize;
   BYTE * pbyInfBuff;
   char szVersion [32];
   char szResource [80];
   char * pszVersion = szVersion;

   dwInfSize = GetFileVersionInfoSize (pszAppName, &dwScratch);

   if (dwInfSize)
   {
      pbyInfBuff = new BYTE [dwInfSize];
      memset (pbyInfBuff, 0, dwInfSize);
      if (pbyInfBuff)
      {
         if (GetFileVersionInfo (pszAppName, 0, dwInfSize, pbyInfBuff))
         {
            if (VerQueryValue (pbyInfBuff, 
                               "\\VarFileInfo\\Translation",
                               (void**)(&pdwLangChar),
                               &uSize))
            {
               if (VerLanguageName (LOWORD (*pdwLangChar), 
                                    szResource,
                                    sizeof(szResource)))
               {
                  strncpy (pszLangBuff, szResource, iLangBuffLen);
               }
               wsprintf (szResource, "\\StringFileInfo\\%04X%04X\\FileVersion",
                         LOWORD (*pdwLangChar), HIWORD (*pdwLangChar));

               if (VerQueryValue (pbyInfBuff, 
                                  szResource,
                                  (void**)(&pszVersion),
                                  &uSize))
               {
                  strncpy (pszVerBuff, pszVersion, iVerBuffLen-1);
               }
            }
         }
         delete [] pbyInfBuff;
      }
   }
}


static void GetVersion ( char *filename, int *major, int *minor, int *fix, int *build )
{
	char verstring[255];
	char langstring[255];
	GetVersionOfFile ( filename, verstring, 255, langstring, 255);

	int n = 0;
	int j = 0;
	for (int i=0;i<255;i++)
	{
		if (verstring[i]=='.' || verstring[i]==',')
		{
			char numeral[255];
			for (int k=j;k<i;k++)
				numeral[k-j] = verstring[k];
			numeral[k] = 0;

			n++;
			switch(n)
			{
			case 1:
				*major = atoi(numeral);
				break;
			case 2:
				*minor = atoi(numeral);
				break;
			case 3:
				*fix = atoi(numeral);
				break;
			case 4:
				*build = atoi(numeral);
				break;
			}
			j = i+1;
		}

		if(n==4)
			break;
	}

}

static void ReplaceFile ( char *src, char *dst )
{
	//  copy the file
	IKFile::SetFileLocked(dst, false);
	bool bCopied = !!::CopyFile(dst, src, false);
	if (!bCopied)
	{
		//	::MessageBox(NULL,"copy failed","aaa",MB_OK|MB_ICONEXCLAMATION);
	}
}

void IKEngine::FixIkeysXfr()
{
	//  make sure the correct IKEYSXFR is in place

	//  get current system path
	char sys[_MAX_PATH];
	::GetSystemDirectory( sys, sizeof ( sys ) );
	IKString pathName(sys);

	//  add the executable name
	IKString ikeysxfrName = DATAS (TEXT("ikeysxfr_name"), TEXT("ikeysxfr.exe"));
	pathName += "\\";
	pathName += ikeysxfrName;

	//  what version is that file?
	int major=0, minor=0, fix=0, build=0;
	GetVersion (pathName , &major, &minor, &fix, &build );

	if (major!=0 && major<=1)
	{
		//  we've got 1.x.  Install a new one.

		IKString newFile = IKUtil::GetPrivateFolder();
		newFile += ikeysxfrName;

		ReplaceFile(pathName,newFile);
	}
}
#endif


void IKEngine::DisconnectDevices()
{
	for (int i=0;i<MAX_INTELLIKEYS;i++)
		m_devices[i].Disconnect();
}

void IKEngine::WatchApplications()
{	
	bool bForce = m_bForceSendAttachedOverlay;
	m_bForceSendAttachedOverlay = false;

	//  have we changed apps?
	bool bChanged = false;
	static IKString lastApp = TEXT("");
#ifdef BUILD_DAEMON
	IKString thisApp = m_currentApp;
#else
	IKString thisApp = IKUtil::GetCurrentApplicationPath();
#endif

	if (!thisApp.IsEmpty())
		if (lastApp.CompareNoCase(thisApp)!=0)
			bChanged = true;
	
	if (bChanged || bForce)
	{
		SwitchApplication(thisApp);
		lastApp = thisApp;
	}
}

void IKEngine::SendOverlayForApp ( IKString &filename, IKString &sender )
{
	//  do some mode checking first
	int Mode = IKSettings::GetSettings()->m_iMode;
	bool bButAllow = IKSettings::GetSettings()->m_bButAllowOverlays;

	//bool bModeWarning = IKSettings::GetSettings()->m_bShowModeWarning;
	
	IKString s;
	switch (Mode)
	{
	case 1:
#if 0  //  NO FLOATING MESSAGE 7/25/06
		if (bModeWarning)
		{
			s = DATAS("s_overlay_not_sent_1","");
			s.ToACP();
			AppLibFloatingMessage ( (TCHAR *)s );
		}
#endif
		return;
		break;

	case 2:
#if 0  //  NO FLOATING MESSAGE 7/25/06
		if (bModeWarning)
		{
			s = DATAS("s_overlay_not_sent_2","");
			s.ToACP();
			AppLibFloatingMessage ( (TCHAR *)s );
		}
#endif
		return;
		break;

	case 3:
		if (bButAllow)
		{
			//  allow the overlay to be sent anyway
			SetRawMode(false,DATAI(TEXT("raw_mode_immunity"),2000));
		}
		else
		{
#if 0  //  NO FLOATING MESSAGE 7/25/06
			if (bModeWarning)
			{
				s = DATAS("s_overlay_not_sent_3","");
				s.ToACP();
				AppLibFloatingMessage ( (TCHAR *)s );
			}
#endif
			return;
		}
		break;

	default:
		break;
	}


	//  format a message and play the banner
	IKString simple = filename;
	IKUtil::StripFileName ( simple, true, true );
	
	IKString message;
	message += DATAS(TEXT("sending"),TEXT("sending"));
	message += TEXT(" ");
	message += simple;
	message += TEXT(" ");
	message += DATAS(TEXT("for"),TEXT("for"));
	message += TEXT(" ");
	message += sender;
	message += TEXT(" ...");
	
#ifdef PLAT_MACINTOSH_X
	
	//  use the messaging system to get this done.
		
	IKString msg;
	msg += filename;    msg += TCHAR(1);  //  file name
	msg += message;     msg += TCHAR(1);  //  banner
	msg += TEXT("0");   msg += TCHAR(1);  //  don't report errors
	msg += TEXT("0");   msg += TCHAR(1);  //  don't wait
	msg += TEXT("1");   msg += TCHAR(1);  //  banner only
	
	int len = msg.GetLength();
	TCHAR * pMsg = (TCHAR *)msg;
	int result = IKMessage::Send ( TEXT("menu"), kQuerySendOverlayProxy, (void *) pMsg, len, 0, 0 );	
	
#endif
	
#ifdef PLAT_WINDOWS
	
	//  OK to call directly here, since it results in
	//  starting a separate program
	//  convert from UTF-8 to ACP first.

	int len = message.GetLength();
	unsigned short wide[1024];
	int res = MultiByteToWideChar ( CP_UTF8, 0, message, len, wide, 1024);
	wide[res] = 0;

	char message2[1024];
	int result2 = WideCharToMultiByte ( CP_ACP, 0, wide, res, message2, 1024, 0, 0);
	message2[result2] = 0;

	
	int result = IntelliKeys::SendOverlay ( filename, false, message2, false, true );
	
#endif
	
	//  send the overlay internally
	OnNewCustomOverlayFile (filename);

	//  tell comtrol panel
	IKString newSender;
	newSender = DATAS(TEXT("intellikeys_usb"),TEXT("IntelliKeys USB"));
	newSender += TEXT(" (");
	newSender += DATAS(TEXT("for"),TEXT("for"));
	newSender += TEXT(" ");
	newSender += sender;
	newSender += TEXT(")");

	IKString name(filename);
	IKUtil::StripFileName(name,true,true);
	OnNewCustomOverlaySender ( name, newSender );

}

//  The following function is called every time the Engine
//  sees a new application in the foreground.

void IKEngine::SwitchApplication(IKString &app)
{
	//  get the application's friendly name
	TCHAR *pFriendly = ::AppLibGetAppFriendlyName((TCHAR *)app);
	IKString friendly(pFriendly);

	//  get the overlay to send for the current group/student
	IKString group, student;
	GetCurrentGroupAndStudent(group,student);
	TCHAR *pOverlay = AppLibGetStudentAttachedOverlay ( (TCHAR *)app, (TCHAR *)group, (TCHAR *)student );

	//  get the app's current owner
	IKString owner = AppLibGetOwner((TCHAR *)app);

	//  IK USB 3.0.3
	//  if there is an owner who is not us, release it.
	//  in 3.0.3, only us and blank are valid owners.
	if (owner.GetLength()>0 && owner.CompareNoCase(TEXT("intellikeys usb"))!=0)
	{
		AppLibReleaseOwnership((TCHAR *)app,(TCHAR *)owner);
		owner = "";
	}

	//  what mode are we currently in?
	int Mode = IKSettings::GetSettings()->m_iMode;

	//  set Raw Mode based on the control panel
	if (Mode==3)
		SetRawMode(true,-1);
	else
		SetRawMode(false,-1);

#ifndef PLAT_WINDOWS
	//  consider sending an overlay only if
	//  there are connected devices and there is an overlay to send.
	//  and we are the owner.
	if (CountOpenDevices()>0 && strlen(pOverlay)>0 && owner.CompareNoCase(TEXT("intellikeys usb"))==0)
	{
		IKString overlay(pOverlay);
		if (IKFile::FileExists(overlay))
		{
			//  send it.
			SendOverlayForApp (overlay,friendly);
		}
		else
		{
#if 0  //  NO FLOATING MESSAGE 7/25/06
			//  overlay not found
			IKString message;
			
			IKString simple(overlay);
			IKUtil::StripFileName(simple,true,true);

			message += DATAS(TEXT("S_error_sending_overlay"),TEXT("Error Sending Overlay"));
			message += "|";

			message += DATAS(TEXT("S_overlay"),TEXT("overlay"));
			message += " ";
			message += "\\\"";
			message += simple;
			message += "\\\"";
			message += " ";

			message += DATAS(TEXT("S_attached_to_application"),TEXT("attached to application"));
			message += " ";
			message += "\\\"";
			message += friendly;
			message += "\\\"";
			message += " ";
			message += DATAS(TEXT("S_cannot_be_sent"),TEXT("cannot be sent.  It may have been moved or deleted."));

			if (IKSettings::GetSettings()->m_bShowModeWarning)
				AppLibFloatingMessage(message);
#endif
		}
	}
#endif
}

void IKEngine::LoadStudentData()
{
	//  get current group and user
	IKString group, student;
	GetCurrentGroupAndStudent(group,student);

	//  load 'em
	OnChangeStudent(group,student);
}

int IKEngine::CountOpenDevices()
{
	int result = 0;

	for (int i=0;i<MAX_INTELLIKEYS;i++)
		if (m_devices[i].IsOpen())
			result++;

	return result;
}

void IKEngine::GetCurrentGroupAndStudent(IKString &group, IKString &student)
{
	//  apply default group and user
	group   = m_preferences.GetValueString(TEXT("Group"),  TEXT(""));
	student = m_preferences.GetValueString(TEXT("Student"),TEXT(""));
	if (group.IsEmpty() && student.IsEmpty())
	{
		group = DATAS(TEXT("Guest"),TEXT(_M(S_GUEST)));
		m_preferences.SetValue(TEXT("Group"),group);
	}
}

void IKEngine::OnNewCustomOverlaySender ( IKString name, IKString sender )
{
	IKSettings::GetSettings()->m_sLastSent   = name;
	IKSettings::GetSettings()->m_sLastSentBy = sender;

	IKSettings::GetSettings()->Write();
	m_bRefreshCP = true;
	//IKControlPanel::Refresh();
}

static IKString GetLastFolder ( IKString &input )
{
	//  where does it end?
	int iEnd = input.GetLength()-2;  //  assume that the string ends wit a delimiter

	//  where does it start?
	int iStart = iEnd;
	while (input.GetAt(iStart-1)!=IKUtil::GetPathDelimiter())
		iStart--;

	//  give it up
	return input.Mid(iStart,(iEnd-iStart+1));
}

void IKEngine::CheckNewInstall()
{
	//  name a file
	IKString mapPath = IKUtil::GetPrivateFolder();
	mapPath += TEXT("installed.txt");

	//  load it as a map
	IKMap installMap;
	installMap.RemoveAll();
	installMap.Read(mapPath);

	//  look for a certain key that says we've done this
	IKString processed = installMap.Lookup("processed_new_install");

	//  if we've already done this,leave.  Otherwise
	//  mark the map and keep going.
	if (!processed.IsEmpty())
		return;
	installMap.Add("processed_new_install","xxx");
	installMap.Write();


	bool bWatch = !!DATAI(TEXT("watch_apps"), 0);
	bool bAddPreinstalls = !!DATAI(TEXT("addpreinstalls"), 1);
	if (bWatch && bAddPreinstalls)
	{
		//  get group/user pairs
		IKStringArray groups, users;

		IKStringArray groupFolders;
		IKUtil::BuildFolderArray(groupFolders,IKUtil::GetUsersFolder());
		int g;
		for (g=0;g<groupFolders.GetSize();g++)
		{
			IKString groupFolder = groupFolders.GetAt(g);
			IKString groupName = GetLastFolder(groupFolder);

			IKStringArray userFolders;
			IKUtil::BuildFolderArray(userFolders,groupFolder);
			int u;
			for (u=0;u<userFolders.GetSize();u++)
			{
				IKString userFolder = userFolders.GetAt(u);
				IKString userName = GetLastFolder(userFolder);

				groups.Add(groupName);
				users.Add (userName);
			}
		}
		// also Guest
		groups.Add(DATAS(TEXT("Guest"),TEXT(_M(S_GUEST)))), users.Add(TEXT(""));

		//  for each user, update the preinstalled apps and overlays
		int i;
		for (i=0;i<groups.GetSize();i++)
		{
			AppLibAddPreinstalledOverlays ( groups.GetAt(i), users.GetAt(i), true );
		}
		AppLibAddDisallowedApps ();

	}
}


void IKEngine :: PostKey   (int code, int direction)
{
	//  send this to the first open device
	for (int i=0;i<MAX_INTELLIKEYS;i++)
	{
		if (m_devices[i].IsOpen())
		{
			m_devices[i].PostKey(code, direction, 0 );
			return;
		}
	}
}

void IKEngine :: PostDelay   (int msDelay)
{
	//  send this to the first open device
	for (int i=0;i<MAX_INTELLIKEYS;i++)
		if (m_devices[i].IsOpen())
		{
			m_devices[i].PostDelay(msDelay);
			return;
		}
}

void IKEngine :: PostMouseButtonToggle(int whichButton)
{
	//  send this to the first open device
	for (int i=0;i<MAX_INTELLIKEYS;i++)
		if (m_devices[i].IsOpen())
		{
			m_devices[i].PostMouseButtonToggle(whichButton);
			return;
		}
}

void IKEngine :: PostMouseButtonClick(int whichButton)
{
	//  send this to the first open device
	for (int i=0;i<MAX_INTELLIKEYS;i++)
		if (m_devices[i].IsOpen())
		{
			m_devices[i].PostMouseButtonClick(whichButton);
			return;
		}
}

void IKEngine :: PostMouseButtonDoubleClick(int whichButton)
{
	//  send this to the first open device
	for (int i=0;i<MAX_INTELLIKEYS;i++)
		if (m_devices[i].IsOpen())
		{
			m_devices[i].PostMouseButtonDoubleClick(whichButton);
			return;
		}
}

void IKEngine :: PostMouseButton(int whichButton, int direction)
{
	//  send this to the first open device
	for (int i=0;i<MAX_INTELLIKEYS;i++)
		if (m_devices[i].IsOpen())
		{
			m_devices[i].PostMouseButton(whichButton, direction);
			return;
		}
}

void IKEngine :: PostMouseMove(int x, int y)
{
	//  send this to the first open device
	for (int i=0;i<MAX_INTELLIKEYS;i++)
		if (m_devices[i].IsOpen())
		{
			m_devices[i].PostMouseMove(x,y);
			return;
		}
}

void IKEngine :: ResetMouseInterface()
{
	//  send this to the first open device
	for (int i=0;i<MAX_INTELLIKEYS;i++)
		if (m_devices[i].IsOpen())
		{
			m_devices[i].ResetMouse();
			return;
		}
}

void IKEngine :: ResetKeyboardInterface()
{
	//  send this to the first open device
	for (int i=0;i<MAX_INTELLIKEYS;i++)
		if (m_devices[i].IsOpen())
		{
			m_devices[i].ResetKeyboard();
			return;
		}
}
