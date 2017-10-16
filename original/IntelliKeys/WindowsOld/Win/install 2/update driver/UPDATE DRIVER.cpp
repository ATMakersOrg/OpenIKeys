// UPDATE DRIVER.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <shellapi.h>
#include <prsht.h>
#include <windef.h>
#include <regstr.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#include <newdev.h>

//#define ENUMERATOR_NAME     L"Toaster"



/*++

Copyright (c) 1999-2000  Microsoft Corporation

Module Name:

    Install.c

Abstract:

    Console app for the installation of Device Drivers in Windows 2000.

Environment:

    user mode only

Notes:

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.

  Copyright (c) 1999-2000 Microsoft Corporation.  All Rights Reserved.


Revision History:

  9/22/99: Created Keith S. Garner, with input from Eliyas and others.

--*/


#include <stdio.h> 
//#include <tchar.h> // Make all functions UNICODE safe.
#include <windows.h>  
#include <newdev.h> // for the API UpdateDriverForPlugAndPlayDevices().
#include <setupapi.h> // for SetupDiXxx functions.

#define MAX_CLASS_NAME_LEN 32 // Stolen from <cfgmgr32.h>


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


#define Trace(_x_) DbgPrint(TEXT("Update Driver: ")); DbgPrint _x_;

#else

#define Trace(_x_)

#endif


int DisplayError(TCHAR * ErrorName)
/*++
Routine Description:

    This Routine will display the LastError in human readable 
    form when possible.

    If the return value is a 32-bit number, and falls in the range:
        ERROR_NO_ASSOCIATED_CLASS   0xE0000200 
    To
        ERROR_CANT_REMOVE_DEVINST   0xE0000232 
    The values defined in setupapi.h can help to determine the error.
    Start by searching for the text string ERROR_NO_ASSOCIATED_CLASS.
  
Arguments:
    
    ErrorName: Human readable description of the last Function called.

Return Value:
    
    Allways returns FALSE.
      
--*/
{
    DWORD Err = GetLastError();
    LPVOID lpMessageBuffer = NULL;
    
    if (FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, 
        Err,  
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMessageBuffer,  
        0,  
        NULL ))
	{
		Trace((TEXT("%s FAILURE: %s\n"),ErrorName,(TCHAR *)lpMessageBuffer));
	}
	else
	{
		Trace((TEXT("%s FAILURE: (0x%08x)\n"),ErrorName,Err));
	}
    
    if (lpMessageBuffer) LocalFree( lpMessageBuffer ); // Free system buffer 
    
    SetLastError(Err);    
    return FALSE;
}

