//-----------------------------------------------------------
//
//  System includes
//

#if TARGET_API_MAC_CARBON
#ifndef BUILD_UNIVERSAL
	#include <Carbon.h>
#endif
#else
	#include <Carbon.h>
	#include <Dialogs.h>
	#include <Fonts.h>
	#include <MacWindows.h>
	#include <Menus.h>
	#include <QuickDraw.h>
	#include <TextEdit.h>
#endif

#include <string.h>
#include <stdio.h>

//-----------------------------------------------------------
//
//  external interface (for return codes)
//

#include "intellikeys.h"

//-----------------------------------------------------------
//
//  internal shared stuff
//

#include "common.h"

//----------------
//
//  stuff from IK USB
//
//#include "IKCommon.h"
#include "IKUtil.h"
#include "IKString.h"
#include "AppLib.h"
#include "IKSettings.h"
#include "IKMessage.h"


//--------
//
//  a general init function for this library
//
static void Init()
{
	static bool bInited = false;
	if (!bInited)
	{
		IKUtil::Initialize();
		bInited = true;
	}
}


//-----------------------------------------------------------
//  declarations for integration with IK USB 2 and above on OS X

#if TARGET_API_MAC_CARBON

typedef int (*BundleIsIntellikeysConnectedPtr) 	(void);
typedef int (*BundleIsIntellikeysOnPtr)			(void);
typedef int (*BundleTestOverlayFilePtr)			(const char *);
typedef int (*BundleBundleSendOverlayPtr)		(const char *, const char *);
typedef int (*BundleFloatingMessagePtr)			(const char *);
typedef int (*BundleBundleSendOverlay2Ptr)		(const char *, const char *, const char *);


BundleIsIntellikeysConnectedPtr myBundleIsIntellikeysConnectedPtr 	= NULL;
BundleIsIntellikeysOnPtr 		myBundleIsIntellikeysOnPtr 			= NULL;
BundleTestOverlayFilePtr 		myBundleTestOverlayFilePtr 			= NULL;
BundleBundleSendOverlayPtr 		myBundleBundleSendOverlayPtr 		= NULL;
BundleFloatingMessagePtr 		myBundleFloatingMessagePtr 			= NULL;
BundleBundleSendOverlay2Ptr 	myBundleBundleSendOverlay2Ptr 		= NULL;


static CFBundleRef theBundle = NULL;

#define OSX_SENDING_BUNDLE_PATH "/applications/intellitools/intellikeys usb/private/overlay sending.bundle"


//-----------------------------------------------------------
//
//  Find, load and intialize pointers to functions in the IK USB 2.0
//  bundle.
//

static bool MacXInitializeBundle ()
{
	static bool bAttempedInit = false;
	static bool bValidBundle  = false;
	
	if (!bAttempedInit)
	{
		bAttempedInit = true;
				
		//  get a ref to the bundle path
		FSRef theRef;
		Boolean bIsDir;
		const char * path = OSX_SENDING_BUNDLE_PATH;
		OSErr err = FSPathMakeRef ( (const UInt8 *)((char *)path), &theRef, &bIsDir );
		if (err == noErr)
		{
			/* Turn the FSRef into a CFURL */
			CFURLRef theBundleURL;
			theBundleURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &theRef);
			if (theBundleURL != NULL)
			{
				/* Turn the CFURL into a bundle reference */
				theBundle = CFBundleCreate(kCFAllocatorSystemDefault, theBundleURL);
				CFRelease(theBundleURL);
				
				//  Load the executable
				Boolean isLoaded;
				isLoaded = CFBundleLoadExecutable(theBundle);
				if (isLoaded)
				{
					//  look up the function pointers
					myBundleIsIntellikeysConnectedPtr = 
						(BundleIsIntellikeysConnectedPtr) CFBundleGetFunctionPointerForName ( theBundle, CFSTR("BundleIsIntellikeysConnected"));
					myBundleIsIntellikeysOnPtr = 
						(BundleIsIntellikeysOnPtr) CFBundleGetFunctionPointerForName ( theBundle, CFSTR("BundleIsIntellikeysOn"));
					myBundleTestOverlayFilePtr = 
						(BundleTestOverlayFilePtr) CFBundleGetFunctionPointerForName ( theBundle, CFSTR("BundleTestOverlayFile"));
					myBundleBundleSendOverlayPtr = 
						(BundleBundleSendOverlayPtr) CFBundleGetFunctionPointerForName ( theBundle, CFSTR("BundleSendOverlay"));
					myBundleFloatingMessagePtr = 
						(BundleFloatingMessagePtr) CFBundleGetFunctionPointerForName ( theBundle, CFSTR("BundleFloatingMessage"));
					myBundleBundleSendOverlay2Ptr = 
						(BundleBundleSendOverlay2Ptr) CFBundleGetFunctionPointerForName ( theBundle, CFSTR("BundleSendOverlay2"));


					if ( myBundleIsIntellikeysConnectedPtr && myBundleBundleSendOverlayPtr )
					{
						bValidBundle = true;
					} 
				}
			}
		}
	}
	
	return bValidBundle;
}
#endif

