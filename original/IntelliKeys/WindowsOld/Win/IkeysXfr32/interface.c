/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		interface.c
//
// Purpose:		messaging primarily involved with overlay sending and (Windows) sleep / wake
//
// 06/18/01 fwr initial implementation	
// 08/21/01 dgs Added handling of WM_INPUTLANGCHANGE in WindowProc()
**************************************************************************************************************************/

#include <Windows.h>
#include <stdio.h>

//  TODO: is this safe on every windows system?
static const char* pFallbackDataFileName = "c:\\ikusb.dat";

static const char* pMessageWindowName = "IKUSB Message Window";
static const int kDefaultTimeout = 1000;  //  msec

#include "interface.h"

static int SetCommand ( HWND hWnd, int command )
{
	//  park the command in the x-coordinate of the window
	BOOL bMoved;
	bMoved = MoveWindow( hWnd, command, 0, 0, 0, 0);
	return 1;
}

static int GetCommand ( HWND hWnd, int *pCommand )
{
	//  get the command from the x-coordinate of the window
	RECT rect;
	BOOL bResult;

	bResult = GetWindowRect( hWnd, &rect );
	*pCommand = rect.left;
	return 1;
}

static SERVERPROC *pTheProc = NULL;

static WINMSGPROC *pWinMsgProc = NULL;

static WINMSGPROCWITHRESULT *pWinMsgProcWithResult = NULL;

int IKUSBRegisterWinMsgProc ( WINMSGPROC *pProc )
{
	pWinMsgProc = pProc;
	return kResponsev1NoError;
}

int IKUSBRegisterWinMsgProcWithResult ( WINMSGPROCWITHRESULT *pProc )
{
	pWinMsgProcWithResult = pProc;
	return kResponsev1NoError;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,WPARAM wParam, LPARAM lParam )
{
	LRESULT procResult;
	BOOL bHaveCustomResult = FALSE;

	if(uMsg==WM_DESTROY)
	{
		if (pTheProc)
			(*pTheProc)(kServerCallbackTerminating,0);
	}

	if ( pWinMsgProcWithResult &&  ( (uMsg==WM_POWERBROADCAST) || (uMsg==WM_DEVICECHANGE)  || (uMsg==WM_INPUTLANGCHANGE) ) )
	{
		procResult = (*pWinMsgProcWithResult)(uMsg,wParam,lParam);
		if (wParam == PBT_APMQUERYSUSPEND)
		{
			bHaveCustomResult = TRUE;
		}
	}
	else if (pWinMsgProc)
	{
		(*pWinMsgProc)(uMsg,wParam,lParam);
	}

	if (!bHaveCustomResult)
	{
		procResult = DefWindowProc ( hwnd, uMsg, wParam, lParam );
	}

	return procResult;
}


int  IKUSBRegisterServer (SERVERPROC *pProc, HWND *window)
{

	WNDCLASS wc;
	HWND hWnd;

	//  have we done this already?
	hWnd = FindWindow( NULL, pMessageWindowName );
	if ( hWnd != NULL )
		return kResponsev1AlreadyRegistered;

	//  try to create the window
	wc.style         = CS_GLOBALCLASS;
	wc.lpfnWndProc   = WindowProc;
	wc.cbClsExtra    = 4; // msgClassExtra;
	wc.cbWndExtra    = 4; // msgWndExtra;
	wc.hInstance     = 0; // AfxGetInstanceHandle();
	wc.hIcon         = 0;
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) COLOR_HIGHLIGHT;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = pMessageWindowName;

	if ( !RegisterClass(&wc) )
		return kResponsev1RegistrationError;

	hWnd = CreateWindow( pMessageWindowName, pMessageWindowName,
		WS_OVERLAPPED /*| WS_VISIBLE*/, 0,0,100,100,
		NULL, NULL, 0 /*AfxGetInstanceHandle()*/, NULL );
	if ( hWnd == NULL )
		return kResponsev1RegistrationError;

	pTheProc = pProc;
	*window = hWnd;

	return kResponsev1NoError;

}

