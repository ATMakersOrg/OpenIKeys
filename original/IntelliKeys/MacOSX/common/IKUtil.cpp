// IKUtil.cpp: implementation of the IKUtil class.
//
//////////////////////////////////////////////////////////////////////


#include "IKCommon.h"
#include "IKUtil.h"
#include "IKFile.h"
#include "IKMessage.h"

#ifdef PLAT_MACINTOSH

#ifdef BUILD_CW
  #ifdef BUILD_CARBON
    #include <Carbon.h>
  #else
    //#include <DriverServices.h>
  #endif
#else
  #include <syslog.h>
#endif
  #include <unistd.h>

//#include "IterateDirectory.h"

#endif

#ifdef PLAT_MACINTOSH_X

//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <assert.h>
//#include <errno.h>
#include <sys/sysctl.h>
#endif



#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PLAT_WINDOWS
  #include "ExecImageVersion.h"
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


unsigned int IKUtil::GetCurrentTimeMS()
{
	//  protect against recursion by just returning the last value.
	//static unsigned int last = 0;
	//static bool bRecurse = false;
	//if(bRecurse)
		//return last;
	//bRecurse = true;

	unsigned int newtime;

#ifdef PLAT_MACINTOSH

#ifdef PLAT_MACINTOSH_X

	AbsoluteTime nowAbsTime;
	Duration nowDurationTime;

	nowAbsTime = UpTime();
	nowDurationTime = AbsoluteToDuration (nowAbsTime);
	if (nowDurationTime < 0)
		nowDurationTime = (nowDurationTime / -1000);

	newtime = (unsigned int ) nowDurationTime;
	
#else

	IKASSERT(false);
	newtime = 0;
	
#endif

#endif

#ifdef PLAT_WINDOWS

	newtime = GetTickCount();

#endif

	//last = newtime;
	//bRecurse = false;

	return newtime;
}

void IKUtil::Sleep(unsigned int waitMS)
{
#ifdef PLAT_MACINTOSH

  #ifdef PLAT_MACINTOSH_X
	usleep(waitMS * 1000);
  #else
	unsigned int origMS = GetCurrentTimeMS();
	RgnHandle rgn = NewRgn();
	while(true)
	{
		//YieldToAnyThread();
		EventRecord er;
		::WaitNextEvent(mDownMask, &er, 1, rgn);
		unsigned int curMS = GetCurrentTimeMS();
		if ( (curMS - origMS) > waitMS)
			break;
	}
	::DisposeRgn( rgn );
  #endif

#endif

#ifdef PLAT_WINDOWS
	::Sleep ( waitMS );
#endif

}


static TCHAR strWinPathDelim = CHAR('\\');
static TCHAR strMacPathDelim = CHAR(':');

TCHAR IKUtil::GetPathDelimiter()
{
#ifdef PLAT_WINDOWS
	return GetWinPathDelimiter();
#endif
#ifdef PLAT_MACINTOSH
	return GetMacPathDelimiter();
#endif
}

TCHAR IKUtil::GetMacPathDelimiter()
{
	return strMacPathDelim;
}

TCHAR IKUtil::GetWinPathDelimiter()
{
	return strWinPathDelim;
}




static void SubPathDelimiter(IKString& string)
{
	for (int i=0;i<string.GetLength();i++)
	{
		if (string.GetAt(i)==CHAR('/'))
			string.SetAt(i,IKUtil::GetPathDelimiter());
	}
}

#ifdef PLAT_MACINTOSH

void MyPtoCstr (Str255 string);
void MyPtoCstr (Str255 string)
{
	int l = string[0];
	for (int i=0;i<l;i++)
		string[i] = string[i+1];
	string[l] = 0;
}

void GetFullPath (long DirID, short vRefNum, TCHAR *path);
void GetFullPath (long DirID, short vRefNum, TCHAR *path)
{
	CInfoPBRec myPB;// parameter block for PBGetCatInfo
	Str255 dirNameBuf; //  a directory name
	TCHAR fullPath[255];//  full pathname being constructed
	OSErr myErr;

	myPB.dirInfo.ioNamePtr = dirNameBuf;
	myPB.dirInfo.ioVRefNum = vRefNum;    //indicate target volume
	myPB.dirInfo.ioDrParID = DirID;      //initialize parent directory ID
	myPB.dirInfo.ioFDirIndex = -1;       //get info about a directory
	
	strcpy(fullPath,TEXT(""));
	strcpy(path,TEXT(""));

	do
	{
		myPB.dirInfo.ioDrDirID = myPB.dirInfo.ioDrParID;
		myErr = PBGetCatInfo(&myPB, FALSE);
		if ( myErr )
			return;

		MyPtoCstr ( dirNameBuf );
		
		TCHAR temp[255];
		strcpy(temp,fullPath);
		strcpy(fullPath,(TCHAR *)dirNameBuf);
		strcat(fullPath,TEXT(":"));
		strcat(fullPath,temp);

	} while ( myPB.dirInfo.ioDrDirID != fsRtDirID );

	strcpy(path,fullPath);
	return;

}

#endif

IKString IKUtil::GetRootFolder()
{
    //  if we've already been called once, just return the
    //  previous result.
    IKString theRootFolder;

    //    static IKString result = TEXT("");
//    if (!result.IsEmpty())
//		return result;

#ifdef PLAT_WINDOWS

	//  look first for a root preferences file
	//  somewhere on your system.

    TCHAR wpath[255];
	::GetWindowsDirectory(wpath,255);
	if (wpath[IKString::strlen(wpath)-1]!=TEXT('\\'))
	{
		IKString::strcat(wpath,TEXT("\\"));
	}
	IKString::strcat(wpath,TEXT("intellitools\\IKUSBRoot.txt"));

    //  load the file
    IKPrefs rootfile;
    rootfile.Read(wpath);
    
    //  check the value
    result = rootfile.GetValueString ( TEXT("Root_Location") );
    if (!result.IsEmpty())
		return result;

	//  look NEXT in an INI file
	TCHAR root[255];
	GetPrivateProfileString (TEXT("IntelliKeys USB"), TEXT("root"), TEXT(""), root, sizeof(root), TEXT("itools_x.ini") );
	if (IKString::strcmp(root,TEXT(""))!=0)
	{
		result = root;
		if (result.GetAt(result.GetLength()-1)!=TEXT('\\'))
			result += TEXT("\\");
		return result;
	}

#endif

#ifdef PLAT_MACINTOSH
    //  look first in a data file
    
    //  get the app support folder
    short vRefNum;
    long dirID;
    FindFolder ( kOnSystemDisk, kApplicationSupportFolderType, kDontCreateFolder, &vRefNum, &dirID );
    
    //  get the full path to the file
    TCHAR path[255];
    GetFullPath(dirID, vRefNum, path);
    strcat(path,TEXT("IntelliTools:IKUSBRoot.txt"));
    
    //  load the file
    IKPrefs rootfile;
    rootfile.Read(path);
    
    //  check the value
    theRootFolder = rootfile.GetValueString ( TEXT("Root_Location") );
    if (!theRootFolder.IsEmpty())
		return theRootFolder;

#endif
	
    //  get this app's folder
    IKString thisAppFolder = GetThisAppPath();
    int i = thisAppFolder.ReverseFind(GetPathDelimiter());
    if (i != -1)
	thisAppFolder.SetAt(i+1,TEXT('\0'));
	
    //  get the value stored in root.txt found in
    //  the same folder as the app
    IKString rootTxt = thisAppFolder;
    rootTxt += TEXT("root.txt");
    //IKPrefs prefs;
    //prefs.Read(rootTxt);
	rootfile.Read(rootTxt);
    IKString fromFile = rootfile.GetValueString(TEXT("Root_Location"),TEXT(""));
    
    //  return the value from the file if there is one
    //  otherwise us the folder the app was found in.

    if (!fromFile.IsEmpty())
		theRootFolder = fromFile;
    else
		theRootFolder = thisAppFolder;

    return theRootFolder;
}

IKString IKUtil::GetChannelsFolder()
{
	IKString s(GetRootFolder ());
	s += DATAS(TEXT("Channels_Folder"),TEXT("Private/Channels/"));
	SubPathDelimiter(s);
	return s;
}

IKString IKUtil::GetGuestFolder()
{
	IKString s = GetRootFolder ();
	s += DATAS(TEXT("Guest_Folder"),TEXT("Private/Channels/Guest/"));
	SubPathDelimiter(s);
	return s;
}

IKString IKUtil::GetEngineFolder()
{
	IKString s = GetRootFolder ();
	s += DATAS(TEXT("Engine_Folder"),TEXT("Private/Channels/Engine/"));
	SubPathDelimiter(s);
	return s;
}

IKString IKUtil::GetHelpFile()
{
	IKString s = GetRootFolder ();
	s += DATAS(TEXT("Help_File"),TEXT("Private/ikusbhelp.html"));
	SubPathDelimiter(s);
	return s;
}

IKString IKUtil::GetUsersFolder()
{
	IKString s = GetRootFolder ();
	s += DATAS(TEXT("Users_Folder"),TEXT("Documents/"));
	SubPathDelimiter(s);
	return s;
}

IKString IKUtil::GetPrivateFolder()
{
	IKString s = GetRootFolder ();
	s += DATAS(TEXT("Private_Folder"),TEXT("Private/"));
	SubPathDelimiter(s);
	return s;
}

IKString IKUtil::GetOverlaysFolder()
{
	IKString s = GetRootFolder ();
	s += DATAS(TEXT("Overlays_Folder"),TEXT("Private/Standard Overlays"));
	SubPathDelimiter(s);
	return s;
}

IKString IKUtil::GetAppOverlaysFolder()
{
	IKString s = GetRootFolder ();
	s += DATAS(TEXT("App_Overlays_Folder"),TEXT("Private/Application Overlays/"));
	SubPathDelimiter(s);
	return s;
}

IKString IKUtil::GetSwitchSettingsFolder()
{
	IKString s = GetRootFolder ();
	s += DATAS(TEXT("Switch_Settings_Folder"),TEXT("Private/Switch Settings"));
	SubPathDelimiter(s);
	return s;
}

IKString IKUtil::GetDateAndTime()
{
	IKString date = GetDate();
	IKString time = GetTime();

	IKString s;
	s += date;
	s += TEXT(" ");
	s += time;
	return s;
}

IKString IKUtil::GetDate()
{
	//  get current date and time
	time_t t;
	time(&t);
	tm *pNow = localtime(&t);

	//  format a string like YEAR-MONTH-DAY
	IKString s;
	s += IKUtil::IntToString(1900+pNow->tm_year);
	s += TEXT("-");
	s += IKUtil::IntToString(1+pNow->tm_mon);
	s += TEXT("-");
	s += IKUtil::IntToString(pNow->tm_mday);

	return s;
}

IKString IKUtil::GetTime()
{
	//  get current date and time
	time_t t;
	time(&t);
	tm *pNow = localtime(&t);

	//  format a tring like HOUR:MIN:SEC
	IKString s;

	if (pNow->tm_hour < 10)
		s += TEXT("0");
	s += IKUtil::IntToString(pNow->tm_hour);
	s += TEXT(":");

	if (pNow->tm_min < 10)
		s += TEXT("0");
	s += IKUtil::IntToString(pNow->tm_min);
	s += TEXT(":");

	if (pNow->tm_sec < 10)
		s += TEXT("0");
	s += IKUtil::IntToString(pNow->tm_sec);

	return s;
}