static void ConvertPathToPOSIX ( const char *srcIn, char *dst )
{
	if ( srcIn[0] == 0 )
	{
		dst[0] = 0;
		return;
	}

	if ( srcIn[0] == '/' )
	{
		strcpy ( dst, srcIn );
		return;
	}

	char src[512];
	strcpy ( src, srcIn );
	
	//  it's a directory if the path ends in a colon.
	bool bIsDirectory = (src[strlen(src)-1]==':');
	
	CFStringRef stringRef = CFStringCreateWithCString( kCFAllocatorDefault, src, kCFStringEncodingMacRoman ); 
	CFURLRef urlRef = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, stringRef, kCFURLHFSPathStyle, bIsDirectory );
	if ( urlRef )
	{
		stringRef = CFURLCopyFileSystemPath( urlRef, kCFURLPOSIXPathStyle );
		CFStringGetCString( stringRef, dst, 512, kCFStringEncodingMacRoman ); 
	}
	else
		dst[0] = 0;
}

static OSErr GetSpecFromPath (const char * path, FSSpec *pSpec)
{
	char dst[512];
	ConvertPathToPOSIX ( path, dst );
	
	FSRef fsRef;
	Boolean bIsDirectory;
	OSErr err = FSPathMakeRef ( ( const UInt8*) (const char *) ( dst ), &fsRef, &bIsDirectory );
	if (err!=noErr)
		return err;
	
	err = FSGetCatalogInfo (&fsRef, kFSCatInfoNone, NULL, NULL, pSpec, NULL);
	return err;
}

//-----------------------------------------------------------
//
//  Make a spec for the helper application
//

static OSErr GetHelperSpec ( FSSpec *spec )
{
	//  find the application support folder
	short vRefNum;
	long dirID;
	OSErr err = FindFolder ( kOnSystemDisk, kApplicationSupportFolderType, kDontCreateFolder, &vRefNum, &dirID );
	if (err != noErr)
		return err;
		
	//  find the helper in there.
	err =  FSMakeFSSpec ( vRefNum, dirID, "\p:intellitools:Classic Sending Helper", spec ) ;
	return err;
}

static char tolower(char c)
{
	char x = c;
	if (x>='A' && x <='Z')
		x = x - 'A' + 'a';
	return x;
}

//-----------------------------------------------------------
//
//  Is the given file a sendable Windows overlay?
//

