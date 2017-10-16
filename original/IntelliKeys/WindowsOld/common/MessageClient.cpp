#ifdef BUILD_CW
#ifdef PLAT_MACINTOSH
#include <string.h>
#endif
#endif

#include "IKCommon.h"
#include "IKMessage.h"
#include "IKMsg.h"
#include "IKString.h"
#include "IKUtil.h"
#include "IKFile.h"

#include "MessageClient.h"

extern "C" int IKSendOverlayWithName(TCHAR* overlayname, TCHAR* name, TCHAR* sender)
{
	BYTE data[1000];
	int ndata = 0;

	//  do we have unicode strings?
	data[ndata] = sizeof(TCHAR);
	ndata++;

	//  length of overlayname in chars
	data[ndata] = IKString::strlen(overlayname);
	ndata++;

	//  chars for overlayname
	IKString::strcpy((TCHAR*)&(data[ndata]), overlayname);
	ndata = ndata + IKString::strlen(overlayname) * sizeof(TCHAR);

	//  length of name in chars
	data[ndata] = IKString::strlen(name);
	ndata++;

	//  chars for name
	IKString::strcpy((TCHAR *)&(data[ndata]),name);
	ndata = ndata + IKString::strlen(name) * sizeof(TCHAR);

	//  length of sender in chars
	data[ndata] = IKString::strlen(sender);
	ndata++;

	//  chars for sender
	IKString::strcpy((TCHAR *)&(data[ndata]), sender);
	ndata = ndata + IKString::strlen(sender) * sizeof(TCHAR);

	// Final Data composition:
	// [Overlay.LEN][overlay name bytes][name.LEN][name bytes][sender.LEN][sender bytes]
	DebugLogToFile("IKSendOverlayWithName: start - Overlay: [%s] Name: [%s] Sender: [%s]", overlayname, name, sender);
	DebugLogToFile("IKSendOverlayWithName: calling IKMessage::Send(TEXT(\"engine\"), %d, data, %d, 0, 0, true)", kQuerySendOverlayWithName, ndata);

	int response = IKMessage::Send(TEXT("engine"), kQuerySendOverlayWithName, data, ndata, 0, 0, true);

	DebugLogToFile("IKSendOverlayWithName: IKMessage::Send end - Response: [%d][%s]", response, (LPSTR)IKMessage::GetMessageName(response));

	return response;

}

extern "C" int IKSendOverlay(TCHAR *overlayname )
{
	TCHAR s[255];
	int response;

	IKString::strcpy(s,overlayname);
	s[IKString::strlen(s)] = TEXT('\0');

	response = IKMessage::Send ( TEXT("engine"), kQuerySendOverlay, s, IKString::strlen(s)+1, 0, 0  );
	return response;
}

extern "C" int IKSendOverlayName(TCHAR *name )
{
	TCHAR s[255];
	int response;

	IKString::strcpy(s,name);
	s[IKString::strlen(s)] = TEXT('\0');

	response = IKMessage::Send ( TEXT("engine"), kQuerySendOverlayName, s, IKString::strlen(s)+1, 0, 0  );
	return response;
}

extern "C" int IKIsIntellikeysConnected()
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryIsIntellikeysConnected, 0, 0, 0, 0  );
	return response;
}

extern "C" int IKSendSettings( BYTE *set )
{
	int response;

	response = IKMessage::Send ( TEXT("engine"), kQuerySendSettings, (BYTE *)set, 24, 0, 0  );
	return response;
}

extern "C" int IKIsStandardOverlayInPlace()
{
	int response;

	response = IKMessage::Send ( TEXT("engine"), kQueryIsStandardOverlayInPlace, 0, 0, 0, 0  );
	return response;
}

extern "C" int IKShowControlPanel()
{
	int response;

	response = IKMessage::Send ( TEXT("engine"), kQueryShowControlPanel, 0, 0, 0, 0  );
	return response;
}

extern "C" int IKStopServer()
{
	int response;

	response = IKMessage::Send ( TEXT("engine"), kQueryShutdown, 0, 0, 0, 0  );
	return response;
}

extern "C" int IKStopSystray()
{
	int response;

	response = IKMessage::Send ( TEXT("system tray"), kQueryShutdown, 0, 0, 0, 0, false  );
	return response;
}

extern "C" int IKIsIntellikeysOn ()
{
	int response;

	response = IKMessage::Send ( TEXT("engine"), kQueryIsIntellikeysOn, 0, 0, 0, 0  );
	return response;
}

