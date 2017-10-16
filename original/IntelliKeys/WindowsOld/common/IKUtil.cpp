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

#include "IterateDirectory.h"

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
	unsigned int newtime = GetTickCount();
	return newtime;
}

void IKUtil::Sleep(unsigned int waitMS)
{
	::Sleep ( waitMS );
}


static TCHAR strWinPathDelim = TEXT('\\');
static TCHAR strMacPathDelim = TEXT(':');

TCHAR IKUtil::GetPathDelimiter()
{
	return GetWinPathDelimiter();
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
		if (string.GetAt(i)==TEXT('/'))
		{
			string.SetAt(i,IKUtil::GetPathDelimiter());
		}
	}
}

IKString IKUtil::GetRootFolder()
{
	//  if we've already been called once, just return the
	//  previous result.
	static IKString result = TEXT("");
	if (!result.IsEmpty())
	{
		return result;
	}

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
	IKString fromFile = rootfile.GetValueString(TEXT("Root Location"),TEXT(""));
	
	//  return the value from the file if there is one
	//  otherwise us the folder the app was found in.

	if (!fromFile.IsEmpty())
	{
		result = fromFile;
	}
	else
	{
		result = thisAppFolder;
	}

	return result;
}

IKString IKUtil::GetChannelsFolder()
{
	IKString s = GetRootFolder ();
	s += DATAS(TEXT("Channels_Folder"), TEXT("Private/Channels/"));
	SubPathDelimiter(s); 
	return s;
}

IKString IKUtil::GetGuestFolder()
{
	IKString s = GetRootFolder ();
	s += DATAS(TEXT("Guest_Folder"), TEXT("Documents/Guest/"));
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

	//  format a string like HOUR:MIN:SEC
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
	dataFile += TEXT("datawin.txt");
	theData.Read(dataFile);
}

IKPrefs * IKUtil::GetData()
{
	return &theData;
}

IKString IKUtil::MakeStudentPath(IKString group, IKString student)
{
	IKString path = GetUsersFolder();
	path += group;
	if (student != IKString(TEXT("")))
	{
		path += GetPathDelimiter();
		path += student;
	}
	path += GetPathDelimiter() ;
	SubPathDelimiter(path);

	return path;
}

void IKUtil::MakeAllFolders(IKString path)
{
	//  given a path, make sure all the folders
	//  exist

	for (int i=0; i < path.GetLength(); i++)
	{
		if (path.GetAt(i) == GetPathDelimiter())
		{
			IKString part = path.Left(i+1);
			IKFile::NewFolder(part);
		}
	}
}

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