IKString IKUtil::IntToString(int num)
{
	TCHAR buffer[33];
	MySprintf ( buffer, TEXT("%d"), num );
	IKString s = buffer;
	return s;
}

IKString IKUtil::IntToHexString(int num)
{
	TCHAR buffer[33];
	MySprintf ( buffer, TEXT("%x"), num );
	IKString s = buffer;
	return s;
}

IKString IKUtil::IntToString(unsigned int num)
{
	TCHAR buffer[33];
	MySprintf ( buffer, TEXT("%d"), num );
	IKString s = buffer;
	return s;
}

int IKUtil::StringToInt (IKString string)
{
	char s[255];
	IKString::ConvertTToC (s, string);

	int i;
	sscanf ( s, "%d", &i );
	return i;
}

float IKUtil::StringToFloat (IKString string)
{
	char s[255];
	IKString::ConvertTToC (s, string);

	float f;
	sscanf ( s, "%f", &f );
	return f;
}

int IKUtil::StringToInt (TCHAR * string)
{
	char s[255];
	IKString::ConvertTToC (s, string);

	int i;
	sscanf ( s, "%d", &i );
	return i;
}

static IKPrefs theData;

void IKUtil::Initialize()
{	
	//  only once
	static bool bInited = false;
	if (bInited)
		return;
	bInited = true;
	
	//  load common static data
	IKString dataFile = GetPrivateFolder();
	dataFile += TEXT("data.txt");
	theData.Read(dataFile);

	//  load platform-specific data
	dataFile = GetPrivateFolder();
	dataFile += TEXT("data");
#ifdef PLAT_WINDOWS
	dataFile += TEXT("win");
#endif
#ifdef PLAT_MACINTOSH
	dataFile += TEXT("mac");
#endif
	dataFile += TEXT(".txt");
	theData.Read(dataFile);

}

IKPrefs * IKUtil::GetData()
{
	return &theData;
}

IKString IKUtil::MakeStudentPath ( IKString group, IKString student )
{
	IKString path = GetUsersFolder();
	path += group;
	if (student!=IKString(TEXT("")))
	{
		path += GetPathDelimiter();
		path += student;
	}
	path += GetPathDelimiter() ;
	SubPathDelimiter(path);

	return path;
}

void IKUtil::MakeAllFolders ( IKString path )
{
	//  given a path, make sure all the folders
	//  exist

	for (int i=0;i<path.GetLength();i++)
	{
		if (path.GetAt(i)==GetPathDelimiter())
		{
			IKString part = path.Left(i+1);
			IKFile::NewFolder(part);
		}
	}
}

#ifdef PLAT_WINDOWS


#include "tlhelp32.h"
#include <vdmdbg.h>

typedef struct
{
	HWND hwnd;
	TCHAR name[1024];
	bool bFound;
} myEnumData;

BOOL WINAPI EnumThreadWindowsProc ( HWND hwnd, LPARAM lParam )
{
	HWND hTargetWnd = ((myEnumData *) lParam)->hwnd;
	if (hwnd==hTargetWnd)
	{
		((myEnumData *) lParam)->bFound = true;
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI Enum16A( DWORD dwThreadId, WORD hMod16, WORD hTask16,
					PSZ pszModName, PSZ pszFileName, LPARAM lpUserDefined )
{
	HWND hwnd = ((myEnumData *) lpUserDefined)->hwnd;

	((myEnumData *) lpUserDefined)->bFound = false;
	EnumThreadWindows(dwThreadId, EnumThreadWindowsProc, (LPARAM)lpUserDefined);

	if (((myEnumData *) lpUserDefined)->bFound)
	{
		IKString::strcpy((((myEnumData *) lpUserDefined)->name),pszFileName);
		return TRUE;
	}

	return FALSE;
} 

static void ExePathFromWindowNT ( HWND hwnd, TCHAR *wname )
{
	//  declarations of PSAPI functions
	typedef DWORD (FAR PASCAL *GetModuleFileNameExProc)(HANDLE,HMODULE,LPCSTR,DWORD);
	typedef BOOL (WINAPI * PFNENUMPROCESSMODULES)(    HANDLE hProcess,
					HMODULE *lphModule,    DWORD cb,    LPDWORD lpcbNeeded    );

	//  pointers to PSAPI functions
	static GetModuleFileNameExProc pGetModuleFileNameEx = NULL;
	static PFNENUMPROCESSMODULES pfnEnumProcessModules = NULL;

	//  get pointers to PSAPI functions
	static bool bLoaded = false;
	if (!bLoaded)
	{
		HINSTANCE hInstPSAPI = LoadLibrary("psapi.dll");
		if (hInstPSAPI)
		{
			pGetModuleFileNameEx = 
				(GetModuleFileNameExProc) ::GetProcAddress(hInstPSAPI,"GetModuleFileNameExA");
			pfnEnumProcessModules = 
				(PFNENUMPROCESSMODULES) ::GetProcAddress( hInstPSAPI, "EnumProcessModules" );
		}
		bLoaded = true;
	}

	//  if no pointers, use window text.
	if (pGetModuleFileNameEx==NULL || pfnEnumProcessModules==NULL)
    {
		::GetWindowText ( hwnd, wname, 1024 );
        return;
    }

	//  get process ID from window handle.
	DWORD			threadID;
	DWORD			dwPID;
	threadID = ::GetWindowThreadProcessId( hwnd, &dwPID );

	//  get process handle
	HANDLE hProc = ::OpenProcess ( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, dwPID );

	//  enumerate the modules in this process to get the module handle.
	HMODULE hModuleArray[1024];
	DWORD cbNeeded;
	BOOL result = pfnEnumProcessModules(hProc, hModuleArray,
                                    sizeof(hModuleArray), &cbNeeded );

	//  get module name
	TCHAR moduleName[1024];
	DWORD c = (*pGetModuleFileNameEx) ( 
		hProc,
		hModuleArray[0],
		moduleName,
		1024 );

	//  convert to UNICODE and back to UTF-8
	int len = IKString::strlen(moduleName);
	unsigned short wide[1024];
	result = MultiByteToWideChar ( CP_ACP, 0, moduleName, len, wide, 1024);
	wide[result] = 0;

	int result2 = WideCharToMultiByte ( CP_UTF8, 0, wide, result, moduleName, 1024, 0, 0);
	moduleName[result2] = 0;

	//  close the handles
	CloseHandle(hProc);

	//  if it's not NTVDM, we're done
	IKString s(moduleName);
	IKString s2 = s.Right(9);
	if (s2.CompareNoCase(TEXT("NTVDM.EXE"))!=0)
	{
		IKString::strcpy(wname,moduleName);
		return;
	}

	//  it's a 16-bit program, so enumerate all of those
	//  to find the one we want.

	static INT (WINAPI *lpfVDMEnumTaskWOWEx)( DWORD,
			 TASKENUMPROCEX  fp, LPARAM ) = NULL;
	static bool bLoaded2 = false;
	if (!bLoaded2)
	{
		HINSTANCE hInstLib2 = LoadLibraryA( "VDMDBG.DLL" ) ;
		if (hInstLib2)
		{
			lpfVDMEnumTaskWOWEx = (INT(WINAPI *)( DWORD, TASKENUMPROCEX, LPARAM))
			GetProcAddress( hInstLib2, "VDMEnumTaskWOWEx" );
		}
		bLoaded2 = true;
	}

	if (lpfVDMEnumTaskWOWEx)
	{
		myEnumData data= {hwnd,"",false};
		lpfVDMEnumTaskWOWEx ( dwPID,
			(TASKENUMPROCEX) Enum16A,
			(LPARAM) &data);
		if (data.bFound)
		{
			IKString::strcpy(wname,data.name);
			return;
		}
	}


	//  if we're here, just use the window text
	::GetWindowText ( hwnd, wname, 1024 );

}

static void ExePathFromWindow9x ( HWND hwnd, TCHAR *wname )
{
	//  declarations for KERNEL32 functions
	typedef HANDLE (FAR PASCAL *CreateSnapshotProc)(DWORD,DWORD);
	typedef BOOL (FAR PASCAL *Module32Proc)(HANDLE, LPMODULEENTRY32);
	typedef BOOL (FAR PASCAL *Process32Proc)(HANDLE, LPPROCESSENTRY32);

	//  get pointers to KERNEL32 functions
	static bool bOnce = false;
	static CreateSnapshotProc	pCreateToolhelp32Snapshot = NULL;
	static Module32Proc			pModule32First = NULL, pModule32Next = NULL;
	static Process32Proc		pProcess32First = NULL, pProcess32Next = NULL;
	if(!bOnce)
	{
		HINSTANCE m_hToolHelp = :: GetModuleHandle ( TEXT("KERNEL32") );
		if ( m_hToolHelp )
		{
			pCreateToolhelp32Snapshot	= (CreateSnapshotProc)	::GetProcAddress( m_hToolHelp, "CreateToolhelp32Snapshot" );
			pModule32First				= (Module32Proc)		::GetProcAddress( m_hToolHelp, "Module32First" );
			pModule32Next				= (Module32Proc)		::GetProcAddress( m_hToolHelp, "Module32Next" );
			pProcess32First				= (Process32Proc)		::GetProcAddress( m_hToolHelp, "Process32First" );
			pProcess32Next				= (Process32Proc)		::GetProcAddress( m_hToolHelp, "Process32Next" );
		}
		bOnce = true;
	}

	//  if no pointers, just use the window text.
    if ( pCreateToolhelp32Snapshot == NULL )
    {
		::GetWindowText ( hwnd, wname, 1024 );
        return;
    }

	//  get process ID from window handle.
	DWORD			threadID;
	DWORD			dwPID;
	threadID = ::GetWindowThreadProcessId( hwnd, &dwPID );

    // Take a snapshot of all modules in the specified process.
	HANDLE			hModuleSnap = NULL;
	hModuleSnap = (*pCreateToolhelp32Snapshot)(TH32CS_SNAPMODULE, dwPID);

	//  if no snapshot, use the window text.
    if (hModuleSnap == (HANDLE)-1)
    {
		::GetWindowText ( hwnd, wname, 1024 );
        return;
    }

    // Walk the module list of the process, and find the module of
    // interest. Then copy the information to the buffer pointed
    // to by lpMe32 so that it can be returned to the caller.
	MODULEENTRY32	me32        = {0};
    me32.dwSize = sizeof(MODULEENTRY32);
	bool bFound = false;
    if ((*pModule32First)(hModuleSnap, &me32)) 
	{
        do {
            if (me32.th32ProcessID == dwPID) 
			{
				IKString s = me32.szExePath;
				if (s.Right(4).CompareNoCase(".exe")==0)
				{
					IKString::strcpy(wname,me32.szExePath); // name = me32.szExePath;
					bFound = true;
					break;
				}

            }
        }
        while ( (*pModule32Next)(hModuleSnap, &me32));
    }

    // Do not forget to clean up the snapshot object.
    CloseHandle (hModuleSnap);

	//  if not found, use the window text.
	if (!bFound)
		::GetWindowText ( hwnd, wname, 1024 );

}

static void ExePathFromID ( DWORD dwPID, TCHAR *wname )
{
	//  declarations for KERNEL32 functions
	typedef HANDLE (FAR PASCAL *CreateSnapshotProc)(DWORD,DWORD);
	typedef BOOL (FAR PASCAL *Module32Proc)(HANDLE, LPMODULEENTRY32);
	typedef BOOL (FAR PASCAL *Process32Proc)(HANDLE, LPPROCESSENTRY32);

	//  get pointers to KERNEL32 functions
	static bool bOnce = false;
	static CreateSnapshotProc	pCreateToolhelp32Snapshot = NULL;
	static Module32Proc			pModule32First = NULL, pModule32Next = NULL;
	static Process32Proc		pProcess32First = NULL, pProcess32Next = NULL;
	if(!bOnce)
	{
		HINSTANCE m_hToolHelp = :: GetModuleHandle ( TEXT("KERNEL32") );
		if ( m_hToolHelp )
		{
			pCreateToolhelp32Snapshot	= (CreateSnapshotProc)	::GetProcAddress( m_hToolHelp, "CreateToolhelp32Snapshot" );
			pModule32First				= (Module32Proc)		::GetProcAddress( m_hToolHelp, "Module32First" );
			pModule32Next				= (Module32Proc)		::GetProcAddress( m_hToolHelp, "Module32Next" );
			pProcess32First				= (Process32Proc)		::GetProcAddress( m_hToolHelp, "Process32First" );
			pProcess32Next				= (Process32Proc)		::GetProcAddress( m_hToolHelp, "Process32Next" );
		}
		bOnce = true;
	}

	//  if no pointers,
    if ( pCreateToolhelp32Snapshot == NULL )
    {
		//::GetWindowText ( hwnd, wname, 1024 );
        return;
    }

    // Take a snapshot of all modules in the specified process.
	HANDLE			hModuleSnap = NULL;
	hModuleSnap = (*pCreateToolhelp32Snapshot)(TH32CS_SNAPMODULE, dwPID);

	//  if no snapshot
    if (hModuleSnap == (HANDLE)-1)
	{
		//::GetWindowText ( hwnd, wname, 1024 );
        return;
    }

    // Walk the module list of the process, and find the module of
    // interest. Then copy the information to the buffer pointed
    // to by lpMe32 so that it can be returned to the caller.
	MODULEENTRY32	me32        = {0};
    me32.dwSize = sizeof(MODULEENTRY32);
	bool bFound = false;
    if ((*pModule32First)(hModuleSnap, &me32)) 
	{
        do 
		{
            if (me32.th32ProcessID == dwPID) 
			{
				IKString s = me32.szExePath;
				if ( s.Right(4).CompareNoCase(TEXT(".exe"))==0 )
				{
					IKString::strcpy(wname,me32.szExePath); // name = me32.szExePath;
					bFound = true;
					break;
				}
            }
        }
        while ( (*pModule32Next)(hModuleSnap, &me32));
    }

    // Do not forget to clean up the snapshot object.
    CloseHandle (hModuleSnap);

	//  if not found,
	if (!bFound)
	{
		//::GetWindowText ( hwnd, wname, 1024 );
	}

}


static void GetWindowFileName(HWND hwnd, TCHAR *wname)
{
	//  no name to start
	wname[0] = 0;

	//  good handle?
	IKASSERT(hwnd);
	if(!hwnd)
		return;

	//  get process ID from the window handle
	//DWORD idProc;
	//::GetWindowThreadProcessId ( hwnd, &idProc );

	//  what OS are we?
	OSVERSIONINFO vInfo;
	vInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);	
	GetVersionEx(&vInfo);

	//  use two different procedures depending on the OS.
	if (vInfo.dwPlatformId==VER_PLATFORM_WIN32_NT)
		ExePathFromWindowNT ( hwnd, wname );
	else
		ExePathFromWindow9x ( hwnd, wname );
}

#endif  //  PLAT_WINDOWS

#undef ANY_MACOS
#ifdef PLAT_MACINTOSH
  #define ANY_MACOS 1
#endif
#ifdef PLAT_MACINTOSH_X
  #define ANY_MACOS 1
#endif

#ifdef ANY_MACOS

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
	char temp[255];

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
		
		strcpy(temp,fullPath);
		strcpy(fullPath,(char *)dirNameBuf);
		strcat(fullPath,":");
		strcat(fullPath,temp);

	} while ( myPB.dirInfo.ioDrDirID != fsRtDirID );

	strcpy(path,fullPath);
	return;

}

