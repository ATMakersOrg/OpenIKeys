// coinstaller.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "resource.h"

#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <tchar.h>
#include <initguid.h>
#include <devguid.h>
#include <regstr.h>
#include <setupapi.h>

//
//  Debug printing
//

#if 1 

static void _cdecl DbgPrint(LPCWSTR lpszFormat, ...)
{

		va_list args;
		va_start(args, lpszFormat);

		int nBuf;
		WCHAR szBuffer[512];

		nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer) / sizeof(WCHAR), lpszFormat, args);


		OutputDebugStringW(szBuffer);
		va_end(args);

}


#define CoTrace(_x_) DbgPrint(TEXT("Coinstaller: ")); DbgPrint _x_;

#else

#define CoTrace(_x_)

#endif




//
// Constants
//

#define IKUSB_SETUP_EXE     L"\\setup.exe"
#define IKUSB_SETUP_PATH    (IKUSB_SETUP_EXE)
#define IKUSB_MEDIA_SOURCE_ID   1

//
// Globals
//
HMODULE g_hInstance;
WCHAR   SetupExeName[] = IKUSB_SETUP_EXE;

//
// Structures
//
typedef struct _VALUEADDWIZDATA {
	BOOL  AppInstallAttempted;          // Have we previously attempted to install app?
	WCHAR MediaRootDirectory[MAX_PATH]; // Fully-qualified path to root of install media
	WCHAR MediaDiskName[LINE_LEN];      // Name of media to prompt for (or empty string)
	WCHAR MediaTagFile[MAX_PATH];       // Tagfile identifying removable media (or empty string)
} VALUEADDWIZDATA, *LPVALUEADDWIZDATA;


//
// Function prototypes
//
INT_PTR
CALLBACK 
ValueAddDlgProc(
	IN HWND     hwndDlg,    
	IN UINT     uMsg,       
	IN WPARAM   wParam, 
	IN LPARAM   lParam  
	);

BOOL
InstallIKUSBApp(
	IN HWND    hwndDlg, 
	IN LPCWSTR PathToSetupExe
	);

UINT
ValueAddPropSheetPageProc(
	IN HWND hwnd,
	IN UINT uMsg,
	IN LPPROPSHEETPAGE ppsp
	);

BOOL
GetMediaRootDirectory(
	IN  HDEVINFO          DeviceInfoSet,  
	IN  PSP_DEVINFO_DATA  DeviceInfoData, 
	OUT PWSTR            *MediaRootDirectory, 
	OUT PWSTR            *MediaDiskName,
	OUT PWSTR            *MediaTagFile
	);

HPROPSHEETPAGE
GetValueAddSoftwareWizPage(
	IN PCWSTR MediaRootDirectory,
	IN PCWSTR MediaDiskName       OPTIONAL,
	IN PCWSTR MediaTagFile        OPTIONAL
	);

VOID
SetDeviceFriendlyName(
	IN HDEVINFO         DeviceInfoSet,
	IN PSP_DEVINFO_DATA DeviceInfoData
	);


VOID
LaunchSetup();

BOOL APIENTRY DllMain( HMODULE hModule, 
					   DWORD  ul_reason_for_call, 
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			CoTrace((TEXT("DLL_PROCESS_ATTACH")));
			g_hInstance = hModule;
			break;

		case DLL_THREAD_ATTACH:
			CoTrace((TEXT("DLL_THREAD_ATTACH")));
			break;

		case DLL_THREAD_DETACH:
			CoTrace((TEXT("DLL_THREAD_DETACH")));
			break;

		case DLL_PROCESS_DETACH:
			CoTrace((TEXT("DLL_PROCESS_DETACH")));
			g_hInstance = NULL;
			break;

	}
	return TRUE;
}

static void DoWin98 ()
{
	MessageBox(NULL,
		TEXT("IntelliKeys USB requires the installation of additional software for it to function properly.  Please launch the Setup program found on your installation CD."),
		TEXT("IntelliKeys USB"),
		MB_OK);
}