BOOL WINAPI Enum16A( DWORD dwThreadId, WORD hMod16, WORD hTask16, PSZ pszModName, PSZ pszFileName, LPARAM lpUserDefined )
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
				if ( s.Right(4).CompareNoCase(TEXT(".exe")) == 0 )
				{
					IKString::strcpy(wname, me32.szExePath);
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

static void ExePathFromWindowNT64(HWND hwndWindow, TCHAR* finalExePath )
{
	DebugLogToFile("IKUtil::ExePathFromWindowNT64 start - WindowHwnd: [%d (0x%x)]", hwndWindow, hwndWindow);

	//  declarations of PSAPI functions
	typedef DWORD (WINAPI *pfnGetModuleFileNameExW)(HANDLE, HMODULE, CONST WCHAR*, DWORD);
	//typedef BOOL (WINAPI *pfnEnumProcessModulesEx)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded, DWORD dwFilterFlag);
	typedef DWORD (WINAPI *pfnGetProcessImageFileNameW)(HANDLE, CONST WCHAR*, DWORD);
	typedef DWORD (WINAPI *pfnQueryFullProcessImageNameW)(HANDLE, DWORD, WCHAR*, DWORD*);

	//  pointers to PSAPI functions
	static pfnGetModuleFileNameExW pGetModuleFileNameExW = NULL;
	//static pfnEnumProcessModulesEx pEnumProcessModulesEx = NULL;
	static pfnGetProcessImageFileNameW pGetProcessImageFileNameW = NULL;
	static pfnQueryFullProcessImageNameW pQueryFullProcessImageNameW = NULL;


	//  get pointers to PSAPI functions
	static bool bLoaded = false;
	if (bLoaded == false)
	{
		HINSTANCE hInstPSAPI = LoadLibraryW(L"psapi.dll");
		if (hInstPSAPI)
		{
			pGetModuleFileNameExW = (pfnGetModuleFileNameExW)::GetProcAddress(hInstPSAPI, "GetModuleFileNameExW");
			//pEnumProcessModulesEx = (pfnEnumProcessModulesEx) ::GetProcAddress( hInstPSAPI, "EnumProcessModulesEx" );
			pGetProcessImageFileNameW = (pfnGetProcessImageFileNameW)::GetProcAddress(hInstPSAPI, "GetProcessImageFileNameW");


			//DebugLogToFile("IKUtil::ExePathFromWindowNT64 - PSAPI Library Functions Loaded");

		}
		if (IKUtil::IsWinVistaOrGreater())
		{
			HINSTANCE hInstKernel32 = LoadLibraryW(L"kernel32.dll");
			if (hInstKernel32)
			{
				pQueryFullProcessImageNameW = (pfnQueryFullProcessImageNameW)::GetProcAddress(hInstKernel32, "QueryFullProcessImageNameW");

				//DebugLogToFile("IKUtil::ExePathFromWindowNT64 - Vista and higher Kernel32 Library Functions: %d", (pQueryFullProcessImageNameW == NULL) ? TEXT("Load Failed") : TEXT("Loaded"));

			}
		}
		bLoaded = true;
	}

	//  if no pointers, use window text.
	if (pGetModuleFileNameExW == NULL)
	{
		::GetWindowText(hwndWindow, finalExePath, 1024 );
		//return;
	}
	else 
	{
		//  get process ID from window handle.
		DWORD dwPID = 0;
		DWORD threadID = ::GetWindowThreadProcessId(hwndWindow, &dwPID );

		//  get process handle with the necessary access privileges
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, dwPID);
		if (hProcess != NULL)
		{
			//  enumerate the modules in this process to get the module handle.
			DWORD nameLen = 1023;
			WCHAR moduleName[1024] = {0};
			TCHAR utf8ModuleName[1024] = {0};
			DWORD apiResult = 0;

			//DebugLogToFile("IKUtil::ExePathFromWindowNT64 - Process Handle: [%d (0x%x)] TID: [%d (0x%x)] PID: [%d (0x%x)]", hProcess, hProcess, threadID, threadID, dwPID, dwPID);

			// get module name using GetModuleFileNameExW which works on Win 2000 and above
			apiResult = (*pGetModuleFileNameExW)(hProcess, NULL, moduleName, nameLen);
			if (apiResult != 0)
			{
				//  convert to UTF-8
				int len = wcslen(moduleName);
				int convertStrResult = WideCharToMultiByte(CP_UTF8, 0, moduleName, len, utf8ModuleName, nameLen, 0, 0);

				DebugLogToFile("IKUtil::ExePathFromWindowNT64 - GetModuleFileNameExW - UTF8 FileName: [%s] ConvertResult: [%d]", utf8ModuleName, convertStrResult);
			}
			else 
			{
				//DebugLogToFile("IKUtil::ExePathFromWindowNT64 - GetModuleFileNameExW: Failed - Error: [%d]", ::GetLastError());

				// probably a 64-bit OS
				// if this is Windows Vista and above, try QueryFullProcessImageNameW
				// else try GetProcessImageFileName
				apiResult = 0;
				if (pQueryFullProcessImageNameW != NULL)
				{
					apiResult = pQueryFullProcessImageNameW(hProcess, 0, moduleName, &nameLen);

					if (apiResult != 0)
					{
						//  convert to UTF-8
						int len = wcslen(moduleName);
						int convertStrResult = WideCharToMultiByte(CP_UTF8, 0, moduleName, len, utf8ModuleName, 1024, 0, 0);

						DebugLogToFile("IKUtil::ExePathFromWindowNT64 - QueryFullProcessImageName - UTF8 FileName: [%s] ConvertResult: [%d]", utf8ModuleName, convertStrResult);
					}
					else 
					{
						DebugLogToFile("IKUtil::ExePathFromWindowNT64 - QueryFullProcessImageName: Failed - Error: [%d]", ::GetLastError());
					}
				}
				// last chance is GetProcessImageFileName, probably Windows XP 64 bit
				if ((apiResult == 0) && (pGetProcessImageFileNameW != NULL))
				{
					apiResult = pGetProcessImageFileNameW(hProcess, moduleName, 1023);
					if (apiResult != 0)
					{
						// the result is in Device form not drive form, i.e.:
						// \Device\HarddiskVolume3\Windows\explorer.exe

						//  convert to UTF-8
						int len = wcslen(moduleName);
						int convertStrResult = WideCharToMultiByte(CP_UTF8, 0, moduleName, len, utf8ModuleName, 1024, 0, 0);

						DebugLogToFile("IKUtil::ExePathFromWindowNT64 - GetProcessImageFileName - UTF8 FileName: [%s] ConvertResult: [%d]", utf8ModuleName, convertStrResult);
					}
					else 
					{
						DebugLogToFile("IKUtil::ExePathFromWindowNT64 - GetProcessImageFileName - Failed - Error: [%d]", ::GetLastError());
					}
				}
			}

			//  close the process handle
			CloseHandle(hProcess);
			// set the final result in the out parameter
			if (apiResult != 0)
			{
				//  if it's not NTVDM, we're done
				IKString s(utf8ModuleName);
				IKString s2 = s.Right(9);
				if (s2.CompareNoCase(TEXT("NTVDM.EXE")) != 0)
				{
					IKString::strcpy(finalExePath, utf8ModuleName);
				}
			}
			else 
			{
				//  if we're here, just use the window text
				::GetWindowText(hwndWindow, finalExePath, 1024 );
			}
		} // OpenProcess failed
		else 
		{
			//  if we're here, just use the window text
			::GetWindowText(hwndWindow, finalExePath, 1024 );

			DebugLogToFile("IKUtil::ExePathFromWindowNT64 - GetWindowText - FileName: [%s]", finalExePath);
		}
	}

	DebugLogToFile("IKUtil::ExePathFromWindowNT64 - Final Exe Path: [%s]", finalExePath);

	return;
}

