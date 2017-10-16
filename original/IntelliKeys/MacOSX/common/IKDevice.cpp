// IKDevice.cpp: implementation of the IKDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "IKCommon.h"
#include "IKDevice.h"
#include "IKUtil.h"
#include "IKControlPanel.h"
#include "IKEngine.h"
#include "IKMessage.h"
#include "AppLib.h"


#ifdef PLAT_MACINTOSH
#include <stdio.h>
#include <string.h>
#endif

#ifdef PLAT_MACINTOSH_CLASSIC
#include <events.h>
#endif

#define JWH 1
#include "firmware/firmware.h"

#include "IKUniversal.h"
#include "IKUniversalToHid.h"

#ifdef PLAT_WINDOWS
  #include "dll.h"
  #define GET_TOGGLE_STATES(a,b) DllGetStates(a,b)
  #define KEYEVENTF_UNICODE 4
  #include <windows.h>
  #include <winable.h>
  #include <Tchar.h>
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IKDevice::IKDevice()
{
	m_devType = 0;
	Reset();
}

IKDevice::~IKDevice()
{

}


void IKDevice::OnDataReceived(BYTE *data)
{
	//  be careful in here to only queueup the data
	//  given.  This function may be called 
	//  directly by a Macintosh
	//  device driver
	
	queueEntry qe;
	for (int i=0;i<IK_REPORT_LEN;i++)
		qe.buffer[i] = data[i];
	m_inputQueue.enqueue(qe);
}

void IKDevice::PostCommand(BYTE *command)
{
	queueEntry qe;
	for (int i=0;i<IK_REPORT_LEN;i++)
		qe.buffer[i] = command[i];
	m_commandQueue.enqueue(qe);
}

void IKDevice::SendCommand(BYTE *command)
{
	//  call registered write routine here

	if (m_writeProc!=NULL)
	{
		(*m_writeProc)(m_index,command);
	}
}

void IKDevice::Periodic()
{
	if (!IsOpen())
		return;

	//  settle overlay
	SettleOverlay();

	//  setLEDs
	unsigned int now = IKUtil::GetCurrentTimeMS();
	if (now > m_lastLEDTime + DATAI(TEXT("Led_Reconcile_Period"),100))
	{
		SetLEDs();
		m_lastLEDTime = now;
	}

	//  request a correction every so often.
	if(now > m_nextCorrect)
	{
		DoCorrect();
		m_nextCorrect = now + DATAI(TEXT("Correct_Period"),500);
	}

	//  send for not-yet valid eeprom bytes
	if (!m_bEepromValid)
	{
		for (unsigned int i=0;i<sizeof(eeprom);i++)
		{
			if (!m_eepromDataValid[i] && 
				m_eepromRequestTime[i]+DATAI(TEXT("EEProm_Byte_Overdue"),500) < now)
			{
				BYTE report[8] = {IK_CMD_EEPROM_READBYTE,0x80+i,0x1F,0,0,0,0,0};
				PostCommand(report);	
				m_eepromRequestTime[i] = now;
			}
		}
	}
	
	ProcessInput();
	ProcessCommands();

	if (IKEngine::GetEngine()->GetDiagnosticMode())
	{
		//  do nothing
	}
	else
	{		
		if (IKEngine::GetEngine()->UseRawMode() && !HasStandardOverlay() && !IKEngine::GetEngine()->GetDiagnosticMode())
			InterpretRaw();
		else
			Interpret();
	}

}

void IKDevice::Start()
{
	PurgeQueues();

	BYTE command[IK_REPORT_LEN] = {0,0,0,0,0,0,0,0};

	command[0] = IK_CMD_INIT;
	command[1] = 0;  //  interrupt event mode
	PostCommand(command);

	command[0] = IK_CMD_SCAN;
	command[1] = 1;	//  enable
	PostCommand(command);

	PostDelay ( 250);

	command[0] = IK_CMD_ALL_SENSORS;
	command[1] = 0;	//  unused
	PostCommand(command);

	//  reset keyboard
	ResetKeyboard();

	//  reset mouse
	ResetMouse();

#ifdef PLAT_WINDOWS
	KickDLL();
#endif

}

void IKDevice::ProcessCommands()
{
	//  come back later if IK_ICMD_DELAY has set a future time
	if (IKUtil::GetCurrentTimeMS() < m_delayUntil)
	{
		return;
	}

	//  process any pending commands

	while (!m_commandQueue.isEmpty())
	{
		queueEntry qe;
		m_commandQueue.dequeue(qe);

		switch (qe.buffer[0])
		{
		case IK_CMD_DELAY:
			m_delayUntil = IKUtil::GetCurrentTimeMS() + qe.buffer[1];
			return;
			break;

		case IK_CMD_KEYBOARD:
			DoKey(qe.buffer[1],qe.buffer[2]);
			{
				int delayAfter = qe.buffer[4]*256 + qe.buffer[3];
				if (delayAfter>0)
				{
					m_delayUntil = IKUtil::GetCurrentTimeMS() + delayAfter;
					return;
				}
			}
			break;

		case IK_CMD_LIFTALLMODIFIERS:
			LiftAllModifiers();
			break;

		case IK_CMD_KEYBOARD_UNICODE:
			DoKeyUnicode(qe.buffer[1],qe.buffer[2],qe.buffer[3]);
			break;

		case IK_CMD_MOUSE_MOVE:
			DoMouseMove(qe.buffer[1],qe.buffer[2]);
			break;

		case IK_CMD_MOUSE_BUTTON:
			DoMouseButton(qe.buffer[1],qe.buffer[2]);
			break;

		case IK_CMD_KEY_DONE:
			DoKeyDone();
			break;

		case IK_CMD_KEY_START:
			DoKeyStart();
			break;

		case IK_CMD_KEY_REPEAT:
			DoKeyRepeat();
			break;

		case IK_CMD_CP_HELP:
			IKControlPanel::Help();
			break;

		case IK_CMD_CP_LIST_FEATURES:
			IKControlPanel::ListFeatures();
			break;

		case IK_CMD_CP_REFRESH:
			IKControlPanel::Refresh();
			break;

		case IK_CMD_CP_TOGGLE:
			IKControlPanel::Toggle();
			break;

		case IK_CMD_LED:
			DoSetLED ( qe.buffer[1], qe.buffer[2] );
			break;

		case IK_CMD_CP_REPORT_REALTIME:
			ReportDataToControlPanel(!!qe.buffer[1]);
			break;

		default:
			SendCommand(qe.buffer);
			break;
		}
	}
}


void IKDevice::PostSetLED(int number, int value)
{
	BYTE command[IK_REPORT_LEN] = {IK_CMD_LED,number,value,0,0,0,0,0};
	PostCommand(command);
}

void IKDevice::PostDelay(int msec)
{
	BYTE command[IK_REPORT_LEN];
	command[0] = IK_CMD_DELAY;
	command[1] = msec;  //  msec delay
	PostCommand(command);
}

void IKDevice::ProcessInput()
{
	while (!m_inputQueue.isEmpty())
	{
		queueEntry qe;
		m_inputQueue.dequeue(qe);

#if defined (DEBUG_CONTROL_PANEL)|defined (DEBUG_IKUSB)
        if (qe.buffer[0] != 55 && qe.buffer[0] != 64 && (qe.buffer[1] + qe.buffer[2]) > 0) {
            NSLog (@"IKDevice::ProcessInput: (%d) (%d) (%d)", 
                   qe.buffer[0], qe.buffer[1], qe.buffer[2]);
        }
#endif
		switch (qe.buffer[0])
		{
		case IK_EVENT_MEMBRANE_PRESS:
if (0) {
            if ((qe.buffer[1] + qe.buffer[2]) > 0) {
                NSLog (@"IKDevice::ProcessInput: IK_EVENT_MEMBRANE_PRESS byte[1] (%d); byte[2] (%d);", 
                       qe.buffer[1], qe.buffer[2]);
            }
}
			OnMembranePress(qe.buffer[1],qe.buffer[2]);
			if (IKEngine::GetEngine()->UseRawMode()&&!HasStandardOverlay()&&!IKEngine::GetEngine()->GetDiagnosticMode())
				InterpretRaw();
			PostReportDataToControlPanel();
			break;

		case IK_EVENT_MEMBRANE_RELEASE:
if (0) {
            if ((qe.buffer[1] + qe.buffer[2]) > 0) {
                NSLog (@"IKDevice::ProcessInput: IK_EVENT_MEMBRANE_RELEASE byte[1] (%d); byte[2] (%d);", 
                       qe.buffer[1], qe.buffer[2]);
            }
}
			OnMembraneRelease(qe.buffer[1],qe.buffer[2]);
			if (IKEngine::GetEngine()->UseRawMode()&&!HasStandardOverlay()&&!IKEngine::GetEngine()->GetDiagnosticMode())
				InterpretRaw();
			PostReportDataToControlPanel();
			break;

		case IK_EVENT_SWITCH:
			OnSwitch(qe.buffer[1],qe.buffer[2]);
			if (IKEngine::GetEngine()->UseRawMode()&&!HasStandardOverlay()&&!IKEngine::GetEngine()->GetDiagnosticMode())
				InterpretRaw();
			PostReportDataToControlPanel();
			break;

		case IK_EVENT_SENSOR_CHANGE:
			OnSensorChange(qe.buffer[1],qe.buffer[2]);
			PostReportDataToControlPanel();
			break;

		case IK_EVENT_VERSION:
            //NSLog (@"IKDevice::ProcessInput(IK_EVENT_VERSION): byte[1] (%d); byte[2] (%d);", qe.buffer[1], qe.buffer[2]);
            if ((qe.buffer[1] + qe.buffer[2]) > 0) {
                SetFirmwareVersion (qe.buffer[1], qe.buffer[2]);
            }
            PostReportDataToControlPanel();
			break;

		case IK_EVENT_EEPROM_READ:
			break;

		case IK_EVENT_ONOFFSWITCH:
			OnToggle(qe.buffer[1]);
			PostReportDataToControlPanel();
			break;

		case IK_EVENT_CORRECT_MEMBRANE:
			OnCorrectMembrane(qe.buffer[1],qe.buffer[2]);
			if (IKEngine::GetEngine()->UseRawMode()&&!HasStandardOverlay()&&!IKEngine::GetEngine()->GetDiagnosticMode())
				InterpretRaw();
			PostReportDataToControlPanel();
			break;

		case IK_EVENT_CORRECT_SWITCH:
			OnCorrectSwitch(qe.buffer[1]);
			if (IKEngine::GetEngine()->UseRawMode()&&!HasStandardOverlay()&&!IKEngine::GetEngine()->GetDiagnosticMode())
				InterpretRaw();
			PostReportDataToControlPanel();
			break;

		case IK_EVENT_CORRECT_DONE:
			OnCorrectDone();
			PostReportDataToControlPanel(true);
			break;

		case IK_EVENT_EEPROM_READBYTE:
			StoreEEProm(qe.buffer[1],qe.buffer[2],qe.buffer[3]);
			break;

		case IK_EVENT_AUTOPILOT_STATE:
			break;

		default:
			break;
		
		//  We should not see these.

		case IK_EVENT_ACK:
		case IK_EVENT_DEVICEREADY:
		case IK_EVENT_NOMOREEVENTS:
		case IK_EVENT_MEMBRANE_REPEAT:
		case IK_EVENT_SWITCH_REPEAT:
			//  error??
			break;

		}
	}
}

void IKDevice::Interpret()
{			
	enum {
		kPress = 0,
		kRelease,
		kRepeat,
		kSlide
	};

	//  don't bother if we're not connected and switched on
	if (!IsOpen())
		return;
	if (!IsSwitchedOn())
		return;

	//  don't bother if there is no current overlay
	IKOverlay *pOverlay = GetCurrentOverlay();
	if (pOverlay==NULL)
		return;

	//  find current settings
	IKSettings *pSettings = IKSettings::GetSettings();

	//  analyze switches and membrane.
	int newDomain = 0;
	bool PressAnywhere = false;
	//bool bWasSwitch = false;
	int switchNumber;
	
	FindDomain ( pOverlay, &newDomain, &PressAnywhere, &switchNumber );
	
	if (m_newLevel>0 && !PressAnywhere)
	{
		SetLevel(m_newLevel);
		m_newLevel = 0;
	}
		
	//  get pointers to the new and repeating (if any) data
	BYTE *newDomainData     = pOverlay->GetUniversalCodesFromDomain(newDomain,GetLevel());
	BYTE *repeatDomainData  = pOverlay->GetUniversalCodesFromDomain(m_repeatDomain,GetLevel());
	BYTE *currentDomainData = pOverlay->GetUniversalCodesFromDomain(m_currentDomain,GetLevel());

	//  sanity
	if (newDomain!=0 && newDomainData==NULL)
		newDomain = 0;
	if (m_repeatDomain!=0 && repeatDomainData==NULL)
		m_repeatDomain = 0;
	if (m_currentDomain!=0 && currentDomainData==NULL)
		m_currentDomain = 0;

	unsigned int now = IKUtil::GetCurrentTimeMS();
	

	//  play dead until the time arrives
	IKASSERT(now+1000 >= m_deadUntil);
	if ( now < m_deadUntil)
		return;

	//  what is the membrane doing?

	int stage = -1;
	if (m_currentDomain == 0 && newDomain != 0)
		stage = kPress;
	if (m_currentDomain != 0 && newDomain == 0)
		stage = kRelease;
	if (m_currentDomain != 0 && newDomain != 0 && m_currentDomain==newDomain)
		stage = kRepeat;
	if (m_currentDomain != 0 && newDomain != 0 && m_currentDomain!=newDomain)
		stage = kSlide;

	//  handle special setup keys here and bail
	if (pOverlay->IsSetupOverlay())
	{
		if (!m_bDidSetup)
		{
			if ((stage == kPress || stage==kSlide))
			{
				m_bDidSetup = DoSetupKey(newDomainData);
				if (m_bDidSetup)
				{
					m_currentDomain = newDomain;
					return;
				}
			}
		}
		else
		{
			//  setup key in progress, wait for release
			if (stage==kRelease || stage==-1)
				m_bDidSetup = false;
			m_currentDomain = newDomain;
			return;
		}
	}

	if (stage == kPress || stage == kSlide)
		m_lastPress = now;
	if (stage == kRelease)
		m_lastRelease = now;

	//  handle repeat latching
	if (m_bRepeatLatched && stage == kRelease)
		m_bRepeatLatchReleased = true;
	if (m_bRepeatLatched && m_bRepeatLatchReleased && PressAnywhere)
	{
		PurgeQueues();
		ResetKeyboard();
		ShortKeySound();
		m_bRepeatLatched = false;
		newDomain = 0;
		stage = -1;
		m_deadUntil = now + DATAI(TEXT("Dead_Until_Time"),1000);
		IKASSERT(m_deadUntil>now);
		return;
	}
	if (m_bRepeatLatched)
		stage = kRepeat;
		
	if (stage == kRepeat && m_timeToRespond!=0)
		stage = kPress;

	//  do nothing if membrane is repeating but the key says no
	if (stage== kRepeat && HasNonRepeating(repeatDomainData))
	{
		stage = -1;

		//  lift any keys that are down.
		if (CountRealCodes(repeatDomainData)==1 && m_ndown!=0)
		{
			//  release this code
			if (!IsModifier(repeatDomainData[0]))
				PostKey(repeatDomainData[0],IK_UP);
			PostKeyDone();
			m_ndown = 0;
		}
	}

	//  do nothing if membrane is repeating but settings say no
	//  and we're NOT moving the mouse.
	if (stage== kRepeat && !pSettings->m_bRepeat && !HasOnlyMouseMoveActions(repeatDomainData))
		stage = -1;

	//  do nothing if the membrane is repeating but we have mouse clicks
	if (stage== kRepeat && HasMouseClicks(repeatDomainData))
		stage = -1;

	//  do nothing if the membrane is repeating but we have setup codes
	if (stage== kRepeat && HasSetupCodes(repeatDomainData))
		stage = -1;

	//  disallow slide with required lift off
	if (stage == kSlide && pSettings->m_bRequiredLiftOff)
		stage = -1;

	//  disallow repeat with required lift off if we're not where we started
	if (stage == kRepeat && pSettings->m_bRequiredLiftOff && newDomain != m_repeatDomain)
        {
		stage = -1;
                LiftAllModifiers();
                ResetKeyboard();
                ResetMouse();
                PostKeyDone();
                m_ndown = 0;
                m_repeatAfter = 0;
                m_repeatDomain = 0;
                m_timeToRespond = 0;
        }

	//  pressing or sliding: do the key
	if ( stage == kPress || stage == kSlide )
	{
		//  if sliding, make sure repeating was stopped.
		if (stage == kSlide)
		{
			if (CountRealCodes(currentDomainData)==1 && m_ndown!=0)
			{
				//  release this code
				if (!IsModifier(currentDomainData[0]))
					PostKey(currentDomainData[0],IK_UP);
				PostKeyDone();
				m_ndown = 0;
			}

			m_repeatAfter = 0;
			m_repeatDomain = 0;
			m_timeToRespond = 0;
		}


		{
			//  response rate

			if (now>m_timeToRespond)
			{
				int delay;
				if (switchNumber>0)
				    delay = DATAI(TEXT("Switch_Response_Period"),50);
				else
				    delay = 20*(15-pSettings->m_iResponseRate)*(15-pSettings->m_iResponseRate) + DATAI(TEXT("Min_Response_Time"),0);
				if(delay>0 && m_timeToRespond==0)
				{
					m_timeToRespond = now + delay;
				}
				else
				{
					m_timeToRespond = 0;

					if (IsIdle() && ((!pSettings->m_bRequiredLiftOff) || 
						(pSettings->m_bRequiredLiftOff && m_lastRelease > m_lastExecute)))
					{
						(void) ComputeMouseInc ( 0 ); //  resets accell

						ShortKeySound();
						PostKeyStart();

						AppLibKillFloatingMessage();

						m_ndown = ExecuteUniversalData(newDomainData);
						m_lastExecute = now; // record time of most recent execution
					}
				}
			}
		}

		//  compute when to start repeating
		//  anwhere from m_minRepeatStart to m_maxRepeatStart, depending on the repeat rate

		int mr1 = DATAI(TEXT("Min_Repeat_Start"),1000);
		int mr2 = DATAI(TEXT("Max_Repeat_Start"),4000);
		int waitTime = mr1 + 
			(15-pSettings->m_iRepeatRate)*(mr2-mr1)/14;
		m_repeatAfter = IKUtil::GetCurrentTimeMS() + waitTime;
		m_repeatDomain = newDomain;
		m_nrepeat = 0;
	}

	//  releasing - stop any repeating
	else if (stage == kRelease)  //   && !PressAnywhere)
	{
		if (CountRealCodes(currentDomainData)==1 && m_ndown!=0)
		{
			//  release this code 'cause it was not released
			//  before
			if (!IsModifier(currentDomainData[0]))
				PostKey(currentDomainData[0],IK_UP);
			m_ndown = 0;
		}

		if (IKSettings::GetSettings()->m_iShiftKeyAction==kSettingsShiftNoLatch)
			LiftAllModifiers();

		PurgeToLastStart();
		PostKeyDone();
		m_repeatAfter = 0;
		m_repeatDomain = 0;
		m_timeToRespond = 0;
	}

	//  repeating
	else if (stage == kRepeat && repeatDomainData!=NULL)
	{
		m_timeToRespond = 0;

		//  is it time to repeat yet?
		if (now>=m_repeatAfter && m_repeatAfter!=0)
		{
			//  run the key again
			PostKeyRepeat();
			m_ndown = ExecuteUniversalData(repeatDomainData);

			//  initialize some data for repeat latching
			if (pSettings->m_bRepeat && pSettings->m_bRepeatLatching)
			{
				m_nrepeat++;
				m_bRepeatLatched = true;
				if (m_nrepeat==1)
					m_bRepeatLatchReleased = false;
			}

			//  compute the next time to repeat.  If any mouse
			//  actions, do it right away.
			int delay = 0;
			if (!HasMouseActions(repeatDomainData))
				delay = GetRepeatDelayMS();
			m_repeatAfter = now + delay;
		}
		else
		{
			//  not time to repeat yet.
			//  But if it's a single mouse move, do it anyway.
			if (IsMouseMove(repeatDomainData[0]) && repeatDomainData[1]==0)
			{
				m_ndown = ExecuteUniversalData(repeatDomainData);
			}
		}
	}

	//  the end.

	m_currentDomain = newDomain;

}

