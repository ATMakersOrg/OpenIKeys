// IKControlPanel.cpp: implementation of the IKControlPanel class.
//
//////////////////////////////////////////////////////////////////////

#include "IKCommon.h"
#include "IKControlPanel.h"
#include "IKMessage.h"
#include "IKUtil.h"
#include "IKFile.h"
#include "IKEngine.h"
//#include "IKDevice.h"
#include "IKSettings.h"

#define _M(a) DATAS((TCHAR *)TEXT(#a),(TCHAR *)TEXT(#a))

#define OUTPUTS(a,b,c) { TCHAR s[255]; MySprintf(s,TEXT("%s = %s"),(TCHAR *)b,(TCHAR *)c); a.WriteLine(s);}
#define OUTPUTD(a,b,c) { TCHAR s[255]; MySprintf(s,TEXT("%s = %d"),(TCHAR *)b,         c); a.WriteLine(s);}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void IKControlPanel::Refresh()
{
	//if (IKEngine::GetEngine()->GetDiagnosticMode())
		//return;

	if (IKMessage::IsOwnerAlive(TEXT("control panel")))
	{
		BYTE data[2000];
		int ndata = 0;

		//  global data
		ndata = 0;
		IKEngine::GetEngine()->MakeGlobalData(data,&ndata);
		IKMessage::Send(TEXT("control panel"),kQueryGlobalData,data,ndata,0,0);

		//  intellikeys data
		ndata = 0;
		IKEngine::GetEngine()->MakeIntellikeysArray(data,&ndata);
		IKMessage::Send(TEXT("control panel"),kQueryUSBIntelliKeysArray,data,ndata,0,0);

		//  re-read student data

		IKString group   = IKEngine::GetEngine()->GetGroup();
		IKString student = IKEngine::GetEngine()->GetStudent();

		//  do we have unicode strings?
		ndata = 0;
		data[ndata] = sizeof(TCHAR);
		ndata++;

		//  length of group in chars
		data[ndata] = IKString::strlen(group);
		ndata++;

		//  chars for group
		IKString::strcpy((TCHAR *)&(data[ndata]),group);
		ndata = ndata + IKString::strlen(group) * sizeof(TCHAR);

		//  length of student in chars
		data[ndata] = IKString::strlen(student);
		ndata++;

		//  chars for student
		IKString::strcpy((TCHAR *)&(data[ndata]),student);
		ndata = ndata + IKString::strlen(student) * sizeof(TCHAR);

		IKMessage::Send(TEXT("control panel"),kQuerySetStudent,data,ndata,0,0);

		//  show yourself
		IKMessage::Send(TEXT("control panel"),kQueryShowControlPanel,0,0,0,0);
	}
}

void IKControlPanel::Toggle()
{
	if (!IKMessage::IsOwnerAlive(TEXT("control panel")))
	{
		Launch();
	}
	else
	{
		IKMessage::Send(TEXT("control panel"),kQueryShutdown,0,0,0,0);
	}
}

