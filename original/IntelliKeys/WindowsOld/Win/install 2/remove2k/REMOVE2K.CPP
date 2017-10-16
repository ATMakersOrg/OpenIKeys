// remove2k.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <stdio.h> 
#include <tchar.h> // Make all functions UNICODE safe.
#include <windows.h>  
#include <setupapi.h> // for SetupDiXxx functions.
#include <string.h>


//  moved into a common file so that
//  installhelper could share...

#include "remove2k-inc.cpp"


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

	Remove2K();
	return 0;
}