extern "C"
DWORD CALLBACK IKUSBCoInstaller (
					IN     DI_FUNCTION               InstallFunction,
					IN     HDEVINFO                  DeviceInfoSet    OPTIONAL,
					IN     PSP_DEVINFO_DATA          DeviceInfoData   OPTIONAL,
					IN OUT PCOINSTALLER_CONTEXT_DATA Context          OPTIONAL
			   )
{
	DWORD result = NO_ERROR;
	SP_NEWDEVICEWIZARD_DATA NewDeviceWizardData;
	HKEY hKey;
	DWORD KeyDisposition;
	DWORD Err;
	DWORD RegDataType;
	DWORD RequiredSize;
	//DWORD UserPrompted;
	DWORD Updating;
	PWSTR MediaRootDirectory, MediaDiskName, MediaTagFile;


	//  get the system version number
	OSVERSIONINFO osvi;
	::ZeroMemory(&osvi, sizeof( osvi ));
	osvi.dwOSVersionInfoSize = sizeof ( osvi );
	::GetVersionEx( &osvi );

	//  do something else if it's windows 98
	
	if (osvi.dwPlatformId!=VER_PLATFORM_WIN32_NT)
	{
		DoWin98();
		return 0;
	}
	else
	{
		
	}


	switch (InstallFunction)
	{

	case DIF_INSTALLDEVICE: 
		CoTrace((TEXT("DIF_INSTALLDEVICE")); 
		SetDeviceFriendlyName(DeviceInfoSet, DeviceInfoData));
		break;

	case DIF_NEWDEVICEWIZARD_FINISHINSTALL:
		CoTrace((TEXT("DIF_NEWDEVICEWIZARD_FINISHINSTALL")));


		//
		// We should always get called with a valid DeviceInfoData, but 
		// just in case we don't we will bail out right away.
		//
		if(DeviceInfoData == NULL) 
		{
			CoTrace((TEXT("DeviceInfoData is NULL")));
			break;
		}

		//  don't do a finish wizard page if
		//  the user is in the middle of a software-first install.

		if(ERROR_SUCCESS != RegCreateKeyEx(
								HKEY_LOCAL_MACHINE,
								TEXT("SOFTWARE\\IntelliTools\\IntelliKeys USB"),
								0,
								NULL,
								REG_OPTION_NON_VOLATILE,
								KEY_READ | KEY_WRITE,
								NULL,
								&hKey,
								&KeyDisposition)) 
		{
			//
			// If we can't create this key, then we can't install the 
			// software, so don't bother the user with any finish install 
			// wizard pages.
			//
			CoTrace((TEXT("cannot create reg key %s"),TEXT("SOFTWARE\\IntelliTools\\IntelliKeys USB")));
			break;
		}

		//
		// Look for non-zero "Updating" value entry to indicate that 
		// the user has previously responded to question about installation 
		// of value-add software.
		//
		RequiredSize = sizeof(Updating);

		Err = RegQueryValueEx(hKey,
							  TEXT("Updating"),
							  NULL,
							  &RegDataType,
							  (PBYTE)&Updating,
							  &RequiredSize
							 );

		if(Err != ERROR_SUCCESS) 
		{
			CoTrace((TEXT("Updating is zero")));
			Updating = 0;
		}
		RegCloseKey(hKey);

		if (Updating != 0)
		{
			CoTrace((TEXT("Updating is not zero")));
			break;
		}

		//  not if the software is already installed

		TCHAR path[255];
		GetPrivateProfileString ( TEXT("IntelliKeys USB"), TEXT("path"), TEXT("c:\\itools\\intellikeys usb\\"), path, 255, TEXT("itools_x.ini") );
		wcscat(path,TEXT("driver package\\ikusb3.inf"));

		WIN32_FIND_DATA FindFileData;
		if (FindFirstFile(path,
			&FindFileData)!=INVALID_HANDLE_VALUE)
		{
			CoTrace((TEXT("IKUSB software already installed.")));
			break;
		}
		else
		{
			CoTrace((TEXT("IKUSB software not yet installed.")));
		}


#if 0
		//
		// Only supply a finish-install wizard page the first time...
		//
		if(ERROR_SUCCESS != RegCreateKeyEx(
								HKEY_LOCAL_MACHINE,
								TEXT("SOFTWARE\\IntelliTools\\IntelliKeys USB"),
								0,
								NULL,
								REG_OPTION_NON_VOLATILE,
								KEY_READ | KEY_WRITE,
								NULL,
								&hKey,
								&KeyDisposition)) 
		{
			//
			// If we can't create this key, then we can't install the 
			// software, so don't bother the user with any finish install 
			// wizard pages.
			//
			CoTrace((TEXT("cannot create reg key %s"),TEXT("SOFTWARE\\IntelliTools\\IntelliKeys USB")));
			break;
		}

		//
		// Look for non-zero "User Prompted" value entry to indicate that 
		// the user has previously responded to question about installation 
		// of value-add software.
		//
		RequiredSize = sizeof(UserPrompted);

		Err = RegQueryValueEx(hKey,
							  TEXT("User Prompted"),
							  NULL,
							  &RegDataType,
							  (PBYTE)&UserPrompted,
							  &RequiredSize
							 );

		if(Err != ERROR_SUCCESS) 
		{
			CoTrace((TEXT("UserPrompted is zero")));
			UserPrompted = 0;
		}

		if(UserPrompted) 
		{
			//
			// We asked the user this question before--don't bother them
			// again.
			//
			RegCloseKey(hKey);
			break;

		} 
		else 
		{
			CoTrace(TEXT("Setting UserPrompted to One"));

			UserPrompted = 1;
			RegSetValueEx(hKey, 
						  TEXT("User Prompted"), 
						  0, 
						  REG_DWORD, 
						  (PBYTE)&UserPrompted, 
						  sizeof(UserPrompted)
						 );

			RegCloseKey(hKey);
		}
#endif


		ZeroMemory(&NewDeviceWizardData, sizeof(NewDeviceWizardData));
		NewDeviceWizardData.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);

		if(SetupDiGetClassInstallParams(DeviceInfoSet,
										DeviceInfoData,
										(PSP_CLASSINSTALL_HEADER)&NewDeviceWizardData,
										sizeof(SP_NEWDEVICEWIZARD_DATA),
										NULL)) 
		{
			//
			// First, make sure there's room for us to add a page...
			//
			if(NewDeviceWizardData.NumDynamicPages >= MAX_INSTALLWIZARD_DYNAPAGES) 
			{
				CoTrace((TEXT("new dev wizard finish fails 2")));
				break;
			}

			//
			// Retrieve the location of the source media based on the INF 
			// we're installing from.
			//
			if(!GetMediaRootDirectory(DeviceInfoSet,  
									  DeviceInfoData,
									  &MediaRootDirectory,
									  &MediaDiskName,
									  &MediaTagFile)) 
			{
				//
				// We couldn't figure out where the source media is, so we
				// can't offer any value-added software to the user.
				//
				CoTrace((TEXT("new dev wizard finish fails 3")));
				break;
			}

			CoTrace((TEXT("about to call GetValueAddSoftwareWizPage")));

			NewDeviceWizardData.DynamicPages[NewDeviceWizardData.NumDynamicPages] =
				GetValueAddSoftwareWizPage(MediaRootDirectory,
										   MediaDiskName,
										   MediaTagFile
										  );

			CoTrace((TEXT("just back from GetValueAddSoftwareWizPage")));

			//
			// We don't need the media strings any more.
			//
			GlobalFree(MediaRootDirectory);

			if(MediaDiskName) 
			{
				GlobalFree(MediaDiskName);
			}

			if(MediaTagFile) 
			{
				GlobalFree(MediaTagFile);
			}

			if(NewDeviceWizardData.DynamicPages[NewDeviceWizardData.NumDynamicPages] != NULL) 
			{
				NewDeviceWizardData.NumDynamicPages++;
			}
			else
			{
				CoTrace((TEXT("new dev wizard finish fails 4")));
			}

			CoTrace((TEXT("about to call SetupDiSetClassInstallParams")));
			BOOL b = SetupDiSetClassInstallParams(DeviceInfoSet,
										 DeviceInfoData,
										 (PSP_CLASSINSTALL_HEADER)&NewDeviceWizardData,
										 sizeof(SP_NEWDEVICEWIZARD_DATA)
										);
			if (!b)
			{
				DWORD e = GetLastError();
				CoTrace((TEXT("new dev wizard finish fails 5, error = "), e));
			}

		}
		else
		{
			CoTrace((TEXT("new dev wizard finish fails 1")));
		}

		break;

	case DIF_REMOVE:
		CoTrace((TEXT("DIF_REMOVE")));
		break;

	case DIF_SELECTDEVICE:
		CoTrace((TEXT("DIF_SELECTDEVICE")));
		break;

	case DIF_ASSIGNRESOURCES:
		CoTrace((TEXT("DIF_ASSIGNRESOURCES")));
		break;

	case DIF_PROPERTIES:
		CoTrace((TEXT("DIF_PROPERTIES")));
		break;

	case DIF_FIRSTTIMESETUP:
		CoTrace((TEXT("DIF_FIRSTTIMESETUP")));
		break;

	case DIF_FOUNDDEVICE:
		CoTrace((TEXT("DIF_FOUNDDEVICE")));
		break;

	case DIF_SELECTCLASSDRIVERS:
		CoTrace((TEXT("DIF_SELECTCLASSDRIVERS")));
		break;

	case DIF_VALIDATECLASSDRIVERS:
		CoTrace((TEXT("DIF_VALIDATECLASSDRIVERS")));
		break;

	case DIF_INSTALLCLASSDRIVERS:
		CoTrace((TEXT("DIF_INSTALLCLASSDRIVERS")));
		break;

	case DIF_CALCDISKSPACE:
		CoTrace((TEXT("DIF_CALCDISKSPACE")));
		break;

	case DIF_DESTROYPRIVATEDATA:
		CoTrace((TEXT("DIF_DESTROYPRIVATEDATA")));
		break;

	case DIF_VALIDATEDRIVER:
		CoTrace((TEXT("DIF_VALIDATEDRIVER")));
		break;

	case DIF_MOVEDEVICE:
		CoTrace((TEXT("DIF_MOVEDEVICE")));
		break;

	case DIF_DETECT:
		CoTrace((TEXT("DIF_DETECT")));
		break;

	case DIF_INSTALLWIZARD:
		CoTrace((TEXT("DIF_INSTALLWIZARD")));
		break;

	case DIF_DESTROYWIZARDDATA:
		CoTrace((TEXT("DIF_DESTROYWIZARDDATA")));
		break;

	case DIF_PROPERTYCHANGE:
		CoTrace((TEXT("DIF_PROPERTYCHANGE")));
		break;

	case DIF_ENABLECLASS:
		CoTrace((TEXT("DIF_ENABLECLASS")));
		break;

	case DIF_DETECTVERIFY:
		CoTrace((TEXT("DIF_DETECTVERIFY")));
		break;

	case DIF_INSTALLDEVICEFILES:
		CoTrace((TEXT("DIF_INSTALLDEVICEFILES")));
		break;

	case DIF_ALLOW_INSTALL:
		CoTrace((TEXT("DIF_ALLOW_INSTALL")));
		break;

	case DIF_SELECTBESTCOMPATDRV:
		CoTrace((TEXT("DIF_SELECTBESTCOMPATDRV")));
		break;

	case DIF_REGISTERDEVICE:
		CoTrace((TEXT("DIF_REGISTERDEVICE")));
		break;

	case DIF_NEWDEVICEWIZARD_PRESELECT:
		CoTrace((TEXT("DIF_NEWDEVICEWIZARD_PRESELECT")));
		break;

	case DIF_NEWDEVICEWIZARD_SELECT:
		CoTrace((TEXT("DIF_NEWDEVICEWIZARD_SELECT")));
		break;

	case DIF_NEWDEVICEWIZARD_PREANALYZE:
		CoTrace((TEXT("DIF_NEWDEVICEWIZARD_PREANALYZE")));
		break;

	case DIF_NEWDEVICEWIZARD_POSTANALYZE:
		CoTrace((TEXT("DIF_NEWDEVICEWIZARD_POSTANALYZE")));
		break;

	case DIF_INSTALLINTERFACES:
		CoTrace((TEXT("DIF_INSTALLINTERFACES")));
		break;

	case DIF_DETECTCANCEL:
		CoTrace((TEXT("DIF_DETECTCANCEL")));
		break;

	case DIF_REGISTER_COINSTALLERS:
		CoTrace((TEXT("DIF_REGISTER_COINSTALLERS")));
		break;

	case DIF_ADDPROPERTYPAGE_ADVANCED:
		CoTrace((TEXT("DIF_ADDPROPERTYPAGE_ADVANCED")));
		break;

	case DIF_ADDPROPERTYPAGE_BASIC:
		CoTrace((TEXT("DIF_ADDPROPERTYPAGE_BASIC")));
		break;

	case DIF_TROUBLESHOOTER:
		CoTrace((TEXT("DIF_TROUBLESHOOTER")));
		break;

	case DIF_POWERMESSAGEWAKE:
		CoTrace((TEXT("DIF_POWERMESSAGEWAKE")));
		break;

	default:
		CoTrace((TEXT("?????")));
		break;

	}
	
	return result;
}


