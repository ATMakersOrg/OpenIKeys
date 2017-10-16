

// ikusb.cpp : Defines the entry point for the application.
//

#include "IKCommon.h"
#include "IKEngine.h"
#include "IKUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "pnp.h"

static IKEngine theEngine;

BOOL nServiceRunning;
DWORD ServiceExecutionThread(LPDWORD param);

#ifdef SERVICE

#include <winsvc.h>
#include "..\servicename.h"

void ServiceMain(DWORD argc, LPTSTR *argv); 
void ServiceCtrlHandler(DWORD nControlCode);
BOOL UpdateServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode,
					 DWORD dwServiceSpecificExitCode, DWORD dwCheckPoint,
					 DWORD dwWaitHint);
BOOL StartServiceThread();
HANDLE hServiceThread;
void KillService();

SERVICE_STATUS_HANDLE nServiceStatusHandle; 
HANDLE killServiceEvent;
DWORD nServiceCurrentStatus;
#endif

typedef struct {
	bool m_bStopReading;
	TCHAR name[255];
	HANDLE m_threadHandle;
	HANDLE m_writeHandle;
	HANDLE m_readHandle;
} device;
device devices[MAX_INTELLIKEYS];

static UINT ReadRoutine(LPVOID pParam)
{
	//  get the device object reference
	int index = (int) pParam;

	devices[index].m_bStopReading = false;

	while (true)
	{
		Sleep(0);

		if (devices[index].m_bStopReading)
			break;

		DWORD	BytesRead;
		ULONG	result;
		BYTE InputReport[IK_REPORT_LEN+1];

		result = ReadFile
			(devices[index].m_readHandle,
			InputReport,
			IK_REPORT_LEN+1,
			&BytesRead,
			NULL);

		if (result == 0)
		{	
			break;
		}
		else
		{
			theEngine.OnDataReceived ( index, &(InputReport[1]) );
		}

		Sleep(0);
	}

	return 0;
}

static void WriteRoutine ( int index, BYTE *data )
{
	BYTE report[IK_REPORT_LEN+1] = {0,0,0,0,0,0,0,0,0};
	for (int i=0;i<IK_REPORT_LEN;i++)
		report[i+1] = data[i];

	unsigned long numberOfBytesWritten = 0;
	long Result = WriteFile
			(devices[index].m_writeHandle,
			report,
			IK_REPORT_LEN+1,
			&numberOfBytesWritten,
			0);

	Sleep(0);
}

static void OpenDevice ( int index,		//  slot number
						 TCHAR *name,	//  system name for the device
						 int devType	//  1=IK, 2=IS
						 )
{
	IKString::strcpy(devices[index].name,name);

	//  create a write handle
	devices[index].m_writeHandle = CreateFile
		(name, 
		GENERIC_READ|GENERIC_WRITE, 
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING, 
		FILE_FLAG_NO_BUFFERING,
		NULL);
	if (devices[index].m_writeHandle==NULL)
		return;

	//  create a read handle
	devices[index].m_readHandle = CreateFile (
		name, 
		GENERIC_READ|GENERIC_WRITE, 
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING, 
		FILE_FLAG_NO_BUFFERING,  
		NULL);

	if (devices[index].m_readHandle==NULL)
	{
		CloseHandle(devices[index].m_writeHandle);
		return;
	}

	//  increase number of buffers for reading
	ULONG nBuffers;
	BOOLEAN bResult;
	bResult = GetNumBuffers ( devices[index].m_readHandle, &nBuffers );
	nBuffers = 64;
	bResult = SetNumBuffers ( devices[index].m_readHandle, nBuffers );


	//  start the read thread
	DWORD ThreadID;
	devices[index].m_threadHandle = CreateThread (NULL, 0, 
		(LPTHREAD_START_ROUTINE)ReadRoutine, (void *)index, 0, &ThreadID);

	if (devices[index].m_threadHandle == NULL)
	{
		CloseHandle(devices[index].m_threadHandle);
		CloseHandle(devices[index].m_writeHandle);
		CloseHandle(devices[index].m_readHandle);
		return;
	}

	//  make us high priority
	BOOL bSet = SetThreadPriority (devices[index].m_threadHandle, THREAD_PRIORITY_TIME_CRITICAL);
	IKASSERT(bSet);

	theEngine.AddDevice ( index, WriteRoutine, devType);
}