extern "C" int IKGetSettings(BYTE *set )
{
	int response;

	response = IKMessage::Send ( TEXT("engine"), kQueryGetSettings, (BYTE *)set, 24, 0, 0  );
	return response;
}

static TCHAR name[255];

extern "C" TCHAR * IKGetMessageName(int message )
{
	IKString::strcpy(name,(TCHAR *)IKMessage::GetMessageName(message));
	return name;
}

extern "C" int IKSetStudent(TCHAR *group, TCHAR *student )
{
	BYTE data[1000];
	int ndata = 0;

	//  do we have unicode strings?
	data[ndata] = sizeof(TCHAR);
	ndata++;

	//  length of group in chars
	data[ndata] = IKString::strlen(group);
	ndata++;

	//  chars for group
	IKString::strcpy((TCHAR *)&(data[ndata]),group);
	ndata = ndata + IKString::strlen(group) * sizeof(TCHAR);

	//  length of student in chars
	data[ndata] = IKString::strlen(student);
	ndata++;

	//  chars for student
	IKString::strcpy((TCHAR *)&(data[ndata]),student);
	ndata = ndata + IKString::strlen(student) * sizeof(TCHAR);

	int response = IKMessage::Send ( TEXT("engine"), kQuerySetStudent, data, ndata, 0, 0  );
	return response;
}

extern "C" bool IKIsEngineAlive()
{
	bool bAlive;
	bAlive = IKMessage::IsOwnerAlive(TEXT("engine"));
	return bAlive;
}

extern "C" bool IKIsControlPanelAlive()
{
	bool bAlive;
	bAlive = IKMessage::IsOwnerAlive(TEXT("control panel"));
	return bAlive;
}

extern "C" int	IKResetKeyboard()
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryResetKeyboard, 0, 0, 0, 0  );
	return response;
}

extern "C" int	IKBeep()
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryBeep, 0, 0, 0, 0  );
	return response;
}

extern "C" int	IKShortBeep()
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryShortBeep, 0, 0, 0, 0  );
	return response;
}

extern "C" int	IKStartDiagnosticMode()
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryStartDiagnosticMode, 0, 0, 0, 0  );
	return response;
}

extern "C" int	IKStopDiagnosticMode()
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryStopDiagnosticMode, 0, 0, 0, 0  );
	return response;
}

extern "C" int	IKGetDeviceArray(BYTE *array )
{
	int datalen;
	int response = IKMessage::Send ( TEXT("engine"), kQueryUSBIntelliKeysArray, 0, 0, array, &datalen  );
	return response;
}

extern "C" int	IKGetSoftwareVersion(TCHAR *version )
{
	int response = IKMessage::Send ( TEXT("engine"), kQuerySoftwareVersion, 0, 0, version, 0  );
	return response;
}

extern "C" int	IKGetFirmwareVersion(TCHAR *version )
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryFirmwareVersion, 0, 0, version, 0  );
	return response;
}

extern "C" int	IKGetLastSentOverlay(TCHAR *overlay )
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryLastSentOverlay, 0, 0, overlay, 0  );
	return response;

}

extern "C" int IKReceive(TCHAR *channel, int *command, void *data, int maxdata, int *datalen )
{	
	int response = IKMessage::Receive ( channel, command, data, maxdata, datalen );
	return response;
}

extern "C" int IKRespond(TCHAR *channel, int command, void *data, int datalen )
{
	int response = IKMessage::Respond ( channel, command, data, datalen );
	return response;
}

extern "C" void IKMessageInitialize()
{
	IKUtil::Initialize();
	IKMessage::Initialize();
}

extern "C" int	IKSetNotifyModeOn()
{
	int response = IKMessage::Send ( TEXT("engine"), kQuerySetNotifyModeOn, 0, 0, 0, 0  );
	return response;
}

extern "C" int	IKSetNotifyModeOff()
{
	int response = IKMessage::Send ( TEXT("engine"), kQuerySetNotifyModeOff, 0, 0, 0, 0  );
	return response;
}


extern "C" int	IKStartRawMode()
{	
	int response = IKMessage::Send ( TEXT("engine"), kQueryStartRawMode, 0, 0, 0, 0  );
	return response;
}

extern "C" int	IKStopRawMode()
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryStopRawMode, 0, 0, 0, 0  );
	return response;
}

