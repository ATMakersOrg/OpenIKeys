/*++

Copyright (c) 1996    Microsoft Corporation

Module Name:

    pnp.c

Abstract:

    This module contains the code
    for finding, adding, removing, and identifying hid devices.

Environment:

    Kernel & user mode

Revision History:

    Nov-96 : Created by Kenneth D. Ray

--*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <basetyps.h>
#include <stdlib.h>
#include <wtypes.h>
#include <setupapi.h>
#include "hidsdi.h"
#include "hid.h"

#include "pnp.h"

typedef
DWORD
(WINAPI *CMP_WAITNOPENDINGINSTALLEVENTS_PROC)(
    IN DWORD dwTimeout
    );

BOOL
IsDeviceInstallInProgress (VOID)
{
    HMODULE hModule;
    CMP_WAITNOPENDINGINSTALLEVENTS_PROC pCMP_WaitNoPendingInstallEvents;
	DWORD dwResult;

    hModule = GetModuleHandle(TEXT("setupapi.dll"));
    if(!hModule)
    {
        // Should never happen since we're linked to SetupAPI, but...
        return FALSE;
    }

    pCMP_WaitNoPendingInstallEvents =
        (CMP_WAITNOPENDINGINSTALLEVENTS_PROC)GetProcAddress(hModule,
                                             "CMP_WaitNoPendingInstallEvents");
    if(!pCMP_WaitNoPendingInstallEvents)
    {
        // We're running on a release of the operating system that doesn't supply this function.
        // Trust the operating system to suppress Autorun when appropriate.
        return FALSE;
    }

	dwResult = (*pCMP_WaitNoPendingInstallEvents)(0);
	if (dwResult==WAIT_TIMEOUT)
		return TRUE;
	return FALSE;
}

BOOLEAN GetNumBuffers ( HANDLE h, PULONG pNumBuffers )
{
	return HidD_GetNumInputBuffers ( h, pNumBuffers );
}

BOOLEAN SetNumBuffers ( HANDLE h, ULONG numBuffers )
{
	return HidD_SetNumInputBuffers ( h, numBuffers );
}

BOOLEAN
OpenMyHidDevice (
    IN       PCHAR          DevicePath,
    IN OUT   PHID_DEVICE    HidDevice
    )
/*++
RoutineDescription:
    Given the HardwareDeviceInfo, representing a handle to the plug and
    play information, and deviceInfoData, representing a specific hid device,
    open that device and fill in all the relivant information in the given
    HID_DEVICE structure.

    return if the open and initialization was successfull or not.

--*/
{
    HidDevice->HidDevice = CreateFile (
                              DevicePath,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, // no SECURITY_ATTRIBUTES structure
                              OPEN_EXISTING, // No special create flags
                              0, // No special attributes
                              NULL); // No template file

    if (INVALID_HANDLE_VALUE == HidDevice->HidDevice) {
        return FALSE;
    }

    return (TRUE);
}


VOID
CloseMyHidDevice (
    IN PHID_DEVICE HidDevice
)
{
    if (INVALID_HANDLE_VALUE != HidDevice -> HidDevice) {
        CloseHandle(HidDevice -> HidDevice);
    }


    return;
}


int
FindMyHidDevices ( 
	ULONG vendorID,
	ULONG productID,
	int maxDevices,
	mystr255 *devNames
	)
{
    HDEVINFO                 hardwareDeviceInfo;
    SP_INTERFACE_DEVICE_DATA deviceInfoData;
    ULONG                    i;
    BOOLEAN                  done;
    GUID                     hidGuid;
    static PSP_INTERFACE_DEVICE_DETAIL_DATA     functionClassDeviceData = NULL;
    ULONG                                predictedLength = 0;
    ULONG                                requiredLength = 0;
	ULONG					NumberDevices;
	HID_DEVICE HidDevice;
	HIDD_ATTRIBUTES			DeviceAttributes;
	ULONG					result;
	
	int numDevices = 0;

    HidD_GetHidGuid (&hidGuid);

    //*NumberDevices = 0;
	NumberDevices = 0;

    //
    // Open a handle to the plug and play dev node.
    //
    hardwareDeviceInfo = SetupDiGetClassDevs (
                                               &hidGuid,
                                               NULL, // Define no enumerator (global)
                                               NULL, // Define no
                                               (DIGCF_PRESENT | // Only Devices present
                                                DIGCF_INTERFACEDEVICE)); // Function class devices.

    //
    // Take a wild guess to start
    //
    
    NumberDevices = 4;
    done = FALSE;
    deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

    i=0;
    while (!done) 
	{
        NumberDevices *= 2;

        for (; i < NumberDevices; i++) 
		{
            if (SetupDiEnumInterfaceDevice (hardwareDeviceInfo,
                                            0, // No care about specific PDOs
                                            &hidGuid,
                                            i,
                                            &deviceInfoData)) 
			{

                //
                // allocate a function class device data structure to receive the
                // goods about this particular device.
                //
                SetupDiGetInterfaceDeviceDetail (
                        hardwareDeviceInfo,
                        &deviceInfoData,
                        NULL, // probing so no output buffer yet
                        0, // probing so output buffer length of zero
                        &requiredLength,
                        NULL); // not interested in the specific dev-node


                predictedLength = requiredLength;
                // sizeof (SP_FNCLASS_DEVICE_DATA) + 512;

				if (!functionClassDeviceData)
					functionClassDeviceData = malloc (1000);
				memset(functionClassDeviceData,0,1000);
                //functionClassDeviceData = malloc (predictedLength);
                functionClassDeviceData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);

                //
                // Retrieve the information from Plug and Play.
                //
                if (! SetupDiGetInterfaceDeviceDetail (
                           hardwareDeviceInfo,
                           &deviceInfoData,
                           functionClassDeviceData,
                           predictedLength,
                           &requiredLength,
                           NULL)) 
				{
					//free(functionClassDeviceData);
                    return numDevices;
                }

                if (OpenMyHidDevice (functionClassDeviceData -> DevicePath, &HidDevice))
				{
					DeviceAttributes.Size = sizeof (HIDD_ATTRIBUTES);
					result = HidD_GetAttributes (HidDevice.HidDevice,&DeviceAttributes);
					if(result!=0)
					{
						if ( DeviceAttributes.VendorID  == vendorID &&
							 DeviceAttributes.ProductID == productID )
						{
// OK, it's the correct device, now need to select the correct interface (it now has three)
// We need the interface with a UsagePage = Vendor Defined = 0xFFA0
							if (HidD_GetPreparsedData (HidDevice.HidDevice, &HidDevice.Ppd) )
							{
								if (HidP_GetCaps (HidDevice.Ppd, &HidDevice.Caps) )
								{
									if (HidDevice.Caps.UsagePage == 0xFFA0)
									{
										strcpy(devNames[numDevices].data,functionClassDeviceData -> DevicePath);
										numDevices++;
										//CloseHidDevice (  &HidDevice);
										//return TRUE;
									}
								}
							}
							HidD_FreePreparsedData (HidDevice.Ppd);
						}
					}
					CloseMyHidDevice (  &HidDevice);
				}

				//free(functionClassDeviceData);
            } 
			else 
			{
                if (ERROR_NO_MORE_ITEMS == GetLastError()) 
				{
                    done = TRUE;
                    break;
                }
            }
        }
    }

    NumberDevices = i;

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    return numDevices;
}