static void ExePathFromWindowNT32(HWND hwndWindow, TCHAR* wname )
{
	//DebugLogToFile("IKUtil::ExePathFromWindowNT32 - WindowHwnd: [%d (0x%x)]", hwndWindow, hwndWindow);

	//  declarations of PSAPI functions
	typedef DWORD (WINAPI *GetModuleFileNameExProc)(HANDLE, HMODULE, LPCSTR, DWORD);
	typedef BOOL (WINAPI *PFNENUMPROCESSMODULES)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);

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
			pGetModuleFileNameEx = (GetModuleFileNameExProc) ::GetProcAddress(hInstPSAPI, "GetModuleFileNameExA");
			pfnEnumProcessModules = (PFNENUMPROCESSMODULES) ::GetProcAddress( hInstPSAPI, "EnumProcessModules" );

			//DebugLogToFile("IKUtil::ExePathFromWindowNT32 - PSAPI Library Functions Loaded");
		}
		bLoaded = true;
	}

	//  if no pointers, use window text.
	if ((pGetModuleFileNameEx == NULL) || (pfnEnumProcessModules == NULL))
	{
		::GetWindowText(hwndWindow, wname, 1024);
		return;
	}

	//  get process ID from window handle.
	DWORD threadID;
	DWORD dwPID;
	threadID = ::GetWindowThreadProcessId(hwndWindow, &dwPID);

	//  get process handle
	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, dwPID);

	//DebugLogToFile("IKUtil::ExePathFromWindowNT32 - Process Handle: [%d (0x%x)] TID: [%d (0x%x)] PID: [%d (0x%x)]", hProcess, hProcess, threadID, threadID, dwPID, dwPID);

	//  enumerate the modules in this process to get the module handle.
	HMODULE hModuleArray[1024];
	DWORD cbNeeded;
	BOOL result = pfnEnumProcessModules(hProcess, hModuleArray, sizeof(hModuleArray), &cbNeeded);

	//  get module name
	TCHAR moduleName[1024];
	DWORD c = (*pGetModuleFileNameEx)(hProcess, hModuleArray[0], moduleName, 1024);

	//DebugLogToFile("IKUtil::ExePathFromWindowNT32 - GetModuleFileNameEx (ANSI): [%s]", moduleName);

	//  convert to UNICODE and back to UTF-8
	int len = IKString::strlen(moduleName);
	WCHAR wide[1024];
	result = MultiByteToWideChar(CP_ACP, 0, moduleName, len, wide, 1024);
	wide[result] = 0;

	int result2 = WideCharToMultiByte(CP_UTF8, 0, wide, result, moduleName, 1024, 0, 0);
	moduleName[result2] = 0;

	//DebugLogToFile("IKUtil::ExePathFromWindowNT32 - GetModuleFileNameEx (UTF8): [%s] ConvertResult: [%d]", moduleName, result2);

	//  close the handles
	CloseHandle(hProcess);

	//  if it's not NTVDM, we're done
	IKString s(moduleName);
	IKString s2 = s.Right(9);
	if (s2.CompareNoCase(TEXT("NTVDM.EXE"))!=0)
	{
		IKString::strcpy(wname, moduleName);

		DebugLogToFile("IKUtil::ExePathFromWindowNT32 - Final Exe Path: [%s]", wname);
		return;
	}

	//  it's a 16-bit program, so enumerate all of those
	//  to find the one we want.

	static INT (WINAPI *lpfVDMEnumTaskWOWEx)(DWORD, TASKENUMPROCEX  fp, LPARAM ) = NULL;
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
		myEnumData data = {hwndWindow, "", false};
		lpfVDMEnumTaskWOWEx(dwPID, (TASKENUMPROCEX) Enum16A, (LPARAM) &data);
		if (data.bFound)
		{
			IKString::strcpy(wname, data.name);
			return;
		}
	}

	//  if we're here, just use the window text
	::GetWindowText(hwndWindow, wname, 1024);

	DebugLogToFile("IKUtil::ExePathFromWindowNT32 - Final Exe Path: [%s]", wname);

	return;
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