static int WriteDataFile (void *data, int datalength)
{
	FILE *out;
	int result;

	//  open the file
	char filename[255];
	IKUSBGetCommFile(filename);

	out = fopen(filename,"w+");
	if(out==NULL)
		return kResponsev1FileError;

	//  seek to beginning
	result = fseek(out,0,0);
	if(result!=0)
	{
		fclose(out);
		return kResponsev1FileError;
	}

	//  write the data
	if (datalength>0)
	{
		result = fwrite(data,datalength,1,out);
		if (result!=1)
		{
			fclose(out);
			return kResponsev1FileError;
		}
	}

	//  flush and close
	fflush(out);
	fclose(out);
	return kResponsev1NoError;
}

static int ReadDataFile ( void *data, int maxdata, int *datalength )
{
	FILE *in;
	int result;
	char c;
	int nbytes;

	//  open the file
	char filename[255];
	IKUSBGetCommFile(filename);
	in = fopen(filename,"r");
	if(in==NULL)
		return kResponsev1FileError;

	//  read the data
	nbytes = 0;
	while(1==1)
	{
		result = fread ( &c, 1, 1, in );
		if(result!=1)
			break;

		if(nbytes+1<=maxdata)
		{
			((char *)data)[nbytes] = c;
			nbytes++;
		}
	}
	*datalength = nbytes;

	// close
	fclose(in);
	return kResponsev1NoError;
}

//static void message(char *string )
//{
//	MessageBox(0,string,"control panel",MB_OK);
//}

int IKUSBIsServerRunning ()
{
	//  find the message window
	HWND hWnd = FindWindow(NULL, pMessageWindowName);
	if (hWnd == NULL)
	{
		return kResponsev1NoServer;
	}
	//  A-OK.
	return kResponsev1NoError;
}

int IKUSBSendMessage(int command, void *data, int datalength, int callertimeout)
{
	int writeStatus;
	WORD cmd=0;
	int timeout;
	int timer;
	int response;

	//  find the message window
	HWND hWnd = FindWindow(NULL, pMessageWindowName);
	if (hWnd == NULL)
	{
		return kResponsev1NoServer;
	}

	//  write any output data
	writeStatus = WriteDataFile(data,datalength);
	if (writeStatus == kResponsev1FileError)
	{
		return kResponsev1FileError;
	}

	//  set the command
	if (SetCommand(hWnd, command ) == 0)
	{
		return kResponsev1Error;
	}

	//  wait for a response
	if (callertimeout == 0)
	{
		timeout = kDefaultTimeout;
	}
	else
	{
		timeout = callertimeout;
	}
	timer = 0;
	while (timer < timeout)
	{
		GetCommand(hWnd, &response);
		if ((response < 0) && (response != kResponsev1Handshake))
		{
			break;
		}

		Sleep(10);
		timer = timer + 10;  //  this could be more accurate
	}

	SetCommand(hWnd, 0);
	if (timer >= timeout)
	{
		//message("timer expired");
		return kResponsev1Timeout;
	}
	if (response == kResponsev1Handshake)
	{
		//message("still handshaking");
		return kResponsev1Timeout;
	}
	if (response > 0)
	{
		//message("response greater than zero");
		return kResponsev1Timeout;
	}
	return response;
}

int  IKUSBRespondToMessage ( int response, void *data, int datalength )
{
	int writeStatus;

	//  find the message window
	HWND hWnd = FindWindow( NULL, pMessageWindowName );
	if ( hWnd == NULL )
		return kResponsev1NoServer;

	//  write any data
	writeStatus = WriteDataFile(data,datalength);
	if (writeStatus==kResponsev1FileError)
		return kResponsev1FileError;

	//  set the response
	if (SetCommand ( hWnd, response)==0)
		return kResponsev1Error;

	//  all done
	return kResponsev1NoError;
}

