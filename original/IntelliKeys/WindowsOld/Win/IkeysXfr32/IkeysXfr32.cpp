/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		IkeysXfr.cpp
//
// Purpose:		Supports comm protocol with either legacy or USB IntelliKeys
//
//
// 1	Send Overlay
// 2	Get Overlay
// 3	Send Set
// 4	Get Set
// 9	Detect IntelliKeys

// U	User initiated - complain if no IntelliKeys
// I	Invisible operation - no progress meter                               
// D	Don't ask to override overlay lock - just return
// P	Pass set in file name
// A	Always send overlay, even if name matches current overlay in IntelliKeys
// L	Latching?
// N    Do not activate the background window
// B	Just display a banner, and that's it.

//
// 06/18/01 fwr initial implementation	
// 07/09/01 dgs made strings resource-based for IDS_NO_IKEYS_FOUND and IDS_OVERLAYFILE_ERR messageboxes 
//
**************************************************************************************************************************/

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>

#include "resource.h"

#include "MessageClient.h"
#include "Interface.h"
#include "AppLib.h"

#include "IKCommon.h"
#include "IKString.h"
#include "IKUtil.h"
#include "IKOverlay.h"
#include "IKSettings.h"
#include "StdAfx.h"

typedef struct {
	int xferCommand;
	bool userInitiated;
	bool progressFlag ;
	bool askOverride;
	bool passSet;
	bool checkName;
	bool setLatching;
	bool parameterN;
	bool bannerOnly;
} flagStruct;

#define _FILE_NOT_FOUND 101
#define _FILE_BAD 102
#define _IO_ERROR 103
#define _UNKNOWN_FLAG 104
#define _UNKNOWN_EXTN 105
#define _IK_NOT_FOUND 106
#define _IK_XFER_ERR 107
#define _OUT_OF_MEMORY 108
#define _VXD_NOT_FOUND 109
#define _KEY_HELD_DOWN 110
#define _STANDARD_IN 111
#define _UPGRADE_IK 112
#define _ASK_LAPTOP	113
#define _ASK_THINKPAD 114
#define _PLEASE_RETRY 115
#define _NOT_OWNER 116
#define _NO_RESEND 117

#define _SENDING 201
#define _RECEIVING 202

#define _LOCK_TEXT 300

#define _ERROR_TITLE 1000
#define _PREPARE_TITLE 1001       
#define _LOCK_TITLE	1002
#define _WARNING_TITLE 1003
#define _SETUP_TITLE 1004
#define IDS_DIAGNOSE 1005

#define _INTERNAL_ERROR 2000


HANDLE thisInstance = NULL;

void CreateProgressWindow ( void );
void CloseProgressWindow ( void );
void ShowProgress( int sofarcount );
void SetWindowTitle( char *name );
void SetWindowTitle2( char *name );

#define MAX_PARAM 4
#define MAX_PARAM_LEN 200
#define MAX_COMMAND_LEN 1024

static char param[MAX_PARAM][MAX_PARAM_LEN];

static IKString group;
static IKString student;
static int Mode = 0;
static bool bShowModeWarning = true;
static bool bButAllowOverlays = true;


static void LoadPreferences ()
{
	//  load preferences
	IKString prefsFile = IKUtil::GetPrivateFolder();
	prefsFile += DATAS(TEXT("Preferences_File"), TEXT("preferences.txt"));
	IKPrefs prefs;
	prefs.Read(prefsFile);

	//  get group and student name
	group   = prefs.GetValueString(TEXT("Group"),  TEXT(""));
	student = prefs.GetValueString(TEXT("Student"),TEXT(""));
	if (group.IsEmpty() && student.IsEmpty())
	{
		group = DATAS(TEXT("Guest"), TEXT("Guest"));
	}

	//  make path to the settings files
	IKString path = IKUtil::MakeStudentPath(group, student);
	IKString settingsFile = path;
	settingsFile += TEXT("settings.txt");

	//  read user settings
	IKSettings::GetSettings()->Read(settingsFile);

	Mode = IKSettings::GetSettings()->m_iMode;
	bShowModeWarning = IKSettings::GetSettings()->m_bShowModeWarning;
	bButAllowOverlays = IKSettings::GetSettings()->m_bButAllowOverlays;
}

typedef struct
{
	int start;
	int end;
} par;

void DoMsg(char *s1, char *s2, int n);
void DoMsg(char *s1, char *s2, int n)
{
	char message[256];
	wsprintf( message, "s1=\"%s\" s2=\"%s\" n=%d", s1,s2,n);
	::MessageBox(NULL,message,"",MB_OK);
}

static void ParseCommand (char *commandline)
{
	//  set all parameters to ""
	for (int i2 = 0; i2 < MAX_PARAM; i2 ++)
	{
		strcpy(param[i2],"");
	}

	//  find start and end of all incoming parameters, and 
	//  how many there are.

	par pars[100];
	int npars = 0;

	int nq = 0;
	int inpar = 0;

	int l = strlen(commandline);

	for (int j = 0; j < l; j++)
	{
		switch (commandline[j])
		{
			case ' ':
			case '\t':
				if (nq == 0)
				{
					if (inpar != 0)
					{
						pars[npars - 1].end = j - 1;
					}
					inpar = 0;
				}
				break;

			case '\'':
			case '\"':
				if ((j > 0) && (j < (l-1)) && (commandline[j - 1] != ' ') && (commandline[j + 1] != ' '))
				{
					goto mylabel;
				}
				nq = 1 - nq;
				if (inpar != 0)
				{
					pars[npars - 1].end = (j - 1);
				}
				inpar = 0;
				break;

			default:
mylabel:
				if (inpar == 0)
				{
					inpar = 1;
					npars++;
					pars[npars - 1].start = j;
				}
				break;
		}
	}

	//  flags
	bool bFlags = true;
	{
		for (int i = pars[npars - 1].start; i <= pars[npars -1 ].end; i++)
		{
			if (strchr("12349UIDPALNBuidpalnb", commandline[i]) == 0)
			{
				bFlags = false;
			}
		}
	}
	if (bFlags)
	{
		strncpy(param[3], &(commandline[pars[npars - 1].start]), pars[npars - 1].end - pars[npars - 1].start + 1);
		param[3][(pars[npars - 1].end - pars[npars - 1].start) + 1] = 0;
		npars--;
	}

	//  text string
	strncpy(param[2], &(commandline[pars[npars - 1].start]), pars[npars - 1].end - pars[npars - 1].start + 1);
	param[2][(pars[npars - 1].end - pars[npars -1 ].start) + 1] = 0;
	npars--;

	//  filename
	char filename[512];
	strcpy(filename,"");
	char p[512];
	{
		for (int i = 1; i < npars; i++)
		{
			strncpy(p, &(commandline[pars[i].start]), pars[i].end - pars[i].start + 1);
			p[pars[i].end - pars[i].start + 1] = 0;
			//strcat(p," ");
			strcat(filename, p);
		}
	}
	strcpy(param[1], filename);

	//  executable
	strncpy(param[0], &(commandline[pars[0].start]), pars[0].end - pars[0].start + 1);
	param[0][pars[0].end - pars[0].start + 1] = 0;
}