void IKDevice::PostKey(int code, int direction, int delayAfter)
{
	//  track shift status and last code up for smart typing.
	
	if (direction==IK_UP)
	{
		if (code == UNIVERSAL_SHIFT || code == UNIVERSAL_RIGHT_SHIFT)
		{
			m_bShifted = true;
		}
		else
		{
			m_lastCodeUp = code;
			m_bShifted = false;
		}
	}

	BYTE command[IK_REPORT_LEN];
	command[0] = IK_CMD_KEYBOARD;
	command[1] = code;
	command[2] = direction;
	command[3] = delayAfter & 0xff;
	command[4] = (delayAfter/256) & 0xff;
	PostCommand(command);
}

void IKDevice::DoKey(int code, int direction)
{
	//  convert to HID
	int usagePage, usageID;
	UniversalToHID ( code, &usagePage, &usageID );

	//  track is the status changed
	bool bChanged = false;

	//  remember caps lock and num lock states
	switch (code)
	{
	case UNIVERSAL_NUM_LOCK:
		if (direction==IK_DOWN)
			m_bNumLock = !m_bNumLock;
		break;

	case UNIVERSAL_CAPS_LOCK:
		if (direction==IK_DOWN)
			m_bCapsLock = !m_bCapsLock;
		break;
	}

	//  handle different cases for the key
	switch (code)
	{
	case UNIVERSAL_RIGHT_SHIFT:
		bChanged = true;
		if (direction==IK_DOWN)
			m_KeyBoardReport[0] |= 0x20;
		else
			m_KeyBoardReport[0] &= 0xDF;
		break;

	case UNIVERSAL_SHIFT:
		bChanged = true;
		if (direction==IK_DOWN)
			m_KeyBoardReport[0] |= 0x02;
		else
			m_KeyBoardReport[0] &= 0xFD;
		break;

	case UNIVERSAL_RIGHT_CONTROL:
		bChanged = true;
		if (direction==IK_DOWN)
			m_KeyBoardReport[0] |= 0x10;
		else
			m_KeyBoardReport[0] &= 0xEF;
		break;

	case UNIVERSAL_CONTROL:
		bChanged = true;
		if (direction==IK_DOWN)
			m_KeyBoardReport[0] |= 0x01;
		else
			m_KeyBoardReport[0] &= 0xFE;
		break;

	case UNIVERSAL_MENU:
		bChanged = true;
		if (direction==IK_DOWN)
			m_KeyBoardReport[0] |= 0x04;
		else
			m_KeyBoardReport[0] &= 0xFB;
		break;

	case UNIVERSAL_ALTGR:
		bChanged = true;
		if (direction==IK_DOWN)
			m_KeyBoardReport[0] |= 0x40;
		else
			m_KeyBoardReport[0] &= 0xBF;
		break;

	case UNIVERSAL_COMMAND:
		bChanged = true;
	    if (direction==IK_DOWN)
		m_KeyBoardReport[0] |= 0x80;
	    else
		m_KeyBoardReport[0] &= 0x7F;
	    break;
	    
	default:
		if (direction==IK_DOWN)
		{
			//  put it in if it's not already there
			int izero = -1;
			int ifound = -1;
			for (unsigned int i=2;i<sizeof(m_KeyBoardReport);i++)
			{
				if(m_KeyBoardReport[i]==0 && izero==-1)
					izero = i;
				if(m_KeyBoardReport[i]==usageID && ifound==-1)
					ifound = i;
			}
			if (ifound==-1 && izero!=-1)
			{
				m_KeyBoardReport[izero] = usageID;
				bChanged = true;
			}
		}
		else
		{
			//  take it out if it's there
			//  put it in if it's not already there
			int ifound = -1;
			for (unsigned int i=2;i<sizeof(m_KeyBoardReport);i++)
			{
				if(m_KeyBoardReport[i]==usageID && ifound==-1)
					ifound = i;
			}
			if (ifound!=-1)
			{
				m_KeyBoardReport[ifound] = 0;
				bChanged = true;
			}
		}
		break;
	}

	
	//  send it..
	if (bChanged)
	{
		BYTE msg[IK_REPORT_LEN] = {IK_CMD_REFLECT_KEYSTROKE,0,0,0,0,0,0,0};
		for (unsigned int j=0;j<sizeof(m_KeyBoardReport);j++)
			msg[j+1] = m_KeyBoardReport[j];
		//PostCommand(msg);
		SendCommand (msg);
	}

}

void IKDevice::UniversalToHID(int universalCode, int *usagePage, int *usageID)
{
	int n=sizeof(UniversalToHid);
	for (int i=0;i<n;i++)
	{
		if (universalCode==UniversalToHid[i].universalCode)
		{
			*usagePage = UniversalToHid[i].HIDUsagePage;
			*usageID   = UniversalToHid[i].HIDUsageID;
			return;
		}
	}

	*usagePage = 0;
	*usageID   = 0;
}

void IKDevice::PostMouseMove(int x, int y)
{
	BYTE command[IK_REPORT_LEN];
	command[0] = IK_CMD_MOUSE_MOVE;
	command[1] = x;
	command[2] = y;
	PostCommand(command);
}

void IKDevice::PostMouseButton(int whichButton, int direction)
{
	BYTE command[IK_REPORT_LEN];
	command[0] = IK_CMD_MOUSE_BUTTON;
	command[1] = whichButton;
	command[2] = direction;
	PostCommand(command);
}


void IKDevice::DoMouseMove(int x, int y)
{
	//  update the mouse report
	m_MouseReport[1] = x;
	m_MouseReport[2] = y;
	BYTE msg[IK_REPORT_LEN] = {IK_CMD_REFLECT_MOUSE_MOVE,
		m_MouseReport[0],m_MouseReport[1],m_MouseReport[2],0,0,0,0};
	PostCommand(msg);
	//SendCommand(msg);
}


void IKDevice::DoMouseButton(int whichButton, int direction)
{
	switch (whichButton)
	{

	case IKUSB_LEFT_BUTTON:
		if (direction==IK_DOWN)
			m_MouseReport[0] |= 0x01;
		else if (direction==IK_TOGGLE)
			if (m_MouseReport[0] & 0x01)
				m_MouseReport[0] &= 0xFE;
			else
				m_MouseReport[0] |= 0x01;
		else
			m_MouseReport[0] &= 0xFE;

		break;

	case IKUSB_RIGHT_BUTTON:
		if (direction==IK_DOWN)
			m_MouseReport[0] |= 0x02;
		else if (direction==IK_TOGGLE)
			if (m_MouseReport[0] & 0x02)
				m_MouseReport[0] &= 0xFD;
			else
				m_MouseReport[0] |= 0x02;
		else
			m_MouseReport[0] &= 0xFD;
		break;

	case IKUSB_MIDDLE_BUTTON:
		if (direction==IK_DOWN)
			m_MouseReport[0] |= 0x04;
		else if (direction==IK_TOGGLE)
			if (m_MouseReport[0] & 0x04)
				m_MouseReport[0] &= 0xFB;
			else
				m_MouseReport[0] |= 0x04;
		else
			m_MouseReport[0] &= 0xFB;
		break;

	case IKUSB_BUTTON_4:
		if (direction==IK_DOWN)
			m_MouseReport[0] |= 0x08;
		else if (direction==IK_TOGGLE)
			if (m_MouseReport[0] & 0x08)
				m_MouseReport[0] &= 0xF7;
			else
				m_MouseReport[0] |= 0x08;
		else
			m_MouseReport[0] &= 0xF7;
		break;

	case IKUSB_BUTTON_5:
		if (direction==IK_DOWN)
			m_MouseReport[0] |= 0x10;
		else if (direction==IK_TOGGLE)
			if (m_MouseReport[0] & 0x10)
				m_MouseReport[0] &= 0xEF;
			else
				m_MouseReport[0] |= 0x10;
		else
			m_MouseReport[0] &= 0xEF;
		break;

	case IKUSB_BUTTON_6:
		if (direction==IK_DOWN)
			m_MouseReport[0] |= 0x20;
		else if (direction==IK_TOGGLE)
			if (m_MouseReport[0] & 0x20)
				m_MouseReport[0] &= 0xDF;
			else
				m_MouseReport[0] |= 0x20;
		else
			m_MouseReport[0] &= 0xDF;
		break;

	case IKUSB_BUTTON_7:
		if (direction==IK_DOWN)
			m_MouseReport[0] |= 0x40;
		else if (direction==IK_TOGGLE)
			if (m_MouseReport[0] & 0x40)
				m_MouseReport[0] &= 0xBF;
			else
				m_MouseReport[0] |= 0x40;
		else
			m_MouseReport[0] &= 0xBF;
		break;

	case IKUSB_BUTTON_8:
		if (direction==IK_DOWN)
			m_MouseReport[0] |= 0x80;
		else if (direction==IK_TOGGLE)
			if (m_MouseReport[0] & 0x80)
				m_MouseReport[0] &= 0x7F;
			else
				m_MouseReport[0] |= 0x80;
		else
			m_MouseReport[0] &= 0x7F;
		break;

	default:
		break;
	}

	m_MouseReport[1] = 0;
	m_MouseReport[2] = 0;
	BYTE msg[IK_REPORT_LEN] = {
			IK_CMD_REFLECT_MOUSE_MOVE,
			m_MouseReport[0],
			m_MouseReport[1],
			m_MouseReport[2],0,0,0,0 };
	PostCommand(msg);
}


void IKDevice::PurgeQueues()
{
	m_inputQueue.makeEmpty();
	m_commandQueue.makeEmpty();
	m_rawQueue.makeEmpty();
}


void IKDevice::PostSound(int freq, int duration, int volume)
{
	BYTE report[IK_REPORT_LEN] = {IK_CMD_TONE,0,0,0,0,0,0,0};

	//  set parameters and blow
	report[1] = freq;
	report[2] = volume;
	report[3] = duration/10;
	PostCommand(report);
}


void IKDevice::OnToggle(int newValue)
{
	if (m_toggle != (newValue))
	{
		PurgeQueues();
		m_toggle = newValue;

		int freqLow  = DATAI(TEXT("Sweep_Sound_Low_Frequency")	,200);
		int freqHigh = DATAI(TEXT("Sweep_Sound_High_Frequency")	,250);
		int duration = DATAI(TEXT("Sweep_Sound_Duration"), 200);
		if (m_toggle==1)
			SweepSound(freqLow, freqHigh,duration);
		else
			SweepSound(freqHigh,freqLow, duration);

		//  reset keyboard
		ResetKeyboard();

		//  reset mouse
		ResetMouse();

		//  reset level
		//SetLevel(1);

		IKControlPanel::Refresh();
	}
}


void IKDevice::SweepSound(int iStartFreq, int iEndFreq, int iDuration)
{

	BYTE report[8] = {IK_CMD_TONE,0,0,0,0,0,0,0};

	int volume = 2;  //  IKUSBInterpreter::GetInterpreter()->GetSettings()->GetVolume();

	int nLight = 0;
	bool bOn = true;
	int j = 0;

	int iLoops = iDuration/5;  // 5-msec chunks
	for (int i=0;i<iLoops;i++)
	{
		report[1] = iStartFreq + i*((iEndFreq-iStartFreq)*100/iLoops)/100;
		report[2] = volume;
		PostCommand(report);

		j++;
		if(j==5)
		{
			j = 0;
			nLight++;
			if(nLight>9)
			{
				bOn = !bOn;
				nLight = 1;
			}
			PostSetLED ( nLight, bOn );
		}
	}

	report[2] = 0;
	PostCommand(report);

	//  restore lights
	for (int i2=0;i2<9;i2++)
	{
		PostSetLED(i2+1,false);
	}

}



void IKDevice::OnSwitch(int nswitch, int state)
{
	//IKTRACE((TEXT("IKDevice::OnSwitch(%d,%d)"),nswitch,state));

	m_switches[nswitch-1] = state;
}


void IKDevice::OnSensorChange(int sensor, int value)
{
	//  what time is it?
	unsigned int now = IKUtil::GetCurrentTimeMS();

	//  save the current sensor value
	m_sensors[sensor] = value;

	//  what's the new overlay value

	int v[IK_NUM_SENSORS];
	int i;
	for (i=0;i<IK_NUM_SENSORS;i++)
	{
		v[i] = 0;
		int midway;
		if (m_bEepromValid)
		{
			int weight = DATAI(TEXT("Overlay_Rec_Black_Weight"),50);  
			midway = (weight*m_eepromData.sensorBlack[i] +
					  (100-weight)*m_eepromData.sensorWhite[i]) / 100;
		}
		else
			midway = DATAI(TEXT("Overlay_Rec_Fallback_Threshhold"),150);
		if (m_sensors[i]>midway)
			v[i] = 1;
	}
	int newVal = 0;
	for (i=0;i<IK_NUM_SENSORS;i++)
	{
		newVal += v[i]*(1<<i);
	}

	//  if the value changed, record what and when
	if (newVal != m_lastOverlay)
	{
		m_lastOverlay = newVal;
		m_lastOverlayTime = now;
	}

}

void IKDevice::SetFirmwareVersion (BYTE major, BYTE minor) 
{
    m_firmware_version_major = major;
    m_firmware_version_minor = minor;
}

const char* IKDevice::GetFirmwareVersionString()
{
    static char versionString[12];
    versionString[0] = 0;
    if (m_firmware_version_major || m_firmware_version_minor) {
        sprintf(versionString, "%d.%d", m_firmware_version_major, m_firmware_version_minor);
    }
    return versionString;
}