static void GetWindowFileName(HWND hwnd, TCHAR *wname)
{
	DebugLogToFile("IKUtil::GetWindowFileName start -  WindowHwnd: [%d (0x%x)]", hwnd, hwnd);
	//  no name to start
	wname[0] = 0;

	//  good handle?
	IKASSERT(hwnd);
	if(!hwnd)
	{
		return;
	}

	//  get process ID from the window handle
	//DWORD idProc;
	//::GetWindowThreadProcessId ( hwnd, &idProc );

	//  what OS are we?
	OSVERSIONINFO vInfo;
	vInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);	
	GetVersionEx(&vInfo);
	static int is64BitOS = - 1; 
	//  use two different procedures depending on the OS.
	if (vInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (is64BitOS == -1)
		{
			WCHAR pgmFilesX86[1024] = {0};
			DWORD apiResult = ::ExpandEnvironmentStringsW(L"%ProgramFiles(x86)%%ProgramW6432%", pgmFilesX86, 1023);
			is64BitOS = (apiResult <= 1) ? 0 : 1; 
		}
		if (is64BitOS == 0) 
		{
			ExePathFromWindowNT32(hwnd, wname);
		}
		else 
		{
			ExePathFromWindowNT64(hwnd, wname);
		}
	}
	else
	{
		ExePathFromWindow9x ( hwnd, wname );
	}

	DebugLogToFile("IKUtil::GetWindowFileName - Final Path: [%s]", wname);
}


IKString IKUtil::GetCurrentApplicationPath()
{
	DebugLogToFile("IKUtil::GetCurrentApplicationPath start");

	HWND hwnd = ::GetForegroundWindow();
	if (hwnd != NULL)
	{
		TCHAR strFileName[1024];
		GetWindowFileName(hwnd, strFileName);

		IKString s = strFileName;
		IKUtil::GetTrueFileName(s);

		DebugLogToFile("IKUtil::GetCurrentApplicationPath - Current Application Path: [%s]", (TCHAR*)s);

		return s;
	}

	return TEXT("");
}

void IKUtil :: StripFileName ( IKString &str,
	bool bStripPath /*= true */,
	bool bStripExtension /*= true */ )
{
	if ( bStripPath )
	{
		int lastMacSep = str.ReverseFind(IKUtil::GetMacPathDelimiter());
		int lastWinSep = str.ReverseFind(IKUtil::GetWinPathDelimiter());
		int lastSeparator = lastMacSep;
		if (lastWinSep > lastSeparator)
		{
			lastSeparator = lastWinSep;
		}

		if (lastSeparator != -1)
		{
			str = str.Right(str.GetLength() - lastSeparator - 1);
		}
	}
	
	if ( bStripExtension )
	{
		int lastPeriod = str.ReverseFind ( '.' );
		if (lastPeriod != -1)
		{
			str = str.Left(lastPeriod);
		}
	}
}