static void ProcessFlags(LPSTR flags, flagStruct *myFlags)
{
	myFlags->xferCommand = 0;
	myFlags->userInitiated = FALSE;
	myFlags->progressFlag = TRUE;
	myFlags->askOverride = TRUE;
	myFlags->passSet = FALSE;
	myFlags->checkName = TRUE;
	myFlags->setLatching = TRUE;
	myFlags->parameterN = FALSE;
	myFlags->bannerOnly = FALSE;

	int l = strlen(flags);
	for (int i = 0; i < l; i++)
	{
		switch (flags[i])
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				myFlags->xferCommand = flags[i] - '0';
				break;

			case '9':
				myFlags->progressFlag = FALSE;
				myFlags->xferCommand = flags[i] - '0';
				break;

			case 'U':	// User-initiated
			case 'u':
				myFlags->userInitiated = TRUE;
				break;

			case 'i':   // Invisible Operation
			case 'I':
				myFlags->progressFlag = FALSE;
				break; 
				
			case 'd':   // Don't ask to override lock
			case 'D':
				myFlags->askOverride = FALSE;
				break;

			case 'p':	// Pass set in file name
			case 'P':
				myFlags->passSet = TRUE;
				break;

			case 'a':   // Always send overlay, even if name matches
			case 'A':
				myFlags->checkName = FALSE;
				break;
				
			case 'l':
			case 'L':
				myFlags->setLatching = FALSE;
				break;

			case 'n':
			case 'N':
				myFlags->parameterN = TRUE;
				break;

			case 'b':
			case 'B':
				myFlags->bannerOnly = TRUE;
				break;

			default:
				break;
		}
	}
}


static int DoIkXfer16 ( char *p1, char *p2, char *p3 )
{
	//  returns:
	//	0 - aok
	//  1 - cannot start program
	//  2 - intellikeys not found
	//  4 - file not found
	//  3 - some other error


	//  figure out whether to inactivate the foreground window
	//  when the progress bar is shown.

	BOOL bInactiveProgWindow = TRUE;  //  assume true

	flagStruct myFlags;
	ProcessFlags ( p3, &myFlags );
	if (myFlags.xferCommand !=1 )
		bInactiveProgWindow = FALSE;  //  false if we're not sending
	
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof ( osvi );
	::GetVersionEx( &osvi );
	if ( osvi.dwPlatformId != VER_PLATFORM_WIN32_NT )
		bInactiveProgWindow = FALSE;  //  false if we're not NT

	char s3[255];
	strcpy(s3,p3);
	if ( bInactiveProgWindow )
		strcat(s3,"N");  //  modify parameter
	
	//  save the current foreground window, we'll restore it later.
	HWND foregroundWnd = ::GetForegroundWindow();
	
	//  construct a command line for the sending program
	char pathName[MAX_PATH];
	::GetSystemDirectory( pathName, sizeof ( pathName ) );
	char cmdLine[300];
	wsprintf ( cmdLine, "%s\\IKEYSXFR16.EXE %s '%s ' %s", pathName, p1, p2, s3 );

	
	//  start the sending program
	DWORD finalStatus = 0;
	STARTUPINFO startupInfo;
	memset( &startupInfo, 0, sizeof ( startupInfo ) );
	startupInfo.cb = sizeof ( startupInfo );
	PROCESS_INFORMATION processInfo;
	BOOL bStart = CreateProcess ( NULL, cmdLine, NULL, NULL, 
		FALSE, HIGH_PRIORITY_CLASS,NULL, NULL, &startupInfo, &processInfo );
	if (!bStart)
		finalStatus = 1;
	else
	{
		//  wait for the program to finish, time out after 30 seconds
		DWORD dwStatus = WaitForSingleObject ( processInfo.hThread, 30000 );
		if(dwStatus==WAIT_OBJECT_0)
		{
			// sss: check return value from the sending program    
			DWORD exitCode = 0;
			if ( ::GetExitCodeProcess( processInfo.hProcess, &exitCode ) )
			{
				if (exitCode == 0x0000006a)
					finalStatus = 2;
				else if (exitCode == 0x00000065)
					finalStatus = 4;
				else
					finalStatus = 0;
			}
			else
			{
				finalStatus = 3; //  ok????
			}
		}
		else
		{
			finalStatus = 3;
		}
	}

	//  clean up
	::CloseHandle ( processInfo.hThread );
	::CloseHandle ( processInfo.hProcess );
	
	//  restore the previous
	::SetForegroundWindow(foregroundWnd);

	return finalStatus;

}

static bool IsSpace(char c)
{
	if (c == ' ')
	{ 
		return true;
	}
	if ( c== '\0')
	{
		return true;
	}
	return false;
}

static void Trim(LPSTR string)
{
	//  where does it start:
	int l = strlen(string);
	int start = 0;
	for (int i=start; i < l; i++)
	{
		if ((IsSpace(string[i]) == false) && (string[i] != '\'') && (string[i] != '\"'))
		{
			start = i;
			break;
		}
	}

	//  where does it end?
	int end = l - 1;
	for (int i2 = end; i2 >= 0; i2--)
	{
		if ((IsSpace(string[i2]) == false) && (string[i2] != '\'') && (string[i2] != '\"'))
		{
			end = i2;
			break;
		}
	}

	//  fix it up
	strncpy(string, &(string[start]), end-start + 1);
	string[end - start + 1] = '\0';

}

static void Quiet(LPSTR string)
{
	//  remove any "I" or "U"
	char newstring[255];
	int n = 0;
	int l = strlen(string);
	for (int i=0; i < l; i++)
	{
		if (!((string[i] == 'i') || (string[i] == 'I') || (string[i] == 'u') || (string[i] == 'U')))
		{
			newstring[n] = string[i];
			n++;
		}
	}
	newstring[n] = '\0';
	strcpy(string, newstring);

	//  now add "I" back in
	strcat(string, "I");
}