void IKDevice::Reset()
{
	m_bLastInit = false;

	m_lastSwitch = 0;

	m_newLevel = 0;

	m_bOpen = false;
	m_writeProc = NULL;
	m_index = -1;

	m_toggle = -1;

	for (int nsw=0;nsw<IK_NUM_SWITCHES;nsw++)
		m_switches[nsw] = 0;

	m_delayUntil  = 0;
	m_nextCorrect = 0;

	m_lastCodeUp = 0;
	m_bShifted = false;

	for (int x=0;x<IK_RESOLUTION_X;x++)
		for (int y=0;y<IK_RESOLUTION_Y;y++)
			m_membrane[y][x] = 0;

	PurgeQueues();

	m_lastOverlay = -1;
	m_lastOverlayTime = IKUtil::GetCurrentTimeMS();
	m_currentOverlay = -1;

	//  clear out copy of eeprom memory,
	//  mark all data as invalid
	for (unsigned int i=0;i<sizeof(m_eepromData);i++)
	{
		BYTE *e = (BYTE *) &m_eepromData;
		e[i] = 0;
		m_eepromDataValid[i] = false;
		m_eepromRequestTime[i] = 0;
	}
	m_bEepromValid = false;

	SetLevel(1);

	for (int j=0;j<IK_NUM_SENSORS;j++)
	{
		m_sensors[j] = 255;
	}

	m_repeatDomain = 0;
	m_currentDomain = 0;
	m_repeatAfter = 0;
	m_bRepeatLatched = false;
	m_bRepeatLatchReleased = false;
	m_nrepeat = 0;
	m_timeToRespond= 0;
	m_deadUntil = 0;
	m_ndown = 0;
	m_bDidSetup = false;
	m_lastPress = 0;
	m_lastRelease = 0;
	m_lastExecute = 0;
	m_startDomain = 0;

	m_counter = 0;
	m_lastcode = 0;

	LiftAllModifiers();

	m_modShift.SetCode  (UNIVERSAL_SHIFT);
	m_modAlt.SetCode    (UNIVERSAL_ALT);
	m_modControl.SetCode(UNIVERSAL_CONTROL);
	m_modCommand.SetCode(UNIVERSAL_COMMAND);

	m_modShift.SetDevice  (this);
	m_modAlt.SetDevice    (this);
	m_modControl.SetDevice(this);
	m_modCommand.SetDevice(this);

	m_lastLEDTime = 0;

	for (int il=0;il<9;il++)
		m_lights[il] = false;

	m_bCapsLock = false;
	m_bNumLock = false;

	m_lastExecuted = NULL;

	for (int i2=0;i2<5;i2++)
		m_last5Overlays[i2] = -1;
		
	for (unsigned int i3=0;i3<sizeof(m_KeyBoardReport);i3++)
		m_KeyBoardReport[i3] = 0;
	
	for (unsigned int i4=0;i4<sizeof(m_MouseReport);i4++)
		m_MouseReport[i4] = 0;

    m_firmware_version_major = 0;
    m_firmware_version_minor = 0;

}

void IKDevice::DoCorrect()
{
	//  clear out data
	for (int i=0;i<IK_NUM_SWITCHES;i++)
		m_switchesPressedInCorrectMode[i] = 0;
	for (int x=0;x<IK_RESOLUTION_X;x++)
		for (int y=0;y<IK_RESOLUTION_Y;y++)
			m_membranePressedInCorrectMode[y][x] = 0;

	//  send the command
	BYTE report[IK_REPORT_LEN] = {IK_CMD_CORRECT,0,0,0,0,0,0,0};
	PostCommand(report);
}

void IKDevice::OnCorrectMembrane(int x, int y)
{
	m_membranePressedInCorrectMode[y][x] = true;
}

void IKDevice::OnCorrectSwitch(int switchnum)
{
	int ns = switchnum;

	//ns += 2;  //  TESTING!!!

	m_switchesPressedInCorrectMode[ns-1] = true;
}

void IKDevice::OnCorrectDone()
{
	for (int i=0;i<IK_NUM_SWITCHES;i++)
		m_switches[i] = m_switchesPressedInCorrectMode[i];

	for (int x=0;x<IK_RESOLUTION_X;x++)
		for (int y=0;y<IK_RESOLUTION_Y;y++)
			m_membrane[y][x] = m_membranePressedInCorrectMode[y][x];

}

void IKDevice::OnMembranePress(int x, int y)
{
	m_membrane[y][x] = 1;
}

void IKDevice::OnMembraneRelease(int x, int y)
{
	m_membrane[y][x] = 0;

}


void IKDevice::OnStdOverlayChange()
{
	ResetKeyboard();
	ResetMouse();
	PostLiftAllModifiers();
	//SetLevel(1);
	
	if (DATAI(TEXT("Reload_Standard_Overlay_When_Recognized"),0)==1)
	{
	    IKEngine::GetEngine()->LoadStandardOverlays();
	}

	LongKeySound();
	OverlayRecognitionFeedback();

	//  tell the CP
	PostCPRefresh();

	// tell raw mode
	bool bRaw = IKEngine::GetEngine()->GetRawMode();
	if (bRaw)
	{
		
		unsigned int time = IKUtil::GetCurrentTimeMS();
		queueEntry qe;
		qe.buffer[0] = 3;	 //  event type

		if (m_currentOverlay==7)
			qe.buffer[1] = 0;    //  overlay number
		else
			qe.buffer[1] = m_currentOverlay+1;    //  overlay number
	
		qe.buffer[2] = 0;    //  unused
		qe.buffer[3] = 0;    //  unused
		*((unsigned int *)&(qe.buffer[4])) = time;
		m_rawQueue.enqueue(qe);
	}

}


void IKDevice::ResetKeyboard()
{
	m_lastExecuted = NULL;

	for (int i2=0;i2<5;i2++)
		m_last5Overlays[i2] = -1;

	//  reset keyboard
	for (unsigned int i=0;i<sizeof(m_KeyBoardReport);i++)
		m_KeyBoardReport[i] = 0;
		
	BYTE msg[IK_REPORT_LEN] = {IK_CMD_REFLECT_KEYSTROKE,0,0,0,0,0,0,0};
	for (unsigned int j=0;j<sizeof(m_KeyBoardReport);j++)
		msg[j+1] = m_KeyBoardReport[j];
		
	PostCommand(msg);

    // ask for firmware version
	BYTE version[IK_REPORT_LEN] = {IK_CMD_GET_VERSION,0,0,0,0,0,0,0};
	PostCommand(version);

	//  reconcile with modifier objects?
	PostLiftAllModifiers();
}

void IKDevice::ResetMouse()
{
	//  reset mouse
	for (unsigned int i=0;i<sizeof(m_MouseReport);i++)
		m_MouseReport[i] = 0;
		
	BYTE msg2[] = {IK_CMD_REFLECT_MOUSE_MOVE,0,0,0,0,0,0,0};
	for (unsigned int j=0;j<sizeof(m_MouseReport);j++)
		msg2[j+1] = m_MouseReport[j];
		
	PostCommand(msg2);

}


bool IKDevice::IsIntelliSwitchV1()
{
	if (m_eepromData.serialnumber[0]!='C')
		return false;
	if (m_eepromData.serialnumber[1]!='-')
		return false;
	for (unsigned int i=2;i<sizeof(eeprom);i++)
		if (m_eepromData.serialnumber[i]!=0)
			return false;

	return true;
}


void IKDevice::StoreEEProm(BYTE data, BYTE addlsb, BYTE addmsb)
{
	//  store the byte received;
	int ndx = addlsb - 0x80;

	BYTE *e = (BYTE *) &m_eepromData;
	e[ndx] = data;

	//  mark the byte valid;
	m_eepromDataValid[ndx] = true;

	//  check to see if all the bytes are valid.
	//  if so, say we're valid and refresh the
	//  control panel.
	int nInvalid = 0;
	for (unsigned int i=0;i<sizeof(eeprom);i++)
		if (!m_eepromDataValid[i])
			nInvalid++;

	if (nInvalid==0 && !m_bEepromValid)
	{
		if (m_eepromData.serialnumber[0]=='C' &&
			m_eepromData.serialnumber[1]=='-')
		{
			m_bEepromValid = true;
			IKString serial(m_eepromData.serialnumber,IK_EEPROM_SN_SIZE);

			//  the first wave of IntelliSwitch dongles are incorrectly
			//  set to use the IntelliKeys VID/PID.  Check for that here
			//  and adjust the device type accordingly.

			if (IsIntelliSwitchV1())
				SetDevType(2);

			PostCPRefresh();
		}

	}

}

void IKDevice::ShortKeySound()
{
	KeySound(DATAI(TEXT("Short_Key_Sound_Length"),50));
}

void IKDevice::LongKeySound()
{
	KeySound(DATAI(TEXT("Long_Key_Sound_Length"),700));
}

void IKDevice::KeySound(int msLength)
{
	KeySoundVol(msLength,IKSettings::GetSettings()->m_iKeySoundVolume);
}

void IKDevice::KeySoundVol(int msLength, int vol)
{
	int myVol = vol;
	if (vol==-1)
		myVol = IKSettings::GetSettings()->m_iKeySoundVolume;

	//  set parameters and blow
	BYTE report[IK_REPORT_LEN] = {IK_CMD_TONE,0,0,0,0,0,0,0};
	report[1] = DATAI(TEXT("Key_Sound_Frequency"),247);
	report[2] = myVol;
	report[3] = msLength/10;
	SendCommand(report);
}

void IKDevice::OverlayRecognitionFeedback()
{
	//PostMonitorState(false);

	int delay = 300;

	if (IsSwitchedOn())
	{
	PostSetLED ( 1, true);
	PostSetLED ( 4, true);
	PostSetLED ( 7, true);
	PostDelay(delay);
	PostSetLED ( 1, false);
	PostSetLED ( 4, false);
	PostSetLED ( 7, false);

	PostSetLED ( 2, true);
	PostSetLED ( 5, true);
	PostSetLED ( 8, true);
	PostDelay(delay);
	PostSetLED ( 2, false);
	PostSetLED ( 5, false);
	PostSetLED ( 8, false);

	PostSetLED ( 3, true);
	PostSetLED ( 6, true);
	PostSetLED ( 9, true);
	PostDelay(delay);
	PostSetLED ( 3, false);
	PostSetLED ( 6, false);
	PostSetLED ( 9, false);
	}
	else
	{
		for (int numFlashes = 0; numFlashes < 6; numFlashes++)
		{
			PostSetLED ( 2, true);
			PostSetLED ( 5, true);
			PostSetLED ( 8, true);
			PostDelay(delay);
			//PostLedReconcile();

			PostDelay(delay);

			PostSetLED ( 2, false);
			PostSetLED ( 5, false);
			PostSetLED ( 8, false);
			PostDelay(delay);
			//PostLedReconcile();

			PostDelay(delay);
		}
	}

	//PostMonitorState(true);

	//PostLedReconcile();
}

bool IKDevice::IsSwitchedOn()
{
	return (m_toggle!=0);
}

IKOverlay * IKDevice::GetCurrentOverlay()
{
	IKOverlay *pOv = NULL;
	
	//  always use standard overlay if there is one
	if (HasStandardOverlay())
	{
		pOv = IKEngine::GetEngine()->GetStandardOverlay(m_currentOverlay);
		return pOv;
	}

	//  return an overlay based on what the user has set on the Advanced
	//  tab of the control panel.
	
	switch (IKSettings::GetSettings()->m_iMode)
	{
	case kSettingsModeLastSentOverlay:
		pOv = IKEngine::GetEngine()->GetCustomOverlay();
		return pOv;
		break;

	case kSettingsModeThisOverlay:
		pOv = IKEngine::GetEngine()->GetUseThisOverlay();
		return pOv;
		break;

	case kSettingsModeSwitch:
		pOv = IKEngine::GetEngine()->GetSwitchOverlay(IKSettings::GetSettings()->m_iUseThisSwitchSetting);
		return pOv;
		break;

	case kSettingsModeDiscover:
		if (IKSettings::GetSettings()->m_bButAllowOverlays)
			pOv = IKEngine::GetEngine()->GetCustomOverlay();
		else
			pOv = NULL;
		return pOv;
		break;

	default:
		break;
	}

	return NULL;
}

int IKDevice::GetLevel()
{
	return m_currentLevel;
}

void IKDevice::SetLevel(int level)
{
	m_currentLevel = level;
}

void IKDevice::FindDomain(IKOverlay *pOverlay, int *domainNumber, bool *bPressAnywhere, int *switchNumber )
{
	*bPressAnywhere = false;
	*switchNumber = 0;
	*domainNumber = 0;

	//  first check membrane by column and then row,
	//  both in reverse order.  This will favor
	//  cells that are farther to the left and higher.

	int col,row;
	{
		for (col=0;col<IK_RESOLUTION_X;col++)
		{
			for (row=0;row<IK_RESOLUTION_Y;row++)
			{
				if (m_membrane[IK_RESOLUTION_Y-1-row][IK_RESOLUTION_X-1-col])
				{
					int nd;
					nd = pOverlay->GetDomainFromMembrane(IK_RESOLUTION_X-1-col,IK_RESOLUTION_Y-1-row,GetLevel());
					if (nd > 0 )
					{
						*domainNumber = nd;
						*bPressAnywhere = true;
						return;
					}
					*bPressAnywhere = true;
				}
			}
		}
	}

	//  which switches are down?

	int down  [IK_NUM_SWITCHES] = {0,0,0,0,0,0};
	int domain[IK_NUM_SWITCHES] = {0,0,0,0,0,0};

	int j;
	int nDown = 0;
	for (j=0;j<IK_NUM_SWITCHES;j++)
		if (m_switches[j])
		{
			down  [nDown] = j+1;
			domain[nDown] = pOverlay->GetDomainFromSwitch(j+1,GetLevel());
			nDown++;
		}

	//  if the same one is still down since last time, just return it.

	if (m_lastSwitch>0)
	{
		for (j=0;j<nDown;j++)
		{
			if (m_lastSwitch == down[j])
			{
				*domainNumber = domain[j];
				*bPressAnywhere = true;
				*switchNumber = m_lastSwitch;
				return;
			}
		}
	}

	//  find the new down.

	m_lastSwitch = 0;
	static bool bForward = true;

	for (j=0;j<nDown;j++)
	{
		bForward = !bForward;
		int jj = j;
		if (!bForward)
			jj = nDown-j-1;

		if (down[jj]>0)
		{
			m_lastSwitch = down[jj];
			*domainNumber = domain[jj];
			*bPressAnywhere = true;
			*switchNumber = m_lastSwitch;
			return;
		}
	}

#if 0

	for (int j=0;j<IK_NUM_SWITCHES;j++)
	{
		int nsw = IK_NUM_SWITCHES-j;
		if (m_switches[nsw-1])
		{
			int nd;
			nd = pOverlay->GetDomainFromSwitch(nsw,GetLevel());
			if (nd>0)
			{
				*domainNumber = nd;
				*bPressAnywhere = true;
				*switchNumber = nsw;
				return;
			}

			*bPressAnywhere = true;
			*switchNumber = nsw;
			return;
		}
	}
#endif
}

void IKDevice::SettleOverlay()
{
	unsigned int now = IKUtil::GetCurrentTimeMS();

	//  settle overlay
	if (m_lastOverlay!=m_currentOverlay && 
		now > m_lastOverlayTime + DATAI(TEXT("Overlay_Rec_Settle_Period"),1000))
	{
		m_currentOverlay = m_lastOverlay;

		SetLevel(1);

		OnStdOverlayChange();
	}
}

