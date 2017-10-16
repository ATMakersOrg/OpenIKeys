// shutdown.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "MessageClient.h"


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

	IKMessageInitialize();

	int response = IKStopServer();
	response = IKStopSystray();
	//response = IK2StopSystray();

	return 0;
}
