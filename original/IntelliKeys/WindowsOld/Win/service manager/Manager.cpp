//
//  Manager.cpp
//
//  this program installs, uninstalls, starts and stops the
//  IntelliKeys USB service.
//

// #include <iostream.h>
#include <windows.h>
#include <winsvc.h>

#include "..\servicename.h"

#define FUNCTION_START    1
#define FUNCTION_STOP     2
#define FUNCTION_INSTALL  3
#define FUNCTION_REMOVE   4

int main(int argc, char* argv[])
{
	//  get the argument;
	char arg[20] = "";
	if (argc>=2)
		strcpy(arg,argv[1]);

	//  what function?  must have a valid one.
	int function = 0;
	if (stricmp(arg,"start")==0)
		function = FUNCTION_START;
	if (stricmp(arg,"stop")==0)
		function = FUNCTION_STOP;
	if (stricmp(arg,"install")==0)
		function = FUNCTION_INSTALL;
	if (stricmp(arg,"remove")==0)
		function = FUNCTION_REMOVE;
	if (function==0)
		return 1;

	//  open service manager
	SC_HANDLE scm;
	scm=OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
	if (!scm)
		return 1;

	//  what to do?
	SC_HANDLE myServiceHandle;
	switch (function)
	{
	case FUNCTION_START:
		myServiceHandle=OpenService(scm,strServiceName,SERVICE_ALL_ACCESS);
		if (myServiceHandle)
		{
			StartService(myServiceHandle,0,NULL);
			CloseServiceHandle(myServiceHandle);
		}
		break;

	case FUNCTION_STOP:
		myServiceHandle=OpenService(scm,strServiceName,SERVICE_ALL_ACCESS);
		if (myServiceHandle)
		{
			SERVICE_STATUS m_SERVICE_STATUS;
			ControlService(myServiceHandle,SERVICE_CONTROL_STOP,&m_SERVICE_STATUS);
			CloseServiceHandle(myServiceHandle);
		}
		break;

	case FUNCTION_INSTALL:
		{
			//  get path of service
			TCHAR svcPath[255];
			GetPrivateProfileString (
				TEXT("IntelliKeys USB"), TEXT("service"), 
				strDefaultServicePath, svcPath,
				sizeof(svcPath), TEXT("itools_x.ini") );

			//  create it
			myServiceHandle=CreateService ( 
				scm,
				strServiceName,
				strServiceName,
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS,
				SERVICE_AUTO_START,
				SERVICE_ERROR_NORMAL,
				svcPath,
				0,0,0,0,0);
			if (myServiceHandle)
			{
				CloseServiceHandle(myServiceHandle);
			}
		}
		break;

	case FUNCTION_REMOVE:
		myServiceHandle=OpenService(scm,strServiceName,SERVICE_ALL_ACCESS);
		if (myServiceHandle)
		{
			//  stop it first
			SERVICE_STATUS m_SERVICE_STATUS;
			ControlService(myServiceHandle,SERVICE_CONTROL_STOP,&m_SERVICE_STATUS);

			//  now delete it
			DeleteService(myServiceHandle);

			CloseServiceHandle(myServiceHandle);
		}
		break;

	default:
		break;
	}

	//  close service manager
	CloseServiceHandle(scm);

	//  all done.
	return 0;

}