static bool IsWinOverlayFile ( char *pFilePath )
{	
	//  make a spec
	
	FSSpec spec;
#if 0
	char macname[255];
	strcpy(&(macname[1]),pFilePath);
	macname[0] = strlen(&(macname[1]));
	OSErr err = FSMakeFSSpec( 0, 0, (unsigned char *)macname, &spec );
#endif
	OSErr err = GetSpecFromPath ( pFilePath, &spec);
	if (  err != noErr )
		return false;
	
	//  open the file
	
	short fileRef;
	err = FSpOpenDF ( &spec, fsRdPerm, &fileRef );
	if (err != noErr)
		return false;
	
	//  see if it is a win overlay

	bool bIsWinOverlay = false;
	err = SetFPos ( fileRef, fsFromStart, 0 );
	
	unsigned short leadSizeBytes;
	unsigned short ovlySize;
	unsigned short codeMOREPos;
	
	long l = sizeof ( unsigned short );
	err = FSRead (fileRef, &l, &leadSizeBytes );
	ovlySize = leadSizeBytes;
	unsigned char b1 = leadSizeBytes & 0x00ff;
	unsigned char b2 = (leadSizeBytes & 0xff00)/256;
	ovlySize = b2 + 256*b1;
#if! TARGET_CPU_PPC
	ovlySize = CFSwapInt16BigToHost(ovlySize);
#endif

	ovlySize += 104;
	codeMOREPos = ovlySize + 24;
	err = SetFPos ( fileRef, fsFromStart, codeMOREPos );

	FourCharCode checkMORE = '    ';
	l = 4;
	err = FSRead (fileRef, &l, &checkMORE );	
#if! TARGET_CPU_PPC
	checkMORE = CFSwapInt32BigToHost(checkMORE);
#endif

	if(err==noErr)
	{
	if (checkMORE == 'eroM')
			bIsWinOverlay = TRUE;
	}
	if (!bIsWinOverlay)
	{
		//  can't find 'eroM'.
		//  use file extension to check instead.
		//  get the last four chars, lowercase
		char ext[5];
		int len = strlen(pFilePath);
		for (int i=0;i<4;i++)
			ext[i] = tolower(pFilePath[len-4+i]);
		ext[4] = '\0';

		//  check for the right extensions
		if (strcmp(ext,".omc")==0)
			bIsWinOverlay = TRUE;
		if (strcmp(ext,".oms")==0)
			bIsWinOverlay = TRUE;
		if (strcmp(ext,".omg")==0)
		bIsWinOverlay = TRUE;
	}
	
	//  close the file
	FSClose ( fileRef );
	
	return bIsWinOverlay;
}

//-----------------------------------------------------------
//
//  Is the given file a sendable mac overlay?
//

static bool IsMacOverlayFile ( char *pFilePath)
{	
	FSSpec spec;
	char macname[255];
	strcpy(&(macname[1]),pFilePath);
	macname[0] = strlen(&(macname[1]));
	OSErr err = FSMakeFSSpec( 0, 0, (unsigned char *)macname, &spec );
	if (  err != noErr )
		return false;
		
	int resID = FSpOpenResFile ( &spec, fsRdPerm );
	if ( resID == -1 )
		return false;
	
	UseResFile ( resID );
		
	Handle hOverlay = ::GetResource ( 'Ovly', 128 );
	if (hOverlay)
	{
		DisposeHandle ( hOverlay );
		CloseResFile ( resID );
		return true;
	}
		
	CloseResFile ( resID );
	return false;
}

//-----------------------------------------------------------
//
//  Launch an application.
//

static OSErr
LaunchIt( const FSSpecPtr fileSpec, ProcessSerialNumber *psn )
{
	LaunchParamBlockRec pb;
	OSErr err;
	
	pb.launchBlockID = extendedBlock;
	pb.launchEPBLength = extendedBlockLen;
	pb.launchFileFlags = launchNoFileFlags;
	pb.launchControlFlags = launchContinue + launchNoFileFlags + launchDontSwitch;
	pb.launchAppSpec = fileSpec;
	pb.launchAppParameters = NULL;
	
	err= LaunchApplication(&pb);
	if( !err )
		*psn = pb.launchProcessSN;
	return err;
}

#if TARGET_API_MAC_CARBON

//  variables for the sending banner.

static Rect      gMeterRect;
static WindowRef gWindowRef;

//-----------------------------------------------------------
//
//  remove the sending banner display.
//

static void RemoveBanner ( )
{
	if (gWindowRef != NULL)
		DisposeWindow(gWindowRef);
	gWindowRef = NULL;
}

//-----------------------------------------------------------
//
//  flush changes to the banner.
//

static void Flush()
{
	CGrafPtr port = GetWindowPort(gWindowRef);
	QDFlushPortBuffer(port, NULL);
}


//-----------------------------------------------------------
//
//  change the banner thermometer based on the amount complete.
//