int IKDevice::ExecuteUniversalData(BYTE *therealdata, bool bParForceKeyUp /* =false */)
{
	//  do nothing if there is no data
	if (therealdata==NULL)
		return 0;
	if (therealdata[0]==0)
		return 0;

	//bool bForceKeyUp = ( (DATAI(TEXT("Force_Key_Up"),0)!=0) || bParForceKeyUp);
	bool bForceKeyUp = false;
	if (IKSettings::GetSettings()->m_iRepeatRate<15)
		bForceKeyUp = true;
	if (!IKSettings::GetSettings()->m_bRepeat)
		bForceKeyUp = true;
	if (bParForceKeyUp)
		bForceKeyUp = true;

	//  get a pointer to the data.
	//  swap in the last executed data
	BYTE *thedata = therealdata;
	if (thedata[0] == UNIVERSAL_SEPARATE_REPEAT_KEY)
	{
		thedata = m_lastExecuted;
	}
	else
	{
		m_lastExecuted = thedata;
	}
	if (thedata==NULL)
		return 0;
	if (thedata[0]==0)
		return 0;


	//  make a copy of the content and hack it
	//  unless this key says repeat, in which case just use what was last there.
	BYTE mydata[MAX_OVL_SIZE] = {0};
	if (thedata[0]!=UNIVERSAL_SEPARATE_REPEAT_KEY)
	{
		//  copy the data
		int i=0;
		while (thedata[i]!=0)
		{
			mydata[i] = thedata[i];
			i++;
		}
		mydata[i] = 0;

		//  hack it
		HackKeyContent(mydata);
	}

	//  compute the data send rate time
	int datasenddelay, mouseinc;
	if (IsMouseMove(mydata[0]) && CountRealCodes(mydata)==1)
	{
		//  single mouse movement, do accelleration
		datasenddelay = 0;
		mouseinc = ComputeMouseInc ( mydata[0] );
	}
	else
	{
		//  multiple movements or mixed key
		datasenddelay = (16-IKSettings::GetSettings()->m_iDataSendRate)*(16-IKSettings::GetSettings()->m_iDataSendRate)-1;
		mouseinc = ComputeMouseInc ( 0 );
	}
	
	//  handle modifiers pre key

	if (IKSettings::GetSettings()->m_iShiftKeyAction==kSettingsShiftLatching)
	{
		if (CountRealCodes(mydata)>1) // string is longer than 1
		{
			PostLiftAllModifiers();
		}
	}
	else if (IKSettings::GetSettings()->m_iShiftKeyAction==kSettingsShiftLocking)
	{
		if (CountRealCodes(mydata)>1) // string is longer than 1
		{
			PostLiftAllModifiers();
		}
	}

	int j = 0;
	int codeCount = CountRealCodes(mydata);
	BYTE lastcode = 0;
	int ndown = 0;
	while (mydata[j]!=0)
	{					
		//  handle modifiers pre code

		if (IKSettings::GetSettings()->m_iShiftKeyAction==kSettingsShiftLatching)
		{
		}
		else if (IKSettings::GetSettings()->m_iShiftKeyAction==kSettingsShiftLocking)
		{
		}

		switch (mydata[j])
		{

#ifdef PLAT_WINDOWS

		case UNIVERSAL_UNICODE:

#if 0
			{
				WORD w = MAKEWORD(mydata[j+2], mydata[j+1]);
				IKString s = IKUtil::IntToHexString(w);

				PostKey(UNIVERSAL_ALT,IK_DOWN,0);

				PostKey(UNIVERSAL_NUMPAD_ADD,IK_DOWN,0);
				PostKey(UNIVERSAL_NUMPAD_ADD,IK_UP,0);

				for (int k=0;k<s.GetLength();k++)
				{
					TCHAR c = s.GetAt(k);
					unsigned int n = unsigned int (c) - unsigned int ('0');
					unsigned int code = UNIVERSAL_NUMPAD_0 + n;

					PostKey(code,IK_DOWN,0);
					PostKey(code,IK_UP,0);
				}

				PostKey(UNIVERSAL_ALT,IK_UP,0);
			}

#else
			PostKeyUnicode ( mydata[j+1], mydata[j+2], IK_DOWN );
			PostDelay ( DATAI(TEXT("Key_Down_Time"),5));
			PostKeyUnicode ( mydata[j+1], mydata[j+2], IK_UP   );
#endif

			if (datasenddelay>0)
				PostDelay  (datasenddelay);

			j = j + 2;

			break;

#endif

		case UNIVERSAL_SETUP1:
			j++;
			switch (mydata[j])
			{
			case UNIVERSAL_SETUP1_LIFT_OFF_ON:
				IKSettings::GetSettings()->m_bRequiredLiftOff = true;
				break;

			case UNIVERSAL_SETUP1_LIFT_OFF_OFF:
				IKSettings::GetSettings()->m_bRequiredLiftOff = false;
				break;

			case UNIVERSAL_SETUP1_REPEAT_LATCHING_ON:
				IKSettings::GetSettings()->m_bRepeatLatching = true;
				break;

			case UNIVERSAL_SETUP1_REPEAT_LATCHING_OFF:
				IKSettings::GetSettings()->m_bRepeatLatching = false;
				break;

			case UNIVERSAL_SETUP1_MOUSE_ARROWS_ON:
				//  not supported
				break;

			case UNIVERSAL_SETUP1_MOUSE_ARROWS_OFF:
				//  not supported
				break;

			case UNIVERSAL_SETUP1_LIST_FEATURES:
				IKControlPanel::ListFeatures();
				break;

			case UNIVERSAL_SETUP1_SHIFT_NO_LATCHING:
				IKSettings::GetSettings()->m_iShiftKeyAction = kSettingsShiftNoLatch;
				break;

			case UNIVERSAL_SETUP1_SHIFT_LATCHING:
				IKSettings::GetSettings()->m_iShiftKeyAction = kSettingsShiftLatching;
				break;

			case UNIVERSAL_SETUP1_SHIFT_LOCKING:
				IKSettings::GetSettings()->m_iShiftKeyAction = kSettingsShiftLocking;
				break;

			case UNIVERSAL_SETUP1_KEYSOUND_OFF:
				IKSettings::GetSettings()->m_iKeySoundVolume = kSettingsKeysoundOff;
				break;

			case UNIVERSAL_SETUP1_KEYSOUND_ON:
				IKSettings::GetSettings()->m_iKeySoundVolume = kSettingsKeysound2;
				break;

			case UNIVERSAL_SETUP1_REPEAT_ON:
				IKSettings::GetSettings()->m_bRepeat = true;
				break;

			case UNIVERSAL_SETUP1_REPEAT_OFF:
				IKSettings::GetSettings()->m_bRepeat = false;
				break;

			case UNIVERSAL_SETUP1_SMART_TYPING_ON:
				IKSettings::GetSettings()->m_bSmartTyping = true;
				break;

			case UNIVERSAL_SETUP1_SMART_TYPING_OFF:
				IKSettings::GetSettings()->m_bSmartTyping = false;
				break;

			case UNIVERSAL_SETUP1_3LIGHTS:
				IKSettings::GetSettings()->m_iIndicatorLights = kSettings3lights;
				break;

			case UNIVERSAL_SETUP1_6LIGHTS:
				IKSettings::GetSettings()->m_iIndicatorLights = kSettings6lights;
				break;

			case UNIVERSAL_SETUP1_RESPONSE_RATE:
				j++;
				IKSettings::GetSettings()->m_iResponseRate = mydata[j];
				break;

			case UNIVERSAL_SETUP1_MOUSE_SPEED:
				j++;
				IKSettings::GetSettings()->m_iMouseSpeed = mydata[j];
				break;

			case UNIVERSAL_SETUP1_DATA_SEND_RATE:
				j++;
				IKSettings::GetSettings()->m_iDataSendRate = mydata[j];
				break;

			case UNIVERSAL_SETUP1_REPEAT_RATE:
				j++;
				IKSettings::GetSettings()->m_iRepeatRate = mydata[j];
				break;

			case UNIVERSAL_SETUP1_CUSTOM_OVERLAY_LEVEL:
				j++;
				//  TODO: should this translate to switch setting number?
				//IKSettings::GetSettings()->??? = ???
				break;
			}

			IKSettings::GetSettings()->Write();
			PostCPRefresh();

			break;

		case UNIVERSAL_SETUP2:
			j++;
			switch (mydata[j])
			{
			case UNIVERSAL_SETUP1_SMART_TYPING_ON:
				IKSettings::GetSettings()->m_bSmartTyping = !IKSettings::GetSettings()->m_bSmartTyping;
				break;

			case UNIVERSAL_SETUP2_FEATURE_RESET:
				if (IsSwitchedOn())
				{
					IKSettings::GetSettings()->SetToDefault(true);
					//g_intellikeys[0].m_CustomOverlay.Unload();  //  TODO
					SetLevel(1);
					//DeleteLastSentOverlay();
					ResetSound();
				}
				break;
			}

			IKSettings::GetSettings()->Write();
			PostCPRefresh();

			break;

		case UNIVERSAL_WAIT:
			{
				//  define as "delay one second".
				PostDelay (1000);
			}
			break;

		case UNIVERSAL_ALT:
			m_modAlt.SetCode(UNIVERSAL_ALT);
			m_modAlt.Execute();
			break;

		case UNIVERSAL_ALTGR:
			m_modAlt.SetCode(UNIVERSAL_ALTGR);
			m_modAlt.Execute();
			break;

		case UNIVERSAL_CONTROL:
			m_modControl.Execute();
			break;

		case UNIVERSAL_RIGHT_CONTROL:
			m_modControl.Execute(UNIVERSAL_RIGHT_CONTROL);
			break;

		case UNIVERSAL_SHIFT:
			m_modShift.Execute();
			break;

		case UNIVERSAL_RIGHT_SHIFT:
			m_modShift.Execute(UNIVERSAL_RIGHT_SHIFT);
			break;

		case UNIVERSAL_COMMAND:

#ifdef PLAT_MACINTOSH
			m_modCommand.Execute();
#endif

#ifdef PLAT_WINDOWS
			m_modControl.Execute();
#endif
			break;

		case UNIVERSAL_MOUSE_BUTTON_CLICK:
			PostMouseButtonClick(IKUSB_LEFT_BUTTON);
			break;

		case UNIVERSAL_MOUSE_BUTTON_DOUBLECLICK:
			PostMouseButtonDoubleClick(IKUSB_LEFT_BUTTON);
			break;

		case UNIVERSAL_MOUSE_BUTTON_DOWN:
			PostMouseButton(IKUSB_LEFT_BUTTON,IK_DOWN);
			break;

		case UNIVERSAL_MOUSE_BUTTON_TOGGLE:
			PostMouseButtonToggle(IKUSB_LEFT_BUTTON);
			break;

		case UNIVERSAL_MOUSE_BUTTON_UP:
			PostMouseButton(IKUSB_LEFT_BUTTON,IK_UP);
			break;

//---
		case UNIVERSAL_MOUSE_RBUTTON_CLICK:
			PostMouseButtonClick(IKUSB_RIGHT_BUTTON);
			break;

		case UNIVERSAL_MOUSE_RBUTTON_DOUBLECLICK:
			PostMouseButtonDoubleClick(IKUSB_RIGHT_BUTTON);
			break;

		case UNIVERSAL_MOUSE_RBUTTON_DOWN:
			PostMouseButton(IKUSB_RIGHT_BUTTON,IK_DOWN);
			break;

		case UNIVERSAL_MOUSE_RBUTTON_TOGGLE:
			PostMouseButtonToggle(IKUSB_RIGHT_BUTTON);
			break;

		case UNIVERSAL_MOUSE_RBUTTON_UP:
			PostMouseButton(IKUSB_RIGHT_BUTTON,IK_UP);
			break;

//---
//---
		case UNIVERSAL_MOUSE_MBUTTON_CLICK:
			PostMouseButtonClick(IKUSB_MIDDLE_BUTTON);
			break;

		case UNIVERSAL_MOUSE_MBUTTON_DOUBLECLICK:
			PostMouseButtonDoubleClick(IKUSB_MIDDLE_BUTTON);
			break;

		case UNIVERSAL_MOUSE_MBUTTON_DOWN:
			PostMouseButton(IKUSB_MIDDLE_BUTTON,IK_DOWN);
			break;

		case UNIVERSAL_MOUSE_MBUTTON_TOGGLE:
			PostMouseButtonToggle(IKUSB_MIDDLE_BUTTON);
			break;

		case UNIVERSAL_MOUSE_MBUTTON_UP:
			PostMouseButton(IKUSB_MIDDLE_BUTTON,IK_UP);
			break;

//---
//---
		case UNIVERSAL_MOUSE_BUTTON4_CLICK:
			PostMouseButtonClick(IKUSB_BUTTON_4);
			break;

		case UNIVERSAL_MOUSE_BUTTON4_DOUBLECLICK:
			PostMouseButtonDoubleClick(IKUSB_BUTTON_4);
			break;

		case UNIVERSAL_MOUSE_BUTTON4_DOWN:
			PostMouseButton(IKUSB_BUTTON_4,IK_DOWN);
			break;

		case UNIVERSAL_MOUSE_BUTTON4_TOGGLE:
			PostMouseButtonToggle(IKUSB_BUTTON_4);
			break;

		case UNIVERSAL_MOUSE_BUTTON4_UP:
			PostMouseButton(IKUSB_BUTTON_4,IK_UP);
			break;

//---
//---
		case UNIVERSAL_MOUSE_BUTTON5_CLICK:
			PostMouseButtonClick(IKUSB_BUTTON_5);
			break;

		case UNIVERSAL_MOUSE_BUTTON5_DOUBLECLICK:
			PostMouseButtonDoubleClick(IKUSB_BUTTON_5);
			break;

		case UNIVERSAL_MOUSE_BUTTON5_DOWN:
			PostMouseButton(IKUSB_BUTTON_5,IK_DOWN);
			break;

		case UNIVERSAL_MOUSE_BUTTON5_TOGGLE:
			PostMouseButtonToggle(IKUSB_BUTTON_5);
			break;

		case UNIVERSAL_MOUSE_BUTTON5_UP:
			PostMouseButton(IKUSB_BUTTON_5,IK_UP);
			break;

//---
		case UNIVERSAL_MOUSE_BUTTON6_CLICK:
			PostMouseButtonClick(IKUSB_BUTTON_6);
			break;

		case UNIVERSAL_MOUSE_BUTTON6_DOUBLECLICK:
			PostMouseButtonDoubleClick(IKUSB_BUTTON_6);
			break;

		case UNIVERSAL_MOUSE_BUTTON6_DOWN:
			PostMouseButton(IKUSB_BUTTON_6,IK_DOWN);
			break;

		case UNIVERSAL_MOUSE_BUTTON6_TOGGLE:
			PostMouseButtonToggle(IKUSB_BUTTON_6);
			break;

		case UNIVERSAL_MOUSE_BUTTON6_UP:
			PostMouseButton(IKUSB_BUTTON_6,IK_UP);
			break;

//---
		case UNIVERSAL_MOUSE_BUTTON7_CLICK:
			PostMouseButtonClick(IKUSB_BUTTON_7);
			break;

		case UNIVERSAL_MOUSE_BUTTON7_DOUBLECLICK:
			PostMouseButtonDoubleClick(IKUSB_BUTTON_7);
			break;

		case UNIVERSAL_MOUSE_BUTTON7_DOWN:
			PostMouseButton(IKUSB_BUTTON_7,IK_DOWN);
			break;

		case UNIVERSAL_MOUSE_BUTTON7_TOGGLE:
			PostMouseButtonToggle(IKUSB_BUTTON_7);
			break;

		case UNIVERSAL_MOUSE_BUTTON7_UP:
			PostMouseButton(IKUSB_BUTTON_7,IK_UP);
			break;

//---
		case UNIVERSAL_MOUSE_BUTTON8_CLICK:
			PostMouseButtonClick(IKUSB_BUTTON_8);
			break;

		case UNIVERSAL_MOUSE_BUTTON8_DOUBLECLICK:
			PostMouseButtonDoubleClick(IKUSB_BUTTON_8);
			break;

		case UNIVERSAL_MOUSE_BUTTON8_DOWN:
			PostMouseButton(IKUSB_BUTTON_8,IK_DOWN);
			break;

		case UNIVERSAL_MOUSE_BUTTON8_TOGGLE:
			PostMouseButtonToggle(IKUSB_BUTTON_8);
			break;

		case UNIVERSAL_MOUSE_BUTTON8_UP:
			PostMouseButton(IKUSB_BUTTON_8,IK_UP);
			break;

//---
		case UNIVERSAL_MOUSE_DOWN:
			PostMouseMove(0,mouseinc);
			break;

		case UNIVERSAL_MOUSE_DOWN_LEFT:
			PostMouseMove(-mouseinc,mouseinc);
			break;

		case UNIVERSAL_MOUSE_DOWN_RIGHT:
			PostMouseMove(mouseinc,mouseinc);
			break;

		case UNIVERSAL_MOUSE_LEFT:
			PostMouseMove(-mouseinc,0);
			break;

		case UNIVERSAL_MOUSE_RIGHT:
			PostMouseMove(mouseinc,0);
			break;

		case UNIVERSAL_MOUSE_UP:
			PostMouseMove(0,-mouseinc);
			break;

		case UNIVERSAL_MOUSE_UP_LEFT:
			PostMouseMove(-mouseinc,-mouseinc);
			break;

		case UNIVERSAL_MOUSE_UP_RIGHT:
			PostMouseMove(mouseinc,-mouseinc);
			break;

		case UNIVERSAL_NON_REPEATING:
			break;

		default:
			if ((mydata[j] & 0xF0) == 0xC0)
			{
				//  go to level
				int newLevel = (mydata[j] & 0x0F) + 1;
				IKOverlay *po = GetCurrentOverlay();
				if (po)
				{
					if (newLevel <= po->GetNumLevels())
					{
						//SetLevel(newLevel);
						m_newLevel = newLevel;
					}
				}
			}
			else
			{
				if (codeCount>1 || bForceKeyUp)
				{
					//  multiple chars, or force up.
					//  do it all now.
					int keyDownDelay = DATAI(TEXT("Key_Down_Time"),5);
					PostKey(mydata[j],IK_DOWN,keyDownDelay);
					PostKey(mydata[j],IK_UP,  datasenddelay);
				}
				else
				{
					//  single char.  Just do the down
					PostKey(mydata[j],IK_DOWN);
					ndown++;
				}


				lastcode = mydata[j];
			}

			break;
		}


		//  handle modifiers post code

		if (IKSettings::GetSettings()->m_iShiftKeyAction==kSettingsShiftLatching)
		{
			if (!IsModifier(mydata[j])&&!IsMouseAction(mydata[j])&&mydata[j]!=UNIVERSAL_NON_REPEATING)
			{
				if(ndown>0 && ((codeCount>1) || bForceKeyUp || (codeCount==1 && IsAnyModifierDown())))
				{
					ndown--;
					PostDelay ( DATAI(TEXT("Key_Down_Time"),5));
					PostKey (mydata[j],IK_UP);
					if (datasenddelay>0)
						PostDelay  (datasenddelay);
				}
				LiftAllModifiers();
			}
		}
		else if (IKSettings::GetSettings()->m_iShiftKeyAction==kSettingsShiftLocking)
		{
			if (!IsModifier(mydata[j])&&!IsMouseAction(mydata[j])&&mydata[j]!=UNIVERSAL_NON_REPEATING)
			{
				if(ndown>0 && ((codeCount>1) || bForceKeyUp || (codeCount==1 && IsAnyModifierDown())))
				{
					ndown--;
					PostDelay ( DATAI(TEXT("Key_Down_Time"),5));
					PostKey (mydata[j],IK_UP);
					if (datasenddelay>0)
						PostDelay  (datasenddelay);
				}
				LiftAllLatchedModifiers();
			}
		}

		j++;
	}
	IKASSERT(j<MAX_OVL_SIZE);

	//  handle modifiers post key
	if (IKSettings::GetSettings()->m_iShiftKeyAction==kSettingsShiftLatching)
	{
	}
	else if (IKSettings::GetSettings()->m_iShiftKeyAction==kSettingsShiftLocking)
	{
	}
	else if (IKSettings::GetSettings()->m_iShiftKeyAction==kSettingsShiftNoLatch)
	{
	}

	return ndown;

}