#endif
	
IKString IKUtil::GetCurrentApplicationPath()
{
#ifdef PLAT_WINDOWS

	HWND hwnd = ::GetForegroundWindow();
	if (hwnd != NULL)
	{
		TCHAR strFileName[1024];
		GetWindowFileName( hwnd, strFileName);

		IKString s = strFileName;
		IKUtil::GetTrueFileName(s);
		return s;
	}

	return TEXT("");
#endif

#ifdef PLAT_MACINTOSH
	
#ifdef BUILD_DAEMON
	
	//  if we're the daemon, we've got to ask USBMenu to do this.
	return IKString(TEXT(""));
	
#else

	//  get the front process
	ProcessSerialNumber psnFrontProcess;
	OSErr e = GetFrontProcess ( &psnFrontProcess );
	if (e != noErr)		
		return IKString(TEXT(""));
	
	//  get info about that process
	ProcessInfoRec pInfo;
	FSSpec spec;
	pInfo.processInfoLength = sizeof(pInfo);
	pInfo.processName = NULL;
	pInfo.processAppSpec = &spec;
	e = GetProcessInformation(&psnFrontProcess, &pInfo);
	if (e != noErr)
		return IKString(TEXT(""));

	//  convert folder part to the full path
	char fullpath[255];
	GetFullPath2 ( spec.parID, spec.vRefNum, fullpath );
	
	//  tack on the file name
	int len = spec.name[0];
	spec.name[len+1] = 0;
	IKString::strcat(fullpath,(const char *)&(spec.name[1]));
	
	//  might be an OS X package
	int i = IKString::strstr(fullpath,TEXT(".app:"));
	if (i != -1)
		fullpath[i+4] = '\0';	
		
	//  might still have a trailing colon
	int l = IKString::strlen(fullpath);
	if (fullpath[l-1]==':')
		fullpath[l-1] = '\0';

	IKString s = fullpath;
	IKUtil::GetTrueFileName(s);
	return s;
	
#endif
	
#endif
}

void IKUtil :: StripFileName ( IKString &str,
	bool bStripPath /*= true */,
	bool bStripExtension /*= true */ )
{
	if ( bStripPath )
	{
		int lastMacSep = str.ReverseFind ( IKUtil::GetMacPathDelimiter() );
		int lastWinSep = str.ReverseFind ( IKUtil::GetWinPathDelimiter() );
		int lastSeparator = lastMacSep;
		if (lastWinSep > lastSeparator)
			lastSeparator = lastWinSep;

		if ( lastSeparator != -1 )
			str = str.Right ( str.GetLength() - lastSeparator - 1 );
	}
	
	if ( bStripExtension )
	{
		int lastPeriod = str.ReverseFind ( '.' );
		if ( lastPeriod != -1 )
			str = str.Left ( lastPeriod );
	}
}

//  statics for logging;
static TCHAR buffer[4096];
static int theEnd = 0;
static int opened = -1;
static IKFile f;
static unsigned int lastFlush = 0;

void IKUtil::LogString ( TCHAR *string )
{
	//  make a string for outputting
	IKString out;

	//  append the time of day
	out += GetTime();
	out += TEXT(" ");

	//  set the start time if this is the first call.
	static unsigned int startTime = 0;
	if (startTime==0)
		startTime = GetCurrentTimeMS();

	//  append secs since start
	//  get whole and decimal parts of ticks since start
	unsigned int tdiff = GetCurrentTimeMS() - startTime;
	unsigned int tdiffa = tdiff/1000;
	unsigned int tdiffb = tdiff - 1000*tdiffa;
		
	//  add the whole part
	IKString s1 = IKUtil::IntToString(tdiffa);
	out += s1;
	out += TEXT(".");

	//  add the decimal part
	IKString s2 = IKUtil::IntToString(tdiffb);
	if(s2.GetLength()==1)
		out += TEXT("0");
	if(s2.GetLength()==2)
		out += TEXT("0");
	out += s2;
	out += TEXT(" ");

	//  finally, tack on the string given.
	out += string;

	//  if debugging, output to screen
#ifdef _DEBUG
  #ifdef PLAT_WINDOWS
	IKString display=out; 
	display += TEXT("\n"); 
	OutputDebugString((TCHAR *)display);
  #else
    #ifdef BUILD_CW
	{
		TCHAR buf[255];
		strcpy(&(buf[1]),out);
		buf[0] = IKString::strlen(&(buf[1]));
		DebugStr((const unsigned TCHAR *)buf);
	}
    #else
		//#ifdef DAEMON
			syslog(1,(TCHAR *)out);
		//#else
			printf(TEXT("%s\n"),(TCHAR *)out);
		//#endif
    #endif
  #endif
#endif

	//  get file to log
	//  no log to file is name not given.
	IKString name = DATAS(TEXT("Debug_Log_File"),TEXT(""));
	if (name.IsEmpty())
		return;

	//  open log file the first time
	if (opened == -1)
	{
		opened = 0;
		IKString filename = GetRootFolder ();
		filename += name;
		if (f.Open(filename,IKFile::modeReadWrite|IKFile::modeNoTruncate|IKFile::modeCreate))
		{
			opened = 1;
			f.SeekToEnd();

			//  put in Unicode BOM if file is empty (just created)
			if (f.GetPosition()==0)
			{
				if (DATAI(TEXT("Save_Text_As_UTF8"),0)==1)
				{
					f.MarkAsUTF8();
				}
				else
					f.MarkAsUnicode();
			}
		}
	}

	//  if the string does not fit,
	//  write the contents and reset the buffer

	if (theEnd + 5 + IKString::strlen(out)>sizeof(buffer))
	{
		FlushLog();
	}

	//  insert the string
	IKString::strcpy((TCHAR *)&(buffer[theEnd]),out);
	theEnd += IKString::strlen(out);

#ifdef PLAT_WINDOWS
	IKString::strcpy((TCHAR *)&(buffer[theEnd]),TEXT("\r\n"));
	theEnd += 2;
#endif
#ifdef PLAT_MACINTOSH
	IKString::strcpy((TCHAR *)&(buffer[theEnd]),TEXT("\r"));
	theEnd += 1;
#endif

}

void IKUtil::Periodic()
{
	//  flush the log file
	unsigned int now = IKUtil::GetCurrentTimeMS();
	if (now>=lastFlush)
	{
		if (theEnd>0)
		{
			FlushLog();
		}
		lastFlush = now + DATAI(TEXT("Debug_Log_Flush_Period"),1000);
	}

}

void IKUtil::FlushLog()
{
	if (opened==1)
	{
		int nc = theEnd;
#ifdef PLAT_WINDOWS
#ifdef UNICODE
		nc = nc * 2;
#endif
#endif
		f.Write((BYTE *)buffer,nc);
		f.Flush();
	}
	theEnd = 0;
}