BOOL FindExistingDevice(IN LPTSTR HardwareId)
/*++

Routine Description:

    This routine finds an existing devnode if present.

Arguments:

    HardwareIdList - Supplies a string containing a hardware ID to 
    be associated with the device.

Return Value:

    The function returns TRUE if it is successful. 

    Otherwise it returns FALSE and the logged error can be retrieved 
    with a call to GetLastError. 

    The most common error will be ERROR_NO_MORE_ITEMS, which means the 
    function could not find a devnode with the HardwareID.

--*/
{
    HDEVINFO DeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD i,err;
    BOOL Found;
    
    //
    // Create a Device Information Set with all present devices.
    //
    DeviceInfoSet = SetupDiGetClassDevs(NULL, // All Classes
        0,
        0, 
        DIGCF_ALLCLASSES | DIGCF_PRESENT ); // All devices present on system
    if (DeviceInfoSet == INVALID_HANDLE_VALUE)
    {
        return DisplayError(TEXT("GetClassDevs(All Present Devices)"));        
    }
    
    Trace((TEXT("Search for Device ID: [%s]\n"),HardwareId));
    
    //
    //  Enumerate through all Devices.
    //
    Found = FALSE;
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i=0;SetupDiEnumDeviceInfo(DeviceInfoSet,i,&DeviceInfoData);i++)
    {
        DWORD DataT;
        LPTSTR p,buffer = NULL;
        DWORD buffersize = 0;
        
        //
        // We won't know the size of the HardwareID buffer until we call
        // this function. So call it with a null to begin with, and then 
        // use the required buffer size to Alloc the nessicary space.
        // Keep calling we have success or an unknown failure.
        //
        while (!SetupDiGetDeviceRegistryProperty(
            DeviceInfoSet,
            &DeviceInfoData,
            SPDRP_HARDWAREID,
            &DataT,
            (PBYTE)buffer,
            buffersize,
            &buffersize))
        {
            if (GetLastError() == ERROR_INVALID_DATA)
            {
                //
                // May be a Legacy Device with no HardwareID. Continue.
                //
                break;
            }
            else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                //
                // We need to change the buffer size.
                //
                if (buffer) 
                    LocalFree(buffer);
                buffer = (LPTSTR)LocalAlloc(LPTR,buffersize);
            }
            else
            {
                //
                // Unknown Failure.
                //
                DisplayError(TEXT("GetDeviceRegistryProperty"));
                goto cleanup_DeviceInfo;
            }            
        }
        
        if (GetLastError() == ERROR_INVALID_DATA) 
            continue;
        
        //
        // Compare each entry in the buffer multi-sz list with our HardwareID.
        //
        for (p=buffer;*p&&(p<&buffer[buffersize]);p+=lstrlen(p)+sizeof(TCHAR))
        {
            Trace((TEXT("Compare device ID: [%s]\n"),p));
            
            if (!wcsicmp(HardwareId,p))
            {
                Trace((TEXT("Found! [%s]\n"),p));
                Found = TRUE;
                break;
            }
        }
        
        if (buffer) LocalFree(buffer);
        if (Found) break;
    }
    
    if (GetLastError() != NO_ERROR)
    {
        DisplayError(TEXT("EnumDeviceInfo"));
    }
    
    //
    //  Cleanup.
    //    
cleanup_DeviceInfo:
    err = GetLastError();
    SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    SetLastError(err);
    
    return err == NO_ERROR; //???
}

BOOL
InstallRootEnumeratedDriver(IN  LPTSTR HardwareId,
    IN  LPTSTR INFFile,
    OUT PBOOL  RebootRequired  OPTIONAL
    )