bool IKDevice::IsKeystroke(BYTE code)
{
	if (code>=UNIVERSAL_F1 && code <=UNIVERSAL_PAUSE)
		return true;
	if (code>=UNIVERSAL_SPACE && code <=UNIVERSAL_SPACE)
		return true;
	if (code>=UNIVERSAL_NUMPAD_0 && code <=UNIVERSAL_NUMPAD_DECIMAL)
		return true;
	if (code>=UNIVERSAL_INSERT && code <=UNIVERSAL_SLASH)
		return true;
	if (code>=UNIVERSAL_0 && code <=UNIVERSAL_9)
		return true;
	if (code>=0x3a && code <=UNIVERSAL_QUOTE)
		return true;
	if (code>=UNIVERSAL_A && code <=UNIVERSAL_Z)
		return true;
	if (code>=UNIVERSAL_LEFT_BRACKET && code <=0x5f)
		return true;
	if (code>=UNIVERSAL_ENTER && code <=0x6f)
		return true;
	if (code>=UNIVERSAL_SHIFT && code <=UNIVERSAL_COMMAND)
		return true;

	return false;
}


bool IKDevice::IsIdle()
{
	// return m_commandQueue.isEmpty();
	return true;
}

static int stringcompare ( BYTE *a, const TCHAR *b )
{
	//  compute two string lengths
	
	int lena = 0;
	while (a[lena]!=0)
		lena++;
		
	int lenb = 0;
	while (b[lenb]!=0)
		lenb++;
		
	//  if they are not the same, not equal
	
	if (lena!=lenb)
		return -1;
		
	//  check each character
		
	for (int i=0;i<lena;i++)
	{
		//  chars do not match, not equal
		if (a[i] != b[i])
			return -1;
	}
	
	//  same length, all chars match, equal!
	return 0;
}


bool IKDevice::DoSetupKey(BYTE *data)
{
	static int state = 0;
	bool bRefreshCP = true;
	bool retVal = false;

	bool bResetLevel = false;

	if(stringcompare(data,TEXT("SHOW CONTROL PANEL"))==0)
	{
		KeySound(50);
		IKControlPanel::Toggle();
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("LIFTOFF ON"))==0)
	{
		KeySound(50);
		IKSettings::GetSettings()->m_bRequiredLiftOff = true;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("LIFTOFF OFF"))==0)
	{
		KeySound(50);
		IKSettings::GetSettings()->m_bRequiredLiftOff = false;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("REPEAT ON"))==0)
	{
		KeySound(50);
		IKSettings::GetSettings()->m_bRepeat = true;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("REPEAT OFF"))==0)
	{
		KeySound(50);
		IKSettings::GetSettings()->m_bRepeat = false;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("REPEAT LATCHING ON"))==0)
	{
		KeySound(50);
		IKSettings::GetSettings()->m_bRepeatLatching = true;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("REPEAT LATCHING OFF"))==0)
	{
		KeySound(50);
		IKSettings::GetSettings()->m_bRepeatLatching = false;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("RESET"))==0)
	{
		KeySound(DATAI(TEXT("Keyboard_Reset_Sound_Duration"),50));

		if(state!=6)
		{
			state = 6;
			bRefreshCP = false;
		}
		else
		{
			UserReset(false);
			state = 0;
		}
		retVal = true;
	}

	else if(stringcompare(data,TEXT("LIST FEATURES"))==0)
	{
		KeySound(50);
		if(state!=7)
		{
			state = 7;
		}
		else
		{
			BYTE command[IK_REPORT_LEN] = {IK_CMD_CP_LIST_FEATURES,0,0,0,0,0,0,0};
			PostCommand(command);

			state = 0;
		}
		bRefreshCP = false;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("HELP"))==0)
	{
		KeySound(50);
		BYTE command[IK_REPORT_LEN] = {IK_CMD_CP_HELP,0,0,0,0,0,0,0};
		PostCommand(command);

		bRefreshCP = false;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("SOUND OFF"))==0)
	{
		IKSettings::GetSettings()->m_iKeySoundVolume = kSettingsKeysound2;
		KeySound(50);
		IKSettings::GetSettings()->m_iKeySoundVolume = kSettingsKeysoundOff;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("SOUND 1"))==0)
	{
		IKSettings::GetSettings()->m_iKeySoundVolume = kSettingsKeysound1;
		KeySound(50);
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("SOUND 2"))==0)
	{
		IKSettings::GetSettings()->m_iKeySoundVolume = kSettingsKeysound2;
		KeySound(50);
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("SOUND 3"))==0)
	{
		IKSettings::GetSettings()->m_iKeySoundVolume = kSettingsKeysound3;
		KeySound(50);
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("SOUND 4"))==0)
	{
		IKSettings::GetSettings()->m_iKeySoundVolume = kSettingsKeysound4;
		KeySound(50);
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("SOUND TEST"))==0)
	{
		LongKeySound();
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("SMART TYPING ON"))==0)
	{
		KeySound(50);
		IKSettings::GetSettings()->m_bSmartTyping = true;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("SMART TYPING OFF"))==0)
	{
		KeySound(50);
		IKSettings::GetSettings()->m_bSmartTyping = false;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("SHIFT LATCHING"))==0)
	{
		KeySound(50);
		IKSettings::GetSettings()->m_iShiftKeyAction = kSettingsShiftLatching;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("SHIFT LOCKING"))==0)
	{
		KeySound(50);
		IKSettings::GetSettings()->m_iShiftKeyAction = kSettingsShiftLocking;
		state = 0;
		retVal = true;
	}

	else if(stringcompare(data,TEXT("SHIFT NONLATCHING"))==0)
	{
		KeySound(50);
		IKSettings::GetSettings()->m_iShiftKeyAction = kSettingsShiftNoLatch;
		state = 0;
		retVal = true;
	}

	else if (stringcompare(data,TEXT("RESPONSE RATE"))==0)
	{
		KeySound(50);
		state = 1;
		retVal = true;
		bRefreshCP = false;
	}

	else if (stringcompare(data,TEXT("REPEAT RATE"))==0)
	{
		KeySound(50);
		state = 2;
		retVal = true;
		bRefreshCP = false;
	}

	else if (stringcompare(data,TEXT("MOUSE SPEED"))==0)
	{
		KeySound(50);
		state = 3;
		retVal = true;
		bRefreshCP = false;
	}

	else if (stringcompare(data,TEXT("DATA SEND RATE"))==0)
	{
		KeySound(50);
		state = 4;
		retVal = true;
		bRefreshCP = false;
	}

	else if (stringcompare(data,TEXT("SWITCH SETTING"))==0)
	{
		KeySound(50);
		state = 5;
		retVal = true;
		bRefreshCP = false;
	}

	else if (
				stringcompare(data, TEXT( "1"))==0 || 
				stringcompare(data, TEXT( "2"))==0 || 
				stringcompare(data, TEXT( "3"))==0 || 
				stringcompare(data, TEXT( "4"))==0 || 
				stringcompare(data, TEXT( "5"))==0 || 
				stringcompare(data, TEXT( "6"))==0 || 
				stringcompare(data, TEXT( "7"))==0 || 
				stringcompare(data, TEXT( "8"))==0 || 
				stringcompare(data, TEXT( "9"))==0 || 
				stringcompare(data, TEXT("10"))==0 || 
				stringcompare(data, TEXT("11"))==0 || 
				stringcompare(data, TEXT("12"))==0 || 
				stringcompare(data, TEXT("13"))==0 || 
				stringcompare(data, TEXT("14"))==0 || 
				stringcompare(data, TEXT("15"))==0
			)
	{
		KeySound(50);

		switch (state)
		{
		case 1:
			IKSettings::GetSettings()->m_iResponseRate = IKUtil::StringToInt((TCHAR *)data);
			break;
		case 2:
			IKSettings::GetSettings()->m_iRepeatRate = IKUtil::StringToInt((TCHAR *)data);
			break;
		case 3:
			IKSettings::GetSettings()->m_iMouseSpeed = IKUtil::StringToInt((TCHAR *)data);
			break;
		case 4:
			IKSettings::GetSettings()->m_iDataSendRate = IKUtil::StringToInt((TCHAR *)data);
			break;
		case 5:
			{
				if (IKSettings::GetSettings()->m_iMode != kSettingsModeSwitch)
					bResetLevel = true;
				IKSettings::GetSettings()->m_iMode = kSettingsModeSwitch;

				int n = IKUtil::StringToInt((TCHAR *)data);
				if (IKSettings::GetSettings()->m_iUseThisSwitchSetting != n)
					bResetLevel = true;
				IKSettings::GetSettings()->m_iUseThisSwitchSetting = n;
			}
			break;
		case 6:
			break;
		case 7:
			break;
		}
		state = 0;
		retVal = true;
	}

	if(retVal)
	{
		if (bRefreshCP)
		{
			IKSettings::GetSettings()->Write();
			IKEngine::GetEngine()->ReloadSwitchOverlay();
			PostCPRefresh();
		}

		(void) ComputeMouseInc ( 0 ); //  resets accell

	}

	if (bResetLevel)
		SetLevel(1);

	return retVal;
}

bool IKDevice::HasNonRepeating(BYTE *mydata)
{
	if (mydata==NULL)
		return false;

	int j = 0;
	while (mydata[j]!=0)
	{					
		switch (mydata[j])
		{
			case UNIVERSAL_NON_REPEATING:
				return true;
				break;
		}

		j++;
	}

	return false;
}


bool IKDevice::HasOnlyMouseMoveActions(BYTE *mydata)
{
	if (mydata==NULL)
		return false;

	int j = 0;
	while (mydata[j]!=0)
	{	
		if (!IsMouseMove(mydata[j]))
			return false;
		j++;
	}

	return true;
}

bool IKDevice::HasMouseClicks(BYTE *mydata)
{
	if (mydata==NULL)
		return false;

	int j = 0;
	while (mydata[j]!=0)
	{	
		if (IsMouseClick(mydata[j]))
			return true;
		j++;
	}

	return false;
}

bool IKDevice::HasMouseActions(BYTE *mydata)
{
	if (mydata==NULL)
		return false;

	int j = 0;
	while (mydata[j]!=0)
	{	
		if (IsMouseClick(mydata[j]))
			return true;
		if (IsMouseMove(mydata[j]))
			return true;
		j++;
	}

	return false;
}

bool IKDevice::HasSetupCodes(BYTE *mydata)
{
	if (mydata==NULL)
		return false;

	int j = 0;
	while (mydata[j]!=0)
	{	
		if (IsSetupCode(mydata[j]))
			return true;
		j++;
	}

	return false;
}

int IKDevice::CountRealCodes(BYTE *data)
{
	// This function counts the codes in the
	// given string, excluding setup keys and the like.

	int i = 0;
	int ncodes = 0;
	while (data[i]!=0)
	{
		if (
				(data[i]>=UNIVERSAL_F1           && data[i]<=UNIVERSAL_F15) ||
				(data[i]>=UNIVERSAL_NUMPAD_0     && data[i]<=UNIVERSAL_NUMPAD_DECIMAL) ||
				(data[i]>=UNIVERSAL_INSERT       && data[i]<=UNIVERSAL_SLASH) ||
				(data[i]>=UNIVERSAL_0            && data[i]<=UNIVERSAL_9) ||
				(data[i]>=UNIVERSAL_SEMICOLON    && data[i]<=UNIVERSAL_SEMICOLON) ||
				(data[i]>=UNIVERSAL_EQUALS       && data[i]<=UNIVERSAL_QUOTE) ||
				(data[i]>=UNIVERSAL_A            && data[i]<=UNIVERSAL_Z) ||
				(data[i]>=UNIVERSAL_LEFT_BRACKET && data[i]<=UNIVERSAL_RIGHT_BRACKET) ||
				(data[i]>=UNIVERSAL_ENTER        && data[i]<=UNIVERSAL_NUM_LOCK) ||
				(data[i]>=UNIVERSAL_SHIFT        && data[i]<=UNIVERSAL_SHIFT) ||
				(data[i]>=UNIVERSAL_RIGHT_SHIFT  && data[i]<=UNIVERSAL_RIGHT_SHIFT) ||
				(data[i]>=UNIVERSAL_CONTROL      && data[i]<=UNIVERSAL_CONTROL) ||
				(data[i]>=UNIVERSAL_RIGHT_CONTROL&& data[i]<=UNIVERSAL_RIGHT_CONTROL) ||
				(data[i]>=UNIVERSAL_MENU         && data[i]<=UNIVERSAL_MENU) ||
				(data[i]>=UNIVERSAL_COMMAND      && data[i]<=UNIVERSAL_COMMAND) ||
				(data[i]>=UNIVERSAL_MOUSE_UP     && data[i]<=UNIVERSAL_MOUSE_BUTTON_TOGGLE)
			)
			ncodes++;

		i++;
	}

	return ncodes;

}

bool IKDevice::IsModifier(BYTE code)
{
	switch (code)
	{
		case UNIVERSAL_ALT:
		case UNIVERSAL_ALTGR:
		case UNIVERSAL_CONTROL:
		case UNIVERSAL_RIGHT_CONTROL:
		case UNIVERSAL_SHIFT:
		case UNIVERSAL_RIGHT_SHIFT:
		case UNIVERSAL_COMMAND:
			return true;
			break;
	}

	return false;
}

bool IKDevice::IsSetupCode(BYTE code)
{
	return false;
	//  TODO what is this?
	switch (code)
	{
		case UNIVERSAL_SETUP1:
		case UNIVERSAL_SETUP2:

		case UNIVERSAL_SETUP1_RESPONSE_RATE: // plus rate
		case UNIVERSAL_SETUP1_LIFT_OFF_ON:
		case UNIVERSAL_SETUP1_LIFT_OFF_OFF:
		case UNIVERSAL_SETUP1_REPEAT_LATCHING_ON:
		case UNIVERSAL_SETUP1_REPEAT_LATCHING_OFF:
		case UNIVERSAL_SETUP1_SHIFT_NO_LATCHING:
		case UNIVERSAL_SETUP1_3LIGHTS:
		case UNIVERSAL_SETUP1_6LIGHTS:
		case UNIVERSAL_SETUP1_MOUSE_SPEED: // plus speed
		case UNIVERSAL_SETUP1_CUSTOM_OVERLAY_LEVEL:	// plus level
		case UNIVERSAL_SETUP1_DATA_SEND_RATE: // plus rate
		case UNIVERSAL_SETUP1_REPEAT_RATE: // plus rate
		case UNIVERSAL_SETUP1_KEYSOUND_ON:
		case UNIVERSAL_SETUP1_KEYSOUND_OFF:
		case UNIVERSAL_SETUP1_SHIFT_LATCHING:
		case UNIVERSAL_SETUP1_LIST_FEATURES:
		case UNIVERSAL_SETUP1_SHIFT_LOCKING:
		case UNIVERSAL_SETUP1_MOUSE_ARROWS_ON:
		case UNIVERSAL_SETUP1_MOUSE_ARROWS_OFF:
		case UNIVERSAL_SETUP1_SMART_TYPING_ON:
		case UNIVERSAL_SETUP1_SMART_TYPING_OFF:

		//case UNIVERSAL_SETUP2_FEATURE_RESET:
			return true;
			break;
	}

	return false;
}

bool IKDevice::IsMouseMove(BYTE code)
{
	switch (code)
	{
		case UNIVERSAL_MOUSE_DOWN:
		case UNIVERSAL_MOUSE_UP:
		case UNIVERSAL_MOUSE_LEFT:
		case UNIVERSAL_MOUSE_RIGHT:

		case UNIVERSAL_MOUSE_DOWN_LEFT:
		case UNIVERSAL_MOUSE_DOWN_RIGHT:

		case UNIVERSAL_MOUSE_UP_LEFT:
		case UNIVERSAL_MOUSE_UP_RIGHT:

			return true;
			break;
	}

	return false;
}

bool IKDevice::IsMouseClick(BYTE code)
{
	switch (code)
	{
		case UNIVERSAL_MOUSE_BUTTON_CLICK:
		case UNIVERSAL_MOUSE_BUTTON_DOUBLECLICK:
		case UNIVERSAL_MOUSE_BUTTON_DOWN:
		case UNIVERSAL_MOUSE_BUTTON_TOGGLE:
		case UNIVERSAL_MOUSE_BUTTON_UP:
		case UNIVERSAL_MOUSE_RBUTTON_CLICK:
		case UNIVERSAL_MOUSE_RBUTTON_DOUBLECLICK:
		case UNIVERSAL_MOUSE_RBUTTON_DOWN:
		case UNIVERSAL_MOUSE_RBUTTON_TOGGLE:
		case UNIVERSAL_MOUSE_RBUTTON_UP:
			return true;
	}

	return false;
}

int IKDevice::ComputeMouseInc(int code)
{
	int inc = 1;

	if ( code==0 || code != m_lastcode)
	{
		//  reset the time
		m_counter = IKUtil::GetCurrentTimeMS();
	}
	else if (code == m_lastcode)
	{
		int elapsed = IKUtil::GetCurrentTimeMS() - m_counter;
		int speed = IKSettings::GetSettings()->m_iMouseSpeed;

		int knee   = DATAI(TEXT("Mouse_Accelleration_Knee"),1000);
		int factor = DATAI(TEXT("Mouse_Accelleration_Scale"),50);
#ifdef PLAT_MACINTOSH
		factor = DATAI(TEXT("Mouse_Accelleration_Scale_Mac"),factor);
#endif
#ifdef PLAT_WINDOWS
		factor = DATAI(TEXT("Mouse_Accelleration_Scale_Win"),factor);
#endif

		if (elapsed<knee)
			inc = speed*elapsed*factor/100/knee;
		else
			inc = speed*factor/100;
		if (inc==0)
			inc = 1;
	}

	m_lastcode = code;
	return inc;
}