IKString IKUtil::GetThisAppPath()
{
    static IKString result = TEXT("");
    if(!result.IsEmpty())
	return result;

#ifdef PLAT_WINDOWS
	TCHAR fileName[MAX_PATH];
	GetModuleFileName(NULL, fileName, MAX_PATH - 1);
	result = fileName;
#endif

#ifdef PLAT_MACINTOSH_CLASSIC
	//  TODO!
	result = "";
#endif

#ifdef PLAT_MACINTOSH_X
	//  TODO!
	result = "";
#endif

	return result;
}

#ifdef PLAT_MACINTOSH
SInt32  IKUtil :: GetSystemVersion()
{
	SInt32   response;
	OSErr err = Gestalt (gestaltSystemVersion, &response);
	if ((err != noErr))
		return 0;
		
	return response;
}
#endif

bool IKUtil::RunningUnderOSX ()
{
#ifdef PLAT_WINDOWS
    return false;
#endif

#ifdef PLAT_MACINTOSH
	static   Boolean     sAlreadyChecked = false;
	static   Boolean     sRunningUnderOSX = false;

	if (!sAlreadyChecked) 
	{
		SInt32   response;
		OSErr err = Gestalt (gestaltSystemVersion, &response);
		if ((err == noErr)) 
		{
		    if (response >= 0x1000)
			sRunningUnderOSX = true;
		}

		sAlreadyChecked = true;
	}
	return sRunningUnderOSX;
#endif
}

bool IKUtil::RunningUnderClassic ()
{
#ifdef PLAT_WINDOWS
    return false;
#endif

#ifdef PLAT_MACINTOSH
	static   Boolean     sAlreadyChecked = false;
	static   Boolean     sRunningUnderClassic = false;

	if (!sAlreadyChecked) 
	{
		SInt32   response;
		OSErr err = Gestalt (gestaltMacOSCompatibilityBoxAttr, &response);
		if ((err == noErr) && ((response & (1 << gestaltMacOSCompatibilityBoxPresent)) != 0)) 
		{
			sRunningUnderClassic = true;
		}

		sAlreadyChecked = true;
	}
	return sRunningUnderClassic;
#endif
}


#ifdef PLAT_MACINTOSH
void IKUtil::ApplyFinderTypeAndCreator(IKString strFile, const TCHAR *file_type, const TCHAR *creator)
{

}
#endif

#ifdef PLAT_WINDOWS

static UINT LaunchRoutine (LPVOID pParam)
{	
	TCHAR *pstrFile = (TCHAR *) pParam;

	if (pstrFile)
	{
		Sleep(100);  //  I do not know why.

		int nResult = (int) ShellExecute ( GetDesktopWindow(), TEXT("Open"), pstrFile, TEXT(""), TEXT("C:\\"), SW_SHOWNORMAL);
		if ( nResult<=32 && nResult!=ERROR_FILE_NOT_FOUND )
			IKASSERT(false);

		delete [] pstrFile;

	}

	return 0;
}


#endif


void IKUtil::LaunchFile(IKString filename)
{

#ifdef PLAT_WINDOWS

	//  start the launch thread

	TCHAR *pstrFile = new TCHAR[255];
	IKString::strcpy(pstrFile,filename);

	DWORD ThreadID;
	HANDLE h = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)LaunchRoutine, (void *)pstrFile, 0, &ThreadID);
	

#endif

#ifdef PLAT_MACINTOSH

#ifdef PLAT_MACINTOSH_X

    //  let's ask the Helper to do this.

	int len = filename.GetLength();
	TCHAR * pMsg = (TCHAR *)filename;
	int result = IKMessage::Send ( TEXT("menu"), kQueryLaunchFile, (void *) pMsg, len, 0, 0 );	

#else

    //  Make an FSSpec first
    FSSpec spec;
    TCHAR macFileName[255];
    strcpy(&(macFileName[1]),(TCHAR *)filename);
    macFileName[0] = IKString::strlen(&(macFileName[1]));
    OSErr err = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &spec );
    if (err!=noErr)
	return;
	
    //  now make the FSRef
    FSRef ref;
    err = FSpMakeFSRef ( &spec, &ref );
    if (err!=noErr)
	return;
	
    //  Launch it, only Carbon

#if TARGET_API_MAC_CARBON
    err = LSOpenFSRef ( &ref, NULL );
#endif
    
#endif

#endif


}

void IKUtil::MemoryCopy(BYTE *dst, BYTE *src, int nbytes)
{
	for (int i=0;i<nbytes;i++)
		dst[i] = src[i];
}

bool IKUtil::MemoryCompare(BYTE *dst, BYTE *src, int nbytes)
{
	for (int i=0;i<nbytes;i++)
	{
		if (src[i] != dst[i])
			return false;
	}

	return true;
}

bool IKUtil::IsWin2KOrGreater()
{
#ifdef PLAT_WINDOWS
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof ( osvi );
	::GetVersionEx( &osvi );
	if (osvi.dwMajorVersion>=5)  //  Windows 2k is first version 5
		return true;
	return false;

#else
	return false;
#endif

}

bool IKUtil::IsWinVistaOrGreater()
{
#ifdef PLAT_WINDOWS
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof ( osvi );
	::GetVersionEx( &osvi );
	if (osvi.dwMajorVersion>=6)  //  Vista is first version 6
		return true;
	return false;

#else
	return false;
#endif

}

#ifdef PLAT_WINDOWS
int IKUtil::GetCodePage()
{
	//  get the code page once
	static int codepage = -1;
	if (codepage==-1)
	{
		//  get locale
		TCHAR locale[10];
		if (::GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_ILANGUAGE, locale, sizeof(locale)/sizeof(TCHAR) ) != 0)
		{
			IKString key("codepage_for_locale_");
			key += locale;
			IKString val = DATAS(key,TEXT("0"));
			codepage = StringToInt(val);
		}
		else
		{
			codepage = 0;
		}

	}

	//  return it
	return codepage;


}
#endif

#ifdef PLAT_MACINTOSH_X

typedef struct kinfo_proc kinfo_proc;

static int GetBSDProcessList(kinfo_proc **procList, size_t *procCount)
// Returns a list of all BSD processes on the system.  This routine
// allocates the list and puts it in *procList and a count of the
// number of entries in *procCount.  You are responsible for freeing
// this list (use "free" from System framework).
// On success, the function returns 0.
// On error, the function returns a BSD errno value.
{
    int                 err;
    kinfo_proc *        result;
    bool                done;
    static const int    name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
    // Declaring name as const requires us to cast it when passing it to
    // sysctl because the prototype doesn't include the const modifier.
    size_t              length;
	
    assert( procList != NULL);
    assert(*procList == NULL);
    assert(procCount != NULL);
	
    *procCount = 0;
	
    // We start by calling sysctl with result == NULL and length == 0.
    // That will succeed, and set length to the appropriate length.
    // We then allocate a buffer of that size and call sysctl again
    // with that buffer.  If that succeeds, we're done.  If that fails
    // with ENOMEM, we have to throw away our buffer and loop.  Note
    // that the loop causes use to call sysctl with NULL again; this
    // is necessary because the ENOMEM failure case sets length to
    // the amount of data returned, not the amount of data that
    // could have been returned.
	
    result = NULL;
    done = false;
    do {
        assert(result == NULL);
		
        // Call sysctl with a NULL buffer.
		
        length = 0;
        err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                      NULL, &length,
                      NULL, 0);
        if (err == -1) {
            err = errno;
        }
		
        // Allocate an appropriately sized buffer based on the results
        // from the previous call.
		
        if (err == 0) {
            result = (kinfo_proc *)malloc(length);
            if (result == NULL) {
                err = ENOMEM;
            }
        }
		
        // Call sysctl again with the new buffer.  If we get an ENOMEM
        // error, toss away our buffer and start again.
		
        if (err == 0) {
            err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                          result, &length,
                          NULL, 0);
            if (err == -1) {
                err = errno;
            }
            if (err == 0) {
                done = true;
            } else if (err == ENOMEM) {
                assert(result != NULL);
                free(result);
                result = NULL;
                err = 0;
            }
        }
    } while (err == 0 && ! done);
	
    // Clean up and establish post conditions.
	
    if (err != 0 && result != NULL) {
        free(result);
        result = NULL;
    }
    *procList = result;
    if (err == 0) {
        *procCount = length / sizeof(kinfo_proc);
    }
	
    assert( (err == 0) == (*procList != NULL) );
	
    return err;
}

static void get_app_name ( char *arg, char *appname )
{
	//  find the last backslash
	int i = strlen(arg)-1;
	while (i>=0)
	{
		if (arg[i]=='/')
			break;
		i--;
	}
	
	//  give it up
	if (arg[i]=='/')
	{
		i++;
		strcpy ( appname, &(arg[i]) );
	}
	else
		strcpy ( appname, "" );
}

static int
get_args (int pid, char *cbuf, int csize)
{
	//  allocate a space to hold arguments
	char *arguments;
	int arguments_size = csize;
	arguments = (char *) malloc(arguments_size);

	//  get the arguments
	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROCARGS;
	mib[2] = pid;
	mib[3] = 0;
	if (sysctl(mib, 3, arguments, (size_t *)&arguments_size, NULL, 0) < 0) 
	{
	    free(arguments);
		cbuf[0] = 0;
		return(0);
	}
	
	//  prevent buffer overrun
	arguments[arguments_size] = 0;
        
	//  replace all of the nulls with bars
	for (int i=0; i<arguments_size-1; i++)
		if (arguments[i] == 0)
                    arguments[i] = '|';
	
	//  give it back
	strcpy ( cbuf, arguments );

	//  all done.
	free (arguments);
	return (1);
}


static bool MacIsAppRunning ( char *appname )
{
    bool bFound = false;

    size_t count = 0;
    kinfo_proc *procList = NULL;
    int result = GetBSDProcessList(&procList, &count);
    if (result==0)
    {        
        static char args[4096];
        
        for (int i=0;i<count;i++)
        {
            get_args (procList[i].kp_proc.p_pid, args, sizeof(args)-1);
            if (strstr(args,appname) != NULL)
            {
                bFound = true;
                break;
            }
            if (strstr(procList[i].kp_proc.p_comm,appname) != NULL)
            {
                bFound = true;
                break;
            }
        }
    }
    
    free(procList);

    return bFound;
}


#endif

#ifdef PLAT_WINDOWS

// 
// EnumProc.c
// 
#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <vdmdbg.h>

typedef BOOL (CALLBACK *PROCENUMPROC)(DWORD, WORD, LPSTR, LPARAM);

typedef struct {
   DWORD          dwPID;
   PROCENUMPROC   lpProc;
   DWORD          lParam;
   BOOL           bEnd;
} EnumInfoStruct;

//BOOL WINAPI EnumProcs(PROCENUMPROC lpProc, LPARAM lParam);

static BOOL WINAPI Enum16(DWORD dwThreadId, WORD hMod16, WORD hTask16,
      PSZ pszModName, PSZ pszFileName, LPARAM lpUserDefined);