VOID
SetDeviceFriendlyName(
	IN HDEVINFO         DeviceInfoSet,
	IN PSP_DEVINFO_DATA DeviceInfoData
	)

/*++

Routine Description:

	This function retrieves the (localized) string format (suitable for use
	with FormatMessage) to be used in generating the device's friendly name, 
	then constructs the name using that format in concert with the device's UI 
	number.

Arguments:

	DeviceInfoSet - Supplies a handle to the device information set containing
		the element for which an INF driver node is currently selected.

	DeviceInfoData - Supplies the address of a device information element for
		which an INF driver node is currently selected.

Return Value:

	none

--*/

{
	SP_DRVINFO_DATA DriverInfoData;
	SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
	HINF hInf;
	WCHAR InfSectionWithExt[255];
	WCHAR FormatString[LINE_LEN];
	INFCONTEXT InfContext;
	DWORD UINumber;
	PVOID FriendlyNameBuffer;
	DWORD FriendlyNameBufferSize;

	CoTrace((TEXT("SetDeviceFriendlyName called.")));

	if (DeviceInfoData==NULL)
	{
		CoTrace((TEXT("SetDeviceFriendlyName failed 1.")));
		goto clean0;
	}

	//
	// First, retrieve the format string from the driver's [DDInstall] section.
	//
	DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
	if(!SetupDiGetSelectedDriver(DeviceInfoSet, 
								 DeviceInfoData, 
								 &DriverInfoData))
	{
		//
		// NULL driver install
		//
		CoTrace((TEXT("SetDeviceFriendlyName failed 2.")));
		goto clean0;
	}

	DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
	if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
								   DeviceInfoData,
								   &DriverInfoData,
								   &DriverInfoDetailData,
								   sizeof(DriverInfoDetailData),
								   NULL) &&
	   (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) 
	{
		//
		// Unable to retrieve detail info about selected driver node
		//
		CoTrace((TEXT("SetDeviceFriendlyName failed 3.")));
		goto clean0;
	}

	hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
							NULL,
							INF_STYLE_WIN4,
							NULL
						   );

	if(hInf == INVALID_HANDLE_VALUE) 
	{
		//
		// Couldn't open the INF
		//
		CoTrace((TEXT("SetDeviceFriendlyName failed 4.")));
		goto clean0;
	}

	//
	// Figure out actual (potentially decorated) DDInstall section being used
	// for this install.
	//
	*FormatString = L'\0';  // default to empty string in case error occurs.

	if(SetupDiGetActualSectionToInstall(hInf,
										DriverInfoDetailData.SectionName,
										InfSectionWithExt,
										sizeof(InfSectionWithExt) / sizeof(WCHAR),
										NULL,
										NULL)) 
	{
		CoTrace ((TEXT("looking in INF for %s"), InfSectionWithExt ) );

		if(SetupFindFirstLine(hInf,
							  InfSectionWithExt,
							  L"FriendlyNameFormat",
							  &InfContext)) 
		{

			if(!SetupGetStringField(&InfContext, 
									1, 
									FormatString, 
									sizeof(FormatString) / sizeof(WCHAR),
									NULL)) 
			{
				//
				// Failed to retrieve format string into our buffer.  Make sure
				// our buffer still contains an empty string.
				//
				*FormatString = L'\0';
				CoTrace((TEXT("SetDeviceFriendlyName failed 5.")));
			}
		}
		else
		{
			CoTrace((TEXT("SetDeviceFriendlyName failed 6.")));
		}
	}
	else
	{
		CoTrace((TEXT("SetDeviceFriendlyName failed 7.")));
	}

	SetupCloseInfFile(hInf);

	CoTrace ( (TEXT("format string = %s"), FormatString) );

	if(!(*FormatString))
	{
		goto clean0;
	}

	//
	// Now retrieve the device's UI number, which is actually the serial number
	// used by the bus driver.
	//
	if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet, 
										DeviceInfoData,
										SPDRP_UI_NUMBER,
										NULL,
										(PBYTE)&UINumber,
										sizeof(UINumber),
										NULL)) 
	{
		
		FriendlyNameBufferSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
											   | FORMAT_MESSAGE_FROM_STRING
											   | FORMAT_MESSAGE_ARGUMENT_ARRAY,
											   FormatString,
											   0,
											   0,
											   (LPWSTR)&FriendlyNameBuffer,
											   0,
											   (va_list *)&UINumber
											  );

		CoTrace ( (TEXT("FriendlyNameBufferSize is %d"), FriendlyNameBufferSize));

		if(FriendlyNameBufferSize) 
		{

			CoTrace ( (TEXT("friendly name is %s"), FriendlyNameBuffer));

			SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
											 DeviceInfoData,
											 SPDRP_FRIENDLYNAME,
											 (PBYTE)FriendlyNameBuffer,
											 (FriendlyNameBufferSize + 1) * sizeof(WCHAR)
											);

			LocalFree(FriendlyNameBuffer);
		}
		else
		{
			CoTrace((TEXT("SetDeviceFriendlyName failed 8.")));
		}
	}
	else
	{
		CoTrace((TEXT("SetDeviceFriendlyName failed 97.")));
	}