extern "C" bool	IKIsRawModeOn()
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryGetRawMode, 0, 0, 0, 0  );
	return (response==kResponseRawModeOn);
}

extern "C" bool	IKIsRawModeUsable()
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryGetRawModeUsable, 0, 0, 0, 0  );
	return (response==kResponseRawModeUsable);
}

extern "C" int IKGetRawEvent(RawEvent *event )
{
	BYTE data[9];
	int ndata = 0;

	int response = IKMessage::Send ( TEXT("engine"), kQueryGetRawEvent, 0, 0, data, &ndata  );
	if (response==kResponseRawEvent)
	{
		//  fill in event
		event->deviceNumber = data[0];
		event->time = *((unsigned int *)&(data[5])) ;

		event->eventType	= 0;
		event->row			= 0;
		event->col			= 0;
		event->switchNumber	= 0;
		event->press		= 0;
		event->overlay		= 0;

		switch (data[1])
		{
		case 1:
			event->eventType = 1;
			event->press = (data[2]==1);
			event->row  = data[3];
			event->col  = data[4];
			break;

		case 2:
			event->eventType = 2;
			event->press = (data[2]==1);
			event->switchNumber = data[3];
			break;

		case 3:
			event->eventType = 3;
			event->overlay = data[2];
			break;
		}
	}
	return response;
}

extern "C" int IKGetRawEvents(RawEvent *events, int nEventsIn, int *nEventsOut )
{
	//  allocate data
	int nEventSize = 9;
	BYTE *data = new BYTE[nEventsIn*nEventSize+1+2];  //  plus two because SendTO ads two bytes

	//  ask for data
	int ndata = 0;
	BYTE ne = nEventsIn;
	int response = IKMessage::Send ( TEXT("engine"), kQueryGetRawEvents, &ne, 1, data, &ndata, false  );
	if (response==kResponseRawEvents)
	{
		//  convert returned data into eventa
		int nEvents = data[0];
		for (int i=0;i<nEvents;i++)
		{
			//  fill in event
			events[i].deviceNumber = data[1+i*nEventSize+0];
			events[i].time = *((unsigned int *)&(data[1+i*nEventSize+5])) ;

			events[i].eventType		= 0;
			events[i].row			= 0;
			events[i].col			= 0;
			events[i].switchNumber	= 0;
			events[i].press			= 0;
			events[i].overlay		= 0;

			switch (data[1+i*nEventSize+1])
			{
			case 1:
				events[i].eventType = 1;
				events[i].press		= (data[1+i*nEventSize+2]==1);
				events[i].row		= data[1+i*nEventSize+3];
				events[i].col		= data[1+i*nEventSize+4];
				break;

			case 2:
				events[i].eventType		= 2;
				events[i].press			= (data[1+i*nEventSize+2]==1);
				events[i].switchNumber	= data[1+i*nEventSize+3];
				break;

			case 3:
				events[i].eventType = 3;
				events[i].overlay	= data[1+i*nEventSize+2];
				break;
			}
		}
		*nEventsOut = nEvents;
	}

	//  free data
	delete [] data;

	return response;

}

// mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm

extern "C" int IK2SendOverlayWithName(TCHAR *overlayname, TCHAR *name, TCHAR *sender )
{
	BYTE data[1000];
	int ndata = 0;

	//  do we have unicode strings?
	data[ndata] = sizeof(TCHAR);
	ndata++;

	//  length of overlayname in chars
	data[ndata] = IKString::strlen(overlayname);
	ndata++;

	//  chars for overlayname
	IKString::strcpy((TCHAR *)&(data[ndata]),overlayname);
	ndata = ndata + IKString::strlen(overlayname) * sizeof(TCHAR);

	//  length of name in chars
	data[ndata] = IKString::strlen(name);
	ndata++;

	//  chars for name
	IKString::strcpy((TCHAR *)&(data[ndata]),name);
	ndata = ndata + IKString::strlen(name) * sizeof(TCHAR);

	//  length of sender in chars
	data[ndata] = IKString::strlen(sender);
	ndata++;

	//  chars for sender
	IKString::strcpy((TCHAR *)&(data[ndata]),sender);
	ndata = ndata + IKString::strlen(sender) * sizeof(TCHAR);

	int response = IKMsg::Send ( TEXT("sending"), kQuerySendOverlayWithName, data, ndata, 0, 0  );
	return response;

}