static void CloseDevice(int index)
{
	IKString::strcpy(devices[index].name,TEXT(""));

	theEngine.RemoveDevice(index);

	//  stop read thread
	//  wait for it to finish gracefully
	devices[index].m_bStopReading = true;
	if (devices[index].m_threadHandle)
		WaitForSingleObject(devices[index].m_threadHandle, INFINITE);

	//  Close open handles.
	CloseHandle(devices[index].m_readHandle);
	CloseHandle(devices[index].m_writeHandle);
	CloseHandle(devices[index].m_threadHandle);
	devices[index].m_readHandle		= NULL;	
	devices[index].m_writeHandle	= NULL;
	devices[index].m_threadHandle	= NULL;

}

bool IsWin2KOrGreater()
{
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof ( osvi );
	::GetVersionEx( &osvi );
	if (osvi.dwMajorVersion>=5)  //  Windows 2k is first version 5
		return true;
	return false;

}

static void SearchForDevices()
{
	//  what time is it
	unsigned int now = IKUtil::GetCurrentTimeMS();

	//  fwr 2/5/2003
	//  don't do the checking if a device installation is in progress
	//  only do for Win2k and up
	//  this seems to add a measurable delay to plug-in

	if (IsWin2KOrGreater())
	{
		if (IsDeviceInstallInProgress())
		{
			//TRACE1(TEXT("device install in progress at %d"),now);
			return;
		}
	}

	//  find new list of devices
	//  look for IntelliKeys and IntelliSwitch

	int nNewDevices = 0;
	mystr255 newDevices[MAX_INTELLIKEYS];
	nNewDevices = FindMyHidDevices ( DATAI(TEXT("IK_Vendor_ID"),0), DATAI(TEXT("IK_Product_ID"),0), MAX_INTELLIKEYS, newDevices );

	int nNewISDevices = 0;
	mystr255 newISDevices[MAX_INTELLIKEYS];
	nNewISDevices = FindMyHidDevices ( DATAI(TEXT("IS_Vendor_ID"),0), DATAI(TEXT("IS_Product_ID"),0), MAX_INTELLIKEYS, newISDevices );


	//  Disappearance: any old devices NOT in the new list?
	for (int iOld=0;iOld<MAX_INTELLIKEYS;iOld++)
	{
		if ( IKString::strcmp(devices[iOld].name,TEXT("")) !=0 )
		{
			bool bStillHere = false;
			int iNew;
			for (iNew=0;iNew<nNewDevices;iNew++)
			{
				TCHAR s[255];
				IKString::ConvertCToT(s,newDevices[iNew].data);
				if (IKString::strcmp(devices[iOld].name,s)==0)
					bStillHere = true;
			}
			for (iNew=0;iNew<nNewISDevices;iNew++)
			{
				TCHAR s[255];
				IKString::ConvertCToT(s,newISDevices[iNew].data);
				if (IKString::strcmp(devices[iOld].name,s)==0)
					bStillHere = true;
			}
			if (!bStillHere)
			{
				//  Gone.  close the device.
				CloseDevice(iOld);
			}
		}
	}


	//  Appearance: any new devices NOT in the old list?
	int iNew;
	for (iNew=0;iNew<nNewDevices;iNew++)
	{
		bool bWasThere = false;
		for (int iOld2=0;iOld2<MAX_INTELLIKEYS;iOld2++)
		{
			TCHAR s[255];
			IKString::ConvertCToT(s,newDevices[iNew].data);
			if (IKString::strcmp(devices[iOld2].name,s)==0)
				bWasThere = true;
		}
		if (!bWasThere)
		{
			//  New.  Fill empty list slot, open handles, start threads,
			//  notify monitor
			for (int i=0;i<MAX_INTELLIKEYS;i++)
			{
				if (IKString::strcmp(devices[i].name,TEXT(""))==0)
				{
					//  found empty slot.  Claim it and
					//  open the device.
					TCHAR s[255];
					IKString::ConvertCToT(s,newDevices[iNew].data);

					OpenDevice(i,s,1);
					break;
				}
			}
			
		}
	}

	for (iNew=0;iNew<nNewISDevices;iNew++)
	{
		bool bWasThere = false;
		for (int iOld2=0;iOld2<MAX_INTELLIKEYS;iOld2++)
		{
			TCHAR s[255];
			IKString::ConvertCToT(s,newISDevices[iNew].data);
			if (IKString::strcmp(devices[iOld2].name,s)==0)
				bWasThere = true;
		}
		if (!bWasThere)
		{
			//  New.  Fill empty list slot, open handles, start threads,
			//  notify monitor
			for (int i=0;i<MAX_INTELLIKEYS;i++)
			{
				if (IKString::strcmp(devices[i].name,TEXT(""))==0)
				{
					//  found empty slot.  Claim it and
					//  open the device.
					TCHAR s[255];
					IKString::ConvertCToT(s,newISDevices[iNew].data);

					OpenDevice(i,s,2);
					break;
				}
			}
			
		}
	}

}