void IKControlPanel::ListFeatures()
{
    static int nf = 0;
    IKFile f;
    IKString filename;
    int ntries = 0;
    while (true)
    {
        //  make a new filename
        nf++;
        filename = IKUtil::GetPrivateFolder();
        TCHAR shortname[255];
        MySprintf(shortname,TEXT("Display Features %d.txt"),nf);        
        IKString s(shortname);
        filename += s;
        
        //  delete it
        IKFile::Remove(filename);
        
        //  try to open it
        if (f.Open ( filename, IKFile::modeCreate | IKFile::modeReadWrite ))
            break;
            
        //  too many tries?
        if (ntries>100)
            return;
    }

	IKEngine   *pEngine   = IKEngine::GetEngine();
	IKSettings *pSettings = IKSettings::GetSettings();

	//  header

	TCHAR header[255];
	IKString group = pEngine->GetGroup();
	IKString student = pEngine->GetStudent();
	if (group.CompareNoCase(_M(S_GUEST))==0)
		MySprintf(header,(TCHAR *)_M(S_LF_HEADER1),(TCHAR *)group);
	else
		MySprintf(header,(TCHAR *)_M(S_LF_HEADER2),(TCHAR *)student,(TCHAR *)group);
	IKString sheader(header);
	f.WriteLine(sheader);
	f.WriteLine(TEXT(""));

	//  versions
	OUTPUTS ( f, _M(S_CP_SOFTWARE_VERSION), _M(Software_Version) );
	OUTPUTS ( f, _M(S_CP_FIRMWARE_VERSION), _M(Firmware_Version) );
	f.WriteLine(TEXT(""));

	// touch features
	OUTPUTD ( f, _M(S_LISTFEATURES_RESPONSE_RATE),	pSettings->m_iResponseRate );
	OUTPUTD ( f, _M(S_LISTFEATURES_REPEAT_RATE),	pSettings->m_iRepeatRate );
	OUTPUTD ( f, _M(S_LISTFEATURES_MOUSE_SPEED),	pSettings->m_iMouseSpeed );
	OUTPUTD ( f, _M(S_LISTFEATURES_DATA_SEND_RATE), pSettings->m_iDataSendRate );

	OUTPUTS ( f, _M(S_LISTFEATURES_SMART_TYPING),		BoolToText(pSettings->m_bSmartTyping) );
	OUTPUTS ( f, _M(S_LISTFEATURES_REQUIRED_LIFT_OFF),	BoolToText(pSettings->m_bRequiredLiftOff) );
	OUTPUTS ( f, _M(S_LISTFEATURES_KEY_SOUND_VOLUME),	VolToText(pSettings->m_iKeySoundVolume) );
	OUTPUTS ( f, _M(S_LISTFEATURES_REPEAT),				BoolToText(pSettings->m_bRepeat) );
	OUTPUTS ( f, _M(S_LISTFEATURES_REPEAT_LATCHING),	BoolToText(pSettings->m_bRepeatLatching) );
	OUTPUTS ( f, _M(S_LISTFEATURES_SHIFT_KEY_ACTION),	ShiftKeyActionAsText(pSettings->m_iShiftKeyAction) );
	OUTPUTS ( f, _M(S_LISTFEATURES_INDICATOR_LIGHTS),	LightsAsText(pSettings->m_iIndicatorLights) );
	f.WriteLine(TEXT(""));

	//  mode stuff
	if (pSettings->m_iMode==kSettingsModeSwitch)
	{
		OUTPUTS ( f, _M(S_LISTFEATURES_MODE), _M(S_LISTFEATURES_MODE_SWITCH));
	}
	else if (pSettings->m_iMode==kSettingsModeThisOverlay)
	{
		OUTPUTS ( f, _M(S_LISTFEATURES_MODE), _M(S_LISTFEATURES_MODE_SPECIFIC));
	}
	else if (pSettings->m_iMode==kSettingsModeDiscover)
	{
		OUTPUTS ( f, _M(S_LISTFEATURES_MODE), _M(S_LISTFEATURES_MODE_DISCOVER));
	}
	else
	{
		OUTPUTS ( f, _M(S_LISTFEATURES_MODE), _M(S_LISTFEATURES_MODE_LASTSENT));
	}

	OUTPUTS ( f, _M(S_LISTFEATURES_BUTALLOWOVERLAYS), BoolToText(pSettings->m_bButAllowOverlays));

	OUTPUTS ( f, _M(S_LISTFEATURES_MODE_LAST_OVERLAY),	    pEngine->GetCustomOverlay()->GetName() );
	OUTPUTS ( f, _M(S_LISTFEATURES_MODE_SPECIFIC_OVERLAY),	pSettings->m_sUseThisOverlay );

	int nn = pSettings->m_iUseThisSwitchSetting;
	if (nn==0)
		nn = 1;
	IKString key = TEXT("switch_file_");
	key += IKUtil::IntToString(nn);
	IKString val = DATAS(key,TEXT(""));
	OUTPUTS ( f, _M(S_LISTFEATURES_MODE_SWITCH_SETTING), val );

	f.WriteLine(TEXT(""));

	//  devices

	OUTPUTD ( f, _M(S_LF_NUMKB), pEngine->NumDevicesConnected() );

	for (int ndev=0;ndev<MAX_INTELLIKEYS;ndev++)
	{
		IKDevice *pDev = pEngine->GetNumberedDevice(ndev);
		if(pDev)
		{

			f.WriteLine(TEXT(""));
			f.Write(TEXT("\t"));  OUTPUTD ( f, _M(S_LF_KBNUM), ndev+1 );
			f.Write(TEXT("\t"));  OUTPUTS ( f, _M(S_CP_SERIAL_NUMBER),	pDev->GetSerialNumber());
			f.Write(TEXT("\t"));  OUTPUTS ( f, _M(S_CP_ONOFF_SWITCH_STATUS), BoolToText(pDev->IsSwitchedOn()));

			if (pDev->HasStandardOverlay())
			{
				f.Write(TEXT("\t"));  OUTPUTS(f, _M(S_CP_STANDARD_OVERLAY), _M(S_YES));
			}
			else
			{
				f.Write(TEXT("\t"));  OUTPUTS(f, _M(S_CP_STANDARD_OVERLAY), _M(S_NO));
			}

			f.Write(TEXT("\t"));  OUTPUTS(f, _M(S_CP_OVERLAY_NAME), pDev->GetCurrentOverlay()->GetName());
			f.Write(TEXT("\t"));  OUTPUTD(f, _M(S_CP_OVERLAY_LEVEL), pDev->GetLevel());

			BYTE sw;
			pDev->GetSwitchStatus(&sw,1);
			f.Write(TEXT("\t")); OUTPUTS ( f, _M(S_CP_SWITCH1),  BoolToText(!!sw) );
			pDev->GetSwitchStatus(&sw,2);
			f.Write(TEXT("\t")); OUTPUTS ( f, _M(S_CP_SWITCH2),  BoolToText(!!sw) );

			BYTE sens[9];
			pDev->GetSensorArray(sens);
			TCHAR sa[255];
			MySprintf(sa,(TCHAR *)_M(S_CP_SENS),sens[0],sens[1],sens[2],sens[3],sens[4],sens[5],sens[6],sens[7],sens[8]);
			f.Write(TEXT("\t"));
			f.WriteLine(IKString(sa));

			BYTE membrane[100];
			int nm = 0;
			pDev->GetMembraneStatus(membrane,&nm);
			if (nm>0)
			{
				int np = membrane[0];
				if (np>0)
				{
					f.Write(TEXT("\t")); 
					OUTPUTD ( f, _M(S_CP_NUMKEYS), np );
					for (int ip = 0;ip<np;ip++)
					{
						f.Write(TEXT("\t")); f.Write(TEXT("\t"));
						TCHAR line[255];
						MySprintf(line,(TCHAR *)_M(S_CP_ROWCOL),membrane[1+2*ip]+1,membrane[1+2*ip+1]+1);
						f.WriteLine(IKString(line));
					}
				}
			}

		}
	}

	f.WriteLine(TEXT(""));


	//  close the file
	f.Close();


#ifdef PLAT_MACINTOSH
	//  apply the text type for mac
    // TODO: why is this failing to compile?
	// IKUtil::ApplyFinderTypeAndCreator (filename, TEXT("TEXT"), TEXT("ttxt"));
#endif

	//  launch it
#ifdef PLAT_WINDOWS
	//  ask the system tray to do it.
	IKFile::MakeWritable(filename);
	int len = filename.GetLength();
	TCHAR * pMsg = (TCHAR *)filename;
	int result = IKMessage::Send ( TEXT("system tray"), kQueryLaunchFile, (void *) pMsg, len, 0, 0 );	
#else
	IKUtil::LaunchFile ( filename );
#endif

}