/*++

Routine Description:

    This routine creates and installs a new root-enumerated devnode.

Arguments:

    HardwareIdList - Supplies a multi-sz list containing one or more hardware
    IDs to be associated with the device.  These are necessary in order 
    to match up with an INF driver node when we go to do the device 
    installation.

    InfFile - Supplies the full path to the INF File to be used when 
    installing this device.

    RebootRequired - Optionally, supplies the address of a boolean that is 
    set, upon successful return, to indicate whether or not a reboot is 
    required to bring the newly-installed device on-line.

Return Value:

    The function returns TRUE if it is successful. 

    Otherwise it returns FALSE and the logged error can be retrieved 
    with a call to GetLastError. 

--*/
{
    HDEVINFO DeviceInfoSet = 0;
    SP_DEVINFO_DATA DeviceInfoData;
    GUID ClassGUID;
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    DWORD err;
    
    //
    // Use the INF File to extract the Class GUID. 
    //
    if (!SetupDiGetINFClass(INFFile,&ClassGUID,ClassName,sizeof(ClassName),0))
    {
        return DisplayError(TEXT("GetINFClass"));
    }
    
    //
    // Create the container for the to-be-created Device Information Element.
    //
    DeviceInfoSet = SetupDiCreateDeviceInfoList(&ClassGUID,0);
    if(DeviceInfoSet == INVALID_HANDLE_VALUE) 
    {
        return DisplayError(TEXT("CreateDeviceInfoList"));
    }
    
    // 
    // Now create the element. 
    // Use the Class GUID and Name from the INF file.
    //
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if (!SetupDiCreateDeviceInfo(DeviceInfoSet,
        ClassName,
        &ClassGUID,
        NULL,
        0,
        DICD_GENERATE_ID,
        &DeviceInfoData))
    {
        DisplayError(TEXT("CreateDeviceInfo"));
        goto cleanup_DeviceInfo;
    }
    
    //
    // Add the HardwareID to the Device's HardwareID property.
    //
    if(!SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
        &DeviceInfoData,
        SPDRP_HARDWAREID,
        (LPBYTE)HardwareId,
        (lstrlen(HardwareId)+1+1)*sizeof(TCHAR))) 
    {
        DisplayError(TEXT("SetDeviceRegistryProperty"));
        goto cleanup_DeviceInfo;
    }
    
    //
    // Transform the registry element into an actual devnode 
    // in the PnP HW tree.
    //
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE,
        DeviceInfoSet,
        &DeviceInfoData))
    {
        DisplayError(TEXT("CallClassInstaller(REGISTERDEVICE)"));
        goto cleanup_DeviceInfo;
    }
    
    //
    // The element is now registered. We must explicitly remove the 
    // device using DIF_REMOVE, if we encounter any failure from now on.
    //
    
    //
    // Install the Driver.
    //
    if (!UpdateDriverForPlugAndPlayDevices(0,
        HardwareId,
        INFFile,
        INSTALLFLAG_FORCE,
        RebootRequired))
    {
        DWORD err = GetLastError();
        DisplayError(TEXT("UpdateDriverForPlugAndPlayDevices"));
        
        if (!SetupDiCallClassInstaller(
            DIF_REMOVE,
            DeviceInfoSet,
            &DeviceInfoData))
        {
            DisplayError(TEXT("CallClassInstaller(REMOVE)"));
        }
        SetLastError(err);
    }
    
    //
    //  Cleanup.
    //    
cleanup_DeviceInfo:
    err = GetLastError();
    SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    SetLastError(err);
    
    return err == NO_ERROR;
}


static HDEVINFO
GetNonPresentDevices(
    IN  LPCWSTR   Enumerator OPTIONAL,
    IN  LPCWSTR   HardwareID
    )
    
/*++

Routine Description:

    This routine retrieves any non-present devices matching the specified 
    criteria, and returns them in a device information set.

Arguments:

    Enumerator - Optionally, supplies the name of the Enumerator under which 
        this device may be found.  If the device may show up under more than 
        one enumerator, the routine can be called with Enumerator specified as
        NULL, in which case all device instances in the registry are examined.
        
    HardwareID - Supplies the hardware ID to be searched for.  This will be
        compared against each of the hardware IDs for all device instances in
        the system (potentially filtered based on Enumerator), present or not.

Return Value:

    If any non-present devices are discovered, this routine returns a device
    information set containing those devices.  This set must be freed via
    SetupDiDestroyDeviceInfoList by the caller.
    
    If no such devices are encountered (or if an error occurs), the return
    value is INVALID_HANDLE_VALUE.  GetLastError will indicate the cause of
    failure.

--*/