void IKDevice::DeleteLastSentOverlay()
{
	IKEngine::GetEngine()->DeleteLastSentOverlay();
}

void IKDevice::ResetSound()
{
	int freqLow  = DATAI(TEXT("Sweep_Sound_Low_Frequency")	,200);
	int freqHigh = DATAI(TEXT("Sweep_Sound_High_Frequency")	,250);
	int duration = DATAI(TEXT("Sweep_Sound_Duration"), 200);

	SweepSound(freqLow, freqHigh,duration);
	SweepSound(freqHigh,freqLow, duration);
}

void IKDevice::HackKeyContent(BYTE *data)
{
	//  examine the key content and change it if you like

#ifdef PLAT_WINDOWS
	//  special handling for IntelliPics Windows. (not II)
	//  IK USB types Control when the overlay says Command,
	//  but IP-W uses Alt-xxx for some functions.
	//  map them here.

	IKString strAppPath = IKUtil::GetCurrentApplicationPath();
	strAppPath.Lower();
	IKUtil::StripFileName(strAppPath,true,false);
	if ( (strAppPath.Find(DATAS(TEXT("Intellipics_Program"),TEXT("intellipics.exe")))!=-1) || 
		 (strAppPath.Find(DATAS(TEXT("Intellipics_Player_Program"),TEXT("intellipics player.exe")))!=-1) )
	{
		//  we are intellipics.  Turn all Commands into Alts
		//  for this key.
		int i = 0;
		while (data[i]!=0)
		{
			if (data[i]==UNIVERSAL_COMMAND && data[i+1]!='R')
				data[i] = UNIVERSAL_ALT;

			i++;
		}
	}

#endif

	//  replace the right click placeholder
	IKString text(DATAS(TEXT("right_click_text"),TEXT("")));
	if (!text.IsEmpty())
	{
		IKString s((char *)data);
		int i = s.Find(text);
		if (i!=-1)
		{
			char yy[2] = {(char)UNIVERSAL_MOUSE_RBUTTON_CLICK,0};
			IKString y(yy);
			y += s.Mid(text.GetLength(),999);
			IKString::strcpy((TCHAR *)data,y);
		}
	}

	//  move go to level commands to the end.
	{
		//  count bytes
		int n = 0;
		while (data[n]!=0)
			n++;

		for (int i=0;i<n;i++)
		{
			if (data[i]>=UNIVERSAL_GOTO_LEVEL_1 && data[i]<=UNIVERSAL_GOTO_LEVEL_15)
			{
				BYTE d = data[i];
				for (int j=i+1;j<n;j++)
					data[j-1] = data[j];
				data[n-1] = d;
				i--;
				n--;
			}
		}
	}
}

void IKDevice::PostMouseButtonClick(int whichButton)
{
	PostMouseButton(whichButton,IK_DOWN);
	PostDelay(DATAI(TEXT("Mouse_Click_Delay"),5));
	PostMouseButton(whichButton,IK_UP);
	PostDelay(DATAI(TEXT("Mouse_Click_Delay"),5));
}

void IKDevice::PostMouseButtonDoubleClick(int whichButton)
{
	PostMouseButtonClick(whichButton);
        PostDelay(10);
	PostMouseButtonClick(whichButton);
        PostDelay(10);
}

void IKDevice::PostMouseButtonToggle(int whichButton)
{
	PostMouseButton(whichButton,IK_TOGGLE);
}

bool IKDevice::IsMouseAction(BYTE code)
{
	BYTE key[2] = {code,0};
	return HasMouseActions(key);
}

bool IKDevice::IsAnyModifierDown()
{
	bool bState = m_modShift.GetState() || m_modAlt.GetState() ||
		m_modControl.GetState() || m_modCommand.GetState();
	return bState;
}

void IKDevice::PostKeyDone()
{
	BYTE command[IK_REPORT_LEN] = {IK_CMD_KEY_DONE,0,0,0,0,0,0,0};
	PostCommand(command);
}

void IKDevice::PostKeyStart()
{
	BYTE command[IK_REPORT_LEN] = {IK_CMD_KEY_START,0,0,0,0,0,0,0};
	PostCommand(command);
}

void IKDevice::PostKeyRepeat()
{
	BYTE command[IK_REPORT_LEN] = {IK_CMD_KEY_REPEAT,0,0,0,0,0,0,0};
	PostCommand(command);
}

void IKDevice::LiftAllModifiers()
{
	m_modShift.SetState(kModifierStateOff);
	m_modAlt.SetState(kModifierStateOff);
	m_modControl.SetState(kModifierStateOff);
	m_modCommand.SetState(kModifierStateOff);
}

void IKDevice::LiftAllLatchedModifiers()
{
	if (m_modShift.GetState() == kModifierStateLatched)
		m_modShift.SetState(kModifierStateOff);
	if (m_modAlt.GetState() == kModifierStateLatched)
		m_modAlt.SetState(kModifierStateOff);
	if (m_modControl.GetState() == kModifierStateLatched)
		m_modControl.SetState(kModifierStateOff);
	if (m_modCommand.GetState() == kModifierStateLatched)
		m_modCommand.SetState(kModifierStateOff);
}

void IKDevice::IKModifier::Execute(int code)
{
	int theCode = m_universalCode;
	m_universalCodeOverride = 0;
	if (code!=0)
	{
		theCode = code;
		m_universalCodeOverride = theCode;
	}
	
	IKSettings *pSettings = IKSettings::GetSettings();
	if (pSettings==NULL)
		return;

	switch (pSettings->m_iShiftKeyAction)
	{
		case kSettingsShiftLatching:
			if (m_state==kModifierStateOff)
			{
				//  was off, turn on and put down
				m_device->PostKey(theCode,IK_DOWN);
				m_state = kModifierStateLatched;
			}
			else
			{
				//  was on, put up and turn off
				m_device->PostKey(theCode,IK_UP);
				m_state = kModifierStateOff;
			}
			break;

		case kSettingsShiftLocking:
			if (m_state==kModifierStateOff)
			{
				//  put key down, state = latched
				m_device->PostKey(theCode,IK_DOWN);
				m_state = kModifierStateLatched;
			}
			else if (m_state==kModifierStateLatched)
			{
				//  leave key alone, state = locked
				m_state = kModifierStateLocked;
			}
			else if (m_state==kModifierStateLocked)
			{
				//  put key up, state = off
				m_device->PostKey(theCode,IK_UP);
				m_state = kModifierStateOff;
			}
			break;

		case kSettingsShiftNoLatch:
			//  just put the key down/up, no change in state
			{
				int keyDownDelay = DATAI(TEXT("Key_Down_Time"),5);
				m_device->PostKey(theCode,IK_DOWN,keyDownDelay);
				//m_device->PostKey(theCode,IK_UP,keyDownDelay);
				m_state = kModifierStateLatched;
			}
			break;

		default:
			break;
	}
}

void IKDevice::IKModifier::SetState(int state)
{
	IKSettings *pSettings = IKSettings::GetSettings();
	if (pSettings==NULL)
		return;

	int theCode = m_universalCode;
	if (m_universalCodeOverride!=0)
		theCode = m_universalCodeOverride;

	switch (pSettings->m_iShiftKeyAction)
	{
		case kSettingsShiftLatching:
			if (state==kModifierStateOff)
			{
				if (m_state==kModifierStateLatched)
					m_device->PostKey(theCode,IK_UP);
			}
			else if (state==kModifierStateLatched)
			{
				if (m_state==kModifierStateOff)
					m_device->PostKey(theCode,IK_DOWN);
			}
			break;

		case kSettingsShiftLocking:
			if (state==kModifierStateOff)
			{
				if (m_state!=kModifierStateOff)
					m_device->PostKey(theCode,IK_UP);
			}
			else if (state==kModifierStateLatched)
			{
				if (m_state==kModifierStateOff)
					m_device->PostKey(theCode,IK_DOWN);
			}
			else if (state==kModifierStateLocked)
			{
				if (m_state==kModifierStateOff)
					m_device->PostKey(theCode,IK_DOWN);
			}
			break;

		case kSettingsShiftNoLatch:
			if (state==kModifierStateOff)
			{
				if (m_state!=kModifierStateOff)
					m_device->PostKey(theCode,IK_UP);
			}
			break;

		default:
			break;
	}

	m_state = state;
}

void IKDevice::SetLEDs()
{
	if (!IsSwitchedOn())
		return; 

	if (!IsOpen())
		return;

	bool bShift    = ( m_modShift.GetState()   != 0 );
	bool bControl  = ( m_modControl.GetState() != 0 );
	bool bAlt      = ( m_modAlt.GetState()     != 0 );
	bool bCommand  = ( m_modCommand.GetState() != 0 );
	bool bNumLock  = IsNumLockOn();
	bool bMouse    = IsMouseDown();
	bool bCapsLock = IsCapsLockOn();

	//  3 lights is shift, caps lock, mouse down
	PostSetLED(1,bShift);
	PostSetLED(4,bCapsLock);
	PostSetLED(7,bMouse);

	//  6 lights is alt, control/command, num lock
	bool b6lights = (IKSettings::GetSettings()->m_iIndicatorLights == kSettings6lights);
	if (b6lights)
	{
		PostSetLED(2,bAlt);
		PostSetLED(5,bControl||bCommand);
		PostSetLED(8,bNumLock);

		PostSetLED(3,false);
		PostSetLED(6,false);
		PostSetLED(9,false);
	}
	else
	{
		PostSetLED(3,bShift);
		PostSetLED(6,bCapsLock);
		PostSetLED(9,bMouse);

		PostSetLED(2,false);
		PostSetLED(5,false);
		PostSetLED(8,false);
	}


}


bool IKDevice::IsMouseDown()
{
	return ((m_MouseReport[0] & 0x01) || (m_MouseReport[0] & 0x02) || (m_MouseReport[0] & 0x04));
}

IKString IKDevice::GetCommandName ( BYTE command )
{
    IKString s = TEXT("");
    
    switch (command)
    {
	case IK_CMD_GET_VERSION:
	    s = TEXT("IK_CMD_GET_VERSION");
	    break;
	    
	case IK_CMD_LED:
	    s = TEXT("IK_CMD_LED");
	    break;

	case IK_CMD_DELAY:
	    s = TEXT("IK_CMD_DELAY");
	    break;

	case IK_CMD_SCAN:
	    s = TEXT("IK_CMD_SCAN");
	    break;

	case IK_CMD_TONE:
	    s = TEXT("IK_CMD_TONE");
	    break;

	case IK_CMD_GET_EVENT:
	    s = TEXT("IK_CMD_GET_EVENT");
	    break;

	case IK_CMD_INIT:
	    s = TEXT("IK_CMD_INIT");
	    break;

	case IK_CMD_EEPROM_READ:
	    s = TEXT("IK_CMD_EEPROM_READ");
	    break;

	case IK_CMD_EEPROM_WRITE:
	    s = TEXT("IK_CMD_EEPROM_WRITE");
	    break;

	case IK_CMD_ONOFFSWITCH:
	    s = TEXT("IK_CMD_ONOFFSWITCH");
	    break;

	case IK_CMD_CORRECT:
	    s = TEXT("IK_CMD_CORRECT");
	    break;

	case IK_CMD_EEPROM_READBYTE:
	    s = TEXT("IK_CMD_EEPROM_READBYTE");
	    break;

	case IK_CMD_RESET_DEVICE:
	    s = TEXT("IK_CMD_RESET_DEVICE");
	    break;

	case IK_CMD_START_AUTO:
	    s = TEXT("IK_CMD_START_AUTO");
	    break;

	case IK_CMD_STOP_AUTO:
	    s = TEXT("IK_CMD_STOP_AUTO");
	    break;

	case IK_CMD_ALL_LEDS:
	    s = TEXT("IK_CMD_ALL_LEDS");
	    break;

	case IK_CMD_START_OUTPUT:
	    s = TEXT("IK_CMD_START_OUTPUT");
	    break;

	case IK_CMD_STOP_OUTPUT:
	    s = TEXT("IK_CMD_STOP_OUTPUT");
	    break;

	case IK_CMD_REFLECT_KEYSTROKE:
	    s = TEXT("IK_CMD_REFLECT_KEYSTROKE");
	    break;

	case IK_CMD_REFLECT_MOUSE_MOVE:
	    s = TEXT("IK_CMD_REFLECT_MOUSE_MOVE");
	    break;

	case IK_CMD_KEY_START:
		s = TEXT("IK_CMD_KEY_START");
		break;

	case IK_CMD_KEY_REPEAT:
		s = TEXT("IK_CMD_KEY_REPEAT");
		break;

	case IK_CMD_KEY_DONE:
		s = TEXT("IK_CMD_KEY_DONE");
		break;
	    
	default:
	    s = TEXT("Unknown-");
	    s += IKUtil::IntToString(command);
	    break;
    }

    return s;
}

bool IKDevice::IsCapsLockOn()
{

#ifdef PLAT_WINDOWS

	if (IKUtil::IsWin2KOrGreater())
	{
		SHORT cl = GetKeyState ( VK_CAPITAL );
		return ((cl & 0x0001)==1);
	}
	else
	{
		SHORT nl=0,cl=0;
		DllGetStates(&nl,&cl);
		if (cl==-1)
			return false;
		return ((cl & 0x0001)==1);	
	}

#endif

#ifdef PLAT_MACINTOSH
	return m_bCapsLock;
#endif

}

bool IKDevice::IsNumLockOn()
{

#ifdef PLAT_WINDOWS

	if (IKUtil::IsWin2KOrGreater())
	{
		SHORT nl = GetKeyState ( VK_NUMLOCK );
		return ((nl & 0x0001)==1);
	}
	else
	{
		SHORT nl=0,cl=0;
		DllGetStates(&nl,&cl);
		if (nl==-1)
			return false;
		return ((nl & 0x0001)==1);	
	}

#endif

#ifdef PLAT_MACINTOSH
	return m_bNumLock;
#endif

}

bool IKDevice::HasStandardOverlay()
{
	if (GetDevType() != 1)
		return false;  //  only IK has standard overlays

	return (m_currentOverlay != 7 && m_currentOverlay!=-1);
}

void IKDevice::OverlaySendingFeedback()
{
	if (!IsOpen())
		return;

	int delay = 300;

	if (IsSwitchedOn())
	{
		PostSetLED ( 1, true);
		PostDelay(delay);
		PostSetLED ( 1, false);

		PostSetLED ( 2, true);
		PostDelay(delay);
		PostSetLED ( 2, false);

		PostSetLED ( 4, true);
		PostDelay(delay);
		PostSetLED ( 4, false);

		PostSetLED ( 5, true);
		PostDelay(delay);
		PostSetLED ( 5, false);

		PostSetLED ( 7, true);
		PostDelay(delay);
		PostSetLED ( 7, false);

		PostSetLED ( 8, true);
		PostDelay(delay);
		PostSetLED ( 8, false);
	}
	else
	{
		for (int numFlashes = 0; numFlashes < 6; numFlashes++)
		{
			PostSetLED ( 2, true);
			PostSetLED ( 5, true);
			PostSetLED ( 8, true);
			PostDelay(delay);

			PostDelay(delay);

			PostSetLED ( 2, false);
			PostSetLED ( 5, false);
			PostSetLED ( 8, false);
			PostDelay(delay);

			PostDelay(delay);
		}
	}
}

static int kbdSettingsState = 0;

#ifdef PLAT_WINDOWS
static int    nKBDelay;  // The old keyboard delay.
static DWORD  nKBSpeed;  // The old keyboard repeat speed.
#endif

#ifdef PLAT_MACINTOSH
static SInt16 nKBDelay;
static SInt16 nKBSpeed;
#endif