int  IKUSBReceiveMessage ( int *command, void *data, int maxdata, int *dataread )
{
	int readStatus;
	int cmd;

	//  find the message window
	HWND hWnd = FindWindow( NULL, pMessageWindowName );
	if ( hWnd == NULL )
		return kResponsev1NoServer;

	//  read the command and set the handshake
	if (GetCommand(hWnd,&cmd)==0)
		return kResponsev1Error;
	if(cmd==kResponsev1NoCommand)
		return cmd;

	//  set the handshake
	if (SetCommand(hWnd,kResponsev1Handshake)==0)
		return kResponsev1Error;

	//  read any data
	readStatus = ReadDataFile ( data, maxdata, dataread );
	if (readStatus==kResponsev1FileError)
		return kResponsev1FileError;

	*command = cmd;
	return kResponsev1NoError;
}

/* ------------------------------------------------------- */

int IKUSBSendOverlay ( char *overlayname )
{
	char s[255];
	int response;

	strcpy(s,overlayname);
	s[strlen(overlayname)] = '\0';
	response = IKUSBSendMessage ( kQueryv1SendOverlay, s, strlen(overlayname)+1, kDefaultTimeout );
	return response;
}

int IKUSBSendOverlayName ( char *name )
{
	char s[255];
	int response;

	strcpy(s,name);
	s[strlen(name)] = '\0';
	response = IKUSBSendMessage ( kQueryv1SendOverlayName, s, strlen(name)+1, kDefaultTimeout );
	return response;
}

int IKUSBSendOverlayWithName ( char *overlayname, char *name )
{
	char s[255];
	int response;

	//  file name
	strcpy(s,overlayname);
	//  separator
	strcat(s,"|");
	//  name
	strcat(s,name);
	s[strlen(s)] = '\0';

	response = IKUSBSendMessage ( kQueryv1SendOverlayWithName, s, strlen(s)+1, kDefaultTimeout );

	return response;
}


int IKUSBShowControlPanel ()
{
	int response = IKUSBSendMessage ( kQueryv1ShowControlPanel, 0, 0, kDefaultTimeout );
	return response;
}

int IKUSBStopServer ()
{
	int response = IKUSBSendMessage ( kQueryv1StopServer, 0, 0, kDefaultTimeout );
	return response;
}


int IKUSBIsIntellikeysConnected ()
{
	int response = IKUSBSendMessage ( kQueryv1IsIntellikeysConnected, 0, 0, kDefaultTimeout );
	return response;
}

int IKUSBIsIntellikeysOn ()
{
	int response = IKUSBSendMessage ( kQueryv1IsIntellikeysOn, 0, 0, kDefaultTimeout );
	return response;
}

int IKUSBSimulateInput ()
{
	int response = IKUSBSendMessage ( kQueryv1SimulateInput, 0, 0, kDefaultTimeout );
	return response;
}

int IKUSBSendSettings ( BYTE *set )
{
	int response;

	response = IKUSBSendMessage ( kQueryv1SendSettings, set, 24, kDefaultTimeout );
	return response;
}

int IKUSBGetSettings ( BYTE *set )
{
	return kResponsev1Error;
}

int IKUSBIsStandardOverlayInPlace()
{
	int response;

	response = IKUSBSendMessage ( kQueryv1IsStandardOverlayInPlace, 0, 0, kDefaultTimeout );
	return response;
}

void IKUSBFlushMessages()
{
	int timeout = 50; // .25 second
	while ( timeout )
	{
		MSG msg;
		if ( PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE ) )
			DispatchMessage ( &msg );
		
		Sleep(5);
		--timeout;
	}

}


int  IKUSBIsServerRegistered ()
{
	HWND hWnd;

	//  have we done this already?
	hWnd = FindWindow( NULL, pMessageWindowName );
	if ( hWnd != NULL )
		return 1;

	return 0;

}

void IKUSBGetCommFile(char *filename)
{
	//strcpy(filename,pDataFileName);

	GetPrivateProfileString( "IntelliKeys USB", "commfile", "", filename,
								255, "itools_x.ini" );

	if (stricmp(filename,"")==0)
		strcpy(filename,pFallbackDataFileName);

}