static bool MakeTempCopy(char *filename, char *tempname)
{
	//  get a temp name
	char path[1024];
	::GetTempPath(1024, path);

	UINT result = GetTempFileName(path, "IKX", 0, tempname);

	if (result == 0)
	{
		::GetTempPath(1024, tempname);
		strcat(tempname, "IKEYSXFR.TMP");
	}

#ifdef _DEBUG
	FILE* f = fopen("c:\\temp\\IkeysXfr32.txt", "a+");
	fprintf(f, "IKeysXfr32.MakeTempCopy: start. FileName=[%s]\n", filename);
	fprintf(f, "IKeysXfr32.MakeTempCopy: - TempFileName=[%s] Result: %u\n", tempname, result);
	fflush(f);
	fclose(f);
#endif

	//  delete any existing file by this name
	IKMakeWriteable(tempname);

#ifdef _DEBUG
	f = fopen("c:\\temp\\IkeysXfr32.txt", "a+");
	fprintf(f, "IKeysXfr32.MakeTempCopy: - calling DeleteFile(%s)\n", tempname);
	fflush(f);
	fclose(f);
#endif
	DeleteFile(tempname);

#ifdef _DEBUG
	f = fopen("c:\\temp\\IkeysXfr32.txt", "a+");
	fprintf(f, "IKeysXfr32.MakeTempCopy: - calling CopyFile(%s, %s)\n", filename, tempname);
	fflush(f);
	fclose(f);
#endif
	//  make a copy
	BOOL bCopied = !!::CopyFile(filename, tempname, FALSE);

	IKMakeWriteable(tempname);

#ifdef _DEBUG
	f = fopen("c:\\temp\\IkeysXfr32.txt", "a+");
	fprintf(f, "IKeysXfr32.MakeTempCopy: end. Result: %s\n", !!bCopied ? "Success" : "Failed");
	fflush(f);
	fclose(f);
#endif
	return !!bCopied;
}


BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int val, LPCTSTR lpFileName)
{
	char s[100];
	wsprintf( s, "%d", val);
	return WritePrivateProfileString(lpAppName, lpKeyName, s, lpFileName);
}