// 
// The EnumProcs function takes a pointer to a callback function
// that will be called once per process with the process filename 
// and process ID.
// 
// lpProc -- Address of callback routine.
// 
// lParam -- A user-defined LPARAM value to be passed to
//           the callback routine.
// 
// Callback function definition:
// BOOL CALLBACK Proc(DWORD dw, WORD w, LPCSTR lpstr, LPARAM lParam);
// 
static BOOL WINAPI EnumProcs(PROCENUMPROC lpProc, LPARAM lParam) {

   OSVERSIONINFO  osver;
   HINSTANCE      hInstLib  = NULL;
   HINSTANCE      hInstLib2 = NULL;
   HANDLE         hSnapShot = NULL;
   LPDWORD        lpdwPIDs  = NULL;
   PROCESSENTRY32 procentry;
   BOOL           bFlag;
   DWORD          dwSize;
   DWORD          dwSize2;
   DWORD          dwIndex;
   HMODULE        hMod;
   HANDLE         hProcess;
   char           szFileName[MAX_PATH];
   EnumInfoStruct sInfo;

   // ToolHelp Function Pointers.
   HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD, DWORD);
   BOOL (WINAPI *lpfProcess32First)(HANDLE, LPPROCESSENTRY32);
   BOOL (WINAPI *lpfProcess32Next)(HANDLE, LPPROCESSENTRY32);

   // PSAPI Function Pointers.
   BOOL (WINAPI *lpfEnumProcesses)(DWORD *, DWORD, DWORD *);
   BOOL (WINAPI *lpfEnumProcessModules)(HANDLE, HMODULE *, DWORD, 
         LPDWORD);
   DWORD (WINAPI *lpfGetModuleBaseName)(HANDLE, HMODULE, LPTSTR, DWORD);

   // VDMDBG Function Pointers.
   static INT (WINAPI *lpfVDMEnumTaskWOWEx)(DWORD, TASKENUMPROCEX, LPARAM) = NULL;

   // Retrieve the OS version
   osver.dwOSVersionInfoSize = sizeof(osver);
   if (!GetVersionEx(&osver))
      return FALSE;
   
   // If Windows NT 4.0
   if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT
         && osver.dwMajorVersion == 4) {

      __try {

         // Get the procedure addresses explicitly. We do
         // this so we don't have to worry about modules
         // failing to load under OSes other than Windows NT 4.0 
         // because references to PSAPI.DLL can't be resolved.
         hInstLib = LoadLibraryA("PSAPI.DLL");
         if (hInstLib == NULL)
            __leave;

        if (lpfVDMEnumTaskWOWEx==NULL)
		{
			hInstLib2 = LoadLibraryA("VDMDBG.DLL");
         if (hInstLib2 == NULL)
            __leave;
			lpfVDMEnumTaskWOWEx = (INT (WINAPI *)(DWORD, TASKENUMPROCEX,
               LPARAM)) GetProcAddress(hInstLib2, "VDMEnumTaskWOWEx");
		}

         // Get procedure addresses.
         lpfEnumProcesses = (BOOL (WINAPI *)(DWORD *, DWORD, DWORD*))
               GetProcAddress(hInstLib, "EnumProcesses");

         lpfEnumProcessModules = (BOOL (WINAPI *)(HANDLE, HMODULE *,
               DWORD, LPDWORD)) GetProcAddress(hInstLib,
               "EnumProcessModules");

         lpfGetModuleBaseName = (DWORD (WINAPI *)(HANDLE, HMODULE,
               LPTSTR, DWORD)) GetProcAddress(hInstLib,
               "GetModuleBaseNameA");
         
         if (lpfEnumProcesses == NULL 
               || lpfEnumProcessModules == NULL 
               || lpfGetModuleBaseName == NULL 
               || lpfVDMEnumTaskWOWEx == NULL)
            __leave;

         // 
         // Call the PSAPI function EnumProcesses to get all of the
         // ProcID's currently in the system.
         // 
         // NOTE: In the documentation, the third parameter of
         // EnumProcesses is named cbNeeded, which implies that you
         // can call the function once to find out how much space to
         // allocate for a buffer and again to fill the buffer.
         // This is not the case. The cbNeeded parameter returns
         // the number of PIDs returned, so if your buffer size is
         // zero cbNeeded returns zero.
         // 
         // NOTE: The "HeapAlloc" loop here ensures that we
         // actually allocate a buffer large enough for all the
         // PIDs in the system.
         // 
         dwSize2 = 256 * sizeof(DWORD);
         do {

            if (lpdwPIDs) {
               HeapFree(GetProcessHeap(), 0, lpdwPIDs);
               dwSize2 *= 2;
            }

            lpdwPIDs = (LPDWORD) HeapAlloc(GetProcessHeap(), 0, 
                  dwSize2);
            if (lpdwPIDs == NULL)
               __leave;
            
            if (!lpfEnumProcesses(lpdwPIDs, dwSize2, &dwSize))
               __leave;

         } while (dwSize == dwSize2);

         // How many ProcID's did we get?
         dwSize /= sizeof(DWORD);

         // Loop through each ProcID.
         for (dwIndex = 0; dwIndex < dwSize; dwIndex++) {

            szFileName[0] = 0;
            
            // Open the process (if we can... security does not
            // permit every process in the system to be opened).
            hProcess = OpenProcess(
                  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                  FALSE, lpdwPIDs[dwIndex]);
            if (hProcess != NULL) {

               // Here we call EnumProcessModules to get only the
               // first module in the process. This will be the 
               // EXE module for which we will retrieve the name.
               if (lpfEnumProcessModules(hProcess, &hMod,
                     sizeof(hMod), &dwSize2)) {

                  // Get the module name
                  if (!lpfGetModuleBaseName(hProcess, hMod,
                        szFileName, sizeof(szFileName)))
                     szFileName[0] = 0;
               }
               CloseHandle(hProcess);
            }
            // Regardless of OpenProcess success or failure, we
            // still call the enum func with the ProcID.
            if (!lpProc(lpdwPIDs[dwIndex], 0, szFileName, lParam))
               break;

            // Did we just bump into an NTVDM?
            if (IKString::stricmp(szFileName, "NTVDM.EXE") == 0) 
			{

               // Fill in some info for the 16-bit enum proc.
               sInfo.dwPID = lpdwPIDs[dwIndex];
               sInfo.lpProc = lpProc;
               sInfo.lParam = (DWORD) lParam;
               sInfo.bEnd = FALSE;

               // Enum the 16-bit stuff.
               lpfVDMEnumTaskWOWEx(lpdwPIDs[dwIndex],
                  (TASKENUMPROCEX) Enum16, (LPARAM) &sInfo);

               // Did our main enum func say quit?
               if (sInfo.bEnd)
                  break;
            }
         }

      } __finally {

         if (hInstLib)
            FreeLibrary(hInstLib);

         //if (hInstLib2)
            //FreeLibrary(hInstLib2);

         if (lpdwPIDs)
            HeapFree(GetProcessHeap(), 0, lpdwPIDs);
      }

   // If any OS other than Windows NT 4.0.
   } else if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
         || (osver.dwPlatformId == VER_PLATFORM_WIN32_NT
         && osver.dwMajorVersion > 4)) 
   {

      __try {

         hInstLib = LoadLibraryA("Kernel32.DLL");
         if (hInstLib == NULL)
            __leave;

         // If NT-based OS, load VDMDBG.DLL.
         if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT && lpfVDMEnumTaskWOWEx==NULL) 
		 {
            hInstLib2 = LoadLibraryA("VDMDBG.DLL");
            if (hInstLib2 == NULL)
               __leave;
			lpfVDMEnumTaskWOWEx = (INT (WINAPI *)(DWORD, TASKENUMPROCEX,
                  LPARAM)) GetProcAddress(hInstLib2, "VDMEnumTaskWOWEx");
            if (lpfVDMEnumTaskWOWEx == NULL)
               __leave;
         }

         // Get procedure addresses. We are linking to 
         // these functions explicitly, because a module using
         // this code would fail to load under Windows NT,
         // which does not have the Toolhelp32
         // functions in KERNEL32.DLL.
         lpfCreateToolhelp32Snapshot =
               (HANDLE (WINAPI *)(DWORD,DWORD))
               GetProcAddress(hInstLib, "CreateToolhelp32Snapshot");

         lpfProcess32First =
               (BOOL (WINAPI *)(HANDLE,LPPROCESSENTRY32))
               GetProcAddress(hInstLib, "Process32First");

         lpfProcess32Next =
               (BOOL (WINAPI *)(HANDLE,LPPROCESSENTRY32))
               GetProcAddress(hInstLib, "Process32Next");

         if (lpfProcess32Next == NULL
               || lpfProcess32First == NULL
               || lpfCreateToolhelp32Snapshot == NULL)
            __leave;


         // Get a handle to a Toolhelp snapshot of all processes.
         hSnapShot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
         if (hSnapShot == INVALID_HANDLE_VALUE) {
            FreeLibrary(hInstLib);
            return FALSE;
         }

         // Get the first process' information.
         procentry.dwSize = sizeof(PROCESSENTRY32);
         bFlag = lpfProcess32First(hSnapShot, &procentry);

         // While there are processes, keep looping.
         while (bFlag) {
            
            // Call the enum func with the filename and ProcID.
            if (lpProc(procentry.th32ProcessID, 0,
                  procentry.szExeFile, lParam)) {

               // Did we just bump into an NTVDM?
				if (IKString::stricmp(procentry.szExeFile, "NTVDM.EXE") == 0) 
				{

                  // Fill in some info for the 16-bit enum proc.
                  sInfo.dwPID = procentry.th32ProcessID;
                  sInfo.lpProc = lpProc;
                  sInfo.lParam = (DWORD) lParam;
                  sInfo.bEnd = FALSE;

                  // Enum the 16-bit stuff.
                  lpfVDMEnumTaskWOWEx(procentry.th32ProcessID,
                     (TASKENUMPROCEX) Enum16, (LPARAM) &sInfo);

                  // Did our main enum func say quit?
                  if (sInfo.bEnd)
                     break;
               }

               procentry.dwSize = sizeof(PROCESSENTRY32);
               bFlag = lpfProcess32Next(hSnapShot, &procentry);

            } else
               bFlag = FALSE;
         }

      } __finally {

         if (hInstLib)
            FreeLibrary(hInstLib);

         //if (hInstLib2)
            //FreeLibrary(hInstLib2);

		 CloseHandle (hSnapShot);
      }

   } else
      return FALSE;

   // Free the library.
   FreeLibrary(hInstLib);

   return TRUE;
}


static BOOL WINAPI Enum16(DWORD dwThreadId, WORD hMod16, WORD hTask16,
      PSZ pszModName, PSZ pszFileName, LPARAM lpUserDefined) {

   BOOL bRet;

   EnumInfoStruct *psInfo = (EnumInfoStruct *)lpUserDefined;

   bRet = psInfo->lpProc(psInfo->dwPID, hTask16, pszFileName,
      psInfo->lParam);

   if (!bRet) 
      psInfo->bEnd = TRUE;

   return !bRet;
} 

static bool bFound = false;
static TCHAR sName[255];

typedef struct
{
	bool bFound;
	char name[255];

} MyData;

static BOOL CALLBACK MyProcessEnumerator(DWORD dwPID, WORD wTask, 
      LPCSTR szProcess, LPARAM lParam) 
{
   MyData *pData = (MyData *) lParam;

   IKString s1((TCHAR *)pData->name);  //  look for this
   IKString s2((TCHAR *)szProcess);    //  in this

   s1.Lower();
   s2.Lower();

   int i = s2.ReverseFind('\\');
   if (i>=0)
	   s2 = s2.Mid(i+1,999);

   if (s2.Find(s1) != -1)
   {
	   pData->bFound = true;
   }

   return TRUE;
}