extern "C" int IK2SendOverlay(TCHAR *overlayname )
{
	TCHAR s[255];
	int response;

	IKString::strcpy(s,overlayname);
	s[IKString::strlen(s)] = TEXT('\0');

	response = IKMsg::Send ( TEXT("sending"), kQuerySendOverlay, s, IKString::strlen(s)+1, 0, 0  );
	return response;
}

extern "C" int IK2SendOverlayName(TCHAR *name )
{
	TCHAR s[255];
	int response;

	IKString::strcpy(s,name);
	s[IKString::strlen(s)] = TEXT('\0');

	response = IKMsg::Send ( TEXT("sending"), kQuerySendOverlayName, s, IKString::strlen(s)+1, 0, 0  );
	return response;
}

extern "C" int IK2IsIntellikeysConnected()
{
	int response = IKMsg::Send ( TEXT("sending"), kQueryIsIntellikeysConnected, 0, 0, 0, 0  );
	return response;
}

extern "C" int IK2SendSettings( BYTE *set )
{
	int response;

	response = IKMsg::Send ( TEXT("sending"), kQuerySendSettings, (BYTE *)set, 24, 0, 0  );
	return response;
}

extern "C" int IK2IsStandardOverlayInPlace()
{
	int response;

	response = IKMsg::Send ( TEXT("sending"), kQueryIsStandardOverlayInPlace, 0, 0, 0, 0  );
	return response;
}

extern "C" int IK2ShowControlPanel()
{
	int response;

	response = IKMsg::Send ( TEXT("sending"), kQueryShowControlPanel, 0, 0, 0, 0  );
	return response;
}

extern "C" int IK2StopServer()
{
	int response;

	response = IKMsg::Send ( TEXT("sending"), kQueryShutdown, 0, 0, 0, 0  );
	return response;
}

extern "C" int IK2StopSystray()
{
	int response;

	response = IKMsg::Send ( TEXT("system tray"), kQueryShutdown, 0, 0, 0, 0  );
	return response;
}

extern "C" int IK2IsIntellikeysOn()
{
	int response;

	response = IKMsg::Send ( TEXT("sending"), kQueryIsIntellikeysOn, 0, 0, 0, 0  );
	return response;
}

extern "C" int IK2GetSettings(BYTE *set )
{
	int response;

	response = IKMsg::Send ( TEXT("sending"), kQueryGetSettings, (BYTE *)set, 24, 0, 0  );
	return response;
}


extern "C" TCHAR * IK2GetMessageName(int message )
{
	IKString::strcpy(name,(TCHAR *)IKMessage::GetMessageName(message));
	return name;
}

extern "C" int IK2SetStudent(TCHAR *group, TCHAR *student )
{
	BYTE data[1000];
	int ndata = 0;

	//  do we have unicode strings?
	data[ndata] = sizeof(TCHAR);
	ndata++;

	//  length of group in chars
	data[ndata] = IKString::strlen(group);
	ndata++;

	//  chars for group
	IKString::strcpy((TCHAR *)&(data[ndata]),group);
	ndata = ndata + IKString::strlen(group) * sizeof(TCHAR);

	//  length of student in chars
	data[ndata] = IKString::strlen(student);
	ndata++;

	//  chars for student
	IKString::strcpy((TCHAR *)&(data[ndata]),student);
	ndata = ndata + IKString::strlen(student) * sizeof(TCHAR);

	int response = IKMsg::Send ( TEXT("sending"), kQuerySetStudent, data, ndata, 0, 0  );
	return response;
}


extern "C" int	IK2ResetKeyboard()
{
	int response = IKMsg::Send ( TEXT("sending"), kQueryResetKeyboard, 0, 0, 0, 0  );
	return response;
}

extern "C" int	IK2Beep()
{
	int response = IKMsg::Send ( TEXT("sending"), kQueryBeep, 0, 0, 0, 0  );
	return response;
}

extern "C" int	IK2StartDiagnosticMode()
{
	int response = IKMsg::Send ( TEXT("sending"), kQueryStartDiagnosticMode, 0, 0, 0, 0  );
	return response;
}

extern "C" int	IK2StopDiagnosticMode()
{
	int response = IKMsg::Send ( TEXT("sending"), kQueryStopDiagnosticMode, 0, 0, 0, 0  );
	return response;
}