{
    HDEVINFO AllDevs, ExistingNonPresentDevices;
    DWORD i, Err;
    SP_DEVINFO_DATA DeviceInfoData;
    LPWSTR HwIdBuffer, CurId;
    DWORD HwIdBufferLen, RegDataType, RequiredSize;
    BOOL bRet;
    ULONG Status, Problem;
    TCHAR DeviceInstanceId[MAX_DEVNODE_ID_LEN];

    ExistingNonPresentDevices = INVALID_HANDLE_VALUE;
    
    AllDevs = SetupDiGetClassDevs(NULL,
                                  Enumerator,
                                  NULL,
                                  DIGCF_ALLCLASSES
                                 );
                                 
    if(AllDevs == INVALID_HANDLE_VALUE) {
        //
        // last error has already been set during the above call.
        //
        return INVALID_HANDLE_VALUE;
    }
                                  
    //
    // Iterate through each device we found, comparing its hardware ID(s)
    // against the one we were passed in.
    //
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    HwIdBuffer = NULL;
    HwIdBufferLen = 0;
    Err = NO_ERROR;
    bRet = FALSE;
    
    i = 0;
        
    while(SetupDiEnumDeviceInfo(AllDevs, i, &DeviceInfoData)) {
        //
        // Retrieve the HardwareID property for this device info element
        //
        if(!SetupDiGetDeviceRegistryProperty(AllDevs,
                                             &DeviceInfoData,
                                             SPDRP_HARDWAREID,
                                             &RegDataType,
                                             (PBYTE)HwIdBuffer,
                                             HwIdBufferLen,
                                             &RequiredSize)) {
            //
            // If the failure was due to buffer-too-small, we can resize and
            // try again.
            //
            if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            
                if(HwIdBuffer) {
                    GlobalFree(HwIdBuffer);
                }
                
                HwIdBuffer = (LPWSTR) GlobalAlloc(0, RequiredSize);
                if(HwIdBuffer) {
                    HwIdBufferLen = RequiredSize;
                    //
                    // try again
                    //
                    continue;
                } else {
                    //
                    // We failed to allocate the buffer we needed.  This is
                    // considered a critical failure that should cause us to
                    // bail.
                    //
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                
            } else {
                //
                // We failed to retrieve the property for some other reason.
                // Skip this device and move on to the next.
                //
                i++;
                continue;
            }
        }
        
        if((RegDataType != REG_MULTI_SZ) || (RequiredSize < sizeof(TCHAR))) {
            //
            // Data is invalid--this should never happen, but we'll skip the
            // device in this case...
            //
            i++;
            continue;
        }
        
        //
        // If we get to here, then we successfully retrieved the multi-sz
        // hardware id list for this device.  Compare each of those IDs with
        // the caller-supplied one.
        //
        for(CurId = HwIdBuffer; *CurId; CurId += (lstrlen(CurId) + 1)) {
        
            if(!lstrcmpi(CurId, HardwareID)) {
                //
                // We found a match!
                //
                bRet = TRUE;
                
                //
                // If the device isn't currently present (as indicated by
                // failure to retrieve its status), then add it to the list of
                // such devices to be returned to the caller.
                //
                if(CR_SUCCESS != CM_Get_DevNode_Status(&Status,
                                                       &Problem,
                                                       (DEVNODE)DeviceInfoData.DevInst,
                                                       0))
                {
                    if(ExistingNonPresentDevices == INVALID_HANDLE_VALUE) {
                        //
                        // This is the first non-present device we've 
                        // encountered--we need to create the HDEVINFO set.
                        //
                        ExistingNonPresentDevices = 
                            SetupDiCreateDeviceInfoList(NULL, NULL);
                        
                        if(ExistingNonPresentDevices == INVALID_HANDLE_VALUE) {
                            //
                            // Failure to create this set is a critical error!
                            //
                            Err = GetLastError();
                            bRet = FALSE;
                            break;
                        }
                    }
                        
                    //
                    // We need to get the device instance's name so we can
                    // open it up into our "non-present devices" list
                    //
                    if(!SetupDiGetDeviceInstanceId(AllDevs,
                                                   &DeviceInfoData,
                                                   DeviceInstanceId,
                                                   sizeof(DeviceInstanceId) / sizeof(TCHAR),
                                                   NULL)) {
                        //
                        // Should never fail, but considered critical if it
                        // does...
                        //
                        Err = GetLastError();
                        bRet = FALSE;
                        break;
                    }

                    //
                    // Now open up the non-present device into our list.
                    //
                    if(!SetupDiOpenDeviceInfo(ExistingNonPresentDevices,
                                              DeviceInstanceId,
                                              NULL,
                                              0,
                                              NULL)) {
                        //
                        // This failure is also considered critical!
                        //                          
                        Err = GetLastError();
                        bRet = FALSE;
                    }

                    break;
                }
            }
        }

        if(Err != NO_ERROR) {
            //
            // Critical error encountered--bail!
            //
            break;
        }

        //
        // Move onto the next device instance
        //
        i++;
    }
    
    if(HwIdBuffer) {
        GlobalFree(HwIdBuffer);
    }

    //
    // We can now destroy our temporary list of all devices under consideration
    //
    SetupDiDestroyDeviceInfoList(AllDevs);

    if((Err != NO_ERROR) && 
       (ExistingNonPresentDevices != INVALID_HANDLE_VALUE)) {
        //
        // We encountered a critical error, so we need to destroy the (partial)
        // list of non-present devices we'd built.
        //
        SetupDiDestroyDeviceInfoList(ExistingNonPresentDevices);
        ExistingNonPresentDevices = INVALID_HANDLE_VALUE;
    }

    SetLastError(Err);

    return ExistingNonPresentDevices;
}