static bool WinIsAppRunning ( char *name )
{
	MyData data;
	data.bFound = false;
	strcpy(data.name,name);
	EnumProcs((PROCENUMPROC) MyProcessEnumerator, (long)&data);

	return data.bFound;
}
#endif

bool IKUtil::IsAppRunning ( char *appname )
{

#ifdef PLAT_WINDOWS
	return WinIsAppRunning(appname);
#endif

#ifdef PLAT_MACINTOSH_X
	return MacIsAppRunning(appname);
#endif

	return false;
}

static IKMap nameSubs;

static void LoadNameSubs ()
{
	//  do this once
	static bool bLoaded = false;
	if (bLoaded)
		return;
	bLoaded = true;

	//  get the global list
	IKString filepath = IKUtil::GetPrivateFolder();
	filepath += TEXT("appnames.txt");
	nameSubs.Read(filepath);

	//  merge in the names from pre-installed packs
	IKString preinstallFolder = IKUtil::GetPrivateFolder();
	preinstallFolder += DATAS(TEXT("APP_OVERLAYS_SRC_FOLDER"),TEXT("Application Overlays"));
	preinstallFolder += IKUtil::GetPathDelimiter();

	// find all of the folders at this level.
	IKStringArray folders;
	IKUtil::BuildFolderArray ( folders, (TCHAR *)preinstallFolder );

	//  for each folder
	for (int nFolder=0;nFolder<folders.GetSize();nFolder++)
	{
		IKString folder = folders[nFolder];
		IKString mapFile = folder;
		mapFile += DATAS(TEXT("APP_OVERLAYS_NAMES_MAP"),TEXT("application display names.txt"));
		nameSubs.Read(mapFile);
	}

	//  OK, dump the nameSubs map
	int n = nameSubs.Count();
	for (int i=0;i<n;i++)
	{
		IKString key, val;
		nameSubs.GetNthPair(i,key,val);
	}

}

IKString IKUtil::GetAppFriendlyName ( IKString &path )
{
	//  load the name subs map
	LoadNameSubs();

	IKString friendly;

	//  first see if there is a substitution 
	//  to do based on the path
	int n = nameSubs.Count();
	int i;
	IKString key = path;
	key.EncodeSpaces();
	for (i=0;i<n;i++)
	{
		IKString s1,s2;
		nameSubs.GetNthPair(i,s1,s2);
		int l = s1.GetLength();

		if (key.Right(l).CompareNoCase(s1)==0)
		{
			friendly = s2;
			return friendly;
		}
	}

	//  get the friendly name from the app
	//  (platform-specific)

#ifdef PLAT_WINDOWS
	CExecImageVersion v(path);
	friendly = v.GetFileDescription();

	if (DATAI(TEXT("Unicode File Names"),0) != 0)
	{
		//  ansi to unicode and back to UTF-8

		int len = friendly.GetLength();
		unsigned short wide[1024];
		int res = MultiByteToWideChar ( CP_ACP, 0, friendly, len, wide, 1024);
		wide[res] = 0;

		char narrow[1024];
		int result2 = WideCharToMultiByte ( CP_UTF8, 0, wide, res, narrow, 1024, 0, 0);
		narrow[result2] = 0;

		friendly = IKString(narrow);
	
	}

	if (friendly.IsEmpty())
	{
		friendly = path;
		IKUtil::StripFileName(friendly,true,true);
	}
#else
	//  remove and ".app" or ".app:" that you find at the end.
	friendly = path;
	if (friendly.Right(5).CompareNoCase(".app:")==0)
		friendly = friendly.Left(friendly.GetLength()-5);
	if (friendly.Right(4).CompareNoCase(".app")==0)
		friendly = friendly.Left(friendly.GetLength()-4);
	if (friendly.Right(1).CompareNoCase(":")==0)
		friendly = friendly.Left(friendly.GetLength()-1);
	IKUtil::StripFileName(friendly,true,true);
#endif

	//  see again if there is a substitution to be made
	key = friendly;
#ifdef PLAT_WINDOWS
	//key += ".exe";
#endif
	key.EncodeSpaces();
	IKString newName = nameSubs.Lookup(key);
	if (!newName.IsEmpty())
	{
		friendly = newName;
	}

	return friendly;
}


#ifdef PLAT_WINDOWS

#define _TEXT(a) a

#define DYNLOADED_FPTR( ptrname, procname, dllname)\
FPTR_##procname ptrname = \
( FPTR_##procname ) GetProcAddress ( GetModuleHandle (  _TEXT( #dllname)), #procname);

#define CREATE_DYNFUNC_5( ptrname, procname, dllname, rettype, callconv, a1, a2, a3, a4, a5)\
typedef  rettype (callconv *FPTR_##procname) ( a1, a2, a3, a4, a5);\
DYNLOADED_FPTR( ptrname, procname, dllname);


typedef LONG NTSTATUS;
typedef LONG KPRIORITY;
typedef struct _PEB *PPEB;

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,          // Note: this is kernel mode only
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    ProcessDeviceMap,
    ProcessSessionInformation,
    ProcessForegroundInformation,
    ProcessWow64Information,
    MaxProcessInfoClass
    } PROCESSINFOCLASS;

#define NTSYSCALLAPI DECLSPEC_IMPORT

#ifndef ULONG_PTR
#define ULONG_PTR unsigned long *
#endif

typedef struct _PROCESS_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PPEB PebBaseAddress;
    ULONG_PTR AffinityMask;
    KPRIORITY BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;
typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;


DWORD   GetParentProcessID  (   DWORD   dwPID)
{
    NTSTATUS                        ntStatus;
    DWORD                           dwParentPID =   0xffffffff;

    HANDLE                          hProcess;
    PROCESS_BASIC_INFORMATION       pbi;
    ULONG                           ulRetLen;

    //  create entry point for 'NtQueryInformationProcess()'
    CREATE_DYNFUNC_5    (   NtQueryInformationProcess,
                            NtQueryInformationProcess,
                            ntdll,
                            NTSTATUS,
                            __stdcall,
                            HANDLE,
                            PROCESSINFOCLASS,
                            PVOID,
                            ULONG,
                            PULONG
                        );

    //  get process handle
    hProcess    =   OpenProcess (   PROCESS_QUERY_INFORMATION,
                                    FALSE,
                                    dwPID
                                );

    //  could fail due to invalid PID or insufficiant privileges
    if  (   !hProcess)
            return  (   0xffffffff);

    //  gather information
    ntStatus    =   NtQueryInformationProcess   (   hProcess,
                                                    ProcessBasicInformation,
                                                    ( void*) &pbi,
                                                    sizeof  (   PROCESS_BASIC_INFORMATION),
                                                    &ulRetLen
                                                );

    //  copy PID on success
    if  (   !ntStatus)
            dwParentPID =   (DWORD)pbi.InheritedFromUniqueProcessId;

    CloseHandle (   hProcess);

    return  (   dwParentPID);
}
#endif

#ifdef PLAT_WINDOWS

static IKString GetParentApp98 ()
{
	DWORD thisProcess = GetCurrentProcessId();

	//  declarations for KERNEL32 functions
	typedef HANDLE (FAR PASCAL *CreateSnapshotProc)(DWORD,DWORD);
	typedef BOOL (FAR PASCAL *Module32Proc)(HANDLE, LPMODULEENTRY32);
	typedef BOOL (FAR PASCAL *Process32Proc)(HANDLE, LPPROCESSENTRY32);

	//  get pointers to KERNEL32 functions
	static bool bOnce = false;
	static CreateSnapshotProc	pCreateToolhelp32Snapshot = NULL;
	static Module32Proc			pModule32First = NULL, pModule32Next = NULL;
	static Process32Proc		pProcess32First = NULL, pProcess32Next = NULL;
	if(!bOnce)
	{
		HINSTANCE m_hToolHelp = :: GetModuleHandle ( TEXT("KERNEL32") );
		if ( m_hToolHelp )
		{
			pCreateToolhelp32Snapshot	= (CreateSnapshotProc)	::GetProcAddress( m_hToolHelp, "CreateToolhelp32Snapshot" );
			pModule32First				= (Module32Proc)		::GetProcAddress( m_hToolHelp, "Module32First" );
			pModule32Next				= (Module32Proc)		::GetProcAddress( m_hToolHelp, "Module32Next" );
			pProcess32First				= (Process32Proc)		::GetProcAddress( m_hToolHelp, "Process32First" );
			pProcess32Next				= (Process32Proc)		::GetProcAddress( m_hToolHelp, "Process32Next" );
		}
		bOnce = true;
	}

	//  get a snap
	HANDLE hSnapShot=(*pCreateToolhelp32Snapshot)(TH32CS_SNAPPROCESS,0);
	if( hSnapShot == INVALID_HANDLE_VALUE )
	{
		return IKString("");
	}

	//  make a process info structure
	PROCESSENTRY32 processInfo;
	processInfo.dwSize=sizeof(PROCESSENTRY32);

	if( !Process32First( hSnapShot, &processInfo ) )
	{
		CloseHandle( hSnapShot );     // Must clean up the snapshot object!
		return IKString("");
	}

	//  paw thru processes looking for us
	DWORD parent = 0;
	while((*pProcess32Next)(hSnapShot,&processInfo)!=FALSE)
	{
		if (processInfo.th32ProcessID == thisProcess)
		{
			parent = processInfo.th32ParentProcessID;
		}
	}

	//  done with snapshot
	CloseHandle(hSnapShot);

	if (parent != 0)
	{
		TCHAR wname[1024];
		ExePathFromID ( (DWORD)parent, wname );
		IKString s(wname);
		return s;
	}

	IKString s = "";
	return s;
}

static IKString GetParentAppNT ()
{
	DWORD thisProcess = GetCurrentProcessId();
	DWORD parent = GetParentProcessID ( thisProcess );
	TCHAR wname[1024];
	ExePathFromID ( (DWORD)parent, wname );
	IKString s(wname);
	return s;
}

#endif


IKString IKUtil::GetParentApp()
{
#ifdef PLAT_WINDOWS

	IKString s = "";

	OSVERSIONINFO vInfo;
	vInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);	
	GetVersionEx(&vInfo);
	if (vInfo.dwPlatformId!=VER_PLATFORM_WIN32_NT)
		s = GetParentApp98();
	else
		s = GetParentAppNT();

	return s;

#else

	//  TODO!!!
	return "";

#endif
}


bool IKUtil::IsParentClickit ()
{

#ifdef PLAT_WINDOWS

	IKString path = GetParentApp();
	if (path.IsEmpty())
		return false;

	IKUtil::StripFileName(path,true,true);
	if (path.CompareNoCase(TEXT("clickit"))==0)
		return true;

	return false;

#else

	return false;

#endif

}