DWORD ServiceExecutionThread(LPDWORD param)
{
	//  make us high priority
	HANDLE hProcess = GetCurrentProcess();
	BOOL bSet2 = SetPriorityClass ( hProcess, REALTIME_PRIORITY_CLASS );
	IKASSERT(bSet2);

	HANDLE hThread = GetCurrentThread();
	BOOL bSet = SetThreadPriority (hThread, THREAD_PRIORITY_TIME_CRITICAL);
	IKASSERT(bSet);

	//  one processor, please.
	IKUtil::SetOneProcessor();

	for (int i=0;i<MAX_INTELLIKEYS;i++)
	{
		devices[i].m_bStopReading = false;
		IKString::strcpy(devices[i].name,TEXT(""));
		devices[i].m_readHandle		= NULL;	
		devices[i].m_writeHandle	= NULL;
		devices[i].m_threadHandle	= NULL;
	}

	theEngine.Initialize();

	while(nServiceRunning && theEngine.IsRunning())
	{
		//  look for devices
		static unsigned int nextSearch = 0;
		unsigned int now = GetTickCount();
		if (now>nextSearch)
		{
			SearchForDevices();
			nextSearch= now + DATAI(TEXT("Device_Search_Period"), 1000);
		}

		//  pump the system
		theEngine.Periodic();

		//  wait a bit
		Sleep(DATAI(TEXT("Engine_Period"), 5));
	}

	theEngine.DisconnectDevices();
	theEngine.Periodic();
	Sleep(DATAI(TEXT("Engine_Period"), 5));

	theEngine.Stop();
	theEngine.Terminate();

	return 0;
}

#include <stdio.h>