clean0:
	;   // nothing to do
}




BOOL
GetMediaRootDirectory(
	IN  HDEVINFO          DeviceInfoSet,  
	IN  PSP_DEVINFO_DATA  DeviceInfoData, 
	OUT PWSTR            *MediaRootDirectory, 
	OUT PWSTR            *MediaDiskName,
	OUT PWSTR            *MediaTagFile
	)

/*++

Routine Description:

	This function retrieves the root of the installation media for the INF
	selected in the specified device information element.

	There are two possibilities here:                                
																	 
		1.  INF is on source media.  If so, then just use that path.
		
		2.  INF is already in %windir%\Inf.  This is less likely, because we
			should've previously prompted the user to install software at the
			time the INF was installed.  However, perhaps someone called 
			SetupCopyOEMInf to install the INF without prompting the user for 
			value-add software selection.  Another way you could get into this
			state is if the user previously elected to install the application,
			then subsequently uninstalled it via "Add/Remove Programs". The MSI
			package is configured to delete the UserPrompted value from the 
			registry during uninstall, so if a new toaster is subsequently
			inserted, the user will be prompted once again.
			  
			When the INF is in %windir%\Inf, we need to retrieve the original
			source location from which the INF was installed.  Plug&Play stores
			this information for 3rd-party INF files, but it is not directly
			accessible.  Fortunately, there is an INF DIRID that corresponds to
			this path, so we retrieve the value of that DIRID from the INF in 
			order to ascertain the media root directory.

Arguments:

	DeviceInfoSet - Supplies a handle to the device information set containing
		the element for which an INF driver node is currently selected.

	DeviceInfoData - Supplies the address of a device information element for
		which an INF driver node is currently selected.

	MediaRootDirectory - Supplies the address of a string pointer that, upon
		successful return, will be set to point to a newly-allocated string
		containing the root directory of the setup media.  The caller must free
		this buffer via GlobalFree.
		
		This pointer will be set to NULL upon error.
		
	MediaDiskName - Supplies the address of a string pointer that, upon
		successful return will be set to either:
		 
			1. A newly-allocated string containing the disk name to be used 
			   when prompting for the setup media (when INF is in %windir%\Inf)
			2. NULL (when INF isn't in %windir%\Inf it is presumed to be on
			   source media, hence no prompting is necessary)
			   
		This pointer will be set to NULL upon error.
			   
	MediaTagFile - Supplies the address of a string pointer that, upon
		successful return will be set to either:
		
			1. A newly-allocated string containing the disk tagfile to be used 
			   when prompting for the setup media (when INF is in %windir%\Inf)
			2. NULL (when INF isn't in %windir%\Inf it is presumed to be on
			   source media, hence no prompting is necessary)

		This pointer will be set to NULL upon error.

Return Value:

	If this function succeeds, the return value is non-zero (TRUE).
	
	If this function fails, the return value is FALSE.

--*/