unsigned int IKUtil::GetSysTimeSecs1970()
{
#ifdef PLAT_WINDOWS

	//  get system time as a file time
	SYSTEMTIME stNow;
	GetSystemTime(&stNow);
	FILETIME ftNow;
	SystemTimeToFileTime(&stNow,&ftNow);

	//  Get Janary 1, 1970 as a FILETIME.
	SYSTEMTIME stJan1970 = {1970,1,0,1,0,0,0,0};
	FILETIME ftJan1970;
	SystemTimeToFileTime(&stJan1970,&ftJan1970);

	//  make large ints for each
	LARGE_INTEGER liNow;
	liNow.LowPart = ftNow.dwLowDateTime;
	liNow.HighPart = ftNow.dwHighDateTime;
	LARGE_INTEGER li1970;
	li1970.LowPart = ftJan1970.dwLowDateTime;
	li1970.HighPart = ftJan1970.dwHighDateTime;

	//  subtract and divide by 10,000,000;
	LARGE_INTEGER liDiff;
	liDiff.QuadPart = liNow.QuadPart - li1970.QuadPart;
	liDiff.QuadPart /= 10000000I64;
	IKASSERT(liDiff.HighPart==0);

	//  return the low part
	unsigned int result = liDiff.LowPart;
	return result;

#else

	time_t now;
	time(&now);

	return now;

#endif
}

void IKUtil::DisplayAlert ( IKString title, IKString message )
{
#ifdef PLAT_MACINTOSH
	
#ifndef BUILD_DAEMON

#if 0
  Str255 strTitle;
  Str255 strMessage;
  CopyCStringToPascal(title, strTitle );
  CopyCStringToPascal( message, strMessage );
  StandardAlert( kAlertPlainAlert, strTitle, strMessage, NULL, 0 );
#endif

#ifdef DISABLE_ON_LION_BUILD
    DialogRef alert;
    DialogItemIndex outHit;
	
	CFStringRef strTitle = CFStringCreateWithCString(kCFAllocatorDefault,title,kCFStringEncodingUTF8);
	CFStringRef strMessage = CFStringCreateWithCString(kCFAllocatorDefault,message,kCFStringEncodingUTF8);
    
    CreateStandardAlert(kAlertDefaultOKText, strTitle, strMessage, NULL, &alert);
    
    RunStandardAlert(alert, NULL, &outHit);
#endif
	
#endif

#else
	
	//  use Unicode version of MessageBox to display message.
	
	int len = message.GetLength();
	unsigned short wide[1024];
	int res = MultiByteToWideChar ( CP_UTF8, 0, message, len, wide, 1024);
	wide[res] = 0;

	int len2 = title.GetLength();
	unsigned short widet[1024];
	int res2 = MultiByteToWideChar ( CP_UTF8, 0, title, len2, widet, 1024);
	widet[res2] = 0;

	MessageBoxW(NULL, wide, widet, MB_OK);

#endif
}

void IKUtil::GetTrueFileName ( IKString & filename )
{

#ifdef PLAT_WINDOWS

	TCHAR longPath[512];
	DWORD result = GetLongPathName ( (TCHAR *)filename, longPath, 512 );
	if (result>0)
		filename = longPath;

#else

	//  mac - do nothing

#endif

}

bool  IKUtil :: IsPrimaryMouseDown(void)
{
  bool bResult=false;

#ifdef PLAT_MACINTOSH

#ifdef BUILD_DAEMON
	bResult = false;
#else
	bResult = (bool) (::StillDown ());
	
#endif

#else

	bResult = 
      (GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) 
         ? VK_RBUTTON : VK_LBUTTON) & 0x8000) != 0;

#endif

	return bResult;
}

void IKUtil :: BuildFolderArray ( IKStringArray &array, const TCHAR * pInitialPath )
{
	BuildFileArray ( array, pInitialPath, kFoldersType );
}


#ifdef PLAT_MACINTOSH

// this struct just serves to allow us to pass some interesting information to the callback
struct DirectoryIterationInfo
{
	FSSpec			dirSpec;
	//	OSType			fileFilter;
	IKString			strExtension;
	IKStringArray *	fileNames;
	bool			bLookForFolders;
};

static pascal void FullPathAccumulatorProc( const CInfoPBRec * const cpbPtr, Boolean *quitFlag, void *pData )
{
	DirectoryIterationInfo* p = (DirectoryIterationInfo*) pData;
	
	bool bIsDirectory = ( ( cpbPtr->dirInfo.ioFlAttrib >> 4 ) & 1 ) != 0;
	
	if ( bIsDirectory == p->bLookForFolders )
	{
		IKString file ( (BYTE *)&(cpbPtr->hFileInfo.ioNamePtr[1]), cpbPtr->hFileInfo.ioNamePtr[0] );
		
		TCHAR cpath[255];
		GetFullPath ( p->dirSpec.parID, p->dirSpec.vRefNum, cpath );
		IKString path = cpath;
		if (bIsDirectory)
		{
			IKString nam(&(p->dirSpec.name[1]),p->dirSpec.name[0]);
			path += nam;
		}

		IKString fullpath;
		fullpath += path;
		if ( fullpath.GetAt ( fullpath.GetLength() - 1 ) != ':' )
			fullpath += ':';
		fullpath += file;
		if (bIsDirectory)
			if ( fullpath.GetAt ( fullpath.GetLength() - 1 ) != ':' )
				fullpath += ':';
		
		if ( ( bIsDirectory ) ||
			 ( p->strExtension == ".*" ) ||
			 ( p->strExtension.CompareNoCase ( fullpath.Right ( p->strExtension.GetLength() ) ) == 0 ) )
			p->fileNames->Add( fullpath );
		
		*quitFlag = false;
	}
}

#endif


void IKUtil :: BuildFileArray ( IKStringArray &array,
		const TCHAR * pInitialPath,
		const TCHAR * pExtension /*= kAllFilesType */,
		int fileTypeOptions /* = -1 */ )
{
	bool bLookForFolders = ( pExtension == kFoldersType );

	IKString strType;
	if ( pExtension == kAllFilesType )
		strType = "*.*";
	else
		if ( bLookForFolders )
#ifdef PLAT_WINDOWS
			strType = "*.*";
#else
			strType = "*.";
#endif
			else
			{
				strType = "*.";
				strType += (TCHAR *)pExtension;
			}

#ifdef PLAT_WINDOWS
	// windows implementation
	IKString strPath;
	
	strPath = LPCSTR ( pInitialPath );

	if ( ! strPath.GetLength() )
		return;

	WIN32_FIND_DATA findFileData;

	if ( strPath.GetAt ( strPath.GetLength() - 1 ) != '\\' )
		strPath += '\\';


	HANDLE h = ::FindFirstFile( strPath + strType, &findFileData );
	
	if ( h == INVALID_HANDLE_VALUE )
		return;

	while (TRUE )
	{
		// sss: filter out "." and ".."
		if ( ( strcmp ( findFileData.cFileName, "." ) != 0 ) &&
			 ( strcmp ( findFileData.cFileName, ".." ) != 0 ) )
		{
			if (bLookForFolders)
			{
				if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					array.Add ( IKString ( strPath + IKString ( findFileData.cFileName ) ) + IKUtil::GetPathDelimiter() );
				}
			}
			else
			{
				if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					array.Add ( IKString ( strPath + IKString ( findFileData.cFileName ) ) );
				}
			}

		}

		if ( ! ::FindNextFile( h, &findFileData ) )
			break;
	}

	::FindClose ( h );
	
#else
	
	// Mac implementaion of directory iteration

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    NSError *error = nil;
    
    NSString *hfsPath = [NSString stringWithCString: pInitialPath encoding: kCFStringEncodingMacRoman];

    //NSLog(@"IKUtil::BuildFileArray pInitialPath %s", pInitialPath);
    //NSLog(@"IKUtil::BuildFileArray hfsPath %@", hfsPath);

    //StringRef = CFStringCreateWithCString( kCFAllocatorDefault, (TCHAR *)pInitialPath, kCFStringEncodingMacRoman ); 
	CFURLRef pathUrlRef = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, (CFStringRef) hfsPath, kCFURLHFSPathStyle, true );     
	CFStringRef posixpath = CFURLCopyFileSystemPath( pathUrlRef, kCFURLPOSIXPathStyle ); 

    NSString* directoryPath = (NSString*) posixpath;
    
    NSURL *url = [[NSURL alloc] initFileURLWithPath: (NSString*)posixpath];
    //NSLog(@"IKUtil::BuildFileArray posixpath %@", posixpath);

    NSArray *properties = nil;
    
    NSArray *itemsList = [[NSFileManager defaultManager] contentsOfDirectoryAtPath: directoryPath error: &error];

    //NSLog(@"IKUtil::BuildFileArray itemsList.count %i", [itemsList count]);

    if (itemsList != nil) {
        for (NSString *p in itemsList) {
            //NSLog(@"IKUtil::BuildFileArray file in itemList %@", p);
            
            NSDictionary *attributes = [[NSFileManager defaultManager] attributesOfItemAtPath: p error: &error];
            bool isDirectory = [[attributes objectForKey: NSFileType] isEqualToString: @"NSFileTypeDirectory"];
            p = [p lastPathComponent];
            const char *path = [[hfsPath stringByAppendingString: p] UTF8String];
            if (bLookForFolders) {
                if (isDirectory) {
                    array.Add (IKString (path) + IKUtil::GetPathDelimiter());
                }
            }
            else {
                if (!isDirectory) {
                    array.Add (IKString (path));
                }
            }
        }
    }

    [pool release];
    
#endif

    array.Sort();
}

bool IKUtil::IsOverlayMakerInstalled ()
{
	IKString strPath = IKUtil::GetOverlayMakerPath();
	return !strPath.IsEmpty();

}

#ifdef PLAT_WINDOWS

static void WinGetFileAssociation ( 
	IKString extension,	//  IN:  three-letter file extension, like ".txt"
	IKString &executable //  OUT: full path to the executable for the given file type
)
{
	executable = "";

	HKEY key;
	if (RegOpenKeyEx ( HKEY_CLASSES_ROOT, extension, 0, KEY_READ, &key) == ERROR_SUCCESS)
	{
		unsigned char value[255];
		unsigned long len = sizeof(value);
		LONG result = RegQueryValueEx ( key, NULL, NULL, NULL, value, &len );
		RegCloseKey (key);
		if (result==ERROR_SUCCESS)
		{
			value[len] = 0;
			IKString keyname = (TCHAR *)value;
			keyname += "\\shell\\open\\command";
			if (RegOpenKeyEx ( HKEY_CLASSES_ROOT, keyname, 0, KEY_READ, &key) == ERROR_SUCCESS)
			{
				len = sizeof(value);
				result = RegQueryValueEx ( key, NULL, NULL, NULL, value, &len );
				RegCloseKey (key);
				if (result==ERROR_SUCCESS)
				{
					executable = IKString((TCHAR *)value);
				}
			}
		}
	}

	if ( executable.GetLength() )
	{
		//  remove leading quote
		if (executable.GetAt(0)=='"')
			executable = executable.Mid(1);

		//  right now it looks something like
		//  c:\sss\eee\rrr\aa.exe "%1"
		//  look for .exe and strip the rest off.
		IKString s(executable);
		s.Lower();
		int j = s.Find(".exe");
		if (j != -1)
			executable = executable.Left(j+4);
	}
}

