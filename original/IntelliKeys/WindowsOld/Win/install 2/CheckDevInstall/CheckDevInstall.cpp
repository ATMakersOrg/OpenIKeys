// CheckDevInstall.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define MAX_INTELLIKEYS 10

typedef struct
{
	char data[255];
} mystr255;

#ifdef __cplusplus
extern "C" 
#endif
int FindMyHidDevices ( 
	ULONG vendorID,
	ULONG productID,
	int maxDevices,
	mystr255 *devNames
	);

#ifdef __cplusplus
extern "C" 
#endif
BOOL
IsDeviceInstallInProgress (VOID);


static bool FileExists ( char * filename )
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile(filename, &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	FindClose(hFind);
	return true;
}


int main(int argc, char* argv[])
{
	//  is dev install underway?
	BOOL bDevInstall = IsDeviceInstallInProgress();

	//  count devices of various sorts
	mystr255 devices[MAX_INTELLIKEYS];
	int nRaw    = FindMyHidDevices ( 0x095e, 0x0100, MAX_INTELLIKEYS, devices );
	int nLoaded = FindMyHidDevices ( 0x095e, 0x0101, MAX_INTELLIKEYS, devices );

	//  where should the software be installed?
	TCHAR path[255];
	GetPrivateProfileString ( TEXT("IntelliKeys USB"), TEXT("path"), TEXT("c:\\itools\\intellikeys usb\\"), path, 255, TEXT("itools_x.ini") );

	//  see if it's installed by looking for three certain files.
	TCHAR test[255];

	strcpy(test,path);
	strcat(test,"intellikeys usb.exe");
	bool b1 = FileExists(test);

	strcpy(test,path);
	strcat(test,"private\\iksystray.exe");
	bool b2 = FileExists(test);

	strcpy(test,path);
	strcat(test,"private\\shutdown.exe");
	bool b3 = FileExists(test);

	bool bInstalled = b1 && b2 && b3;

	int ret = bDevInstall + (nRaw<<8) + (nLoaded<<16) + (bInstalled<<24);
	return ret;
}