static pascal void displayProgress(short position, short totalBytes)	/* draw a bar */
{
	if (gWindowRef == NULL)
		return;

	short p = (short)((long)(gMeterRect.right - gMeterRect.left - 4) * (long)position / (long) totalBytes) + 2;
	FrameRect(&gMeterRect);
	
	Rect r;
	SetRect(&r,gMeterRect.left+2,gMeterRect.top+2,gMeterRect.left+p, gMeterRect.bottom - 2);
	PaintRect(&r);
	
	SetRect(&r,r.right+1,gMeterRect.top+2,gMeterRect.right-2, gMeterRect.bottom - 2);
	EraseRect(&r);
	
	Flush();
	
	BringToFront(gWindowRef);
}


//-----------------------------------------------------------
//
//  Display the sending banner and title.
//

static void DisplayBanner ( Str255 strTitle )
{
	//  where should it go?

	Rect r;
	SetRect ( &r, 0, 0, 340, 90 );
	GDHandle screen = ::GetMainDevice();
	Rect screenRect = (**screen).gdRect;
	
	short left = (screenRect.right - (r.right - r.left)) / 2;
	short top  = (screenRect.bottom - (r.bottom - r.top)) / 3;
	
	r.right += left - r.left;
	r.left = left;
	r.bottom += top - r.top;
	r.top = top;
	
	//  where is the meter?
		
	gMeterRect.left   = 8;
	gMeterRect.right  = r.right - r.left - 8;
	gMeterRect.bottom = r.bottom - r.top - 16;
	gMeterRect.top    = gMeterRect.bottom - 12;

	//  create the window
	
	OSStatus result = CreateNewWindow(kUtilityWindowClass, kWindowStandardHandlerAttribute, &r, &gWindowRef);
	result = ChangeWindowAttributes ( gWindowRef, kWindowNoAttributes, kWindowHideOnFullScreenAttribute );
	
	//OSStatus result = CreateNewWindow(kPlainWindowClass, kWindowStandardHandlerAttribute, &r, &gWindowRef);
	//OSStatus result = CreateNewWindow(kFloatingWindowClass, kWindowStandardHandlerAttribute, &r, &gWindowRef);
	if (result == noErr)
	{
		CGrafPtr thePort = GetWindowPort(gWindowRef);
		SetPort(thePort);
		
		ShowWindow(gWindowRef);

		MoveTo(20,30);
		TextFace(bold);
		TextSize(12);
  		DrawString(strTitle);		
 
		Flush();
		
		BringToFront(gWindowRef);
	}
}

//-----------------------------------------------------------
//
//  Display the banner
//  Simulate time going by by making the thermometer increase.
//  Reomove the banner.
//

static void PlayBanner ( char * pString, int type )
{
	IKString message;

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    if (type == 2) {
        message += pString;
        message += TEXT(" ");
    }
    else {
        NSString *hfsPath = [NSString stringWithCString: pString encoding: kCFStringEncodingMacRoman];
        CFURLRef pathUrlRef = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, (CFStringRef) hfsPath, kCFURLHFSPathStyle, true );
        [((id) pathUrlRef) autorelease];
        CFStringRef posixpath = CFURLCopyFileSystemPath( pathUrlRef, kCFURLPOSIXPathStyle ); 
        [((id) posixpath) autorelease];
        NSURL *url = [[[NSURL alloc] initFileURLWithPath: (NSString*)posixpath] autorelease];
        NSString *longFilename = [url absoluteString];
        NSArray *arrayFromPath = [longFilename componentsSeparatedByString:@"/"];
        NSString *filename= [[arrayFromPath lastObject] stringByReplacingOccurrencesOfString:@"%20"
                                                                                  withString:@" "];
    
        message += DATAS(TEXT("sending"),TEXT("sending"));
        message += " ";
        message += [filename cString];
        message += TEXT(" ...");
	}
    //NSLog(@"PlayBanner sending");
	
    NSString *s = [NSString stringWithUTF8String: (char*) message];
    NSDictionary *userinfo = [NSDictionary dictionaryWithObjectsAndKeys: s, @"overlayMessage", nil];
    [[NSDistributedNotificationCenter defaultCenter] 
        postNotificationName:@"com.cambium.usbmenu.notifications.overlayalert" object: nil userInfo: userinfo];
    [pool release];
}
#endif


//-----------------------------------------------------------
//
//  Use the helper app to do a command.
//     1 = is connected?
//     2 = send overlay.
//