static DWORD
GetDeviceConfigFlags(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
    
/*++

Routine Description:

    This routine retrieves the ConfigFlags registry property for the specified
    device info element, or zero if the property cannot be retrieved (e.g.,
    because ConfigFlags haven't yet been set by Found New Hardware process).

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        the device of interest.
        
    DeviceInfoData - Supplies context of a device info element for which
        ConfigFlags is to be retrieved.

Return Value:

    If device's REG_DWORD ConfigFlags property can be retrieved, it is returned.
    Otherwise, zero is returned.

--*/

{
    DWORD ConfigFlags, RegDataType;
    
    if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_CONFIGFLAGS,
                                         &RegDataType,
                                         (PBYTE)&ConfigFlags,
                                         sizeof(ConfigFlags),
                                         NULL)
       || (RegDataType != REG_DWORD))
    {
        //
        // It's possible that this property isn't there, although we should
        // never enounter other problems like wrong datatype or data length
        // longer than sizeof(DWORD).  In any event, just return zero.
        //
        ConfigFlags = 0;
    }
    
    return ConfigFlags;
}


static VOID
SetDeviceConfigFlags(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData, 
    IN DWORD            ConfigFlags
    )

/*++

Routine Description:

    This routine sets a device's ConfigFlags property to the specified value.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        the device of interest.
        
    DeviceInfoData - Supplies context of a device info element for which
        ConfigFlags is to be set.
        
    ConfigFlags - Specifies the value to be stored to the device's ConfigFlags
        property.

Return Value:

    none

--*/

{
    SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDRP_CONFIGFLAGS,
                                     (PBYTE)&ConfigFlags,
                                     sizeof(ConfigFlags)
                                    );
}


static VOID
MarkDevicesAsNeedReinstall(
    IN HDEVINFO DeviceInfoSet
    )

/*++

Routine Description:

    This routine enumerates every device information element in the specified
    list and sets the CONFIGFLAG_REINSTALL registry flag for each one.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set whose 
        members are to be marked as need-reinstall.

Return Value:

    none

--*/

{
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD i, ConfigFlags;
    
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    for(i = 0;
        SetupDiEnumDeviceInfo(DeviceInfoSet, i, &DeviceInfoData);
        i++)
    {
        ConfigFlags = GetDeviceConfigFlags(DeviceInfoSet, &DeviceInfoData);
        ConfigFlags |= CONFIGFLAG_REINSTALL;
        SetDeviceConfigFlags(DeviceInfoSet, &DeviceInfoData, ConfigFlags);
    }
}