void IKControlPanel::Help()
{
	IKString helpfile = IKUtil::GetHelpFile();
        
	//  launch it
#ifdef PLAT_WINDOWS
	//  ask the system tray to do it.
	IKFile::MakeWritable(helpfile);
	int len = helpfile.GetLength();
	TCHAR * pMsg = (TCHAR *)helpfile;
	int result = IKMessage::Send ( TEXT("system tray"), kQueryLaunchFile, (void *) pMsg, len, 0, 0 );	
#else
	IKUtil::LaunchFile ( helpfile );
#endif


}

void IKControlPanel::Show()
{
	if (!IKMessage::IsOwnerAlive(TEXT("control panel")))
		Launch();
	else
		IKMessage::Send(TEXT("control panel"),kQueryShowControlPanel,0,0,0,0);
}


IKString IKControlPanel::BoolToText(bool b)
{
	if(b)
		return _M(S_BOOL_ON);
	else
		return _M(S_BOOL_OFF);
}

IKString IKControlPanel::VolToText(int v)
{
	switch (v)
	{
	case kSettingsKeysoundOff:
		return _M(S_KEYSOUND_OFF);
		break;
	case kSettingsKeysound1:
		return _M(S_KEYSOUND_1);
		break;
	case kSettingsKeysound2:
		return _M(S_KEYSOUND_2);
		break;
	case kSettingsKeysound3:
		return _M(S_KEYSOUND_3);
		break;
	case kSettingsKeysound4:
		return _M(S_KEYSOUND_4);
		break;
	default:
		return _M(S_UNKNOWN_VALUE);
		break;
	}
}

IKString IKControlPanel::ShiftKeyActionAsText(int i)
{
	switch (i)
	{
	case 0:
		return _M(S_LATCHING);
		break;
	case 1:
		return _M(S_LOCKING);
		break;
	case 2:
		return _M(S_NO_LATCHING);
		break;
	default:
		return _M(S_UNKNOWN_VALUE);
		break;
	}
}

IKString IKControlPanel::LightsAsText(int i)
{
	switch (i)
	{
	case 0:
		return _M(S_LIGHTS_THREE);
		break;
	case 1:
		return _M(S_LIGHTS_SIX);
		break;
	default:
		return _M(S_UNKNOWN_VALUE);
		break;
	}
}

void IKControlPanel::Launch()
{
	IKString filename= IKUtil::GetRootFolder();
	filename += DATAS(TEXT("Control_Panel_Name"),TEXT("USB Control Panel.exe"));

#ifdef PLAT_MACINTOSH
       IKUtil::LaunchFile ( filename );
#else

	//  ask the system tray to do it
	int response = IKMessage::Send ( TEXT("system tray"), kQueryShowControlPanel, 0, 0, 0, 0  );
	if (response != kResponseNoError)
	{
		//  system tray could not, so we will.
		IKUtil::LaunchFile ( filename );
	}

#endif
}