//  windows implementation
IKString IKUtil::GetOverlayMakerPath ( void )
{
	//  cache the value
	static IKString savedPath = "";
	static bool bOnce = false;
	if (bOnce)
		return savedPath;
	bOnce = true;

	IKString strOMTitle;
	IKString strOMPath;

	//  first try the registry
	WinGetFileAssociation ( ".oms", strOMPath );
	if (!strOMPath.IsEmpty())
	{
		//  found in the registry.  Does it really exist?
		if (IKFile::FileExists(strOMPath))
		{
			//  does it have "maker" in its name?
			IKString s = strOMPath;
			IKUtil::StripFileName(s,true,true);
			s.Lower();
			if (s.Find(TEXT("maker")) >= 0)
			{
				savedPath = strOMPath;
				return strOMPath;
			}
		}
	}
	
	// Get Overlay Maker's name (as displayed in frame window)
	TCHAR s[256];
	::GetPrivateProfileString( "Overlay Maker", "AppName", "Overlay Maker",
		s,256, "ITOOLS_X.INI" );
	strOMTitle = s;
	
	// Now get Overlay Maker's full path.
	::GetPrivateProfileString( "Overlay Maker", "AppPath", "",
		s,256, "ITOOLS_X.INI" );
	strOMPath = s;
	
	if ( strOMPath.IsEmpty() )
	{
		// Path was not in INI file, so we'll have to fake it.
		
		// Get the My Overlays path
		::GetPrivateProfileString( "Overlay Maker", "MyOverlays",
			"C:\\ITOOLS\\OMAKER\\My Overlays",
			s,256, "ITOOLS_X.INI" );
		
		strOMPath = s;
		
		// Now strip off "My Overlays"
		int lastSlash = strOMPath.ReverseFind( '\\' );
		if ( lastSlash != -1 )
			strOMPath = strOMPath.Left ( lastSlash );
		
		// And add "Overlay Maker" to get the full path
		strOMPath += "\\Overlay Maker.exe";
	}
	
	//  now go look for the file.
	WIN32_FIND_DATA findData;
	HANDLE hFile = FindFirstFile( strOMPath, &findData);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		savedPath = "";
		return savedPath;
	}
	else
	{
		::FindClose( hFile );
		savedPath = strOMPath;
		return savedPath;
	}
}

#else

static OSErr GetIndVolume( short index, short *vRefNum )
{

#if TARGET_API_MAC_CARBON

	HParamBlockRec pb;
	OSErr err;
	
	pb.volumeParam.ioCompletion = NULL;
	pb.volumeParam.ioNamePtr = NULL;
	pb.volumeParam.ioVolIndex = index;
	
	err= PBHGetVInfo(&pb,false);
	*vRefNum = pb.volumeParam.ioVRefNum;
	return err;
	
#else

	ParamBlockRec pb;
	OSErr err;
	
	pb.volumeParam.ioCompletion = NULL;
	pb.volumeParam.ioNamePtr = NULL;
	pb.volumeParam.ioVolIndex = index;
	
	err= PBGetVInfo(&pb,false);
	*vRefNum = pb.volumeParam.ioVRefNum;
	return err;
	
#endif
}


static pascal	OSErr	HGetVolParms(ConstStr255Param volName,
							 short vRefNum,
							 GetVolParmsInfoBuffer *volParmsInfo,
							 long *infoSize)
{
	HParamBlockRec pb;
	OSErr error;

	pb.ioParam.ioNamePtr = (StringPtr)volName;
	pb.ioParam.ioVRefNum = vRefNum;
	pb.ioParam.ioBuffer = (Ptr)volParmsInfo;
	pb.ioParam.ioReqCount = *infoSize;
	error = PBHGetVolParmsSync(&pb);
	if ( error == noErr )
	{
		*infoSize = pb.ioParam.ioActCount;
	}
	return ( error );
}

//
// determine if the given volume is a network share
//

static pascal	Boolean	isNetworkVolume(const GetVolParmsInfoBuffer *volParms)
{
	return ( volParms->vMServerAdr != 0 );
}

static OSErr IsNetworkVolume(		// RET: noErr if successful
	short inVRefNum, 				// IN : volume reference number
	Boolean *outIsNetworkVolume )	// OUT: if successful, true if volume is network share
									//      no change on error.
{
	//  determine whether this is a network drive.
	//  do not do cat search on network drives.			 
	GetVolParmsInfoBuffer VolParams;
	long vpSize = sizeof( GetVolParmsInfoBuffer );
	OSErr myErr = HGetVolParms ( NULL, inVRefNum, &VolParams, &vpSize );
	if ( myErr == noErr )
	{
		*outIsNetworkVolume = isNetworkVolume(&VolParams);
	}
	return myErr;
}

static bool FindAppBySignature ( long sig, bool bIncludeNetwork, FSSpec *fs )
{
#ifndef BUILD_DAEMON
#ifndef CLASSIC_SHLIB

	//  if we're running on OS X and launch services is available,
	//  do it that way first.

	if (IKUtil::RunningUnderOSX())
	{
		if ((UInt32) LSGetApplicationForInfo != (UInt32) kUnresolvedCFragSymbolAddress)
		{
			FSRef ref;
			OSErr err2 = LSGetApplicationForInfo ( kLSUnknownType, sig, nil, kLSRolesAll, &ref, nil );
			if (err2==noErr)
			{
				err2 = FSGetCatalogInfo ( &ref, kFSCatInfoNone, NULL, NULL, fs, NULL);
				if (err2==noErr)
					return true;
			}	
		}
	}
	
	OSErr err;
	short vRefNum, index;
	UInt8 bestMajor = 0;
	UInt8 bestMinor = 0;
	bool bFound = false;
	FSSpec fsBest;
	int nfound = 0;
	
	//  for each volume
	for( index = 1; ( err = GetIndVolume(index, &vRefNum) ) == noErr ; ++index )
	{
		//  see if we should include this volume.
		bool bInclude = true;
		Boolean bIsNetwork;
		if (IsNetworkVolume(vRefNum,&bIsNetwork)==noErr)
			if (bIsNetwork && !bIncludeNetwork)
				bInclude = false;
		if (bInclude)
		{
			//  OK, we're including this volume.  Loop thru all the matches
			
			DTPBRec   dtpb;
			Str255   theName;
			memset((void *)&dtpb, '\0', sizeof(dtpb));
			memset((void *)theName, '\0', sizeof(Str255));
			dtpb.ioNamePtr = theName;
			dtpb.ioVRefNum = vRefNum;
   			err = PBDTGetPath(&dtpb);
			if ((err == noErr) && (dtpb.ioDTRefNum != 0)) 
			{
				dtpb.ioFileCreator = sig;
				dtpb.ioIndex = 1;

				// There could be multiple entries in db file, some referring to files that
				// have been deleted, so loop through all of them to find ones that are valid.
				while (true)
    			{
					err = PBDTGetAPPLSync(&dtpb);
					if (dtpb.ioResult==afpItemNotFound)
						break;
					//if (err != noErr) 
						//break;

					FSSpec file;		
					err = FSMakeFSSpec(dtpb.ioVRefNum, dtpb.ioAPPLParID, theName, &file);
					if (err == noErr) 
					{
						//  found a valid match.
						nfound++;
						UInt8 major = 0;
						UInt8 minor = 0;
					
						//  get it's version if we can
						int resID = ::FSpOpenResFile ( &file, fsRdPerm );
						{
							::UseResFile ( resID );
							Handle hVers = ::Get1IndResource ( 'vers', 1 );
							if(hVers)
							{
								::DetachResource ( hVers );
								VersRecPtr p = (VersRecPtr) *hVers;
								if (p)
								{
									major = p->numericVersion.majorRev;
									minor = p->numericVersion.minorAndBugRev;
								}
								::DisposeHandle ( hVers );
							}
							::CloseResFile ( resID );
						}
						
						//  if it's the best so far, make it so
						if ( (major>bestMajor) || (major==bestMajor && minor>bestMinor) || nfound==1)
						{
							bFound = true;
							fsBest = file;
							bestMajor = major;
							bestMinor = minor;
						}
					}
					dtpb.ioIndex++;
    			} 
			}
		}
	}
	
	//  did we find one?
	if (bFound)
	{
		//  yes
		*fs = fsBest;
		return true;
	}
	
#endif  //  BUILD_DAEMON
#endif  //  CLASSIC_SHLIB

	return false;
}


//  mac implementation
IKString IKUtil::GetOverlayMakerPath ( void )
{
#ifdef BUILD_DAEMON
	return IKString(TEXT(""));
#else
	
	//  cache the result.
	static bool bFound = false;
	static IKString result = "";
	if (bFound)
		return result;
	
	FSSpec fileSpec;
	bool bInstalled = FindAppBySignature ( 'Cmzr', false, &fileSpec );
	if (!bInstalled)
	{
		result = "";
		bFound = true;
		return result;
	}
	
	//  convert to the full path
	char fullpath[255];
	GetFullPath2 ( fileSpec.parID, fileSpec.vRefNum, fullpath );
	
	//  give to the user
	result = fullpath;
	bFound = true;
	return result;
	
#endif
	
}

#endif  //  PLAT_WINDOWS

#ifdef PLAT_MACINTOSH
  #define kPathSeparator ':'
#else
  #define kPathSeparator '\\'
#endif

IKString IKUtil::GetOMOverlaysFolder()
{
	//  get overlay maker path
	IKString ompath = IKUtil::GetOverlayMakerPath ();

	//  exists?
	if (ompath.IsEmpty())
		return IKString(TEXT(""));

	//  strip off the filename
	int i = ompath.ReverseFind(kPathSeparator);
	ompath = ompath.Left(i+1);

	//  tack on the correct folder name
	ompath += TEXT("Overlays");

	//  exists?
	if (!IKFile::FolderExists(ompath))
		return IKString(TEXT(""));

	ompath += kPathSeparator;
	return ompath;
	
}


bool IKUtil::HasClickitAdaptation	( TCHAR * pAppPath )
{
	IKString AppPath = pAppPath;
#ifdef PLAT_MACINTOSH
	if (AppPath.GetAt(0)=='/')
		AppPath = IKFile::UnixToHFS (AppPath);
#endif
	
	IKString path(AppPath);

#ifdef PLAT_WINDOWS

	//  get this app's simple name
	IKString simple = path;
	IKUtil::StripFileName(simple,true,true);

	//  where is Clickit?
	char wp[_MAX_PATH];
	UINT nc = ::GetWindowsDirectory(wp,sizeof(wp));
	wp[nc] = 0;
	IKString winPath(wp);
	winPath += "\\ClickIt\\";

	//  is there a matching app folder with contents?
	IKString appFolder = winPath;
	appFolder += simple;
	appFolder += "\\";
	{
		IKString filesInFolder = appFolder;
		filesInFolder += "*.IT_";

		WIN32_FIND_DATA findFileData;
		HANDLE h = ::FindFirstFile( filesInFolder, &findFileData );
		if (h == INVALID_HANDLE_VALUE)
			return false;
		::FindClose ( h );
	}
	
	return true;

#else

	return false;

#endif
}

void IKUtil::SetOneProcessor()
{
#ifdef PLAT_WINDOWS

	if (IsWin2KOrGreater())
	{

		HANDLE hProcess = GetCurrentProcess();
		DWORD dwProcessAffinityMask, dwSystemAffinityMask;
		GetProcessAffinityMask( hProcess, &dwProcessAffinityMask, &dwSystemAffinityMask );
		SetProcessAffinityMask( hProcess, 1L );// use CPU 0 only

	}

#endif
}