int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR     lpszCmdParam,
					 int       nCmdShow)
{
	thisInstance = hInstance;

	//  pull the command apart.
	//  first parameter is the name of this program
	//  second is the file name
	//  third is the screen name
	//  fourth is flags

	char commandline[MAX_COMMAND_LEN];
	strcpy(commandline, ::GetCommandLine());

	DebugLogToFile("IkeysXfr32.Main: CommandLine: [%s]", commandline);

	strcat(commandline," ");
	ParseCommand(commandline);

	DebugLogToFile("IkeysXfr32.Main: Parsed Parameters[0 - PgmName]: [%s]", param[0]);
	DebugLogToFile("IkeysXfr32.Main: Parsed Parameters[1 - OverlayFileName]: [%s]", param[1]);
	DebugLogToFile("IkeysXfr32.Main: Parsed Parameters[2 - ScreenName]: [%s]", param[2]);
	DebugLogToFile("IkeysXfr32.Main: Parsed Parameters[3 - Flags]: [%s]", param[3]);

	//  is IK USB 1.x there?
	bool bIsV1ServerRunning = false;
	int server = IKUSBIsServerRunning();
	if (server == kResponsev1NoError)
	{
		bIsV1ServerRunning = true;
	}
	
	DebugLogToFile("IkeysXfr32.Main: IsV1ServerRunning: [%s]", bIsV1ServerRunning ? "Yes" : "No");

	//  if no IK USB 1.x, assume 2.x and initialize
	//  the 2.x message system
	if (bIsV1ServerRunning == false)
	{
		IKMessageInitialize();
	}

	//  supply default flags if none given
	if (! param[3][0])
	{
		strcpy(param[3], "1A");	
	}

	//  decode flags
	flagStruct myFlags;
	ProcessFlags((LPSTR)param[3], &myFlags);

	DebugLogToFile("IkeysXfr32.Main: -ProcessFlags -1- Flags=[%s] - xferCommand(1-9+)=%d userInitiated(u+)=%d progressFlag(i-)=%d askOverride(d-)=%d passSet(p+)=%d checkName(a-)=%d setLatching(l-)=%d parameterN(n+)=%d bannerOnly(b+)=%d",
				(LPSTR)param[3],
				myFlags.xferCommand,
				myFlags.userInitiated,
				myFlags.progressFlag,
				myFlags.askOverride,
				myFlags.passSet,
				myFlags.checkName,
				myFlags.setLatching,
				myFlags.parameterN,
				myFlags.bannerOnly);

	//  force send if no command given
	if (myFlags.xferCommand == 0)
	{
		strcat(param[3],"1");
	}

	ProcessFlags((LPSTR)param[3], &myFlags);

	DebugLogToFile("IkeysXfr32.Main: -ProcessFlags -2- Flags=[%s] - xferCommand(1-9+)=%d userInitiated(u+)=%d progressFlag(i-)=%d askOverride(d-)=%d passSet(p+)=%d checkName(a-)=%d setLatching(l-)=%d parameterN(n+)=%d bannerOnly(b+)=%d",
				(LPSTR)param[3],
				myFlags.xferCommand,
				myFlags.userInitiated,
				myFlags.progressFlag,
				myFlags.askOverride,
				myFlags.passSet,
				myFlags.checkName,
				myFlags.setLatching,
				myFlags.parameterN,
				myFlags.bannerOnly);

	LoadPreferences();

	//  if send requested, check for overlay file
	bool bOverlayExists = true;
	bool bOverlayValid  = false;
	char fullname[255];
	char *filepart;
	if (myFlags.xferCommand == 1)
	{
		if (strcmp(param[1],"") == 0)
		{
			bOverlayExists = false;
		}
		else
		{
			FILE *in;
			in = ::fopen(param[1], "rb");
			if (in == NULL)
			{
				bOverlayExists = false;
			}
			else
			{
				unsigned char b1 = 0;
				unsigned char b2 = 0;
				int iresult = ::fread(&b1, 1, 1, in);
				iresult = ::fread(&b2, 1, 1, in);
				WORD nbytes = b1 | 256 * b2;
				::fclose(in);
				if((nbytes > 0) && (nbytes <= 0x7200))
				{
					bOverlayValid  = true;
				}

				DWORD result = ::GetFullPathName(param[1], 255, fullname, &filepart);
				if (result == 0)
				{
					strcpy(fullname, param[1]);
				}
				else
				{
					fullname[result] = '\0';
				}
			}
		}
	}

	DebugLogToFile("IkeysXfr32.Main: OverlayExists: %s OverlayValid: %s FullName: %s",
		bOverlayExists ? "Yes" : "No",
		bOverlayValid ? "Yes" : "No",
		fullname);

	//  are old or new Intellikeys connected?

	bool bIK16Connected = false;
	bool bIK32Connected = false;
	bool bIK32v1Connected = false;

	if (myFlags.bannerOnly)
	{
		bIK32Connected = true;  //  
	}
	else
	{
		//  for NT, don't even use the old SS
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		::GetVersionEx( &osvi );

		if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT)
		{
			int ik16status = DoIkXfer16("x","x","9I");
			if ((ik16status == 0) || (ik16status == 4))
			{
				bIK16Connected = true;
			}
		}

		if (bIsV1ServerRunning == false)
		{
			int ik32status = IKIsIntellikeysConnected();
			if (ik32status == kResponseConnected)
			{
				bIK32Connected = true;
			}
		}

		if (bIsV1ServerRunning)
		{
			int ik32statusv1 = IKUSBIsIntellikeysConnected ();  //  legacy
			if (ik32statusv1 == kResponsev1Connected)
			{
				bIK32v1Connected = true;
			}
		}
	}

	DebugLogToFile("IkeysXfr32.Main: IK16Connected: %s IK32Connected: %s IK32v1Connected: %s",
		bIK16Connected ? "Yes" : "No",
		bIK32Connected ? "Yes" : "No",
		bIK32v1Connected ? "Yes" : "No");

	//  if nothing connected, give error message if
	//  not silent, then bail.
	if ((bIK16Connected == false) && (bIK32Connected == false) && (bIK32v1Connected == false))
	{
		if (myFlags.userInitiated)
		{
			// April 2013 - JR - need to localize the message box so moved text to data.txt
			IKString dlgTitle = DATAS(TEXT("intellikeys_name"), TEXT("IntelliKeys USB"));
			IKString dlgMsg = DATAS(TEXT("S_NO_IKEYS_FOUND"), TEXT("No IntelliKeys found"));
			// May 2013 - because of localized strings, use the Unicode version of the Message box
			int msgLen = dlgMsg.GetLength();
			WCHAR dlgMsgWide[1024];
			int res = MultiByteToWideChar ( CP_UTF8, 0, dlgMsg, msgLen, dlgMsgWide, 1024);
			dlgMsgWide[res] = 0;

			msgLen = dlgTitle.GetLength();
			WCHAR dlgTitleWide[1024];
			res = MultiByteToWideChar ( CP_UTF8, 0, dlgTitle, msgLen, dlgTitleWide, 1024);
			dlgTitleWide[res] = 0;

			::MessageBoxW(NULL, dlgMsgWide, dlgTitleWide, MB_OK);
			//::MessageBox(NULL, dlgTitle, dlgMsg, MB_OK);

			//char title2[255];
			//char text2[255];
			//::LoadString(NULL,IDS_IKEYS, title2, 255);
			//::LoadString(NULL, IDS_NO_IKEYS_FOUND, text2, 255);
			//::MessageBox(NULL,text2, title2, MB_OK);
		}

		DebugLogToFile("IkeysXfr32.Main: Nothing Connected");

		return _IK_NOT_FOUND;
	}

	bool bIsParentClickit = IKUtil::IsParentClickit ();
	IKString parentAppPath = IKUtil::GetParentApp();

	bool bDoSendChecking = false;
	if (myFlags.xferCommand == 1)
	{
		bDoSendChecking = true;
	}
		
	//  don't do send checking if we're also not watching apps
	bool bWatch = !!DATAI(TEXT("watch_apps"), 0);
	if (!bWatch)
	{
		bDoSendChecking = false;
	}

	DebugLogToFile("IkeysXfr32.Main: IsParentClickit=%s ParentAppPath=[%s] DoSendChecking=%s", bIsParentClickit ? "Yes" : "No", (LPSTR)parentAppPath, bDoSendChecking ? "Yes" : "No");

	if (bDoSendChecking)
	{
		//  is our parent ClickIt?
		if (bIsParentClickit)
		{
			strcat(param[2]," for ClickIt!");
		}

		//  find the "owner" of Intellikeys for this app
		IKString owner("");
		IKString appPath = IKUtil::GetCurrentApplicationPath();

		if (parentAppPath.Right(19).CompareNoCase(TEXT("\\overlay sender.exe")) == 0)
		{
			appPath = parentAppPath;
		}

		if (!appPath.IsEmpty())
		{
			owner = AppLibGetOwner(appPath);
		}

		DebugLogToFile("IKeysXfr32.Main: - IntellKeys Owner=[%s]", (LPSTR)owner);

		bool bUpdateDisallowed = false;
		if (myFlags.bannerOnly)
		{
			//  from IK USB?
			if (!owner.IsEmpty())
			{
				if (owner.CompareNoCase(DATAS(TEXT("intellikeys_name"),TEXT("IntelliKeys USB"))) != 0)
				{
					return _NOT_OWNER;
				}
			}

			if (owner.IsEmpty())
			{
				return _NOT_OWNER;
			}
		}
		else
		{
			//  generic overlay sender
			if (!owner.IsEmpty())
			{
				if (owner.CompareNoCase(DATAS(TEXT("intellikeys_name"),TEXT("IntelliKeys USB"))) != 0)
				{
					return _NOT_OWNER;
				}
				//  don't allow ClickIt to send if owner is intellikeys
				if (bIsParentClickit && (owner.CompareNoCase(DATAS(TEXT("intellikeys_name"), TEXT("IntelliKeys USB"))) == 0))
				{
					return _NOT_OWNER;
				}

				//  somehow got owned by IKUSB, take that back. 
				//AppLibReleaseOwnership(appPath,owner);
				bUpdateDisallowed = true;
			}
			else
			{
				bUpdateDisallowed = true;
			}
		}

		//  update the disallowed list
		if (bUpdateDisallowed && (myFlags.xferCommand == 1))
		{
			//  add to the disallowed list.
			if (!parentAppPath.IsEmpty())
			{
				AppLibSetAppAllowed ( parentAppPath, false );
			}
		}

		//  if we're IK USB, and Mode is not zero,
		//  just display a message.

		if (bIK32Connected && (myFlags.xferCommand == 1) && bOverlayExists)
		{
			IKString s;
			switch (Mode)
			{
			case 1:
				//  NO FLOATING MESSAGE 7/25/06
				return 0;
				break;

			case 2:
				//  NO FLOATING MESSAGE 7/25/06
				return 0;
				break;

			case 3:
				if (!bButAllowOverlays)
				{	
					//  NO FLOATING MESSAGE 7/25/06
					return 0;
				}
				break;
			}
		}

	}

	//  Banner Only?
	if (myFlags.bannerOnly)
	{
		DebugLogToFile("IKeysXfr32.Main: - BannerOnly - [%s]", (LPSTR)param[2]);

		CreateProgressWindow();

		SetWindowTitle2((LPSTR)param[2]);

		int duration = 2000;
		int segments = 400;
		unsigned int start = ::GetTickCount();
		while (1 == 1)
		{
			unsigned int now = ::GetTickCount();
			if (now > (start+duration))
			{
				break;
			}

			ShowProgress((now - start) * 100 / duration);  //  100 units

			Sleep(duration/segments);
		}

		CloseProgressWindow();

		DebugLogToFile("IKeysXfr32.Main: - BannerOnly - end.");
		return 0;
	}

	//  if just ik16 connected, hand everything over
	if (bIK16Connected && (bIK32Connected == false) && (bIK32v1Connected == false))
	{
		int result = DoIkXfer16((LPSTR) param[1], (LPSTR)param[2], (LPSTR)param[3]);
		switch (result)
		{
			case 0:  return 0;               break;
			case 1:  return _VXD_NOT_FOUND;  break;
			case 2:  return _IK_NOT_FOUND;   break;
			case 3:  return _IK_XFER_ERR;    break;
			case 4:  return _FILE_NOT_FOUND; break;
			default: return _INTERNAL_ERROR; break;
		}
	}

	//  send to old device
	int status16 = 0;
	if (bIK16Connected)
	{
		//  send same command to the old 16-bit version
		//  but, make it invisible since we're doing the UI
		char newp3[255];
		strcpy(newp3, param[3]);
		Quiet(newp3);
		status16 = DoIkXfer16((LPSTR)param[1], (LPSTR)param[2], (LPSTR)newp3);
	}

	//  send to new device
	int status32 = 0;
	if ((bIK32Connected || bIK32v1Connected) && bOverlayExists)
	{
		if ((myFlags.xferCommand == 1) && (bOverlayValid == false))
		{
			status32 = _FILE_BAD;
		}
		else
		{
			//  put up the banner
			if  (myFlags.progressFlag && (myFlags.xferCommand != 9))
			{
				CreateProgressWindow();
				switch (myFlags.xferCommand)
				{
					case 3:
						DebugLogToFile("IKeysXfr32.Main: - Xfer Banner: [%s]", "Sending settings...");

						SetWindowTitle2(DATAS(TEXT("S_SENDING_SETTINGS"), TEXT("Sending settings...")));
						break;

					case 4:
						DebugLogToFile("IKeysXfr32.Main: - Xfer Banner: [%s]", "Getting settings...");

						SetWindowTitle2(DATAS(TEXT("S_GETTING_SETTINGS"), TEXT("Getting settings...")));
						break;

					default:
						DebugLogToFile("IKeysXfr32.Main: - Xfer Banner: [%s]", (LPSTR)param[2]);

						SetWindowTitle((LPSTR)param[2]);
						break;
				}
			}

			//  send the same command to IKUSB.
			switch (myFlags.xferCommand)
			{
				case 1:
					{
						//  get the name of our parent.
						IKString sender;
						if (bIsParentClickit)
						{
							IKString appPath = IKUtil::GetCurrentApplicationPath();
							IKString friendly = IKUtil::GetAppFriendlyName(appPath);
							sender = TEXT("ClickIt! (");
							sender += DATAS(TEXT("for"), TEXT("for"));
							sender += TEXT(" ");
							sender += friendly;
							sender += TEXT(")");
						}
						else
						{
							DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - calling IKUtil::GetParentApp");

							//IKString appPath = IKUtil::GetCurrentApplicationPath();
							IKString appPath = IKUtil::GetParentApp();

							DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - ParentApp=[%s] - calling IKUtil::GetAppFriendlyName", (LPSTR)appPath);

							sender = IKUtil::GetAppFriendlyName(appPath);

							DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - AppFriendlyName=[%s]", (LPSTR)sender);
						}

						DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - calling GetShortPathName(%s)", (LPSTR)fullname);
						//  make a short name
						char shortname[255];
						GetShortPathName((LPSTR)fullname, shortname, 255);

						DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - calling MakeTempCopy(%s)", (LPSTR)shortname);

						//  make a temp copy
						char tempname[255];
						//DeleteFile(tempname);
						bool bCopied = MakeTempCopy(shortname, tempname);

						if (bCopied)
						{
							if (bIK32Connected)
							{
								DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - Copied - IKSendOverlayWithName - Engine -: OverlayName: [%s] Name: [%s] Sender: [%s]", tempname, (LPSTR)param[2], (LPSTR)sender);

								status32 = IKSendOverlayWithName(tempname, (LPSTR)param[2], sender);
								DeleteFile(tempname);
							}
							else
							{
								DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - Copied - IKUSBSendOverlayWithName: OverlayName: [%s] Name: [%s]", tempname, (LPSTR)param[2]);

								status32 = IKUSBSendOverlayWithName(tempname, (LPSTR)param[2]);
								//  do not delete the copied file in the v1 case.  This is because
								//  the IK USB 1.04 engine may not have copied the file yet.
							}
						}
						else
						{
							if (bIK32Connected)
							{
								DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - ! Copied - IKSendOverlayWithName - Engine -: OverlayName: [%s] Name: [%s] Sender: [%s]", shortname, (LPSTR)param[2], (LPSTR)sender);

								status32 = IKSendOverlayWithName(shortname, (LPSTR)param[2], sender);
							}
							else
							{
								DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - ! Copied - IKUSBSendOverlayWithName: OverlayName: [%s] Name: [%s]", shortname, (LPSTR)param[2]);

								status32 = IKUSBSendOverlayWithName (shortname, (LPSTR) param[2]);
							}
						}
					}

					break;

				case 3:
					{
						BYTE settings[24] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
							0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
							0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

						FILE *sf;
						sf = fopen(param[1],"r");
						if (sf != NULL)
						{
							fread(settings, 1, 24, sf);
						}
						fclose(sf);

						if (bIK32Connected)
						{
							DebugLogToFile("IKeysXfr32.Main: - XferCommand=%d ProgressFlag=%d - IKSendSettings from [%s]", myFlags.xferCommand, myFlags.progressFlag, param[1]);

							status32 = IKSendSettings(settings);
						}
						else
						{
							DebugLogToFile("IKeysXfr32.Main: - XferCommand=%d ProgressFlag=%d - IKUSBSendSettings from [%s]", myFlags.xferCommand, myFlags.progressFlag, param[1]);

							status32 = IKUSBSendSettings(settings);
						}
					}
					break;

				case 4:  //  get settings
				default:
					break;
			}

			DebugLogToFile("IKeysXfr32.Main: - XferCommand=%d ProgressFlag=%d - Response: [%d]", myFlags.xferCommand, myFlags.progressFlag, status32);

			//  fake some progress

			if (myFlags.xferCommand != 9)
			{
				int duration = 2000;
				int segments = 400;
				unsigned int start = ::GetTickCount();
				while (1 == 1)
				{
					unsigned int now = ::GetTickCount();
					if (now > (start + duration))
					{
						break;
					}

					ShowProgress((now-start) * 100 / duration);  //  100 units

					Sleep(duration / segments);
				}
			}

			//  take down the banner
			if (myFlags.progressFlag && (myFlags.xferCommand != 9))
			{
				CloseProgressWindow();
			}

			if (myFlags.xferCommand == 1)
			{
				//  get data on the last sent overlay
				char lastfile[500]; 
				lastfile[0] = 0; 
				GetPrivateProfileString("Last Sent Overlay", "File", "", lastfile, sizeof(lastfile), "ikeysxfr.ini");
				char lastname[500]; 
				lastname[0] = 0;
				GetPrivateProfileString("Last Sent Overlay", "Name", "", lastname, sizeof(lastname), "ikeysxfr.ini");

				DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - Read - Last Sent Overlay File=[%s]", lastfile);
				DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - Read - Last Sent Overlay Name=[%s]", lastname);

				//  send a message if appropriate

				bool bIssueMessage = false;
				if (bIK32Connected)
				{
					DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - IKIsStandardOverlayInPlace");

					int response = IKIsStandardOverlayInPlace();
					if ((response == kResponseStandardOverlayIsInPlace) && ((stricmp(lastfile, fullname) != 0) || (stricmp(lastname, param[2]) != 0)))
					bIssueMessage = true;
				}
				else
				{
					DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - IKUSBIsStandardOverlayInPlace");

					int response = IKUSBIsStandardOverlayInPlace();
					if ((response == kResponsev1StandardOverlayIsInPlace) && ((stricmp(lastfile,fullname) != 0) || (stricmp(lastname, param[2]) != 0)))
					bIssueMessage = true;
				}

				if (bIssueMessage)
				{
					// April 2013 - JR - need to localize the message box so moved text to data.txt
					IKString dlgTitle = DATAS(TEXT("S_OVERLAY_IN_PLACE_TITLE"), TEXT("Note"));
					IKString dlgMsg = DATAS(TEXT("S_OVERLAY_IN_PLACE_TEXT"), TEXT("Your IntelliKeys USB appears to have a Standard Overlay in place.\nTo use the Custom Overlay you just sent, you will need to remove the Standard Overlay."));
					
					// May 2013 - because of localized strings, use the Unicode version of the Message box
					int msgLen = dlgMsg.GetLength();
					WCHAR dlgMsgWide[1024];
					int res = MultiByteToWideChar ( CP_UTF8, 0, dlgMsg, msgLen, dlgMsgWide, 1024);
					dlgMsgWide[res] = 0;

					msgLen = dlgTitle.GetLength();
					WCHAR dlgTitleWide[1024];
					res = MultiByteToWideChar ( CP_UTF8, 0, dlgTitle, msgLen, dlgTitleWide, 1024);
					dlgTitleWide[res] = 0;

					::MessageBoxW(NULL, dlgMsgWide, dlgTitleWide, MB_OK);
					
					//::MessageBox(NULL, dlgMsg, dlgTitle, MB_OK);

					//char title[255];
					//char text[255];
					//::LoadString(NULL, IDS_OINPLACETITLE, title, 255);
					//::LoadString(NULL, IDS_OINPLACETEXT, text, 255);
					//::MessageBox(NULL, text, title, MB_OK | MB_ICONEXCLAMATION);
				}

				//  record for next time
				WritePrivateProfileString("Last Sent Overlay", "File", fullname, "ikeysxfr.ini");
				WritePrivateProfileString("Last Sent Overlay", "Name", param[2], "ikeysxfr.ini");

				DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - Write - Last Sent Overlay File=[%s]", fullname);
				DebugLogToFile("IKeysXfr32.Main: - Xfer(1) - Write - Last Sent Overlay Name=[%s]", param[2]);
			}
		}
	}

	//  report on results
	if ((bIK16Connected == false) && (bIK32Connected || bIK32v1Connected))
	{
		if (bIK32Connected)
		{
			switch (status32)
			{
				case _FILE_BAD:	
					if (myFlags.userInitiated)
					{
						// April 2013 - JR - need to localize the message box so moved text to data.txt
						IKString dlgTitle = DATAS(TEXT("S_IKEYS_ERROR"), TEXT("IntelliKeys USB Error"));
						IKString dlgMsg = DATAS(TEXT("S_OVERLAYFILE_ERROR"), TEXT("File is in an unknown format, corrupted, or is not an overlay file."));

						// May 2013 - because of localized strings, use the Unicode version of the Message box
						int msgLen = dlgMsg.GetLength();
						WCHAR dlgMsgWide[1024];
						int res = MultiByteToWideChar ( CP_UTF8, 0, dlgMsg, msgLen, dlgMsgWide, 1024);
						dlgMsgWide[res] = 0;

						msgLen = dlgTitle.GetLength();
						WCHAR dlgTitleWide[1024];
						res = MultiByteToWideChar ( CP_UTF8, 0, dlgTitle, msgLen, dlgTitleWide, 1024);
						dlgTitleWide[res] = 0;

						::MessageBoxW(NULL, dlgMsgWide, dlgTitleWide, MB_OK);
						//::MessageBox(NULL, dlgTitle, dlgMsg, MB_OK);

						//char title3[255];
						//char text3[255];
						//::LoadString(NULL, IDS_IKEYS_ERROR, title3, 255);
						//::LoadString(NULL, IDS_OVERLAYFILE_ERR, text3, 255);
						//::MessageBox(NULL, text3, title3, MB_OK);
					}
					return _FILE_BAD;		
					break;

				case kResponseFileError:		return _FILE_BAD;		break;
				case kResponseNotEnoughtData:	return _IO_ERROR;		break;
				case kResponseNoServer:			return _IK_NOT_FOUND;	break;
				case kResponseError:			return _IO_ERROR;		break;
				case kResponseTimeout:			return _IO_ERROR;		break;
				case kResponseNoError:			return 0;				break;
				default:						return 0;				break;
			}
		}
		else  //  bIK32v1Connected
		{
			switch (status32)
			{
				case _FILE_BAD:	
					if (myFlags.userInitiated)
					{
						// April 2013 - JR - need to localize the message box so moved text to data.txt
						IKString dlgTitle = DATAS(TEXT("S_IKEYS_ERROR"), TEXT("IntelliKeys USB Error"));
						IKString dlgMsg = DATAS(TEXT("S_OVERLAYFILE_ERROR"), TEXT("File is in an unknown format, corrupted, or is not an overlay file."));
						
						// May 2013 - because of localized strings, use the Unicode version of the Message box
						int msgLen = dlgMsg.GetLength();
						WCHAR dlgMsgWide[1024];
						int res = MultiByteToWideChar ( CP_UTF8, 0, dlgMsg, msgLen, dlgMsgWide, 1024);
						dlgMsgWide[res] = 0;

						msgLen = dlgTitle.GetLength();
						WCHAR dlgTitleWide[1024];
						res = MultiByteToWideChar ( CP_UTF8, 0, dlgTitle, msgLen, dlgTitleWide, 1024);
						dlgTitleWide[res] = 0;

						::MessageBoxW(NULL, dlgMsgWide, dlgTitleWide, MB_OK);
						//::MessageBox(NULL, dlgTitle, dlgMsg, MB_OK);

						//char title3[255];
						//char text3[255];
						//::LoadString(NULL, IDS_IKEYS_ERROR, title3, 255);
						//::LoadString(NULL, IDS_OVERLAYFILE_ERR, text3, 255);
						//::MessageBox(NULL, text3, title3, MB_OK);
					}
					return _FILE_BAD;		
					break;

				case kResponsev1FileError:		return _FILE_BAD;		break;
				case kResponsev1NotEnoughtData:	return _IO_ERROR;		break;
				case kResponsev1NoServer:		return _IK_NOT_FOUND;	break;
				case kResponsev1Error:			return _IO_ERROR;		break;
				case kResponsev1Timeout:		return _IO_ERROR;		break;
				case kResponsev1NoError:		return 0;				break;
				default:						return 0;				break;
			}
		}
	}

	return 0;
}