{
	SP_DRVINFO_DATA DriverInfoData;
	PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData = NULL;
	LPWSTR FileNamePart;
	WCHAR InfDirPath[MAX_PATH];
	BOOL b = FALSE;
	HINF hInf = INVALID_HANDLE_VALUE;
	INFCONTEXT InfContext;
	DWORD PathLength;

	*MediaRootDirectory = NULL;
	*MediaDiskName = NULL;
	*MediaTagFile = NULL;

	//
	// First, retrieve the full path of the INF being used to install this
	// device.
	//
	DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

	if(!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData)) 
	{
		//
		// This shouldn't fail, but if it does, just bail.
		//
		CoTrace((TEXT("GetMediaRootDirectory fails 1")));
		goto clean0;
	}

	//
	// Retrieve the driver info details.  We don't care about the id list at
	// the end, so we can just allocate a buffer for the fixed-size part...
	//
	DriverInfoDetailData = (PSP_DRVINFO_DETAIL_DATA)GlobalAlloc(0, sizeof(SP_DRVINFO_DETAIL_DATA));
	if(!DriverInfoDetailData) {
		goto clean0;
	}
	DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

	if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
								   DeviceInfoData,
								   &DriverInfoData,
								   DriverInfoDetailData,
								   sizeof(SP_DRVINFO_DETAIL_DATA),
								   NULL)
	   && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) 
	{
		//
		// Again, this should never fail, but if it does we're outta here.
		//
		CoTrace((TEXT("GetMediaRootDirectory fails 2")));
		goto clean0;
	}

	*MediaRootDirectory = (PWSTR)GlobalAlloc(0, MAX_PATH * sizeof(WCHAR));
	if(!*MediaRootDirectory) 
	{
		CoTrace((TEXT("GetMediaRootDirectory fails 2a")));
		goto clean0;
	}

	//
	// Strip the INF name off of the path.  (Resultant path will always end
	// with a path separator char ('\\').)
	//
	PathLength = GetFullPathName(DriverInfoDetailData->InfFileName, 
								 MAX_PATH, 
								 *MediaRootDirectory, 
								 &FileNamePart
								);

	if(!PathLength || (PathLength >= MAX_PATH)) 
	{
		CoTrace((TEXT("GetMediaRootDirectory fails 3")));
		goto clean0;
	}

	*FileNamePart = L'\0';

	//
	// Check to see this INF is already in %windir%\Inf.
	//
	PathLength = GetWindowsDirectory(InfDirPath, MAX_PATH);
	//PathLength = GetSystemWindowsDirectory(InfDirPath, MAX_PATH);

	if(!PathLength || (PathLength >= MAX_PATH)) 
	{
		CoTrace((TEXT("GetMediaRootDirectory fails 4")));
		goto clean0;
	}

	//
	// Append INF directory to path (making sure we don't end up with two path
	// separator chars).
	//
	if((InfDirPath[PathLength-1] != L'\\') && (InfDirPath[PathLength-1] != L'/')) 
	{
		lstrcpyn(&(InfDirPath[PathLength]), L"\\Inf\\", MAX_PATH - PathLength);
	} 
	else 
	{
		lstrcpyn(&(InfDirPath[PathLength]), L"Inf\\", MAX_PATH - PathLength);
	}

	if(lstrcmpi(*MediaRootDirectory, InfDirPath)) 
	{
		//
		// The INF isn't in %windir%\Inf, so assume its location is the root of
		// the installation media.  (We don't bother to retrieve the disk name
		// or tagfile name in this case.)
		//
		b = TRUE;
		goto clean0;
	}

	//
	// Since the INF is already in %windir%\Inf, we need to find out where it
	// originally came from.  There is no direct way to ascertain an INF's
	// path of origin, but we can indirectly determine it by retrieving a field
	// from our INF that uses a string substitution of %1% (DIRID_SRCPATH).
	//
	hInf = SetupOpenInfFile(DriverInfoDetailData->InfFileName,
							NULL,
							INF_STYLE_WIN4,
							NULL
						   );

	if(hInf == INVALID_HANDLE_VALUE) 
	{
		CoTrace((TEXT("GetMediaRootDirectory fails 5")));
		goto clean0;
	}

	//
	// Contained within our INF should be a [ToastCoInfo] section with the
	// following entry:
	//
	//     OriginalInfSourcePath = %1%
	//
	// If we retrieve the value (i.e., field 1) of this line, we'll get the
	// full path where the INF originally came from.
	//
	if(!SetupFindFirstLine(hInf, L"IntelliKeysCoInfo", L"OriginalInfSourcePath", &InfContext)) 
	{
		CoTrace((TEXT("GetMediaRootDirectory fails 6")));
		goto clean0;
	}

	if(!SetupGetStringField(&InfContext, 1, *MediaRootDirectory, MAX_PATH, &PathLength) ||
	   (PathLength <= 1)) 
	{
		CoTrace((TEXT("GetMediaRootDirectory fails 7")));
		goto clean0;
	}

	//
	// PathLength we get back includes the terminating null character. Subtract
	// one to get actual length of string.
	//
	PathLength--;

	//
	// Ensure the path we retrieved has a path separator character at the end.
	//
	if(((*MediaRootDirectory)[PathLength-1] != L'\\') && 
	   ((*MediaRootDirectory)[PathLength-1] != L'/')) 
	{
		lstrcpyn(*MediaRootDirectory+PathLength, L"\\", MAX_PATH - PathLength);
	}

	//
	// Now retrieve the disk name and tagfile for our setup media.
	//
	*MediaDiskName = (PWSTR)GlobalAlloc(0, LINE_LEN * sizeof(WCHAR));
	*MediaTagFile = (PWSTR)GlobalAlloc(0, MAX_PATH * sizeof(WCHAR));

	if(!(*MediaDiskName && *MediaTagFile)) 
	{
		CoTrace((TEXT("GetMediaRootDirectory fails 8")));
		goto clean0;
	}

	if(!SetupGetSourceInfo(hInf, 
						   IKUSB_MEDIA_SOURCE_ID,
						   SRCINFO_DESCRIPTION,
						   *MediaDiskName,
						   LINE_LEN,
						   NULL)) 
	{
		CoTrace((TEXT("GetMediaRootDirectory fails 9")));
		goto clean0;
	}

	if(!SetupGetSourceInfo(hInf, 
						   IKUSB_MEDIA_SOURCE_ID,
						   SRCINFO_TAGFILE,
						   *MediaTagFile,
						   MAX_PATH,
						   NULL)) 
	{
		CoTrace((TEXT("GetMediaRootDirectory fails 10")));
		goto clean0;
	}

	b = TRUE;