static int DoCommand ( int ncommand, void *p1, void *p2 )
{
	//  spec the files
	FSSpec cspec;
	GetCommandSpec ( &cspec );
	FSSpec rspec;
	GetResultSpec ( &rspec );
	
	//  delete them
	FSpDelete ( &cspec );
	FSpDelete ( &rspec );
	
	//  write the command file
	FSpCreate ( &cspec, 'APPL', 'TEXT', smSystemScript );
	short fileRef;
	FSpOpenDF ( &cspec, fsRdWrPerm, &fileRef );
	SetFPos ( fileRef, fsFromStart, 0 );
	long len;
	len = 1;
	unsigned char c = ncommand;
	FSWrite (fileRef, &len, &c );  //  one byte for the command
	if (ncommand==2)
	{
		int i = strlen((char *)p1);
		len = 1;
		unsigned char x = i;
		FSWrite (fileRef, &len, &x );  //  one byte for the length of the string
		len = i;
		FSWrite (fileRef, &len, p1 );  //  bytes of the string
		len = 1;
		bool b = (bool) p2;
		FSWrite (fileRef, &len, &b );  //  one byte for the flag
	}
	FSClose (fileRef);
	
	//  launch the helper
	FSSpec spec;
	OSErr err = GetHelperSpec ( &spec );
	ProcessSerialNumber psn;
	err = LaunchIt ( &spec, &psn );
	
	//  wait for it
	while (IsProcessRunning(&psn))
	{
		EventRecord er;
		WaitNextEvent ( 0, &er, 1, NULL );
	}
	
	//  read the result file
	char c2;
	short fileRef2;
	FSpOpenDF ( &rspec, fsRdPerm, &fileRef2 );
	SetFPos ( fileRef2, fsFromStart, 0 );
	len = 1;
	FSRead (fileRef2, &len, &c2 );	
	FSClose (fileRef2);
	
	return c2;
}


//-----------------------------------------------------------
//
//  Use the helper app to send an overlay.
//

static int  ClassicSendOverlay ( char * pFilePath, bool bReportErrors )
{
	int result = DoCommand ( 2, (void *) pFilePath, (void *) bReportErrors );
	
	//  convert from old result codes to new
	
	if (result == NO_ERR)
		result = kNoError;
	else if (result == CHKSUM_ERR )
		result = kTransmitError;
	else if (result == TIME_OUT )
		result = kTransmitError;
	else if (result == BAD_CMD )
		result = kTransmitError;
	else if (result == LOW_MEM )
		result = kTransmitError;
	else if (result == NO_INTELL )
		result = kIntelliKeysNotFound;
	else
		result = kUnknownError;
		
	return result;
}

//-----------------------------------------------------------
//
//  Use the helper app to see if intellikeys is
//  connected.
//

static bool ClassicIsIntelliKeysConnected ()
{
	return !!DoCommand ( 1, (void *) 0, (void *) 0 );
}



#pragma export on

#if 0
extern "C" void DisplayBannerOnly ( char *text, int duration )
{
#if TARGET_API_MAC_CARBON
	//  make a pascal string
	Str255 strTitle;
	ConvertCToPStr255 ( text, strTitle );
	
	//  display the banner.
	DisplayBanner ( strTitle );
		
	//  simulate the time going by.
	int maxloop  = 50;
	for (int i=0;i<maxloop;i++)
	{
		displayProgress ( i+1, maxloop);
		Sleep(duration/maxloop);
	}
	
	//  take down the banner.
	RemoveBanner ();
#endif

}
#endif  //  0

//-----------------------------------------------------------
//
//  Is the file sendable?
//  Exported.
//  Called by the thin layer in the hosting app.
//

extern "C" bool IsSendableOverlayFile ( char * pFilePath )
{
	Init();

	if (IsWinOverlayFile(pFilePath))
		return true;
		
	if (IsMacOverlayFile(pFilePath))
		return true;
		
	return false;
}

//-----------------------------------------------------------
//
//  Is intellikeys connected?
//  Exported.
//  Called by the thin layer in the hosting app.
//

extern "C" bool IsIntelliKeysConnected ( void )
{
	Init();
	
#if TARGET_API_MAC_CARBON
	if (MacGetOSVersion() < 0x1000)
	{
		//  carbon os 9
		return ClassicIsIntelliKeysConnected ();
	}
	else
	{
		//  carbon os X
		if (!MacXInitializeBundle())
			return false;
			
		int result = myBundleIsIntellikeysConnectedPtr();
		return (result==1);
	}
	
#else
	//  classic os 9
	return ClassicIsIntelliKeysConnected ();
#endif

}