int __cdecl main(int argc, TCHAR **argv, TCHAR **envp)
/*++
Routine Discription:

    Entry point to install.exe.
    Parse the command line, call subroutines.

Arguments:
    
    Standard console 'c' application arguments.

    argv[1] - Full path of INF file.
    argv[2] - PnP HardwareID of device.

Return Value:
    
    Standard Console ERRORLEVEL values:

    0 - Install Successfull, no reboot required.
    1 - Install Successfull, reboot required.
    2 - Install Failure.
    
--*/
{
    WIN32_FIND_DATA FindFileData;
    BOOL RebootRequired = 0; // Must be cleared.

    HKEY hKey;
    DWORD KeyDisposition;
    //DWORD Err;
    //DWORD RegDataType;
    //DWORD RequiredSize;
    DWORD Updating;

	//  make wide char versions of the arguments

	WCHAR wargv1[255];
	int lensrc = strlen((char *)argv[1]);
	int result = MultiByteToWideChar (
							CP_UTF8,				// utf-8
							0,						// character-type options
							(const char *) argv[1], // string to map
							lensrc,				// number of bytes in string
							wargv1,				// wide-character buffer
							255		// size of buffer
							);
	wargv1[result] = 0;
	Trace((TEXT("wargv1 = %s"),wargv1));

	WCHAR wargv2[255];
	lensrc = strlen((char *)argv[2]);
	result = MultiByteToWideChar (
							CP_UTF8,				// utf-8
							0,						// character-type options
							(const char *) argv[2], // string to map
							lensrc,				// number of bytes in string
							wargv2,				// wide-character buffer
							255		// size of buffer
							);
	wargv2[result] = 0;
	Trace((TEXT("wargv2 = %s"),wargv2));

	//  assume we're good

	result = 0;


	//  mark the registry to indicate that this program is running.
	//  The coinstaller will detect this and NOT launch the installer
	//  again.

    if(ERROR_SUCCESS == RegCreateKeyEx(
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
		Updating = 0;

        RegSetValueEx(hKey, 
                      TEXT("Updating"), 
                      0, 
                      REG_DWORD, 
                      (PBYTE)&Updating, 
                      sizeof(Updating)
                     );

        RegCloseKey(hKey);
	}
    
    //
    // Verify the Arguments.
    //
    if (argc != 3)
    {
        Trace((TEXT("usage: install <INF_File> <Hardware_ID>\n")));

        result =  2; // Install Failure
		goto cleanup;
    }
    
    if (FindFirstFile(wargv1,&FindFileData)==INVALID_HANDLE_VALUE)
    {
        Trace((TEXT("  File not found.\n")));
        Trace((TEXT("usage: install <INF_File> <Hardware_ID>\n")));

        result =  2; // Install Failure
		goto cleanup;
    }
    
#if 0  //----------------------------------------------------------------------------
    //
    // Look to see if this device allready exists.
    //
    if (FindExistingDevice(wargv2))
    {
        //
        // No Need to Create a Device Node, just call our API.
        //
        if (!UpdateDriverForPlugAndPlayDevices(0, // No Window Handle
            wargv2, // Hardware ID
            wargv1, // FileName
            INSTALLFLAG_FORCE,
            &RebootRequired))
        {
            DisplayError(TEXT("UpdateDriverForPlugAndPlayDevices"));

			result =  2; // Install Failure
			goto cleanup;
        }
    }
    else
    {
        if (GetLastError()!= ERROR_NO_MORE_ITEMS)
        {
            //
            // An unknown failure from FindExistingDevice()
            //

			result =  2; // Install Failure
			goto cleanup;
        }
        
        // 
        // Driver Does not exist, Create and call the API.
        // HardwareID must be a multi-sz string, which argv[2] is.
        //
        if (!InstallRootEnumeratedDriver(wargv2, // HardwareID
            wargv1, // FileName
            &RebootRequired))
        {

			result =  2; // Install Failure
			goto cleanup;
        }
    }
    
    Trace((TEXT("Driver Installed successfully.\n")));
    
    if (RebootRequired)
    {
        Trace((TEXT("(Reboot Required)\n")));
        result = 1; // Install Success, reboot required.
		goto cleanup;
    }

#else  //  ---------------------------------------------------------------------------


    DWORD Err;
    //LPSHAREDWIZDATA pdata;
    //WCHAR FullInfPath[MAX_PATH];
    //LPWSTR LastChar;
    HDEVINFO ExistingNonPresentDevices;


    //pdata = (LPSHAREDWIZDATA)ThreadData;
    Err = NO_ERROR;

    //
    // First, attempt to update any present devices to our driver...
    //
    //lstrcpyn(FullInfPath, pdata->MediaRootDirectory, MAX_PATH);
    //LastChar = FullInfPath + lstrlen(FullInfPath);

    //lstrcpyn(LastChar, 
             //DEVICE_INF_NAME, 
             //MAX_PATH - (int)(LastChar - FullInfPath)
            //);

    if(UpdateDriverForPlugAndPlayDevices(NULL,
                                         wargv2,
                                         wargv1,
                                         0,
                                         &RebootRequired)) 
	{
        //
        // We know that at least one device existed, and was upgraded.
        //
        //pdata->HwInsertedFirst = TRUE;

    } 
	else 
	{

        Err = GetLastError();

        //
        // We failed to update the driver.  If we failed simply because
        // there were no toasters currently attached to the computer, then
        // we still want to install the INF.
        //
        if(Err == ERROR_NO_SUCH_DEVINST) 
		{

            //pdata->HwInsertedFirst = FALSE;

            //
            // Since we didn't do any device installs, the INF (and CAT)
            // didn't get automatically installed.  We'll install them now,
            // so that they'll be present when the user subsequently plugs
            // their hardware in.
            //
            if(!SetupCopyOEMInf(wargv1,
                                NULL,
                                SPOST_PATH,
                                0,
                                NULL,
                                0,
                                NULL,
                                NULL)) {
                //
                // Failure to install the INF is more important (worse) than
                // the absence of any devices!
                //
                Err = GetLastError();
            }

        } 
		else 
		{
            //
            // Apparently there _were_ existing devices--we just failed to
            // upgrade their drivers.  This might be due to an installation
            // problem, or perhaps because the devices already have drivers
            // newer than the one we offered.
            //
            //pdata->HwInsertedFirst = TRUE;
        }
    }

    if((Err == NO_ERROR) || (Err == ERROR_NO_SUCH_DEVINST)) 
	{
        //
        // Either we successfully upgraded one or more toasters, or there were
        // no present toasters but we successfully installed our INF and CAT.
        //
        // There may exist, however, devices that were once connected to the
        // computer, but presently are not.  If such devices are connected
        // again in the future, we want to ensure they go through device
        // installation.  We will retrieve the list of non-present devices, and
        // mark each as "needs re-install" to kick them back through the "New
        // Hardware Found" process if they ever show up again.  (Note that this
        // doesn't destroy any device-specific settings they may have, so this
        // is just forcing an upgrade, not an uninstall/re-install.)
        //
        // (The HardwareID used is the one defined for the toaster sample, 
        // BUS_HARDWARE_IDS in src\general\toaster\bus\common.h.  We also take
        // advantage of the fact that we know these devices will always be
        // enumerated under the "Toaster" enum namespace.)
        //
        ExistingNonPresentDevices = GetNonPresentDevices(NULL,
                                                         wargv2
                                                        );

        if(ExistingNonPresentDevices != INVALID_HANDLE_VALUE) 
		{

            MarkDevicesAsNeedReinstall(ExistingNonPresentDevices);

            SetupDiDestroyDeviceInfoList(ExistingNonPresentDevices);
        }
    }

    //PostMessage(pdata->hwndDlg, WMX_UPDATE_DRIVER_DONE, 0, 0);



#endif  //  ---------------------------------------------------------------------------
    
    result = 0; // Install Success, no reboot required.
	goto cleanup;

cleanup:

	//  restore the registry

	//  mark the registry to indicate that this program is running.
	//  The coinstaller will detect this and NOT launch the installer
	//  again.

    if(ERROR_SUCCESS == RegCreateKeyEx(
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
		Updating = 0;

        RegSetValueEx(hKey, 
                      TEXT("Updating"), 
                      0, 
                      REG_DWORD, 
                      (PBYTE)&Updating, 
                      sizeof(Updating)
                     );

        RegCloseKey(hKey);
	}

	return result;

}