clean0:
	
	if(hInf != INVALID_HANDLE_VALUE) {
		SetupCloseInfFile(hInf);
	}

	if(DriverInfoDetailData) {
		GlobalFree(DriverInfoDetailData);
	}

	if(!b) {
		if(*MediaRootDirectory) {
			GlobalFree(*MediaRootDirectory);
			*MediaRootDirectory = NULL;
		}
		if(*MediaDiskName) {
			GlobalFree(*MediaDiskName);
			*MediaDiskName = NULL;
		}
		if(*MediaTagFile) {
			GlobalFree(*MediaTagFile);
			*MediaTagFile = NULL;
		}
	}

	return b;
}





HPROPSHEETPAGE
GetValueAddSoftwareWizPage(
	IN PCWSTR MediaRootDirectory,
	IN PCWSTR MediaDiskName       OPTIONAL,
	IN PCWSTR MediaTagFile        OPTIONAL
	)

/*++

Routine Description:

	This function returns a newly-created property sheet page handle that may
	be used in a wizard to allow user-selection of value-added software.
	
	This wizard page is used by both the toaster co-installer (as a finish-
	install wizard page), as well as by the toastva installation application.

Arguments:

	MediaRootDirectory - Supplies the fully-qualified path to the root of the
		installation media.
		
	MediaDiskName - Optionally, supplies the name of the disk to be used when
		prompting the user for source media.  If this parameter is not 
		supplied, the media is assumed to already be present, and no prompting
		occurs
		
	MediaTagFile - Optionally, supplies the tagfile for the disk to be used
		when prompting the user for source media.  If MediaDiskName is not
		specified, this parameter is ignored.

Return Value:

	If this function succeeds, the return value is a newly-created property
	sheet page handle.
	
	If this function fails, the return value is NULL.

--*/