//-----------------------------------------------------------
//
//  report errors to the user
//

static void ReportErrors ( bool bReportErrors, char * pFilePath, int resultCode )
{
	//  do nothing
	if ( !bReportErrors )
		return;
		
	//  not an error, do nothing
	if ( resultCode == kNoError )
		return;
		
	//  get the simple file name
	
	int i = strlen(pFilePath)-1;
	while (i>=0)
	{
		if (pFilePath[i]==':')
		{
			i++;
			break;
		}
		i--;
	}
	
	char message[255];
	sprintf(&(message[1]),"Error sending overlay %s: Unknown error.",&(pFilePath[i]));
		
	switch (resultCode)
	{
	case kFileNotFound:
		sprintf(&(message[1]),"Error sending overlay %s: File not found.",&(pFilePath[i]));
		break;
		
	case kFileNotOverlay:
		sprintf(&(message[1]),"Error sending overlay %s: File is not an overlay file.",&(pFilePath[i]));
		break;
		
	case kTransmitError:
		sprintf(&(message[1]),"Error sending overlay %s: Transmission Error.",&(pFilePath[i]));
		break;
		
	case kIntelliKeysNotFound:
		sprintf(&(message[1]),"Error sending overlay %s: IntelliKeys not found.",&(pFilePath[i]));
		break;
		
	case kUnknownError:
		break;
		
	default:
		break;
	}
	
	message[0] = strlen(&(message[1]));
	
#if TARGET_API_MAC_CARBON
	SInt16 whichButton;
	OSErr err = StandardAlert ( kAlertNoteAlert, (const unsigned char *)message, nil, nil, &whichButton );
#endif
}

static int GetMode ()
{
	IKString group;
	IKString student;
	int Mode;
	
	//  load preferences
	IKString prefsFile = IKUtil::GetPrivateFolder();
	prefsFile += DATAS(TEXT("Preferences_File"),TEXT("preferences.txt"));
	IKPrefs prefs;
	prefs.Read(prefsFile);
	
	//  get group and student name
	group   = prefs.GetValueString(TEXT("Group"),  TEXT(""));
	student = prefs.GetValueString(TEXT("Student"),TEXT(""));
	if (group.IsEmpty() && student.IsEmpty())
		group = DATAS(TEXT("Guest"),TEXT("Guest"));  //  localize!!
	
	//  make path to the settings files
	IKString path = IKUtil::MakeStudentPath ( group, student );
	IKString settingsFile = path;
	settingsFile += TEXT("settings.txt");
	
	//  read user settings
	IKSettings::GetSettings()->Read(settingsFile);
	Mode = IKSettings::GetSettings()->m_iMode;
	
	return Mode;
}

static bool GetButAllow ()
{
	IKString group;
	IKString student;
	bool bButAllow;
	
	//  load preferences
	IKString prefsFile = IKUtil::GetPrivateFolder();
	prefsFile += DATAS(TEXT("Preferences_File"),TEXT("preferences.txt"));
	IKPrefs prefs;
	prefs.Read(prefsFile);
	
	//  get group and student name
	group   = prefs.GetValueString(TEXT("Group"),  TEXT(""));
	student = prefs.GetValueString(TEXT("Student"),TEXT(""));
	if (group.IsEmpty() && student.IsEmpty())
		group = DATAS(TEXT("Guest"),TEXT("Guest"));  //  localize!!
	
	//  make path to the settings files
	IKString path = IKUtil::MakeStudentPath ( group, student );
	IKString settingsFile = path;
	settingsFile += TEXT("settings.txt");
	
	//  read user settings
	IKSettings::GetSettings()->Read(settingsFile);
	bButAllow = IKSettings::GetSettings()->m_bButAllowOverlays;
	
	return bButAllow;
}