//  statics for logging;
static TCHAR buffer[4096];
static int theEnd = 0;
static int opened = -1;
static IKFile f;
static unsigned int lastFlush = 0;

void IKUtil::LogString(TCHAR *string)
{
	//  make a string for outputting
	IKString out;

	//  append the time of day
	out += GetTime();
	out += TEXT(" ");

	//  set the start time if this is the first call.
	static unsigned int startTime = 0;
	if (startTime==0)
	{
		startTime = GetCurrentTimeMS();
	}

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
	{
		out += TEXT("0");
	}
	if(s2.GetLength()==2)
	{
		out += TEXT("0");
	}
	out += s2;
	out += TEXT(" ");

	//  finally, tack on the string given.
	out += string;

	//  if debugging, output to screen
#ifdef _DEBUG
	IKString display=out; 
	display += TEXT("\n"); 
	OutputDebugString((TCHAR *)display);
#endif

	//  get file to log
	//  no log to file is name not given.
	IKString name = DATAS(TEXT("Debug_Log_File"),TEXT(""));
	if (name.IsEmpty())
	{
		return;
	}

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
				{
					f.MarkAsUnicode();
				}
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

	IKString::strcpy((TCHAR *)&(buffer[theEnd]),TEXT("\r\n"));
	theEnd += 2;
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
#ifdef UNICODE
		nc = nc * 2;
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
	{
		return result;
	}

	TCHAR fileName[MAX_PATH];
	GetModuleFileName(NULL, fileName, MAX_PATH - 1);
	result = fileName;

	return result;
}


bool IKUtil::RunningUnderOSX ()
{
	return false;
}

bool IKUtil::RunningUnderClassic ()
{
	return false;
}

static UINT LaunchRoutine (LPVOID pParam)
{	
	TCHAR *pstrFile = (TCHAR *) pParam;

	if (pParam)
	{
		Sleep(100);  //  I do not know why.

		int nResult = (int) ShellExecute ( GetDesktopWindow(), TEXT("Open"), (TCHAR *)pParam, TEXT(""), TEXT("C:\\"), SW_SHOWNORMAL);
	}

	return 0;
}

void IKUtil::LaunchFile(IKString filename)
{
	//  start the launch thread
	TCHAR *pstrFile = new TCHAR[255];
	IKString::strcpy(pstrFile,filename);

	DWORD ThreadID;
	HANDLE h = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)LaunchRoutine, (void *)pstrFile, 0, &ThreadID);
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
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof ( osvi );
	::GetVersionEx( &osvi );
	if (osvi.dwMajorVersion >= 5)  //  Windows 2k is first version 5
	{
		return true;
	}
	return false;
}

bool IKUtil::IsWinVistaOrGreater()
{
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof ( osvi );
	::GetVersionEx( &osvi );
	if (osvi.dwMajorVersion>=6)  //  Vista is first version 6
	{
		return true;
	}
	return false;
}

int IKUtil::GetCodePage()
{
	//  get the code page once
	static int codepage = -1;
	if (codepage == -1)
	{
		//  get locale
		TCHAR sysLocale[10] = {0};
		TCHAR userLocale[10] = {0};
		int bytesInSystemLocale = 0;
		int bytesInUserLocale = 0;

		bytesInSystemLocale = ::GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_ILANGUAGE, sysLocale, sizeof(sysLocale)/sizeof(TCHAR));
		if (bytesInSystemLocale != 0)
		{
			DebugLogToFile("IKUtils::GetCodePage - SystemLocale=[%s]", sysLocale);
		}

		bytesInUserLocale = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILANGUAGE, userLocale, sizeof(userLocale)/sizeof(TCHAR) );
		if (bytesInUserLocale != 0)
		{
			DebugLogToFile("IKUtils::GetCodePage - UserLocale=[%s]", userLocale);
		}

		if (bytesInSystemLocale != 0)
		{
			// for the supported languages of IKUSB there are two code pages:
			// 1251 for Russian - locales 0019 or 0419
			// 1252 for English, French Canadian, Brazilian Portuguese and Spanish
			IKString key("codepage_for_locale_");
			key += sysLocale;
			IKString val = DATAS(key, TEXT("0"));
			codepage = StringToInt(val);

			DebugLogToFile("IKUtils::GetCodePage end.  - SystemLocale - [%s]=[%d]", (LPTSTR)key, codepage);
		}
		else if (bytesInUserLocale != 0)
		{
			// for the supported languages of IKUSB there are two code pages:
			// 1251 for Russian - locales 0019 or 0419
			// 1252 for English, French Canadian, Brazilian Portuguese and Spanish
			IKString key("codepage_for_locale_");
			key += userLocale;
			IKString val = DATAS(key, TEXT("0"));
			codepage = StringToInt(val);
			DebugLogToFile("IKUtils::GetCodePage end.  - UserLocale - [%s]=[%d]", (LPTSTR)key, codepage);
		}
		else
		{
			// use the default Windows ANSI code page
			codepage = 0;
			DebugLogToFile("IKUtils::GetCodePage end.  - Default - [%d]",  CP_ACP);
		}
	}

	//  return it
	return codepage;
}

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

