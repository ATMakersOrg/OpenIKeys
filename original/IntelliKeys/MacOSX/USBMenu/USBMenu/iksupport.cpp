#include <Carbon/Carbon.h>

#include "IKCommon.h"
#include "IKString.h"
#include "IKMessage.h"
#include "IKUtil.h"
#include "IKSettings.h"
#include "MessageClient.h"
#include "AppLib.h"
#include "IntelliKeys.h"
#include "IKFile.h"

int savedKeyThresh;
int savedKeyRepThresh;

//  FUNCTION
//  name = myCallBack
//
//  This function is called when a notification is sent and parses
//  a library. Based on library info, this function will either set the
//  key repeat and key repeat threshold, record the key repeat
//  and key repeat threshhold, or will launch a file

static void myCallback ( CFNotificationCenterRef center, void *observer,CFStringRef name, 
		    const void *object, CFDictionaryRef userInfo)
{
    //  create variables to hold the library assets sent by the notification
    CFStringRef myCommand, myArg1, myArg2;

    char commandCstring[50];
    //retrieve the "command" from the library
    myCommand = (CFStringRef) CFDictionaryGetValue(userInfo, CFSTR("command"));
    //copy the "command" to a C string for TRACE
    CFStringGetCString(myCommand,commandCstring,50,0);
    //TRACE1("HELPER:received the command \"%s\"",commandCstring);
    //myCommand options : "launch file","get repeat values","set repeat values" - more can be added as needed
    if (CFStringCompare(myCommand,CFSTR("launch file"),0)==0)
    {
        //if we are in launch file mode
        //then arg1 in the dictionary is the absolute path of the file to open
        myArg1 = (CFStringRef) CFDictionaryGetValue(userInfo, CFSTR("arg1"));
        //create a c string for later user
        char myCstring[255];
        //convert arg1 to a C string
        CFStringGetCString(myArg1,myCstring,255,0);
        //add freds string formatting and file launching code
        FSSpec spec;
        char macFileName[255];

        strcpy(&(macFileName[1]),myCstring);//myCstring is the file to open
        macFileName[0] = strlen(&(macFileName[1]));

        OSErr err = FSMakeFSSpec(0,0,(unsigned char *)macFileName, &spec);
        if (err!=noErr)
            return;

        FSRef ref;
        err = FSpMakeFSRef ( &spec, &ref);
        if (err!=noErr)
            return;

        err = LSOpenFSRef( &ref, NULL);//launch the file
        //TRACE1("HELPER:file to open is %s\n",myCstring);
    }
	
	if (CFStringCompare(myCommand,CFSTR("send overlay"),0)==0)
    {
		//  send an overlay
		CFStringRef filename      = (CFStringRef) CFDictionaryGetValue(userInfo, CFSTR("filename"));
		CFStringRef message       = (CFStringRef) CFDictionaryGetValue(userInfo, CFSTR("message"));
		CFStringRef reporterrors  = (CFStringRef) CFDictionaryGetValue(userInfo, CFSTR("report errors"));
		CFStringRef wait          = (CFStringRef) CFDictionaryGetValue(userInfo, CFSTR("wait"));
		CFStringRef banneronly    = (CFStringRef) CFDictionaryGetValue(userInfo, CFSTR("banner only"));
		
		char filenameC[255];       CFStringGetCString(filename,filenameC,255,0);
		char messageC[255];        CFStringGetCString(message,messageC,255,0);
		char reporterrorsC[10];	   CFStringGetCString(reporterrors,reporterrorsC,255,0);
		char waitC[10];	           CFStringGetCString(wait,waitC,255,0);
		char banneronlyC[10];      CFStringGetCString(banneronly,banneronlyC,255,0);
		
		bool bReportErrors = (reporterrorsC[0]=='1');
		bool bWait         = (waitC[0]=='1');
		bool bBannerOnly   = (banneronlyC[0]=='1');
		
		int result = IntelliKeys::SendOverlay ( filenameC, bReportErrors, messageC, bWait, bBannerOnly );
	}
    
    if (CFStringCompare(myCommand,CFSTR("save repeat values"),0)==0)
    {
	//  save current values for key repeat
        
	savedKeyThresh    = LMGetKeyThresh();
	savedKeyRepThresh = LMGetKeyRepThresh();
    }

    if (CFStringCompare(myCommand,CFSTR("restore repeat values"),0)==0)
    {
	//  restore saved values
	LMSetKeyThresh   (savedKeyThresh);
	LMSetKeyRepThresh(savedKeyRepThresh);
    }
    
    //if the command is "get repeat values"
    if (CFStringCompare(myCommand,CFSTR("get repeat values"),0)==0)
    {
        //create 2 buffers
        char curr_key_thresh[32],curr_key_rep_thresh[32];
        
        //fill the buffers with the key repeat values
        sprintf(curr_key_thresh,"%d",LMGetKeyThresh());
        sprintf(curr_key_rep_thresh,"%d", LMGetKeyRepThresh());
        
        //TRACE1("The keys begin to repeat after %s ticks",curr_key_thresh);
        //TRACE1("The key repeat speed is %s ticks",curr_key_rep_thresh);
        
        //create 2 CFStrings, repeat properties will be converted to CFStrings
        CFStringRef CF_key_thresh, CF_key_rep_thresh;
        
        //convert 
        CF_key_thresh = CFStringCreateWithCString(NULL,curr_key_thresh,0);
        CF_key_rep_thresh = CFStringCreateWithCString(NULL,curr_key_rep_thresh,0);
        //find the reference to the notification center
        CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();

        //create a dictionary of values to send to the USBHelper
        //the first value will be the command we want to execute followed by any arguments
        CFMutableDictionaryRef keyRepeat_Dict = CFDictionaryCreateMutable(NULL, 2,
            &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFDictionaryAddValue(keyRepeat_Dict, CFSTR("repeat delay"),CF_key_thresh);
        CFDictionaryAddValue(keyRepeat_Dict, CFSTR("repeat speed"),CF_key_rep_thresh);

		CFNotificationCenterPostNotification ( center, CFSTR("IKUSBHelper Notification"), NULL, keyRepeat_Dict, FALSE);
        	
        CFRelease(keyRepeat_Dict);
        CFRelease(CF_key_thresh);
        CFRelease(CF_key_rep_thresh);
    }
    //if the command is "set repeat values"
    if (CFStringCompare(myCommand,CFSTR("set repeat values"),0)==0)
    {
        //create 2 variables to hold the values
        int key_thresh,key_rep_thresh;
        char temp1[255],temp2[255];
	
        // the key threshhold is arg1 from the library - how long till keys start repeating
        myArg1 = (CFStringRef) CFDictionaryGetValue(userInfo, CFSTR("arg1"));
	
        //the key repeat threshhold is arg2 from the library - how fast do the keys repea
        myArg2 = (CFStringRef) CFDictionaryGetValue(userInfo, CFSTR("arg2"));
	
        //turn the CFstrings myArg1 and myArg2 into C strings and then into integers
        CFStringGetCString(myArg1,temp1,255,0);
        CFStringGetCString(myArg2,temp2,255,0);
        key_thresh = atoi(temp1);
        key_rep_thresh = atoi(temp2);
	
        //set the key threshhold to our new value
        LMSetKeyThresh(key_thresh);
	
        //set the key repeat threshhold to our new value
        LMSetKeyRepThresh(key_rep_thresh);
	
    }

}

static	pthread_t MessageThread;

static void* MessageThreadFunction(void* input)
{
	while (true )
	{
		IKUtil::Sleep(500);
		
		BYTE data[2000];
		int datalen;
		int command;
		int response;
		
		const char *channel = TEXT("menu");
		
		response = IKMessage::Receive ( TEXT(channel), &command, (void *)data, 2000, &datalen );
		if (response != kResponseNoCommand)
		{
			switch (command)
			{
				case kQueryRawNotify:
					IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
					{
						//send dist notification
						CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();
						CFMutableDictionaryRef Helper_Dict = CFDictionaryCreateMutable(NULL, 3,
							&kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
						CFStringRef s = CFStringCreateWithCString(kCFAllocatorDefault,(const char *)data,kCFStringEncodingUTF8);
						CFDictionaryAddValue(Helper_Dict, CFSTR("event"),s);
						CFNotificationCenterPostNotification ( center, CFSTR("IKUSB Raw Mode Notification"), NULL, Helper_Dict, FALSE);

						CFRelease(Helper_Dict);						
					}
					break;
					
				case kQuerySendOverlayProxy:
					IKMessage::Respond ( TEXT(channel),  kResponseNoError, 0, 0 );
					{
						IKString filename, message;
						bool bReport=false, bWait=false, bBanner=false;
						IKString s(data,datalen);
						int i;
						i = s.Find(TCHAR(1));
						if (i!=-1)
						{
							filename = s.Left(i);
							s = s.Mid(i+1);
						}
						i = s.Find(TCHAR(1));
						if (i!=-1)
						{
							message = s.Left(i);
							s = s.Mid(i+1);
						}
						i = s.Find(TCHAR(1));
						if (i!=-1)
						{
							bReport = (s.Left(i)==TEXT("1"));
							s = s.Mid(i+1);
						}
						i = s.Find(TCHAR(1));
						if (i!=-1)
						{
							bWait = (s.Left(i)==TEXT("1"));
							s = s.Mid(i+1);
						}
						i = s.Find(TCHAR(1));
						if (i!=-1)
						{
							bBanner = (s.Left(i)==TEXT("1"));
							s = s.Mid(i+1);
						}
						
						if (!filename.IsEmpty())
						{
//                            if (0) {
//                                NSApplication *myApp = [NSApplication sharedApplication];
//                                if ([myApp respondsToSelector:@selector(displayOverlayAlert:)]) {
//                                    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
//                                    NSString *s = [NSString stringWithUTF8String: (char*) message];
//                                    NSDictionary *userinfo = [NSDictionary dictionaryWithObjectsAndKeys: s, @"overlayMessage", nil];
//                                    [[NSDistributedNotificationCenter defaultCenter] 
//                                        postNotificationName:@"com.cambium.usbmenu.notifications.overlayalert" object: nil userInfo: userinfo];
//                                    [pool release];
//                                }
//                            }
							IntelliKeys::SendOverlay ( filename, bReport, message, bWait, bBanner );
						}
						
					}
						break;
						
				case kQueryGetCurrentApp:
					{
					}
					break;

				case kQueryLaunchFile:
					{
						// make a pascal string of the path
						data[datalen] = 0;
						char macFileName[255];
						strcpy(&(macFileName[1]),(char *)data);  macFileName[0] = strlen(&(macFileName[1]));
						
						//  spec it.
						FSSpec spec;
						OSErr err = FSMakeFSSpec(0,0,(unsigned char *)macFileName, &spec);
						if (err==noErr)
						{
							// FSRef it.
							FSRef ref;
							err = FSpMakeFSRef ( &spec, &ref);
							if (err==noErr)
							{
								//  launch it.
								err = LSOpenFSRef( &ref, NULL);
							}
						}
					}
					break;

					
				default:
					IKMessage::Respond ( TEXT(channel),  kResponseError, 0, 0 );
					break;
			}
			
		}
	
	}
    
    //  never get here
    //syslog(1,"USBMenu MessageThreadFunction terminated");
    return 0;
	
}

void IKMBSendAttachedOverlay ()
{
	//  this function forces the Engine to "send" the currently 
	//  attached overlay.
	int response = IKMessage::SendTO ( TEXT("engine"), kQuerySendAttachedOverlay, 0, 0, 0, 0, 1000, false );
}


void IKMBInitialize ()
{
    IKUtil::Initialize();
    IKMessage::Initialize();
    
    //  save current values for key repeat
    savedKeyThresh    = LMGetKeyThresh();
    savedKeyRepThresh = LMGetKeyRepThresh();
    
    //  establish notification callback
    CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();
    void *observer=NULL;
    CFNotificationCenterAddObserver(center,CFSTR("IKUSB helper 2"), myCallback, 
            CFSTR("IKUSBHelper Notification"),NULL, 
            CFNotificationSuspensionBehaviorDeliverImmediately);
	
	//  create a thread for receiving messages.
    pthread_create(&MessageThread,NULL,MessageThreadFunction,NULL);
	
	
}

int IKMBIsConnectedAndOn ()
{
    if (IKIsIntellikeysOn()==kResponseOn)
        return 1;
    return 0;
}

static bool Launch()
{
	//  is it already launched?
	int response = IKMessage::SendTO ( TEXT("control panel"), kQueryOKToSend, 0, 0, 0, 0, 1000, false );
	bool bReady = (response==kResponseUnsupported);
	if (bReady)
	{
		//  bring it forward
		response = IKMessage::SendTO ( TEXT("control panel"), kQueryForwardControlPanel, 0, 0, 0, 0, 1000, false);
		
		return true;
	}
	
	//  remember when we started for timeout purposes
	unsigned int then = IKUtil::GetCurrentTimeMS();
	
	//  get launch name
    IKString filename= IKUtil::GetRootFolder();
    filename += DATAS(TEXT("Control_Panel_Name"),TEXT("USB Control Panel.exe"));
    
    //NSLog (@"open control panel now %s", (char *) filename);
    
    //  Make an FSSpec first
    FSSpec spec;
    TCHAR macFileName[255];
    strcpy(&(macFileName[1]),(TCHAR *)filename);
    macFileName[0] = IKString::strlen(&(macFileName[1]));
    OSErr err = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &spec );
    if (err!=noErr)
		return false;
	
    //  now make the FSRef
    FSRef ref;
    err = FSpMakeFSRef ( &spec, &ref );
    if (err!=noErr)
		return false;
	
    //  Launch it.
    LSLaunchFSRefSpec lrspec;
    lrspec.appRef = &ref;
    lrspec.numDocs = 0;
    lrspec.passThruParams = NULL;
    lrspec.itemRefs = NULL;
    lrspec.launchFlags = kLSLaunchDefaults; 
    lrspec.asyncRefCon = NULL;
    err = LSOpenFromRefSpec(&lrspec,0);  
	
	//  wait for it to become responsive
	bReady = false;
	while (true)
	{
		int response;
		response = IKMessage::SendTO ( TEXT("control panel"), kQueryOKToSend, 0, 0, 0, 0, 1000, false );
		if (response==kResponseUnsupported)
		{
			bReady = true;
			break;
		}
		unsigned int now = IKUtil::GetCurrentTimeMS();
		if (now-then > 10000)  //  10 seconds
			break;
		IKUtil::Sleep(250);
	}
	
	return bReady;
}



void IKMBLaunchControlPanel ()
{
	bool bReady = Launch();
}

void IKMBGetLocalizedString ( char *tag, char *output )
{
	IKString s = DATAS ( tag, tag );
	IKString::strcpy ( output, (char *)s );
}


//  settings for the current user
static IKString student;
static IKString group;
static IKSettings currentSettings;

//  load settings
static void LoadCurrentSettings ()
{
	//  load global preferences
	IKString prefsFile = IKUtil::GetPrivateFolder();
	prefsFile += DATAS(TEXT("Preferences_File"),TEXT("preferences.txt"));
	IKPrefs prefs;
	prefs.Read(prefsFile);

	//  get group and student name
	group   = prefs.GetValueString(TEXT("Group"),  TEXT(""));
	student = prefs.GetValueString(TEXT("Student"),TEXT(""));
	if (group.IsEmpty() && student.IsEmpty())
		group = DATAS(TEXT("Guest"),TEXT("Guest"));

	//  make path to the user's settings file
	IKString path = IKUtil::MakeStudentPath ( group, student );
	IKString settingsFile = path;
	settingsFile += TEXT("settings.txt");

    //NSLog(@"LoadCurrentSettings - reading - [%s]", (char*)settingsFile);

	//  read user settings
	IKSettings *pSettings = IKSettings::GetSettings();
	pSettings->Read(settingsFile);

	//  copy them locally.
	currentSettings = *pSettings;
}

//  save settings
static void SaveCurrentSettings ()
{
	//  make path to the settings file
	IKString path = IKUtil::MakeStudentPath ( group, student );
	IKString settingsFile = path;
	settingsFile += TEXT("settings.txt");

    //NSLog(@"SaveCurrentSettings - writing - [%s]", (char*)settingsFile);
    
	//  write out the settings
	currentSettings.Write(settingsFile);
}

//  get switch setting name from the number
static IKString GetSwitchSettingName ( int iSwitchSetting )
{
	int nn = iSwitchSetting;
	if (nn==0)
		nn = 1;
	IKString key = TEXT("Switch_File_");
	key += IKUtil::IntToString(nn);
	IKString switchSetting = DATAS(key,TEXT(""));
	IKUtil::StripFileName(switchSetting,true,true);

	return switchSetting;
}

void IKMBSetSwitchSetting(int i)
{
	currentSettings.m_iUseThisSwitchSetting = i;
}

char * IKMBGetSwitchSettingName (int nSetting)
{
	if (nSetting != -1)
		return (char *) GetSwitchSettingName(nSetting);
		
	return (char *) GetSwitchSettingName(currentSettings.m_iUseThisSwitchSetting);
}

char * IKMBGetOverlaySettingName ()
{
	IKString s;
	s = currentSettings.m_sUseThisOverlay;
	IKUtil::StripFileName(s, true, true);
	return (char *)s;
}

void IKMBSetMode ( int mode )
{
    //NSLog(@"SetMode from [%d] to [%d]", currentSettings.m_iMode, mode);
    
	if (mode == 1)
	{
		if (currentSettings.m_sUseThisOverlay.IsEmpty())
		{
			//  tell them they must specify an overlay
			IKString title = DATAS(TEXT("Intellikeys_Usb"),TEXT("IntelliKeys USB"));
			IKString message = DATAS(TEXT("Must_Specify_Overlay"),TEXT("must_specify_overlay"));
			IKUtil::DisplayAlert(title,message);
			
			//  ask for the overlay
			IKString strOverlay;
			title = DATAS(TEXT("Choose_Overlay"),TEXT("choose_overlay"));
			if (IKFile::GetOneFile(title,strOverlay))
			{
				if (IntelliKeys::IsSendableOverlayFile (strOverlay ))
				{
					currentSettings.m_sUseThisOverlay = strOverlay;
					currentSettings.m_iMode = mode;
					return;
				}
				else
				{
					title = DATAS(TEXT("Intellikeys_Usb"),TEXT("IntelliKeys USB"));
					message = DATAS(TEXT("Not_Valid_Overlay"),TEXT("not_valid_overlay"));
					IKUtil::DisplayAlert(title,message);
					return;
				}
			}
			else
			{
				title = DATAS(TEXT("Intellikeys_Usb"),TEXT("IntelliKeys USB"));
				message = DATAS(TEXT("No_Overlay_Chosen"),TEXT("no_overlay_chosen"));
				IKUtil::DisplayAlert(title,message);
				return;
			}
		}
	}
	
	currentSettings.m_iMode = mode;
}

void IKMBSaveCurrentSettings ()
{
	SaveCurrentSettings();
}

void IKMBOnUpdateMenu ()
{
	LoadCurrentSettings();
}

void IKMBGetCurrentAppMenuTitle  ( char *path, char *output )
{
	IKString strPath = path;
	IKString friendly = IKUtil::GetAppFriendlyName(strPath);

	IKString strName = friendly;
	IKUtil::StripFileName(strName,true,true);

	IKString strItem;
	//strItem += "Attach/Edit overlays for ";  //  localize
	strItem += DATAS(TEXT("Attach_Edit_Overlays_For"),TEXT("Attach/Edit overlays for"));
	strItem += " ";
	strItem += "\"";
	strItem += strName;
	strItem += "\"";
	strItem += "...";
	
	IKString::strcpy ( output, (char *)strItem );
}

void IKMBLaunchControlPanelForApp ( char *app )
{
	IKString strLastApp = app;
	
	//m_lastWindow = false;

	if (!AppLibIsAppAllowed(strLastApp))
	{
		IKString title = DATAS(TEXT("Intellikeys_Usb"),TEXT("IntelliKeys USB"));
		IKString message = DATAS(TEXT("App_Not_Allowed"),TEXT("app_not_allowed"));
		IKUtil::DisplayAlert(title,message);
	}
	else

	{
		AppLibAddApplication ( strLastApp );
		AppLibAddApplicationForStudent ( strLastApp, group, student );
		
		IKMessage::Send(TEXT("engine"),kQueryReloadStudent, 0, 0, 0, 0);
		
		//  launch CP and wait for it
		bool bReady = Launch();
		
		//  Tell control panel to switch to the apps tab
		//  and tell it which app to slect
		
		if (bReady)
		{
			unsigned char data[2000];
			int ndata = 0;
			data[ndata] = strlen((char *)strLastApp);
			ndata++;
			strcpy ((char *)&(data[ndata]),(char *)strLastApp);
			ndata = ndata + strlen((char *)strLastApp);
			
			int result = IKMessage::SendTO ( TEXT("control panel"), kQueryApplication, data, ndata, 0, 0, 1000, false );
		}
	}
	
}

bool IKMBIsIntelliSwitchInstalled ()
{
	return AppLibIsIntelliSwitchInstalled ();
}

void IKMBTroubleshooting()
{
	//  launch CP and wait for it
	bool bReady = Launch();
	
	//  Tell control panel to switch to the Troubleshooting tab
	if (bReady)
	{		
		int result = IKMessage::SendTO ( TEXT("control panel"), kQueryTroubleshooting, 0, 0, 0, 0, 1000, false );
	}
	
}

char * IKMBGetGroup ()
{
	return (char *)group;
}

char * IKMBGetStudent ()
{
	return (char *)student;
}

int IKMBGetMode ()
{
	return currentSettings.m_iMode;
}

void IKMBStripFileName ( char *path, bool b1, bool b2 )
{
	IKString s = path;
	IKUtil::StripFileName ( s, b1, b2 );
	strcpy(path,(char *)s);
}

void IKMBReloadStudent()
{
	int result = IKMessage::Send("engine",kQueryReloadStudent, 0, 0, 0, 0);
    
    //NSLog(@"IKBMReloadStudent - engine - result = %d", result);
}

char * IKMBGetToolTip ()
{
	//  get info about the devices
	int nIK = 0, nIKOn = 0;
	int nIS = 0, nISOn = 0;
	BYTE deviceArray[2000];
	int response = IKGetDeviceArray ( deviceArray );
	if (response == kResponseUSBIntelliKeysArray)
	{
		int n = 0;
		int nDevices = deviceArray[n];  n++;

		for (int iDev=0;iDev<nDevices;iDev++)
		{
			int deviceIndex = deviceArray[n];  n++;
			int nOnOff =  deviceArray[n];  n++;
			n = n + 9;  //  skip sensors
			int level = deviceArray[n];  n++;
			int std = deviceArray[n];  n++;

			//  overlay
			IKString overlay;
			int len = deviceArray[n];  n++;
			int i;
			for (i=0;i<len;i++)
				overlay += char(deviceArray[n+i]);
			n = n + len;

			//  serial number
			IKString serial;
			len = deviceArray[n];  n++;
			for (i=0;i<len;i++)
				serial += char(deviceArray[n+i]);
			n = n + len;

			// skip switch status
			n = n + 2;

			//  skip membrane status
			len = deviceArray[n]; n++;
			n = n + len*2;

			//  device type
			int devType = deviceArray[n]; n++;
			
			//  skip overlay number
			n = n + 1;

			//  skip switch status for 3, 4, and 5
			n = n + 3;
						
			switch (devType)
			{
			case 1:
				nIK++;
				if (nOnOff)
					nIKOn++;
				break;
			case 2:
				nIS++;
				if (nOnOff)
					nISOn++;
				break;
			default:
				break;
			}
		}
	}

	//  set the tooltip
	//  TODO:  localize!!!
	static IKString toolTip;
	if (nIK>0 && nIS<=0)
	{
		if (nIKOn>0)
			toolTip = DATAS(TEXT("S_TOOLTIP_IK_ON"),TEXT("S_TOOLTIP_IK_ON"));
		else
			toolTip = DATAS(TEXT("S_TOOLTIP_IK_OFF"),TEXT("S_TOOLTIP_IK_OFF"));

	}
	else if (nIK<=0 && nIS>0)
	{
		if (nISOn>0)
			toolTip = DATAS(TEXT("S_TOOLTIP_IS_ON"),TEXT("S_TOOLTIP_IS_ON"));
		else
			toolTip = DATAS(TEXT("S_TOOLTIP_IS_OFF"),TEXT("S_TOOLTIP_IS_OFF"));
	}
	else if (nIK>=0 && nIS>0)
	{
		toolTip = DATAS(TEXT("S_TOOLTIP_MULTIPLE_DEVICES"),TEXT("S_TOOLTIP_MULTIPLE_DEVICES"));
	}
	else
	{
		toolTip = DATAS(TEXT("S_TOOLTIP_NO_DEVICES"),TEXT("S_TOOLTIP_NO_DEVICES"));
	}

	return (char *) toolTip;
}


void IKMBOnTimer ()
{
	//  send the current app
	IKString s = IKUtil::GetCurrentApplicationPath();
	BYTE data[1024];
	int ndata = s.GetLength();
	for (int i=0;i<ndata;i++)
		data[i] = s.GetAt(i);
	data[ndata] = 0;
	IKMessage::Send ( TEXT("engine"),  kResponseGetCurrentApp, data, ndata, 0, 0 );
}

void IKMBGetCurrentAppPath  ( char *output )
{
	IKString app = IKUtil::GetCurrentApplicationPath();
	IKString::strcpy ( output, (char *)app );
}

void IKMBGetCurrentAppOwner  ( char *app, char *output )
{
	IKString owner = AppLibGetOwner(app);
	IKString::strcpy ( output, (char *)owner );
}

int IKMBCompareNoCase ( const char *s1, const char *s2 )
{
	IKString string1 = s1;
	return string1.CompareNoCase(s2);
}

bool IKMBIsAppControlPanel(char *app)
{
	IKString s = app;
	IKUtil::StripFileName(s,true,false);
	return (s.CompareNoCase("Intellikeys USB")==0);
}

bool IKMBAppIsRunning(char *app)
{
	IKString s2 = app;
	IKUtil::StripFileName(s2,true,true);
	
	return (IKUtil::IsAppRunning(s2));
}

bool IKMBGetButAllowOverlays ()
{
	return currentSettings.m_bButAllowOverlays;
}

void IKMBSetButAllowOverlays (bool bSet)
{
	currentSettings.m_bButAllowOverlays = bSet;
}

int IKMBgetNumSwitchSettings()
{
	return DATAI(TEXT("Num_Switch_Files"), 0);
}

void IKMBFixSwitchSetting()
{
    //NSLog(@"IKMBFixSwitchSetting - start - currentSettings.m_iUseThisSwitchSetting = [%d]", currentSettings.m_iUseThisSwitchSetting);
    
	if (currentSettings.m_iUseThisSwitchSetting == 0)
		currentSettings.m_iUseThisSwitchSetting = 1;

    //NSLog(@"IKMBFixSwitchSetting - end - currentSettings.m_iUseThisSwitchSetting = [%d]", currentSettings.m_iUseThisSwitchSetting);
}