static bool CanShowModeMessage ()
{
	IKString group;
	IKString student;
	bool bShow;
	
	//  load preferences
	IKString prefsFile = IKUtil::GetPrivateFolder();
	prefsFile += DATAS(TEXT("Preferences_File"),TEXT("preferences.txt"));
	IKPrefs prefs;
	prefs.Read(prefsFile);
	
	//  get group and student name
	group   = prefs.GetValueString(TEXT("Group"),  TEXT(""));
	student = prefs.GetValueString(TEXT("Student"),TEXT(""));
	if (group.IsEmpty() && student.IsEmpty())
		group = DATAS(TEXT("Guest"),TEXT("Guest"));  //  localize!!
	
	//  make path to the settings files
	IKString path = IKUtil::MakeStudentPath ( group, student );
	IKString settingsFile = path;
	settingsFile += TEXT("settings.txt");
	
	//  read user settings
	IKSettings::GetSettings()->Read(settingsFile);
	bShow = IKSettings::GetSettings()->m_bShowModeWarning;
	
	return bShow;
}


static int GetIKVersion ()
{
	IKString strVersion = DATAS("Software_Version","0.0.0.0");
	int i = strVersion.Find('.');
	if(i==-1)
		return 0;
		
	IKString strMajor = strVersion.Left(i);
	
	int v = IKUtil::StringToInt(strMajor);
	return v;				
}


//-----------------------------------------------------------
//
//  Send the overlay.
//  Exported.
//  Called by the thin layer in the hosting app.
//


static int Send_Overlay ( int version, char * pFilePath, bool bReportErrors, char *pMessage, bool bWait, bool bBannerOnly )
{	
	int result;

#if TARGET_API_MAC_CARBON
	if (MacGetOSVersion() < 0x1000)
	{
		//  carbon os 9
		result = ClassicSendOverlay ( pFilePath, bReportErrors );
	}
	else
	{
		//  carbon os X
		if ( !MacXInitializeBundle() )
		{
			result = kIntelliKeysNotFound;
		}
		else
		{
			if ( !IsIntelliKeysConnected() )
			{
				result = kIntelliKeysNotFound;
			}
			else
			{
				Str255 strName;
				ConvertCToPStr255(pFilePath,strName);

				FSSpec spec;
				OSErr err = FSMakeFSSpec( 0, 0, strName, &spec );
				if (  err != noErr )
				{
					result =  kFileNotFound;
				}
				else
				{
					if ( !IsSendableOverlayFile(pFilePath) )
					{
						result = kFileNotOverlay;
					}
					else
					{	
						//  ready to send.
						
						//  get IK Version
						int ikVersion = GetIKVersion();
						
						//  for IK USB 3.x or greater
						if (ikVersion >=3)
						{	
							//  find the "owner" of intellikeys for this app
							IKString owner("");
							IKString appPath = IKUtil::GetCurrentApplicationPath();
							if (!appPath.IsEmpty())
							{
								owner = AppLibGetOwner(appPath);
							}
								
							if (version==2 && bBannerOnly)
							{
								//  from IK USB?
								if (!owner.IsEmpty())
									if (owner.CompareNoCase(DATAS(TEXT("intellikeys_name"),TEXT("IntelliKeys USB")))!=0)
										return kNoError;

								if (owner.IsEmpty())
									return kNoError;
							}
							else
							{
								//  generic overlay sender
								if (!owner.IsEmpty())
								{
									if (owner.CompareNoCase(DATAS(TEXT("intellikeys_name"),TEXT("IntelliKeys USB")))!=0)
										return kNoError;
										
									//  somehow got owned by IKUSB.  Release it.
									if (!appPath.IsEmpty())
									{
										AppLibReleaseOwnership ( appPath, owner);
										AppLibSetAppAllowed ( appPath, false );
									}
								}
								else
								{
									//  add to the disallowed list.
									//  we use the app path in Mac OS because we are not ikeysxfr.
									if (!appPath.IsEmpty())
										AppLibSetAppAllowed ( appPath, false );
								}
							}
						
							//  get mode.  If not zero,
							//  display a floating message and stop.
							int Mode = GetMode();
							bool bButAllow = GetButAllow();
							switch (Mode)
							{
							case 1:
#if 0  //  NO FLOATING_MESSAGE  7/25/06
								if (CanShowModeMessage())
									myBundleFloatingMessagePtr(DATAS("s_overlay_not_sent_1",""));
#endif
								return kNoError;
								break;

							case 2:
#if 0  //  NO FLOATING_MESSAGE  7/25/06
								if (CanShowModeMessage())
									myBundleFloatingMessagePtr(DATAS("s_overlay_not_sent_2",""));	
#endif
								return kNoError;
								break;

							case 3:
								if (!bButAllow)
								{
#if 0   //  NO FLOATING_MESSAGE  7/25/06
									if (CanShowModeMessage())
										myBundleFloatingMessagePtr(DATAS("s_overlay_not_sent_3",""));
#endif
									return kNoError;
								}
								break;

							default:
								break;
							}

						}
						
						//  send the overlay.
						if (bBannerOnly)
						{
							//  BTW, this is never true for IK version <=2
							if(pMessage)
								PlayBanner ( pMessage, 2 );
							else
								PlayBanner ( pFilePath, 1 );
							result = kNoError;
						}
						else
						{
							int response;
							if (myBundleBundleSendOverlay2Ptr)
							{
								//  if we can, tell engine about the "sender"
								IKString appPath = IKUtil::GetCurrentApplicationPath();
								IKString appName = IKUtil::GetAppFriendlyName(appPath);
								response = myBundleBundleSendOverlay2Ptr ("",pFilePath,appName);
							}
							else
							{
								response = myBundleBundleSendOverlayPtr ("",pFilePath);
							}
							if ( response != 1 )
								result = kTransmitError;
							else
							{
								if (pMessage)
                                    PlayBanner ( pMessage, 2 );
								else
									PlayBanner (pFilePath, 1 );
								result = kNoError;
							}					
						}
					}
				}
			}
		}
	}
	
#else	
	//  classic os 9
	result = ClassicSendOverlay ( pFilePath, bReportErrors );
#endif

	ReportErrors ( bReportErrors, pFilePath, result );
	
	return result;
}