//////// -----------

#define PROGRESS_WIDTH 450
#define PROGRESS_HEIGHT 50
#define PROGRESS_CAPTION_HEIGHT 20

#define BAR_RGB RGB(191,0,191)

RECT meterRect;
bool childFlag = TRUE;
HWND progressW = NULL;
HBRUSH meterbr = NULL;//, backbr = NULL;

HWND gOldSysModalWind = NULL;

WORD totalCount, soFarCount;

long FAR PASCAL MyWindProc (HWND hwnd, UINT message, UINT wParam, LONG lParam)
	 {
	 HDC         hdc ;
	 PAINTSTRUCT ps ;
	 //RECT        rect ;
	 HPEN        hpen, hpenOld;

	 switch (message)
		  {

		  case WM_PAINT:
			   hdc = BeginPaint (hwnd, &ps) ;

			   SetRect ( &meterRect, 0, 0, PROGRESS_WIDTH, PROGRESS_HEIGHT - 20 );
			   InflateRect ( &meterRect, -8, -8 );
			   
			   if ( childFlag )
			   {
					RECT r;
					HBRUSH hFillBrush; // , hOldBrush;
					HFONT hFont;
					COLORREF fillColor, textColor;
					int oldBackMode;
					char title[128];
					
					GetWindowText ( hwnd, title, sizeof ( title ) );
						
					GetClientRect ( hwnd, &r );
					r.bottom = r.top + PROGRESS_CAPTION_HEIGHT;
   
					hFont = (HFONT)GetStockObject ( SYSTEM_FONT );
					fillColor = (COLORREF)GetSysColor ( COLOR_ACTIVECAPTION );
					textColor = (COLORREF)GetSysColor ( COLOR_CAPTIONTEXT );
					
					hFillBrush = CreateSolidBrush ( fillColor );
					FillRect ( hdc, &r, hFillBrush );
					DeleteObject ( hFillBrush );
					
					SelectObject ( hdc, hFont );
					
					SetTextColor ( hdc, textColor );
					oldBackMode = GetBkMode ( hdc );
					SetBkMode ( hdc, TRANSPARENT );
					
					r.left += 8;
					r.top += 1;
					DrawText ( hdc, title, strlen ( title ), &r, DT_LEFT );
					SetBkMode ( hdc, oldBackMode );
					
					meterRect.top += PROGRESS_CAPTION_HEIGHT;
					meterRect.bottom += PROGRESS_CAPTION_HEIGHT;
			   }

				hpen = (HPEN)CreatePen(PS_SOLID, 1, RGB( 128, 128, 128));
				hpenOld = (HPEN)SelectObject(hdc, hpen);

			   InflateRect ( &meterRect, -1, -1 );
			   //MoveTo( hdc, meterRect.left, meterRect.bottom );
			   MoveToEx( hdc, meterRect.left, meterRect.bottom, NULL );
			   LineTo( hdc, meterRect.left, meterRect.top );
			   LineTo( hdc, meterRect.right, meterRect.top );
			   InflateRect ( &meterRect, 1, 1 );

				SelectObject(hdc, hpenOld);
				DeleteObject(hpen);

			   SelectObject ( hdc, GetStockObject ( WHITE_PEN ) );
//               MoveTo( hdc, meterRect.right, meterRect.top );
			   MoveToEx( hdc, meterRect.right, meterRect.top, NULL );

			   LineTo( hdc, meterRect.right, meterRect.bottom );
			   LineTo( hdc, meterRect.left, meterRect.bottom );

			   FrameRect( hdc, &meterRect, (HBRUSH)GetStockObject ( BLACK_BRUSH ) );

			   InflateRect ( &meterRect, -3, -3 );

			   EndPaint (hwnd, &ps) ;
			   return 0 ;
			   
		  default:
			 return DefWindowProc (hwnd, message, wParam, lParam) ;
		  }
}