static BOOL CALLBACK MyProcessEnumerator(DWORD dwPID,
										WORD wTask, 
										LPCSTR szProcess, 
										LPARAM lParam) 
{
	MyData *pData = (MyData *) lParam;

	IKString s1((TCHAR *)pData->name);  //  look for this
	IKString s2((TCHAR *)szProcess);    //  in this

	s1.Lower();
	s2.Lower();

	int i = s2.ReverseFind('\\');
	if (i >= 0)
	{
		s2 = s2.Mid(i+1,999);
	}

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

bool IKUtil::IsAppRunning ( char *appname )
{
	return WinIsAppRunning(appname);
}

static IKMap nameSubs;

static void LoadNameSubs()
{
	//  do this once
	static bool bLoaded = false;
	if (bLoaded)
	{
		return;
	}
	bLoaded = true;

	//  get the global list
	IKString filepath = IKUtil::GetPrivateFolder();
	filepath += TEXT("appnames.txt");
	nameSubs.Read(filepath);

	//  merge in the names from pre-installed packs
	IKString preinstallFolder = IKUtil::GetPrivateFolder();
	preinstallFolder += DATAS(TEXT("APP_OVERLAYS_SRC_FOLDER"), TEXT("Application Overlays"));
	preinstallFolder += IKUtil::GetPathDelimiter();

	// find all of the folders at this level.
	IKStringArray folders;
	IKUtil::BuildFolderArray(folders, (TCHAR*)preinstallFolder );

	//  for each folder
	for (int nFolder = 0; nFolder < folders.GetSize(); nFolder++)
	{
		IKString folder = folders[nFolder];
		IKString mapFile = folder;
		mapFile += DATAS(TEXT("APP_OVERLAYS_NAMES_MAP"),TEXT("application display names.txt"));
		nameSubs.Read(mapFile);
	}

	//  OK, dump the nameSubs map
	int n = nameSubs.Count();
	for (int i = 0; i < n; i++)
	{
		IKString key;
		IKString value;
		nameSubs.GetNthPair(i, key, value);
	}

}

IKString IKUtil::GetAppFriendlyName(IKString &path)
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
	for (i = 0; i < n; i++)
	{
		IKString s1;
		IKString s2;
		nameSubs.GetNthPair(i, s1, s2);
		int l = s1.GetLength();

		if (key.Right(l).CompareNoCase(s1) == 0)
		{
			friendly = s2;

			//DebugLogToFile("IKUtil::GetAppFriendlyName end. -  Path=[%s] Substitution FriendlyName: [%s]", (TCHAR*)path, (TCHAR*)friendly);

			return friendly;
		}
	}

	//  get the friendly name from the app
	//  (platform-specific)

	CExecImageVersion v(path);
	friendly = v.GetFileDescription();

	if (!friendly.IsEmpty())
	{	
		//DebugLogToFile("IKUtil::GetAppFriendlyName - using FileDescription: [%s]", (TCHAR*)friendly);
	
		// July 2012 - on Win2K or greater must convert to get any embedded trademark or copyright
		// symbols properly converted for display.
		if (IKUtil::IsWin2KOrGreater())
		{
			////  ANSI to Unicode and back to UTF-8
			//int len = friendly.GetLength();
			//WCHAR wide[1024];
			//int res = MultiByteToWideChar(CP_ACP, 0, friendly, len, wide, 1024);
			//wide[res] = 0;
			//char narrow[1024];
			//int result2 = WideCharToMultiByte(CP_UTF8, 0, wide, res, narrow, 1024, 0, 0);
			//narrow[result2] = 0;
			//friendly = IKString(narrow);

			//DebugLogToFile("IKUtil::GetAppFriendlyName - Converted FriendlyName: [%s]", (TCHAR*)friendly);
		}
	}
	else
	{
		friendly = path;
		IKUtil::StripFileName(friendly, true, true);

		//DebugLogToFile("IKUtil::GetAppFriendlyName - using Stripped Incoming Path[%s]=[%s]", (TCHAR*)path, (TCHAR*)friendly);
	}

	//  see again if there is a substitution to be made
	key = friendly;
	key.EncodeSpaces();
	IKString newName = nameSubs.Lookup(key);
	if (newName.IsEmpty() == false)
	{
		friendly = newName;

		//DebugLogToFile("IKUtil::GetAppFriendlyName - Substituted FriendlyName: [%s]", (TCHAR*)friendly);
	}

	//DebugLogToFile("IKUtil::GetAppFriendlyName end.  Final FriendlyName: [%s]", (TCHAR*)friendly);

	return friendly;
}


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