{
	HPROPSHEETPAGE hpsp;
	LPVALUEADDWIZDATA ValueAddWizData; //data for the value-add sw chooser page
	PROPSHEETPAGE page;

	CoTrace((TEXT("GetValueAddSoftwareWizPage called.")));

	ValueAddWizData = (LPVALUEADDWIZDATA)GlobalAlloc(0, sizeof(VALUEADDWIZDATA));

	if(ValueAddWizData) 
	{
		ZeroMemory(ValueAddWizData, sizeof(VALUEADDWIZDATA));
	} 
	else 
	{
		CoTrace((TEXT("GetValueAddSoftwareWizPage fails 1")));
			
		return NULL;
	}

	lstrcpyn(ValueAddWizData->MediaRootDirectory, MediaRootDirectory, MAX_PATH);

	if(MediaDiskName) 
	{
		//
		// The caller wants us to prompt user for media (e.g., installation
		// occurring from %windir%\Inf\OEM<n>.INF, and we want to ensure that
		// our CD is in the drive before launching setup.exe from it).
		//
		lstrcpyn(ValueAddWizData->MediaDiskName, MediaDiskName, LINE_LEN);
		lstrcpyn(ValueAddWizData->MediaTagFile, MediaTagFile, MAX_PATH);
	}

	ZeroMemory(&page, sizeof(PROPSHEETPAGE));

	//
	// Create the sample Wizard Page
	//
	page.dwSize =            sizeof(PROPSHEETPAGE);
	page.dwFlags =           PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE|PSP_USETITLE|PSP_USECALLBACK;
	page.hInstance =         g_hInstance;
	page.pszHeaderTitle =    MAKEINTRESOURCE(IDS_TITLE);
	page.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_SUBTITLE);
	page.pszTemplate =       MAKEINTRESOURCE(IDD_SAMPLE_INSTALLAPP);
	page.pfnDlgProc =        ValueAddDlgProc;
	page.lParam =            (LPARAM)ValueAddWizData;
	page.pfnCallback =       NULL; // (LPFNPSPCALLBACK)ValueAddPropSheetPageProc;

	CoTrace((TEXT("about to create the property sheet")));

	hpsp = CreatePropertySheetPage(&page);

	CoTrace((TEXT("created the property sheet = %x"), hpsp));

	if(!hpsp) 
	{
		CoTrace((TEXT("GetValueAddSoftwareWizPage fails 2")));
		GlobalFree(ValueAddWizData);
	}
	else
	{
		CoTrace((TEXT("property sheet is %x"), hpsp));
	}

	return hpsp;
}



UINT
ValueAddPropSheetPageProc(
	IN HWND hwnd,
	IN UINT uMsg,
	IN LPPROPSHEETPAGE ppsp
	)

/*++

Routine Description:

	This function is the property sheet page procedure, used to free the
	context data associated with the page when it is released.

Arguments:

	hwnd - Supplies a handle to the property page window
	
	uMsg - Supplies the message identifying the action being taken
	
	ppsp - Supplies the PROPSHEETPAGE structure for our page

Return Value:

	This routine always return non-zero (1).

--*/

{
	UNREFERENCED_PARAMETER(hwnd);

	switch(uMsg) 
	{

		case PSPCB_RELEASE :
			CoTrace((TEXT("ValueAddPropSheetPageProc called for release")));
			GlobalFree((LPVALUEADDWIZDATA)(ppsp->lParam));
			return TRUE;
			break;

		default :
			CoTrace((TEXT("ValueAddPropSheetPageProc called for create %x"), ppsp));
			return TRUE;
			break;
	}

	return TRUE; // let the page be created (return ignored on page release)
}




INT_PTR
CALLBACK
ValueAddDlgProc(
	IN HWND hwndDlg,
	IN UINT uMsg,
	IN WPARAM wParam,
	IN LPARAM lParam
	)

/*++

Routine Description:

	This function is the dialog procedure for the value-add software selection
	wizard page.  If the user selects any software on this page, the software
	will be automatically installed when the user presses "Next".

Arguments:

	hwndDlg - Supplies a handle to the dialog box window
	
	uMsg - Supplies the message
	
	wParam - Supplies the first message parameter
	
	lParam - Supplies the second message parameter

Return Value:

	This dialog procedure always returns zero.

--*/