char szMyClass[] = "IkXprogressWClass";

void CreateProgressWindow ( void )
{
	char progressTitle[128];
	WNDCLASS wc;
	DWORD dwVersion;

/* Register the window class. */

/* Create the window. */

	wc.style         = CS_SAVEBITS;
	wc.lpfnWndProc   = MyWindProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = (HINSTANCE)thisInstance;
	wc.hIcon         = LoadIcon(NULL,IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_WAIT /* IDC_ARROW */ );
		
	wc.hbrBackground = (HBRUSH)GetStockObject( LTGRAY_BRUSH );
	dwVersion = GetVersion();
	if (( LOBYTE(LOWORD(dwVersion)) > 3 ) || (HIBYTE(LOWORD(dwVersion))==95))
		wc.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
		
	wc.lpszMenuName  = NULL;
		
	wc.lpszClassName = szMyClass;
		
	if (!RegisterClass(&wc))
		{
		return;
		}

	LoadString((HINSTANCE)thisInstance, 1001, progressTitle, 127); 

	if(childFlag)
	{
		progressW = CreateWindowEx(WS_EX_TOPMOST, szMyClass, progressTitle,
			WS_BORDER|WS_POPUP|WS_DISABLED, 0, 0,
			PROGRESS_WIDTH, PROGRESS_HEIGHT+PROGRESS_CAPTION_HEIGHT,
			NULL, NULL,
		(HINSTANCE)thisInstance, NULL );  
		
		//SetParent(progressW,GetActiveWindow());
		//ShowWindow(progressW,SW_SHOW); 
		//SetWindowPos(progressW,HWND_TOPMOST,0,0,0,0,SWP_SHOWWINDOW);
	}
	else
	{
		progressW = CreateWindow(szMyClass, progressTitle,
		WS_OVERLAPPED, 0, 0,
			PROGRESS_WIDTH, PROGRESS_HEIGHT,
			NULL, NULL,
		(HINSTANCE)thisInstance, NULL );
	}


	{
		HWND hDesktop;
		RECT r;

		hDesktop = GetDesktopWindow();
		GetWindowRect(hDesktop, &r );

		MoveWindow(progressW, (r.right-PROGRESS_WIDTH)>>1, (r.bottom-PROGRESS_HEIGHT)>>2,
				PROGRESS_WIDTH, PROGRESS_HEIGHT, FALSE );
	}

	ShowWindow ( progressW, childFlag ? SW_SHOWNOACTIVATE : SW_SHOW );
	UpdateWindow ( progressW );
	meterbr = CreateSolidBrush( BAR_RGB );
	
	if ( ! childFlag )
		gOldSysModalWind = SetSysModalWindow(progressW);

	totalCount = 100;
	
//	SetFocus ( progressW );
}