extern "C" int  SendOverlay ( char * pFilePath, bool bReportErrors )
{
	Init();
	return Send_Overlay ( 1, pFilePath, bReportErrors, NULL, false, false );
}

extern "C" int  SendOverlay2 ( char * pFilePath, bool bReportErrors, char *pMessage, bool bWait, bool bBannerOnly )
{
	Init();
	return Send_Overlay ( 2, pFilePath, bReportErrors, pMessage, bWait, bBannerOnly );
}

#pragma export off


//-----------------------------------------------------------
//
//  Main, used when building a test app.
//
//  define TESTAPP based on 
//  target so we can build an app and debug it.
//



#ifndef BUILD_UNIVERSAL

#undef TESTAPP

#if __ide_target("carbon test")
  #define TESTAPP 1
#endif
#if __ide_target("classic test")
  #define TESTAPP 1
#endif

#endif

int main ( void )
{
#ifdef TESTAPP

	Init();
	
	bool b;
	int i;
	
	//b = IsIntelliKeysConnected();

	//i = SendOverlay2 ( "os x 10.4:users:fredrossperry:desktop:One.oms", true, "sending bbb for fred...", true, false );
	//i = SendOverlay ( "Macintosh HD:users:fredross-perry:desktop:One.oms", true );
	//i = SendOverlay ( "Macintosh HD:Projects:Suite_4_0:Overlays:Classroom ABC", true );
	//i = SendOverlay ( "Macintosh HD:users:fredross-perry:desktop:a", true );

	//b = IsSendableOverlayFile ( "Macintosh HD:Projects:Suite_4_0:Overlays:Classroom ABC");
	//b = IsSendableOverlayFile ( "Macintosh HD:Users:fredross-perry:Desktop:step scan overlay R ljb.oms");
	//b = IsSendableOverlayFile ( "Two Swtiches To Success 8 06:Classroom Suite Files:2 Switches To Success REV06:Overlays:step scan overlay R ljb.oms");
	//b = IsSendableOverlayFile ( "Untitled:Two Swtiches To Success 8 06:Classroom Suite Files:2 Switches To Success REV06:Overlays:step scan overlay R ljb.oms");
	//b = IsSendableOverlayFile ( "Untitled CD:step scan overlay R ljb.oms");
	b = IsSendableOverlayFile ( "Two Swtiches To Success 8 06:step scan overlay R ljb.oms");

#endif

	return 0;
}