DWORD GetParentProcessID(DWORD   dwPID)
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


IKString IKUtil::GetParentApp()
{
	IKString s = "";

	OSVERSIONINFO vInfo;
	vInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);	
	GetVersionEx(&vInfo);
	if (vInfo.dwPlatformId != VER_PLATFORM_WIN32_NT)
	{
		s = GetParentApp98();
	}
	else
	{
		s = GetParentAppNT();
	}
	return s;
}


bool IKUtil::IsParentClickit ()
{
	IKString path = GetParentApp();
	if (path.IsEmpty())
	{
		return false;
	}

	IKUtil::StripFileName(path,true,true);
	if (path.CompareNoCase(TEXT("clickit"))==0)
	{
		return true;
	}

	return false;
}

unsigned int IKUtil::GetSysTimeSecs1970()
{
	//  get system time as a file time
	SYSTEMTIME stNow;
	GetSystemTime(&stNow);
	FILETIME ftNow;
	SystemTimeToFileTime(&stNow,&ftNow);

	//  Get January 1, 1970 as a FILETIME.
	SYSTEMTIME stJan1970 = {1970,1,0,1,0,0,0,0};
	FILETIME ftJan1970;
	SystemTimeToFileTime(&stJan1970,&ftJan1970);

	//  make large int for each
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
}

void IKUtil::DisplayAlert(IKString title, IKString message)
{
	//  use Unicode version of MessageBox to display message.
	int len = message.GetLength();
	WCHAR wide[1024];
	int res = MultiByteToWideChar ( CP_UTF8, 0, message, len, wide, 1024);
	wide[res] = 0;

	int len2 = title.GetLength();
	WCHAR widet[1024];
	int res2 = MultiByteToWideChar ( CP_UTF8, 0, title, len2, widet, 1024);
	widet[res2] = 0;

	MessageBoxW(NULL, wide, widet, MB_OK);
}

void IKUtil::GetTrueFileName ( IKString & filename )
{
	TCHAR longPath[512];
	DWORD result = GetLongPathName ( (TCHAR *)filename, longPath, 512 );
	if (result > 0)
	{
		filename = longPath;
	}
}

bool  IKUtil::IsPrimaryMouseDown(void)
{
	bool bResult = (GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON) & 0x8000) != 0;
	return bResult;
}

void IKUtil::BuildFolderArray(IKStringArray &array, const TCHAR * pInitialPath)
{
	BuildFileArray(array, pInitialPath, kFoldersType );
}