void CloseProgressWindow ( void )
{
		 if ( progressW )
				{
//				SetFocus ( NULL );
				
				if ( ! childFlag )
					SetSysModalWindow(gOldSysModalWind);

				DestroyWindow ( progressW );
				UnregisterClass(szMyClass, (HINSTANCE)thisInstance );
				DeleteObject ( meterbr );
				progressW = NULL;
				}
}

void SetWindowTitle2 ( char *string )
{
	RECT r;

	SendMessage( progressW, WM_SETTEXT, 0, (LPARAM) ((LPSTR) string ) );
				  
	if ( childFlag )
	{
		GetClientRect ( progressW, &r );
	}
					  
	soFarCount = 0;
	InvalidateRect(progressW, &r, TRUE );
	UpdateWindow ( progressW );
}

void SetWindowTitle( char *screenName )
{
	char titleString[256];
	char *closeQuote = "\x22";
	RECT r;

	r = meterRect;

	//LoadString((HINSTANCE)thisInstance, (xferCommand & 1) ? _SENDING : _RECEIVING, titleString, 255 );
	//LoadString((HINSTANCE)thisInstance, _SENDING, titleString, 255 );

	char *pBar = strchr ( screenName, '|' );
	if (pBar)
	{
		lstrcpy ( titleString, "" );

		char name[256];
		char sender[256];
		strcpy ( name, screenName );
		strcpy ( sender, &(pBar[1]) );
		pBar = strchr ( name, '|' );
		if (pBar)
			*pBar = '\0';

		//lstrcat ( titleString, "Sending " );
		lstrcat ( titleString, DATAS(TEXT("Sending"), TEXT("Sending")) );
		lstrcat ( titleString, " " );
		lstrcat ( titleString, closeQuote );
		lstrcat ( titleString, name );
		lstrcat ( titleString, closeQuote );
		//lstrcat ( titleString, " for " );
		lstrcat ( titleString, " " );
		lstrcat ( titleString, DATAS(TEXT("For"), TEXT("for")) );
		lstrcat ( titleString, " " );
		lstrcat ( titleString, closeQuote );
		lstrcat ( titleString, sender );
		lstrcat ( titleString, closeQuote );
	}

	else
	{
		lstrcpy ( titleString, "" );

		//lstrcat ( titleString, "Sending " );
		lstrcat ( titleString, DATAS(TEXT("Sending"), TEXT("Sending")) );
		lstrcat ( titleString, " " );
		lstrcat ( titleString, closeQuote );
		lstrcat ( titleString, screenName );
		lstrcat ( titleString, closeQuote );
	}

	SendMessage( progressW, WM_SETTEXT, 0, (LPARAM) ((LPSTR) titleString ) );
				  
	if ( childFlag )
	{
		GetClientRect ( progressW, &r );
	}
					  
	soFarCount = 0;
	InvalidateRect(progressW, &r, TRUE );
	UpdateWindow ( progressW );
}