extern "C" int	IK2GetDeviceArray(BYTE *array )
{
	int datalen;
	int response = IKMsg::Send ( TEXT("sending"), kQueryUSBIntelliKeysArray, 0, 0, array, &datalen  );
	return response;
}

extern "C" int	IK2GetSoftwareVersion(TCHAR *version )
{
	int response = IKMsg::Send ( TEXT("sending"), kQuerySoftwareVersion, 0, 0, version, 0  );
	return response;
}

extern "C" int	IK2GetFirmwareVersion(TCHAR *version )
{
	int response = IKMsg::Send ( TEXT("sending"), kQueryFirmwareVersion, 0, 0, version, 0  );
	return response;
}

extern "C" int	IK2GetLastSentOverlay(TCHAR *overlay )
{
	int response = IKMsg::Send ( TEXT("sending"), kQueryLastSentOverlay, 0, 0, overlay, 0  );
	return response;
}

extern "C" int IK2Receive(TCHAR *channel, int *command, void *data, int maxdata, int *datalen )
{	
	int response = IKMsg::Receive ( channel, command, data, maxdata, datalen );
	return response;
}

extern "C" int IK2Respond(TCHAR *channel, int command, void *data, int datalen )
{
	int response = IKMsg::Respond ( channel, command, data, datalen );
	return response;
}

extern "C" bool IKDebugSender()
{
	int i = DATAI(TEXT("Debug_Sender"),0);
	return !!i;
}

extern "C" void IKMakeWriteable(TCHAR *filename)
{
	IKString s(filename);
	IKFile::MakeWritable(s, 0);
}

extern "C" int IKPostKey(int code, int direction)
{
	char data[100]; int ndata;
	sprintf ( data, "%d,%d,", code, direction );  ndata = strlen(data)+1;
	
	int response = IKMessage::Send ( TEXT("engine"), kQueryPostKey, (BYTE *)data, ndata, 0, 0  );
	return response;
}

extern "C" int IKPostDelay(int delay)
{
	char data[100]; int ndata;
	sprintf ( data, "%d,", delay );  ndata = strlen(data)+1;
	
	int response = IKMessage::Send ( TEXT("engine"), kQueryPostDelay, (BYTE *)data, ndata, 0, 0  );
	return response;
}

extern "C" int IKPostMouseButtonToggle(int whichButton)
{
	char data[100]; int ndata;
	sprintf ( data, "%d,", whichButton );  ndata = strlen(data)+1;
	
	int response = IKMessage::Send ( TEXT("engine"), kQueryPostMouseButtonToggle, (BYTE *)data, ndata, 0, 0  );
	return response;
}

extern "C" int IKPostMouseButtonClick(int whichButton)
{
	char data[100]; int ndata;
	sprintf ( data, "%d,", whichButton );  ndata = strlen(data)+1;
	
	int response = IKMessage::Send ( TEXT("engine"), kQueryPostMouseButtonClick, (BYTE *)data, ndata, 0, 0  );
	return response;
}

extern "C" int IKPostMouseButtonDoubleClick(int whichButton)
{
	char data[100]; int ndata;
	sprintf ( data, "%d,", whichButton );  ndata = strlen(data)+1;
	
	int response = IKMessage::Send ( TEXT("engine"), kQueryPostMouseButtonDoubleClick, (BYTE *)data, ndata, 0, 0  );
	return response;
}

extern "C" int IKPostMouseButton(int whichButton, int direction)
{
	char data[100]; int ndata;
	sprintf ( data, "%d,%d,", whichButton, direction );  ndata = strlen(data)+1;
	
	int response = IKMessage::Send ( TEXT("engine"), kQueryPostMouseButton, (BYTE *)data, ndata, 0, 0  );
	return response;
}

extern "C" int IKPostMouseMove(int x, int y)
{
	char data[100]; int ndata;
	sprintf ( data, "%d,%d,", x, y );  ndata = strlen(data)+1;
	
	int response = IKMessage::Send ( TEXT("engine"), kQueryPostMouseMove, (BYTE *)data, ndata, 0, 0  );
	return response;
}

extern "C" int IKResetMouseInterface()
{	
	int response = IKMessage::Send ( TEXT("engine"), kQueryPostKey, 0, 0, 0, 0  );
	return response;
}

extern "C" int IKResetKeyboardInterface()
{
	int response = IKMessage::Send ( TEXT("engine"), kQueryPostKey, 0, 0, 0, 0  );
	return response;
}