void IKUtil::BuildFileArray ( 
	IKStringArray &array,
	const TCHAR * pInitialPath,
	const TCHAR * pExtension /*= kAllFilesType */,
	int fileTypeOptions /* = -1 */ )
{
	bool bLookForFolders = (pExtension == kFoldersType);

	IKString strType;
	if (pExtension == kAllFilesType)
	{
		strType = "*.*";
	}
	else
		if (bLookForFolders)
		{
			strType = "*.*";
		}
		else
		{
			strType = "*.";
			strType += (TCHAR *)pExtension;
		}

	IKString strPath((TCHAR *)pInitialPath);
	if (strPath.GetLength() == 0)
	{
		return;
	}

	if (strPath.GetAt(strPath.GetLength() - 1) != '\\')
	{
		strPath += '\\';
	}

	if (DATAI(TEXT("Unicode_File_Names"),0) != 0)
	{
		IKString findPath = strPath + strType;
		int len = findPath.GetLength();
		WCHAR wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,				// utf-8
							0,						// character-type options
							(const char*)findPath,	// string to map
							len,					// number of bytes in string
							wide,					// wide-character buffer
							255						// size of buffer
							);
		wide[result] = 0;

		WIN32_FIND_DATAW findFileData;
		HANDLE h = ::FindFirstFileW(wide, &findFileData);
		if (h == INVALID_HANDLE_VALUE)
		{
			return;
		}

		while (TRUE)
		{
			// convert the filename in the returned structure back to UTF
			char narrowFileName[1024];
			int result2 = WideCharToMultiByte (CP_ACP, 0, findFileData.cFileName, -1, narrowFileName, 1024, 0, 0);
			narrowFileName[result2] = 0;

			// sss: filter out "." and ".."
			if ((strcmp(narrowFileName, ".") != 0) &&
				(strcmp(narrowFileName, "..") != 0))
			{
				if (bLookForFolders)
				{
					if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						array.Add(IKString(strPath + IKString(narrowFileName)) + IKUtil::GetPathDelimiter());

						DebugLogToFile("IKUtils::BuildFileArray - Unicode - Dir=[%s][%s]", (TCHAR*)strPath, narrowFileName);
					}
				}
				else
				{
					// make sure it is a file
					if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						array.Add(IKString(strPath + IKString(narrowFileName)));

						DebugLogToFile("IKUtils::BuildFileArray - Unicode - File=[%s][%s]", (TCHAR*)strPath, narrowFileName);
					}
				}
			}

			if ( ! ::FindNextFileW(h, &findFileData) )
			{
				// no more, so break out of the loop
				break;
			}
		}

		::FindClose(h);

	}
	else
	{
		WIN32_FIND_DATA findFileData;
		HANDLE h = ::FindFirstFile((TCHAR*)(strPath + strType), &findFileData);
		if (h == INVALID_HANDLE_VALUE)
		{
			return;
		}

		while (TRUE)
		{
			// sss: filter out "." and ".."
			if ((strcmp(findFileData.cFileName, ".") != 0) &&
				(strcmp(findFileData.cFileName, "..") != 0))
			{
				if (bLookForFolders)
				{
					if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						array.Add(IKString(strPath + IKString(findFileData.cFileName)) + IKUtil::GetPathDelimiter());
						
						DebugLogToFile("IKUtils::BuildFileArray - ANSI - Dir=[%s][%s]", (TCHAR*)strPath, findFileData.cFileName);
					}
				}
				else
				{
					// make sure it is a file
					if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						array.Add(IKString(strPath + IKString(findFileData.cFileName)));

						DebugLogToFile("IKUtils::BuildFileArray - ANSI - File=[%s][%s]", (TCHAR*)strPath, findFileData.cFileName);
					}
				}
			}

			if ( ! ::FindNextFile(h, &findFileData) )
			{
				// no more, so break out of the loop
				break;
			}
		}

		::FindClose(h);
	}

	array.Sort();
}


bool IKUtil::IsOverlayMakerInstalled ()
{
	IKString strPath = IKUtil::GetOverlayMakerPath();
	return !strPath.IsEmpty();

}


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


#define kPathSeparator '\\'

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
	ompath += "Overlays";

	//  exists?
	if (!IKFile::FolderExists(ompath))
		return IKString(TEXT(""));

	ompath += kPathSeparator;
	return ompath;
	
}


bool IKUtil::HasClickitAdaptation	( TCHAR * pAppPath )
{
	IKString AppPath = pAppPath;
	IKString path(AppPath);

	//  get this app's simple name
	IKString simple = path;
	IKUtil::StripFileName(simple,true,true);

	//  where is Click-it?
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
}

void IKUtil::SetOneProcessor()
{
	if (IsWin2KOrGreater())
	{

		HANDLE hProcess = GetCurrentProcess();
		DWORD dwProcessAffinityMask, dwSystemAffinityMask;
		GetProcessAffinityMask( hProcess, &dwProcessAffinityMask, &dwSystemAffinityMask );
		SetProcessAffinityMask( hProcess, 1L );// use CPU 0 only
	}
}