int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR     lpCmdLine,
					 int       nCmdShow)
{


#ifdef SERVICE
	SERVICE_TABLE_ENTRY servicetable[]=
	{
		{strServiceName,(LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL,NULL}
	};

	BOOL success;
	success=StartServiceCtrlDispatcher(servicetable);

	if(!success)
	{
		//error occured
	}



#else

#ifdef WIN9X_SERVICE
	typedef DWORD (WINAPI * RegisterServiceProc)(DWORD, DWORD);
	RegisterServiceProc RegisterService = NULL;
	HINSTANCE kerneldll = NULL;

	// Obtain a handle to the kernel library
	kerneldll = LoadLibrary(TEXT("KERNEL32.DLL"));
	if (kerneldll)
	{
		// find the RegisterServiceProcess function
		RegisterService = (RegisterServiceProc) GetProcAddress(kerneldll, "RegisterServiceProcess");
	}
	
	// Register this process with the OS as a service!
	if (RegisterService)
		RegisterService(NULL, 1);

#endif

	nServiceRunning = true;
	ServiceExecutionThread(0);

#ifdef WIN9X_SERVICE
	// Then remove the service from the system service table
	if (RegisterService)
		RegisterService(NULL, 0);

	// Free the kernel library
	if (kerneldll)
		FreeLibrary(kerneldll);
#endif

#endif

	return 0;
}


#ifdef SERVICE
void ServiceMain(DWORD argc, LPTSTR *argv)
{
	BOOL success;
	nServiceStatusHandle=RegisterServiceCtrlHandler(strServiceName,
		(LPHANDLER_FUNCTION)ServiceCtrlHandler);
	if(!nServiceStatusHandle)
	{
		return;
	}
	success=UpdateServiceStatus(SERVICE_START_PENDING,NO_ERROR,0,1,3000);
	if(!success)
	{
		return;
	}
	killServiceEvent=CreateEvent(0,TRUE,FALSE,0);
	if(killServiceEvent==NULL)
	{
		return;
	}
	success=UpdateServiceStatus(SERVICE_START_PENDING,NO_ERROR,0,2,1000);
	if(!success)
	{
		return;
	}
	success=StartServiceThread();
	if(!success)
	{
		return;
	}
	nServiceCurrentStatus=SERVICE_RUNNING;
	success=UpdateServiceStatus(SERVICE_RUNNING,NO_ERROR,0,0,0);
	if(!success)
	{
		return;
	}
	WaitForSingleObject(killServiceEvent,INFINITE);
	CloseHandle(killServiceEvent);
}



BOOL UpdateServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode,
					 DWORD dwServiceSpecificExitCode, DWORD dwCheckPoint,
					 DWORD dwWaitHint)
{
	BOOL success;
	SERVICE_STATUS nServiceStatus;
	nServiceStatus.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
	nServiceStatus.dwCurrentState=dwCurrentState;
	if(dwCurrentState==SERVICE_START_PENDING)
	{
		nServiceStatus.dwControlsAccepted=0;
	}
	else
	{
		nServiceStatus.dwControlsAccepted=SERVICE_ACCEPT_STOP			
			|SERVICE_ACCEPT_SHUTDOWN;
	}
	if(dwServiceSpecificExitCode==0)
	{
		nServiceStatus.dwWin32ExitCode=dwWin32ExitCode;
	}
	else
	{
		nServiceStatus.dwWin32ExitCode=ERROR_SERVICE_SPECIFIC_ERROR;
	}
	nServiceStatus.dwServiceSpecificExitCode=dwServiceSpecificExitCode;
	nServiceStatus.dwCheckPoint=dwCheckPoint;
	nServiceStatus.dwWaitHint=dwWaitHint;

	success=SetServiceStatus(nServiceStatusHandle,&nServiceStatus);

	if(!success)
	{
		KillService();
		return success;
	}
	else
		return success;
}

BOOL StartServiceThread()
{	
	DWORD id;
	hServiceThread=CreateThread(0,0,
		(LPTHREAD_START_ROUTINE)ServiceExecutionThread,
		0,0,&id);
	if(hServiceThread==0)
	{
		return false;
	}
	else
	{
		nServiceRunning=true;
		return true;
	}
}


void KillService()
{
	nServiceRunning = false;
	while (theEngine.IsRunning())
		Sleep(100);

	SetEvent(killServiceEvent);
	UpdateServiceStatus(SERVICE_STOPPED,NO_ERROR,0,0,0);
}

void ServiceCtrlHandler(DWORD nControlCode)
{
	BOOL success;
	switch(nControlCode)
	{	
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		nServiceCurrentStatus=SERVICE_STOP_PENDING;
		success=UpdateServiceStatus(SERVICE_STOP_PENDING,NO_ERROR,0,1,3000);
		KillService();		
		return;
	default:
		break;
	}

	UpdateServiceStatus(nServiceCurrentStatus,NO_ERROR,0,0,0);
}

#endif