void IKDevice::SaveKeyboardSettings()
{
	if (kbdSettingsState==0)
	{
	    //  save 'em

#ifdef PLAT_WINDOWS

	    // Get the old values
	    DWORD result = SystemParametersInfo( SPI_GETKEYBOARDDELAY, 0, &nKBDelay, 0 );
	    result = SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKBSpeed, 0 );
        
	    //  get current IK USB keyboard repeat setting
	    int ourRepeatRate = IKSettings::GetSettings()->m_iRepeatRate;
    
	    // Set the new keyboard repeat delay
	    // This value can be between 0 and 3.
	    // 0 = 250 ms delay, 3 = 1 second (1000 ms) delay.
	    int delay;
	    if (ourRepeatRate>=1 && ourRepeatRate<=4)
		    delay = 3;
	    if (ourRepeatRate>=5 && ourRepeatRate<=8)
		    delay = 2;
	    if (ourRepeatRate>=9 && ourRepeatRate<=12)
		    delay = 1;
	    if (ourRepeatRate>=13 && ourRepeatRate<=15)
		    delay = 0;
	    result = SystemParametersInfo( SPI_SETKEYBOARDDELAY, delay, NULL, SPIF_UPDATEINIFILE );
    
	    // Set the keyboard repeat speed
	    // It can be any number between 0 and 31,
	    // 0 = 2.5 repetitions per second, 31 = 31 repetitions per second.
	    DWORD speed = (ourRepeatRate-1) * 31 / 14;
	    result = SystemParametersInfo( SPI_SETKEYBOARDSPEED, speed, NULL, SPIF_UPDATEINIFILE );
        
	    //  TODO: this seems arbitrary, but works for XP at least.
	    IKUtil::Sleep(1);

#endif

#ifdef PLAT_MACINTOSH

	    CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();
	
	    //  save old values
	    CFMutableDictionaryRef Helper_Dict = CFDictionaryCreateMutable(NULL, 3,
			&kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	    CFDictionaryAddValue(Helper_Dict, CFSTR("command"),CFSTR("save repeat values"));
	    	    	
        CFNotificationCenterPostNotification ( center, CFSTR("IKUSBHelper Notification"), NULL, Helper_Dict, FALSE);
	    	
	    CFRelease(Helper_Dict);
	    
	    //  set new values
	    CFMutableDictionaryRef Helper_Dict2 = CFDictionaryCreateMutable(NULL, 3,
		&kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	    CFDictionaryAddValue(Helper_Dict2, CFSTR("command"),CFSTR("set repeat values"));
		    
	    //  get current IK USB keyboard repeat setting
	    int ourRepeatRate = IKSettings::GetSettings()->m_iRepeatRate;
		    
	    //  calculate new delay and repeat
	    int delay = DATAI(TEXT("Min_Repeat_Start"),0) + 
		(15-ourRepeatRate)*(DATAI(TEXT("Max_Repeat_Start"),0)-DATAI(TEXT("Min_Repeat_Start"),0))/14;
	    int speed = 50*(16-ourRepeatRate);
		    
	    //  convert to ticks
	    SInt16 tDelay = delay*60/1000;
	    SInt16 tSpeed = speed*60/1000;
	    
	    IKString s = IKUtil::IntToString(tDelay);
	    CFStringRef cfDelay = CFStringCreateWithCString(NULL, (TCHAR *) s, kCFStringEncodingMacRoman);
	    s = IKUtil::IntToString(tSpeed);
	    CFStringRef cfSpeed = CFStringCreateWithCString(NULL, (TCHAR *) s, kCFStringEncodingMacRoman);
		    
	    CFDictionaryAddValue(Helper_Dict2, CFSTR("arg1"),cfDelay);
	    CFDictionaryAddValue(Helper_Dict2, CFSTR("arg2"),cfSpeed); 
	    
#if defined (DEBUG_CONTROL_PANEL)|defined (DEBUG_IKUSB)
        NSLog(@"IKDevice::SaveKeyboardSettings cfDelay %@", cfDelay);
        NSLog(@"IKDevice::SaveKeyboardSettings cfSpeed %@", cfSpeed);
#endif
        CFNotificationCenterPostNotification ( center, CFSTR("IKUSBHelper Notification"), NULL, Helper_Dict2, FALSE);
	
	    CFRelease(Helper_Dict2);

#endif

	}

	kbdSettingsState++;
}

void IKDevice::RestoreKeyboardSettings()
{
	kbdSettingsState--;
	if (kbdSettingsState==0)
	{
		//  restore 'em

#ifdef PLAT_WINDOWS

	    DWORD result = SystemParametersInfo( SPI_SETKEYBOARDDELAY, nKBDelay, NULL, SPIF_UPDATEINIFILE );
	    result = SystemParametersInfo( SPI_SETKEYBOARDSPEED, nKBSpeed, NULL, SPIF_UPDATEINIFILE );

#endif

#ifdef PLAT_MACINTOSH

	    CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();
	
	    //  save old values
	    CFMutableDictionaryRef Helper_Dict = CFDictionaryCreateMutable(NULL, 3,
		&kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	    CFDictionaryAddValue(Helper_Dict, CFSTR("command"),CFSTR("restore repeat values"));
	    	    
        CFNotificationCenterPostNotification ( center, CFSTR("IKUSBHelper Notification"), NULL, Helper_Dict, FALSE);
 
	    CFRelease(Helper_Dict);
	    
#endif

	}

	IKASSERT(kbdSettingsState>=0);
	if (kbdSettingsState<0)
		kbdSettingsState = 0;
}


void IKDevice::DoKeyStart()
{
	bool bUseSys = IKSettings::GetSettings()->m_bUseSystemRepeatSettings;
	bool bUseSysOverride = !!DATAI(TEXT("Override_Use_System_Repeat_Settings"),1);

	if (bUseSys && bUseSysOverride)
		IKDevice::SaveKeyboardSettings();

#ifdef PLAT_WINDOWS

	int codepage = DATAI(TEXT("Unicode_Code_Page"),0);
	if (codepage != 0)
	{
		if (IKUtil::IsWin2KOrGreater())
		{
			//  TODO:  what to do for the japanese version?
			IKASSERT(false);
		}
		else
		{
			DLLSaveIME();
			KickDLL();
		}
	}


#endif

}


void IKDevice::DoKeyRepeat()
{

}


void IKDevice::DoKeyDone()
{
	if (IKSettings::GetSettings()->m_bSmartTyping)
	{
		if (m_lastCodeUp==UNIVERSAL_Q)
		{
			PostKeyStroke ( UNIVERSAL_U );
		}

		else if ( m_lastCodeUp==UNIVERSAL_SEMICOLON && m_bShifted )
		{
			PostKeyStroke (UNIVERSAL_SPACE);
			PostKeyStroke (UNIVERSAL_SPACE);
		}

		else if ( m_lastCodeUp==UNIVERSAL_SEMICOLON ||
			      m_lastCodeUp==UNIVERSAL_COMMA )
		{
			PostKeyStroke (UNIVERSAL_SPACE);
		}

		else if (  m_lastCodeUp == UNIVERSAL_PERIOD ||
			      (m_lastCodeUp == UNIVERSAL_SLASH && m_bShifted) ||
			      (m_lastCodeUp == UNIVERSAL_1 && m_bShifted) )
		{
			PostKeyStroke (UNIVERSAL_SPACE);
			PostKeyStroke (UNIVERSAL_SPACE);
			m_modShift.Execute();
		}

	}

	m_lastCodeUp = 0;

	LiftNonModifiers();

	bool bUseSys = IKSettings::GetSettings()->m_bUseSystemRepeatSettings;
	bool bUseSysOverride = !!DATAI(TEXT("Override_Use_System_Repeat_Settings"),1);

	if (bUseSys && bUseSysOverride)
		IKDevice::RestoreKeyboardSettings();

#ifdef PLAT_WINDOWS

	int codepage = DATAI(TEXT("Unicode_Code_Page"),0);
	if (codepage != 0)
	{
		if (IKUtil::IsWin2KOrGreater())
		{
			//  TODO:  what to do for the japanese version?
			IKASSERT(false);
		}
		else
		{
			DLLRestoreIME();
			KickDLL();
		}
	}

#endif

}


void IKDevice::LiftNonModifiers()
{
	for (unsigned int i=2;i<sizeof(m_KeyBoardReport);i++)
		m_KeyBoardReport[i] = 0;
		
	BYTE msg[IK_REPORT_LEN] = {IK_CMD_REFLECT_KEYSTROKE,0,0,0,0,0,0,0};
	for (unsigned int j=0;j<sizeof(m_KeyBoardReport);j++)
		msg[j+1] = m_KeyBoardReport[j];
		
	SendCommand(msg);
}

void IKDevice::PurgeToLastStart()
{
	m_commandQueue.Lock();

	int length = m_commandQueue.GetSize();

	for (int i=0;i<length;i++)
	{
		queueEntry qe;
		bool bGot = m_commandQueue.getAt(qe,i);
		if(bGot)
		{
			if(qe.buffer[0] == IK_CMD_KEY_START || qe.buffer[0] == IK_CMD_KEY_REPEAT)
			{
				m_commandQueue.newBack(i);
				break;
			}
		}
	}

	m_commandQueue.Unlock();

}

void IKDevice::UserReset(bool bFromCP)
{
	if (IsSwitchedOn())
	{
		IKSettings::GetSettings()->SetToDefault();
		//g_intellikeys[0].m_CustomOverlay.Unload();  //  TODO
		SetLevel(1);

		//  2005-0769 - don't forget last sent overlay
		DeleteLastSentOverlay();

		IKSettings::GetSettings()->Write();
		IKEngine::GetEngine()->ReloadSwitchOverlay();
		ResetSound();
		if (!bFromCP)
		{
			//  tell CP
			IKMessage::Send(TEXT("control panel"),kQueryResetKeyboard,0,0,0,0);
		}
	}
}

void IKDevice::GetMembraneStatus ( BYTE *data, int *ndata )
{
	//  assume no data
	*ndata = 0;

	//  number of x,y pairs, assume none
	int npairs = 0;

	//  scan the membrane looking for presses.
	int col,row;
	{
		for (col=0;col<IK_RESOLUTION_X;col++)
		{
			for (row=0;row<IK_RESOLUTION_Y;row++)
			{
				if (m_membrane[IK_RESOLUTION_Y-1-row][IK_RESOLUTION_X-1-col])
				{
					//  found a press.  Record it.
					data[npairs*2+1] = col;
					data[npairs*2+2] = row;
					npairs++;
				}
			}
		}
	}

	data[0] = npairs;
	*ndata = npairs * 2 + 1;

}

void IKDevice::GetSwitchStatus ( BYTE *data, int nswitch )
{
	data[0] = m_switches[nswitch-1];
}

void IKDevice::GetSensorArray(BYTE *data)
{
	int n = 0;
	for (int i=0;i<IK_NUM_SENSORS;i++)
	{
		data[n] = m_sensors[i];
		n++;
		data[n] = m_eepromData.sensorBlack[i];
		n++;
		data[n] = m_eepromData.sensorWhite[i];
		n++;
	}
}

int IKDevice::GetIndex()
{
	return m_index;
}

IKString IKDevice::GetSerialNumber()
{
	IKString serial;

	if (m_bEepromValid)
	{
		IKString s(m_eepromData.serialnumber,IK_EEPROM_SN_SIZE);
		serial = s;
	}
	else
	{
		serial = DATAS(TEXT("Unknown"),TEXT("Unknown"));
	}

	return serial;
}

void IKDevice::PostCPRefresh()
{
	BYTE command[IK_REPORT_LEN] = {IK_CMD_CP_REFRESH,0,0,0,0,0,0,0};
	PostCommand(command);
}


void IKDevice::PostKeyStroke(int code)
{
	PostKey ( code, IK_DOWN, 5 );
	//PostDelay(5);
	PostKey ( code, IK_UP, 5 );
	//PostDelay(5);
}

void IKDevice::PostKeyUnicode(BYTE lead, BYTE trail, int direction)
{
	BYTE command[IK_REPORT_LEN];
	command[0] = IK_CMD_KEYBOARD_UNICODE;
	command[1] = lead;
	command[2] = trail;
	command[3] = direction;
	PostCommand(command);
}

void IKDevice::DoKeyUnicode(BYTE lead, BYTE trail, int direction)
{

#ifdef PLAT_WINDOWS

	if (IKUtil::IsWinVistaOrGreater())
	{
		//  in Vista, we're asking the system tray to do this.

		BYTE data[10];
		data[0] = lead;
		data[1] = trail;
		data[2] = direction;
		int ndata = 3;

		// retry this 5 times with a short timeout
		for (int i=0;i<5;i++)
		{
			int result = IKMessage::SendTO(TEXT("system tray"),kQuerySendUnicode,data,ndata,0,0,1000);
			if (result == kResponseNoError)
				break;
		}

	}
	else if (IKUtil::IsWin2KOrGreater())
	{
		WORD wd = MAKEWORD ( trail, lead );

		INPUT in;
		in.type				= INPUT_KEYBOARD;
		in.ki.wVk			= 0;
		in.ki.time			= 0;
		in.ki.dwExtraInfo	= 0;
		in.ki.wScan			= wd;

		in.ki.dwFlags = KEYEVENTF_UNICODE;
		if (direction==IK_UP)
			in.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

		UINT result = result = ::SendInput( 1, &in, sizeof(INPUT));
	}

	else
	{
		if (direction==IK_DOWN)
		{

			int codepage = IKUtil::GetCodePage();
			if (codepage != 0)
			{
				WORD wd = MAKEWORD ( trail, lead );

				WORD oldstring[2] = {wd,0};
				char newstring[3] = {0,0,0};
				BOOL bDef = FALSE;

				int result = WideCharToMultiByte ( codepage, 0, oldstring, 1, newstring, 10, 0, &bDef);

				BYTE lead1  = 0;
				if (newstring[0]!=0)
					lead1 = newstring[0];
				BYTE trail1 = 0;
				if (newstring[1]!=0)
					trail1 = newstring[1];

				WORD wd2;
				if (trail1==0)
					wd2= MAKEWORD(lead1,0);
				else
					wd2 = MAKEWORD ( trail1, lead1 );

				HWND targetWin = GetForegroundWindow();
				DWORD targetThread = GetWindowThreadProcessId(targetWin, NULL);
				DWORD selfThread = GetCurrentThreadId();
				AttachThreadInput(selfThread, targetThread, TRUE);
				HWND activeWin = GetFocus();

				LRESULT lR=::SendMessage ( activeWin, WM_IME_CHAR, wd2 , 1 );

				AttachThreadInput(selfThread, targetThread, FALSE);

			}
		}

	}

#endif

}

void IKDevice::KickDLL()
{
	//  wiggle a key to kick the dll

#ifdef PLAT_WINDOWS
	
	UINT result;
	INPUT in;
	in.type = INPUT_KEYBOARD;
	in.ki.time = 0;
	in.ki.dwExtraInfo = 0;
	in.ki.wScan = 0;


	in.ki.wVk = DATAI(TEXT("win_kick_dll_key"),VK_F24);

	in.ki.dwFlags = 0;
	result = ::SendInput( 1, &in, sizeof(INPUT));
	Sleep(5);

	in.ki.dwFlags = KEYEVENTF_KEYUP;
	result = ::SendInput( 1, &in, sizeof(INPUT));
	Sleep(5);

#endif

}

void IKDevice::PostLiftAllModifiers()
{
	//  send the command
	BYTE report[IK_REPORT_LEN] = {IK_CMD_LIFTALLMODIFIERS,0,0,0,0,0,0,0};
	PostCommand(report);
}


void IKDevice::ReportDataToControlPanel(bool bForce)
{
	if (DATAI(TEXT("report_diagnostics_realtime"),0)==0)
		return;

	//  don't report if we're not in diagnostic mode
	if (!IKEngine::GetEngine()->GetDiagnosticMode())
		return;

	//  don't report if control panel is not there.
	if (!IKMessage::IsOwnerAlive(TEXT("control panel")))
		return;

	//  see if we did this "very" recently
	static unsigned int lastSend = 0;
	if (!bForce)
	{
		unsigned int now = IKUtil::GetCurrentTimeMS();
		if (now<lastSend+DATAI(TEXT("report_diagnostics_realtime_period"),100))
		{
			//  re-post
			PostReportDataToControlPanel();
			return;
		}
		lastSend = now;
	}

	//  make the data block
	BYTE data[2000];
	int ndata = 0;
	ndata = 0;
	IKEngine::GetEngine()->MakeIntellikeysArray(data,&ndata);


	//  send it
	//  small TIMEOUT!!!!
	int result = IKMessage::SendTO(TEXT("control panel"),kQueryUSBIntelliKeysArray,data,ndata,0,0,
		DATAI(TEXT("report_diagnostics_realtime_timeout"),50));
	if (result==kResponseTimeout)
	{
		//  re-post
		PostReportDataToControlPanel();
	}

}

void IKDevice::DoSetLED(int number, int on)
{
	bool b = !!on;
	if (b != m_lights[number-1])
	{
		BYTE buffer[8] = {IK_CMD_LED,number,on,0,0,0,0,0};
		SendCommand(buffer);
		m_lights[number-1] = b;
	}

}


int IKDevice::GetRepeatDelayMS()
{
#ifdef PLAT_WINDOWS

	//  Get the system keyboard repeat speed
	//	It can be any number between 0 and 31,
	//	0 = 2.5 repetitions per second, 31 = 31 repetitions per second.

	DWORD systemSpeed;
	DWORD result = SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &systemSpeed, 0 );

	//  convert to MS.  This is the minimum repeat delay
	int minRepeatDelay = 32 + (31-systemSpeed)*(400-32)/31;

#endif

#ifdef PLAT_MACINTOSH

	//  todo:  use system values

	//  This is the minimum repeat delay
	int minRepeatDelay = 50;


#endif

	//  get the maximum repeat delay
	int maxRepeatDelay = DATAI(TEXT("max_repeat_delay"),750);

	//  adjust the max to be in line with the minimum.
	if (maxRepeatDelay < 2*minRepeatDelay)
		maxRepeatDelay = 2*minRepeatDelay;

	//  compute the current repeat delay
	int currentRepeatDelay = minRepeatDelay + 
		(15-IKSettings::GetSettings()->m_iRepeatRate)*(maxRepeatDelay-minRepeatDelay)/14;

	return currentRepeatDelay;
}

typedef struct
{
	const TCHAR *name;
	BYTE code;
} keyname;

static const keyname keynames[] = 
{
{"UNIVERSAL_F1", 0x01 },
{"UNIVERSAL_F2", 0x02 },
{"UNIVERSAL_F3", 0x03 },
{"UNIVERSAL_F4", 0x04 },
{"UNIVERSAL_F5", 0x05 },
{"UNIVERSAL_F6", 0x06 },
{"UNIVERSAL_F7", 0x07 },
{"UNIVERSAL_F8", 0x08 },
{"UNIVERSAL_F9", 0x09 },
{"UNIVERSAL_F10", 0x0a },
{"UNIVERSAL_F11", 0x0b },
{"UNIVERSAL_F12", 0x0c },
{"UNIVERSAL_F13", 0x0d },
{"UNIVERSAL_PRINT_SCREEN", 0x0d },
{"UNIVERSAL_F14", 0x0e},
{"UNIVERSAL_SCROLL_LOCK", 0x0e},
{"UNIVERSAL_F15", 0x0f },
{"UNIVERSAL_PAUSE", 0x0f },
{"UNIVERSAL_SPACE", 0x20 },
{"UNIVERSAL_NUMPAD_0", 0x10 },
{"UNIVERSAL_NUMPAD_1", 0x11 },
{"UNIVERSAL_NUMPAD_2", 0x12 },
{"UNIVERSAL_NUMPAD_3", 0x13 },
{"UNIVERSAL_NUMPAD_4", 0x14 },
{"UNIVERSAL_NUMPAD_5", 0x15 },
{"UNIVERSAL_NUMPAD_6", 0x16 },
{"UNIVERSAL_NUMPAD_7", 0x17 },
{"UNIVERSAL_NUMPAD_8", 0x18 },
{"UNIVERSAL_NUMPAD_9", 0x19 },
{"UNIVERSAL_NUMPAD_ADD", 0x1a },
{"UNIVERSAL_NUMPAD_SUBTRACT", 0x1b },
{"UNIVERSAL_NUMPAD_MULTIPLY", 0x1c },
{"UNIVERSAL_NUMPAD_DIVIDE", 0x1d },
{"UNIVERSAL_NUMPAD_EQUAL", 0x1e},
{"UNIVERSAL_NUMPAD_ENTER", 0x1f },
{"UNIVERSAL_NUMPAD_DECIMAL", 0x21 },
{"UNIVERSAL_INSERT", 0x22 },
{"UNIVERSAL_DELETE", 0x23 },
{"UNIVERSAL_UP_ARROW", 0x24 },
{"UNIVERSAL_DOWN_ARROW", 0x25 },
{"UNIVERSAL_LEFT_ARROW", 0x26 },
{"UNIVERSAL_RIGHT_ARROW", 0x27 },
{"UNIVERSAL_HOME", 0x28 },
{"UNIVERSAL_END", 0x29 },
{"UNIVERSAL_PAGE_UP", 0x2a },
{"UNIVERSAL_PAGE_DOWN", 0x2b },
{"UNIVERSAL_COMMA", 0x2c },
{"UNIVERSAL_MINUS", 0x2d },
{"UNIVERSAL_PERIOD", 0x2e },
{"UNIVERSAL_SLASH", 0x2f },
{"UNIVERSAL_0", 0x30 },
{"UNIVERSAL_1", 0x31 },
{"UNIVERSAL_2", 0x32 },
{"UNIVERSAL_3", 0x33 },
{"UNIVERSAL_4", 0x34 },
{"UNIVERSAL_5", 0x35 },
{"UNIVERSAL_6", 0x36 },
{"UNIVERSAL_7", 0x37 },
{"UNIVERSAL_8", 0x38 },
{"UNIVERSAL_9", 0x39 },
{"UNIVERSAL_SEMICOLON", 0x3b},
{"UNIVERSAL_EQUALS", 0x3d },
{"UNIVERSAL_TILDE", 0x3e },
{"UNIVERSAL_QUOTE", 0x3f },
{"UNIVERSAL_A", 0x41 },
{"UNIVERSAL_B", 0x42 },
{"UNIVERSAL_C", 0x43 },
{"UNIVERSAL_D", 0x44 },
{"UNIVERSAL_E", 0x45 },
{"UNIVERSAL_F", 0x46 },
{"UNIVERSAL_G", 0x47 },
{"UNIVERSAL_H", 0x48 },
{"UNIVERSAL_I", 0x49 },
{"UNIVERSAL_J", 0x4a },
{"UNIVERSAL_K", 0x4b },
{"UNIVERSAL_L", 0x4c },
{"UNIVERSAL_M", 0x4d },
{"UNIVERSAL_N", 0x4e },
{"UNIVERSAL_O", 0x4f },
{"UNIVERSAL_P", 0x50 },
{"UNIVERSAL_Q", 0x51 },
{"UNIVERSAL_R", 0x52 },
{"UNIVERSAL_S", 0x53 },
{"UNIVERSAL_T", 0x54 },
{"UNIVERSAL_U", 0x55 },
{"UNIVERSAL_V", 0x56 },
{"UNIVERSAL_W", 0x57 },
{"UNIVERSAL_X", 0x58 },
{"UNIVERSAL_Y", 0x59 },
{"UNIVERSAL_Z", 0x5a },
{"UNIVERSAL_LEFT_BRACKET", 0x5b },
{"UNIVERSAL_BACKSLASH", 0x5c },
{"UNIVERSAL_RIGHT_BRACKET", 0x5d },
{"UNIVERSAL_ENTER", 0x60 },
{"UNIVERSAL_RETURN", 0x60},
{"UNIVERSAL_ESCAPE", 0x61 },
{"UNIVERSAL_TAB", 0x62 },
{"UNIVERSAL_BACKSPACE", 0x63 },
{"UNIVERSAL_CAPS_LOCK", 0x64 },
{"UNIVERSAL_NUM_LOCK", 0x65 },
{"UNIVERSAL_UNICODE", 0x67 },
{"UNIVERSAL_SHIFT", 0x70 },
{"UNIVERSAL_RIGHT_SHIFT", 0x71 },
{"UNIVERSAL_CONTROL", 0x72 },
{"UNIVERSAL_RIGHT_CONTROL", 0x73 },
{"UNIVERSAL_MENU", 0x74},
{"UNIVERSAL_ALT", 0x74},
{"UNIVERSAL_OPTION", 0x74},
{"UNIVERSAL_ALTGR", 0x75},
{"UNIVERSAL_RIGHT_OPTION", 0x75},
{"UNIVERSAL_COMMAND", 0x76},
{"UNIVERSAL_MOUSE_UP", 0x80 },
{"UNIVERSAL_MOUSE_UP_RIGHT", 0x81 },
{"UNIVERSAL_MOUSE_RIGHT", 0x82 },
{"UNIVERSAL_MOUSE_DOWN_RIGHT", 0x83 },
{"UNIVERSAL_MOUSE_DOWN", 0x84 },
{"UNIVERSAL_MOUSE_DOWN_LEFT", 0x85 },
{"UNIVERSAL_MOUSE_LEFT", 0x86 },
{"UNIVERSAL_MOUSE_UP_LEFT", 0x87 },
{"UNIVERSAL_MOUSE_BUTTON_CLICK", 0x88 },
{"UNIVERSAL_MOUSE_BUTTON_DOUBLECLICK", 0x89 },
{"UNIVERSAL_MOUSE_BUTTON_DOWN", 0x8a },
{"UNIVERSAL_MOUSE_BUTTON_UP", 0x8b },
{"UNIVERSAL_MOUSE_ON_CODE", 0x8c },
{"UNIVERSAL_MOUSE_OFF_CODE", 0x8d },
{"UNIVERSAL_MOUSE_BUTTON_TOGGLE", 0x8e },
{"UNIVERSAL_MOUSE_RBUTTON_CLICK", 0x8f },
{"UNIVERSAL_MOUSE_RBUTTON_DOUBLECLICK", 0x90 },
{"UNIVERSAL_MOUSE_RBUTTON_DOWN", 0x91 },
{"UNIVERSAL_MOUSE_RBUTTON_UP", 0x92 },
{"UNIVERSAL_MOUSE_RBUTTON_TOGGLE", 0x93},
{"UNIVERSAL_GOTO_LEVEL", 0xc0 },
{"UNIVERSAL_GOTO_LEVEL_1", 0xc1 },
{"UNIVERSAL_GOTO_LEVEL_2", 0xc2 },
{"UNIVERSAL_GOTO_LEVEL_3", 0xc3 },
{"UNIVERSAL_GOTO_LEVEL_4", 0xc4 },
{"UNIVERSAL_GOTO_LEVEL_5", 0xc5 },
{"UNIVERSAL_GOTO_LEVEL_6", 0xc6 },
{"UNIVERSAL_GOTO_LEVEL_7", 0xc7 },
{"UNIVERSAL_GOTO_LEVEL_8", 0xc8 },
{"UNIVERSAL_GOTO_LEVEL_9", 0xc9 },
{"UNIVERSAL_GOTO_LEVEL_10", 0xca },
{"UNIVERSAL_GOTO_LEVEL_11", 0xcb },
{"UNIVERSAL_GOTO_LEVEL_12", 0xcc },
{"UNIVERSAL_GOTO_LEVEL_13", 0xcd },
{"UNIVERSAL_GOTO_LEVEL_14", 0xce },
{"UNIVERSAL_GOTO_LEVEL_15", 0xcf },
{"UNIVERSAL_NON_REPEATING", 0xd0 },
{"UNIVERSAL_SEPARATE_REPEAT_KEY", 0xd1 },
{"UNIVERSAL_NON_SMART_TYPING", 0xd2 },
{"UNIVERSAL_SETUP1", 0xe0 },
{"UNIVERSAL_SETUP2", 0xe1 },
{"UNIVERSAL_SETUP1_RESPONSE_RATE", 0x41 },
{"UNIVERSAL_SETUP1_LIFT_OFF_ON", 0x42},
{"UNIVERSAL_SETUP1_LIFT_OFF_OFF", 0x43},
{"UNIVERSAL_SETUP1_REPEAT_LATCHING_ON", 0x4A},
{"UNIVERSAL_SETUP1_REPEAT_LATCHING_OFF", 0x4B},
{"UNIVERSAL_SETUP1_SHIFT_NO_LATCHING", 0x4E},
{"UNIVERSAL_SETUP1_3LIGHTS", 0x51},
{"UNIVERSAL_SETUP1_6LIGHTS", 0x52},
{"UNIVERSAL_SETUP1_MOUSE_SPEED", 0x55 },
{"UNIVERSAL_SETUP1_CUSTOM_OVERLAY_LEVEL", 0x58 },
{"UNIVERSAL_SETUP1_DATA_SEND_RATE", 0x32},
{"UNIVERSAL_SETUP1_REPEAT_RATE", 0x49 },
{"UNIVERSAL_SETUP1_KEYSOUND_ON", 0x45},
{"UNIVERSAL_SETUP1_KEYSOUND_OFF", 0x44},
{"UNIVERSAL_SETUP1_SHIFT_LATCHING", 0x4C},
{"UNIVERSAL_SETUP1_LIST_FEATURES", 0x33},
{"UNIVERSAL_SETUP1_SHIFT_LOCKING", 0x4D},
{"UNIVERSAL_SETUP1_MOUSE_ARROWS_ON", 0x56},
{"UNIVERSAL_SETUP1_MOUSE_ARROWS_OFF", 0x57},
{"UNIVERSAL_SETUP1_SMART_TYPING_ON", 0x53},
{"UNIVERSAL_SETUP1_SMART_TYPING_OFF", 0x54},
{"UNIVERSAL_SETUP1_REPEAT_ON", 0x47},
{"UNIVERSAL_SETUP1_REPEAT_OFF", 0x48},
{"UNIVERSAL_SETUP1_LIST_FEATURES", 0x33},
{"UNIVERSAL_SETUP2_FEATURE_RESET", 0x45},

};



IKString IKDevice::GetKeyName(int code)
{
	for (int i=0;i<sizeof(keynames)/sizeof(keyname);i++)
	{
		if (keynames[i].code==code)
		{
			return IKString(keynames[i].name);
		}
	}

	return IKString(TEXT("unknown"));
}

void IKDevice::PostReportDataToControlPanel(bool bForce /* = false */)
{
	//return;

	PostDelay(5);
	BYTE command[IK_REPORT_LEN] = {IK_CMD_CP_REPORT_REALTIME,bForce,0,0,0,0,0,0};
	PostCommand(command);
}

void IKDevice::Disconnect()
{
	//KeySound(500);

	BYTE command[IK_REPORT_LEN] = {IK_CMD_STOP_OUTPUT,0,0,0,0,0,0,0};
	SendCommand(command);

	//IKUtil::Sleep(100);

	//BYTE command2[IK_REPORT_LEN] = {IK_CMD_DISCON,0,0,0,0,0,0,0};
	//SendCommand(command2);
}

void IKDevice::RawNotify ( int eventType, int i1, int i2, int time )
{
#ifdef PLAT_MACINTOSH

	TCHAR buffer[1000];
	
	sprintf ( buffer, "%d,%d,%d,%d", eventType, i1, i2, time );
	int result = IKMessage::Send ( TEXT("menu"), kQueryRawNotify, (void *) buffer, strlen(buffer), 0, 0 );	

#else
	//  nothing (yet) for Windows
#endif
}

void IKDevice::InterpretRaw()
{
	//  don't bother if we're not connected and switched on
	if (!IsOpen())
		return;
	if (!IsSwitchedOn())
		return;

	int col,row,nsw;

	if (!m_bLastInit)
	{
		m_bLastInit = true;
		for (col=0;col<IK_RESOLUTION_X;col++)
			for (row=0;row<IK_RESOLUTION_Y;row++)
				m_last_membrane[row][col] = 0;
		for (nsw=0;nsw<IK_NUM_SWITCHES;nsw++)
			m_last_switches[nsw] = 0;

	}

	unsigned int time = IKUtil::GetCurrentTimeMS();

	bool bChange = false;

	//  look for membrane releases
	for (col=0;col<IK_RESOLUTION_X;col++)
		for (row=0;row<IK_RESOLUTION_Y;row++)
			if (m_membrane[row][col]==0 && m_last_membrane[row][col]!=0)
			{
				if ( IKEngine::GetEngine()->GetNotifyMode() )
				{
					RawNotify ( 2, row, col, time );
				}
				else
				{
					//  release row and col
					queueEntry qe;
					qe.buffer[0] = 1;	 //  event type
					qe.buffer[1] = 0;    //  release
					qe.buffer[2] = row;  //  row
					qe.buffer[3] = col;  //  col
					*((unsigned int *)&(qe.buffer[4])) = time;
					m_rawQueue.enqueue(qe);
					bChange = true;
				}
			}

	//  look for switch releases
	for (nsw=0;nsw<IK_NUM_SWITCHES;nsw++)
		if (m_switches[nsw]==0 && m_last_switches[nsw]!=0)
		{
			//  release switch
			if ( IKEngine::GetEngine()->GetNotifyMode() )
			{
				RawNotify ( 4, nsw, 0, time );
			}
			else
			{
				queueEntry qe;
				qe.buffer[0] = 2;		//  event type
				qe.buffer[1] = 0;		//  release
				qe.buffer[2] = nsw;		//  switch number
				qe.buffer[3] = 0;		//  unused
				*((unsigned int *)&(qe.buffer[4])) = time;			
				m_rawQueue.enqueue(qe);
				bChange = true;
			}
		}

	//  look for membrane presses
	for (col=0;col<IK_RESOLUTION_X;col++)
		for (row=0;row<IK_RESOLUTION_Y;row++)
			if (m_membrane[row][col]!=0 && m_last_membrane[row][col]==0)
			{
				if ( IKEngine::GetEngine()->GetNotifyMode() )
				{
					RawNotify ( 1, row, col, time );
				}
				else
				{
					//  press row and col
					queueEntry qe;
					qe.buffer[0] = 1;	 //  event type
					qe.buffer[1] = 1;    //  press
					qe.buffer[2] = row;  //  row
					qe.buffer[3] = col;  //  col
					*((unsigned int *)&(qe.buffer[4])) = time;
					m_rawQueue.enqueue(qe);
					bChange = true;
				}
			}

	//  look for switch presses
	for (nsw=0;nsw<IK_NUM_SWITCHES;nsw++)
		if (m_switches[nsw]!=0 && m_last_switches[nsw]==0)
		{
			//  press switch
			if ( IKEngine::GetEngine()->GetNotifyMode() )
			{
				RawNotify ( 3, nsw, 0, time );
			}
			else
			{
				queueEntry qe;
				qe.buffer[0] = 2;		//  event type
				qe.buffer[1] = 1;		//  press
				qe.buffer[2] = nsw;		//  switch number
				qe.buffer[3] = 0;		//  unused
				*((unsigned int *)&(qe.buffer[4])) = time;
				m_rawQueue.enqueue(qe);
				bChange = true;
			}
		}

	//  save current state for next time

	for (col=0;col<IK_RESOLUTION_X;col++)
		for (row=0;row<IK_RESOLUTION_Y;row++)
			m_last_membrane[row][col] = m_membrane[row][col];

	for (nsw=0;nsw<IK_NUM_SWITCHES;nsw++)
		m_last_switches[nsw] = m_switches[nsw];

	// kill baloon if need be
	if (bChange)
		AppLibKillFloatingMessage();

}

void IKDevice::PurgeRawQueue()
{
	m_rawQueue.makeEmpty();
}

bool IKDevice::GetRawEvent(BYTE *bytes)
{
	if (m_rawQueue.isEmpty())
		return false;

	queueEntry qe;
	m_rawQueue.dequeue(qe);

	for (int i=0;i<8;i++)
		bytes[i] = qe.buffer[i];
	return true;

}


int IKDevice::CountRawEvents()
{
	return m_rawQueue.GetSize();
}

void IKDevice::KeySound2(int msLength, int freq)
{
	int myVol = IKSettings::GetSettings()->m_iKeySoundVolume;

	//  set parameters and blow
	BYTE report[IK_REPORT_LEN] = {IK_CMD_TONE,0,0,0,0,0,0,0};
	report[1] = freq;
	report[2] = myVol;
	report[3] = msLength/10;
	SendCommand(report);
}