void ShowProgress( int soFarCount )
{
	HDC theDC;
	RECT frameRect, fillRect2;
	float ratio;
	int pixelWidth;

	if ( ! soFarCount )
			UpdateWindow ( progressW );

		if ( ( !totalCount ) || ( !progressW ) ) // || ( ! progressFlag ) )
				return;

		if ( ( soFarCount != totalCount ) && ( soFarCount & 3 ) )
			return;
							
		theDC = GetDC(progressW);
 
		frameRect = meterRect;

		ratio = ( ( float ) soFarCount ) / ( ( float ) totalCount );
		if ( ratio > 1.0 )  
			{
#if USE_LOG_FILE
			LOG(  "Progress meter exceeded, soFarCount = %d, totalCount = %d\n", soFarCount, totalCount );
#endif
			ratio = 1.0;
			}
			
		pixelWidth = int(( ( float ) ( frameRect.right - frameRect.left ) ) * ratio);
		fillRect2 = frameRect;
		frameRect.right = frameRect.left + pixelWidth;
		fillRect2.left = frameRect.right + 1;

		FillRect( theDC, &frameRect, meterbr );
		
//        FillRect( theDC, &fillRect2, GetStockObject ( LTGRAY_BRUSH ) );
		ReleaseDC ( progressW, theDC );

//        InvalidateRect(progressW, &fillRect2, TRUE );
}