{
	LPVALUEADDWIZDATA pdata;
	LPNMHDR lpnm;

	//
	// Retrieve the shared user data from GWL_USERDATA
	//
	pdata = (LPVALUEADDWIZDATA) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	CoTrace((TEXT("ValueAddDlgProc called uMsg = %d"), uMsg));

	switch(uMsg) 
	{
	
		case WM_INITDIALOG :
			//
			// Get the PROPSHEETPAGE lParam value and load it into GWL_USERDATA
			//
			pdata = (LPVALUEADDWIZDATA) ((LPPROPSHEETPAGE)lParam)->lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pdata);
			break;

		case WM_NOTIFY :
			
			lpnm = (LPNMHDR)lParam;

			switch(lpnm->code) 
			{

				case PSN_SETACTIVE :
					//
					// Enable the Next and Back buttons
					//
					PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
					break;

				case PSN_WIZNEXT :
					//
					// Install any applications the user selected.
					//
					//if(IsDlgButtonChecked(hwndDlg, IDC_CHECK1) && !pdata->AppInstallAttempted) 
					if( !pdata->AppInstallAttempted) 
					{
						CoTrace((TEXT("attempting setup...")));

						BOOL MediaPresent = FALSE;
						WCHAR PathToSetupExe[MAX_PATH];
						PWSTR LastChar;

						//
						// If we need to prompt the user for media, do so now.
						//
						if(*(pdata->MediaDiskName)) 
						{
							CoTrace((TEXT("need to prompt user.")));

							WCHAR TempString[64];
							DWORD PathLength;

							if(!LoadString(g_hInstance,
										   IDS_MEDIA_PROMPT_TITLE,
										   TempString,
										   sizeof(TempString) / sizeof(WCHAR))) 
							{

								*TempString = TEXT('\0');
							}

							//
							// Append subdirectory where toastapp's setup.exe
							// is located, so we can prompt user for media.
							//
							//LastChar = pdata->MediaRootDirectory + lstrlen(pdata->MediaRootDirectory);
							//lstrcpyn(LastChar, 
							//         IKUSB_SETUP_SUBDIR,
							//         MAX_PATH - (int)(LastChar - pdata->MediaRootDirectory)
							//        );

							//
							// (Note, we skip the first character in 
							// SetupExeName for our "FileSought" argument 
							// below, because we don't want to include the
							// first character, which is a path separator.)
							//
							if(DPROMPT_SUCCESS == SetupPromptForDisk(
													  GetParent(hwndDlg),
													  TempString,
													  pdata->MediaDiskName,
													  pdata->MediaRootDirectory,
													  SetupExeName+1,
													  pdata->MediaTagFile,
													  IDF_CHECKFIRST | IDF_NOBEEP,
													  PathToSetupExe,
													  MAX_PATH,
													  &PathLength))
							{
								MediaPresent = TRUE;
							}
							
							//
							// Strip the ToastApp subdir off media root path
							//
							//*LastChar = L'\0';

							if(MediaPresent) 
							{
								//
								// SetupPromptForDisk gives us the directory 
								// where our setup program is located--now we
								// need to append the setup program onto the
								// end of that path.
								//
								PathLength--; // Don't include terminating null

								if((PathToSetupExe[PathLength-1] != L'\\') && 
								   (PathToSetupExe[PathLength-1] != L'/')) 
								{
									//
									// We need the path separator char...
									//
									lstrcpyn(PathToSetupExe+PathLength, 
											 SetupExeName, 
											 MAX_PATH - PathLength
											);
								} else 
								{
									//
									// We don't need the path separator char...
									//
									lstrcpyn(PathToSetupExe+PathLength, 
											 SetupExeName+1, 
											 MAX_PATH - PathLength
											);
								}
							}

						} 
						else 
						{
							CoTrace((TEXT("No need to prompt user.")));

							//
							// Assume media is already present (i.e., because 
							// we're in our auto-launch setup program running
							// off the media.
							//
							MediaPresent = TRUE;

							//
							// Construct the fully-qualified path to the setup 
							// executable.
							//
							lstrcpyn(PathToSetupExe, pdata->MediaRootDirectory, MAX_PATH);
							LastChar = PathToSetupExe + lstrlen(PathToSetupExe);
							lstrcpyn(LastChar, 
									 IKUSB_SETUP_PATH,
									 MAX_PATH - (int)(LastChar - PathToSetupExe)
									);
						}

						if(MediaPresent) 
						{
							//
							// We're attempting app install.  Success or
							// failure, we don't want to try again.
							//
							pdata->AppInstallAttempted = TRUE;

							CoTrace((TEXT("path to setup is %s"),PathToSetupExe));

							if(!InstallIKUSBApp(GetParent(hwndDlg), PathToSetupExe)) 
							{
								CoTrace((TEXT("Install Failed 1.")));

								//
								// We failed to install the toast app. Un-check 
								// the checkbox before we disable it.
								//
								//CheckDlgButton(hwndDlg, IDC_CHECK1, BST_UNCHECKED);
							}

							//nableWindow(GetDlgItem(hwndDlg, IDC_CHECK1), FALSE);
						}
						else
						{
							CoTrace((TEXT("No media present")));
						}
					}

					break;

				case PSN_WIZBACK :
					//Handle a Back button click, if necessary
					break;

				case PSN_RESET :
					//Handle a Cancel button click, if necessary
					break;

				default :
					break;
			}

			break;

		
		default:
			break;
	}

	return 0;
}


BOOL
InstallIKUSBApp(
	IN HWND    hwndWizard, 
	IN LPCWSTR FullSetupPath
	)

/*++

Routine Description:

	This routine hides the wizard, kicks off the ToastApp setup program, then
	unhides the wizard when the ToastApp setup process terminates.

Arguments:

	hwndWizard - Handle to the wizard window to be hidden.
	
	FullSetupPath - Supplies the path to the setup program to be launched.

Return Value:

	If the setup app was successfully launched, the return value is TRUE.
	Otherwise, it is FALSE.

--*/

{
	BOOL b;
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;

	//
	// Hide our wizard for the duration of the Toaster app's installation...
	//
	if (hwndWizard)
		ShowWindow(hwndWizard, SW_HIDE);
	GetStartupInfo(&StartupInfo);

	WCHAR command[512] = TEXT("xxx hwfirst");

	b = CreateProcess(FullSetupPath,
					  command,  //  NULL,
					  NULL,
					  NULL,
					  FALSE,
					  DETACHED_PROCESS | NORMAL_PRIORITY_CLASS,
					  NULL,
					  NULL,
					  &StartupInfo,
					  &ProcessInformation
					 );

	if(b) 
	{
		//
		// Don't need a handle to the thread...
		//
		CloseHandle(ProcessInformation.hThread);

		//
		// ...but we _do_ want to wait on the process handle.
		//
		WaitForMultipleObjects(1, &ProcessInformation.hProcess, FALSE, INFINITE);

		CloseHandle(ProcessInformation.hProcess);
	}
	else
	{
		CoTrace((TEXT("cannot start install program")));
	}

	//
	// Now show our wizard once again...
	//
	if (hwndWizard)
		ShowWindow(hwndWizard, SW_SHOW);

	return b;
}
