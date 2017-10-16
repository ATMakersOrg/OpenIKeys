// ------------------------------------------------------------------ FILE: Ik.C
//      Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//     FILE:	Ik.C
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00		Created.
//
// JWH - processing for two additional commands added
// -----------------------------------------------------------------------------
//
#include <ezusb.h>
#include <ezregs.h>

#include "Ik.h"

BOOL bEP1Complete;

// -----------------------------------------------------------------------------
// GLOBALS															     GLOBALS
// -----------------------------------------------------------------------------
//
BYTE	volatile IkCmdPending	= FALSE;
DWORD	volatile IkTimerTick;
DWORD	IkLastMatrixScanTick;
DWORD	IkLastSwitchScanTick;
DWORD	IkLastSensorScanTick;

BOOL	IkSwitch1State;
BOOL	IkSwitch2State;

BYTE	volatile IkShadowSensorSel;
BYTE	volatile IkShadowLedSel;

BOOL	IkScanEnabledFlag;

BOOL	IkDeviceSwitchedOn;

WORD	xdata IkPressBufStore[ IK_MAX_PRESS_BUF_LEN ];
BYTE	IkPressBufStoreIdx;
WORD	xdata IkPressBufNew[ IK_MAX_PRESS_BUF_LEN ];
BYTE	IkPressBufNewIdx;
WORD	xdata IkPressBufTemp[ IK_MAX_PRESS_BUF_LEN ];

BYTE	IkSensorState[ IK_MAX_SENSORS ];
BYTE	IkSensorEventDelta = IK_DEFAULT_SENSOR_EVENT_DELTA;

IK_EVENT xdata IkEvents[ IK_MAX_EVENTS ];
WORD	volatile IkEventHead;
WORD	volatile IkEventTail;

BYTE	IkEventMode = IK_EVENT_MODE_POLLED;
BOOL	IkEventPolledFlag = FALSE;
DWORD	IkToneDurationTicks;

BOOL bDoAnyOutput = TRUE;

int	numEventsQueued;

#ifdef AUTOPILOT
//  fwr begin autopilot
BYTE numPress = 0;
BYTE numPressLast = 0;
DWORD timePressLast = 0L;
BYTE numFastPress = 0;
BYTE autoPilotState = 0;
BYTE autopilotSupported = 0;
BYTE autopilotPhase = 0;
DWORD autopilotTimeDown = 0;
DWORD autopilotTimeUp = 0;
DWORD autoPilotTimer = 0L;
DWORD autopilotWaitTime = 0L;

void AutoPilotRelease ()
{
	IkEventLog( IK_EVENT_MEMBRANE_RELEASE, 20,4);
	IkEventLog( IK_EVENT_MEMBRANE_RELEASE, 20,10);
	IkEventLog( IK_EVENT_MEMBRANE_RELEASE, 20,16);
	IkEventLog( IK_EVENT_MEMBRANE_RELEASE, 20,22);
	IkCmdLed(1,0);
	IkCmdLed(2,0);
	IkCmdLed(4,0);
	IkCmdLed(5,0);
}

void AutoPilotPress ( BYTE col, BYTE row)
{
	IkEventLog( IK_EVENT_MEMBRANE_PRESS, col, row);
	//IkPressBufNew[ IkPressBufNewIdx++ ] = ((WORD)col << 8 ) + row;
}

void DoAutoPilot ()
{

	if (autopilotSupported==1 && autoPilotState==1)
	{
		if (IkTimerTick-autoPilotTimer>autopilotWaitTime/10)
		{
			AutoPilotRelease ();
			switch(autopilotPhase)
			{
			case 1:
				AutoPilotPress(20,4);
				IkCmdLed(1,1);
				autopilotWaitTime = autopilotTimeDown;
				break;
			case 3:
				AutoPilotPress(20,10);
				IkCmdLed(2,1);
				autopilotWaitTime = autopilotTimeDown;
				break;
			case 5:
				AutoPilotPress(20,16);
				IkCmdLed(4,1);
				autopilotWaitTime = autopilotTimeDown;
				break;
			case 7:
				AutoPilotPress(20,22);
				IkCmdLed(5,1);
				autopilotWaitTime = autopilotTimeDown;
				break;
			default:
				autopilotWaitTime = autopilotTimeUp;
				break;
			}

			autopilotPhase++;
			if (autopilotPhase>=8)
				autopilotPhase = 0;
			autoPilotTimer = IkTimerTick;
		}
	}
}

void ToggleAutoPilot ( )
{
	autoPilotState = 1 - autoPilotState;
	autoPilotTimer = IkTimerTick;
	autopilotPhase = 0;
	autopilotWaitTime = 0;

	if (autopilotSupported==1)
	{
		AutoPilotRelease ();
		IkEventLog( IK_EVENT_AUTOPILOT_STATE, autoPilotState, 0);
	}
}
//  fwr end autopilot
#endif


// -------------------------------------------------------------------- IkInit()
// FUNCTION:	IkInit()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkInit( void )
{
	// Setting ISODISAB to 1 disables isochronous endpoints and frees up the
	// RAM from 0x2000 - 0x27FF for micro use. The compiler is configured to
	// start its xdata memory space at 0x2000.
	//
	ISOCTL |= 0x01;		// ISODISAB = 1;
	
	// Configure Port pins.
	//
	PORTACFG = 0x00;	// Port A: All pins I/O.
	PORTBCFG = 0x00;	// Port B: All pins I/O.
	PORTCCFG = 0x02;	// Port C: C1 = TXDO, All others I/O.
   	 					
	OEA	= 0x10;	// Port A: A5 = INPUT, A4 = OUTPUT.
	OEB = 0x00; // Port B: Used as pseudo data bus. Default state is INPUT.
	OEC = 0xF3; // Port C: C3, C2 = INPUT, all others OUTPUT.
	
	// Set initial state of output pins.
	//
	OUTA = 0x10;	// Port A: ADC-CS* = 1.
	OUTC = 0xE1;	// Port C: C7-C5 = 1 - No valid address decode.
					//		   C4 = 0 - 5V not enabled.
					//		   C0 = LEDSEL = 1 (not selected).

#if JWH
// Data endpoints should not be enabled until the device is configured
#else
	// Enable endpoints.
	//
// DBC: HID code start
	IN07VAL |= bmEP1 ;
	OUT07VAL |= bmEP2;
// DBC: HID code end

	IN1CS |= bmEPBUSY;	// Setting EPBUSY disarms the IN endpoints.
	OUT2BC = 0;			// Arm the OUT endpoints so they can receive

	// Enable endpoint interrupts.
	//
// DBC: HID code start
// Enable interrrupts for Endpoint 2 OUT
	OUT07IEN |= bmEP2;
// Enable interrupts for Endpoint 1 IN
	IN07IEN |= bmEP1;
// FWR  advertise that the endpoint is not busy
	bEP1Complete = 0;
// DBC: HID code end
#endif

	// Set Timer1 for Mode 2 (8-bit counter with auto-reload). This timer
	// is used as a baud rate generator for Serial Port 0. This serial port
	// is used to generate a pulse train to the buzzer.
	//
	TMOD = 0x20;
	TH1 = 204;		// Set default to 2400Hz.
	TR1 = 1;		// Enable counting.
	
	// Configure Serial Port 0 for Mode 1.
	//
	SCON0 = 0x40;
	ES0 = 0;		// Disable interrupt. Interrupt will be enabled when a
					// tone is to be generated.
	
	// Set Timer2 to generate 10mSec interrupts.
	//
	CKCON &= 0xDF;	// Set T2M = 0 to use CLK24/12 as timer clock source.
					// Hence, clock source = 2MHz.
    RCAP2H = 0xB1;	// Set autoreload value. Timer trips its interrupt when
    RCAP2L = 0xDF;	// it reaches 0xFFFF. Using CLK24/12 , timer increments
					// at 2MHz or .0000005 seconds. 10ms / .0000005 = 20,000
					// counts. 0xFFFF - 0x4E20 (20,000) = 0xB1DF, the reload
					// value.
	T2CON = 0x04;	// Start timer.
    ET2 = 1;		// Enable Timer 2 interrupt.
	
	// Initialize I2C Bus.
	//
	EZUSB_InitI2C();

	// Turn on 5V. Delay to stabilize.
	//
	OUTC |= 0x10;
	EZUSB_Delay1ms();

	IkFunctionInit();
}

// ------------------------------------------------------------ IkFunctionInit()
// FUNCTION:	IkFunctionInit()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkFunctionInit( void )
{
	int i;

	IkScanEnabledFlag = FALSE;
	IkDeviceSwitchedOn = TRUE;
	
	ES0 = 0;	// Shut off tone by disabling serial port interrupt.

	// Initialize latches.
	//
// BUZZER MOD BEGIN
//	IkShadowSensorSel = 0x8F;	// D4 off, buzzer muted, no sensor selected.
	IkShadowSensorSel = 0xCF;	// D4 off, buzzer disabled, no sensor selected.
// BUZZER MOD END
	IK_SENSOR_SEL( IkShadowSensorSel );

	IkShadowLedSel = 0x03; 		// D1, D2, D3, D5, D6 = off.
	IK_LED_SEL( IkShadowLedSel );

	IK_ROW_SEL_0( 0xFF );
	IK_ROW_SEL_1( 0xFF );
	IK_ROW_SEL_2( 0xFF );

	// Flush event buffer.
	//
	IkEventHead = 0;
	IkEventTail = 0;

	IkTimerTick = 0L;

#ifdef SCRIBBLER
	//  record our VID/PID in this device.
	EepromWriteByte( 0, 0xB0 );
	EepromWriteByte( 1, 0x5E );
	EepromWriteByte( 2, 0x09 );
	EepromWriteByte( 3, 0x00 );
	EepromWriteByte( 4, 0x01 );
	EepromWriteByte( 5, 0x00 );
	EepromWriteByte( 6, 0x00 );	
#endif

	IkSensorScan ( TRUE );

	// send an initial keyboard report
	for (i=0; i<8; i++) IN3BUF[i] = 0;
	IN3BC = 8;

}

// -------------------------------------------------------------------- IkPoll()
// FUNCTION:	IkPoll()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//

#ifdef SEQUENCE
static WORD nsend = 0;
#endif

#ifndef NO_EARLY_CHATTER
void ArmOnce();
static BOOL keepArming = TRUE;
#endif

void IkPoll( void )
{
	BOOL sendEvent;

	BOOL bTimeForMatrix;
	BOOL bTimeForSwitches;
	BOOL bTimeForSensors;

#ifndef NO_EARLY_CHATTER
	//
	//  fwr 10-29-02
	//  doing this until we receive our first command
	//  mysteriously solves a problem on some legacy USB systems.
	//  worth another look I suppose.
	//
	if (keepArming)
		ArmOnce();
#endif

	if( IkCmdPending )
	{
#ifndef NO_EARLY_CHATTER
		keepArming = FALSE;
#endif
		IkCmdProcess();
	}

	//  fwr 1/10/01 figure out if it's time to do the three kinds of scanning.
	bTimeForMatrix		= FALSE;
	bTimeForSwitches	= FALSE;
	bTimeForSensors		= FALSE;
	if (( IkTimerTick - IkLastMatrixScanTick ) >= 1 )  	//  10 msec
		bTimeForMatrix = TRUE;
	if (IkTimerTick != IkLastSwitchScanTick)			//  10 msec
		bTimeForSwitches = TRUE;
	if (( IkTimerTick - IkLastSensorScanTick ) >= 10 )	//  100 msec
		bTimeForSensors = TRUE;

	// IkTimerTick is incremented every 10ms.
	//

	// Scan membrane
	//  fwr 1/8/01 do this even if scanning disabled.
	//             just don't create the events later if disabled or if device is off.
	if(bTimeForMatrix)
	{
		numEventsQueued = 0;
		IkLastMatrixScanTick = IkTimerTick;
		IkMatrixScan();
	}

	//  fwr 1/8/01 scan switches only if device is enabled and on
	if( IkScanEnabledFlag && IkDeviceSwitchedOn && bTimeForSwitches )
	{
		IkLastSwitchScanTick = IkTimerTick;
		IkSwitchScan();
	}

	// Scan sensor every 100ms.
	//
	//  fwr 1/8/01 scan sensors even if device is "off"
	if( IkScanEnabledFlag && bTimeForSensors )
	{
		IkLastSensorScanTick = IkTimerTick;
		IkSensorScan(FALSE);
	}

#ifdef SEND_NO_MORE_EVENTS
	//  fwr 1/10/00  scanning is complete for this cycle.
	//  Signal no more events so software knows that this
	//  cycle is done.

	//  fwr 10-29-02
	//  this is not strictly needed for the firmware to work,
	//  and is not needed for the firmware test program.
	//  
	if(bTimeForMatrix)
		IkEventLog(IK_EVENT_NOMOREEVENTS,0,0);
#endif
	
	// If a tone is being generated, then ES0 will = 1. Check to see if
	// the set duration has been exceeded. If so, shut off the tone.
	//
	if( ES0 == 1 )
	{
		if( IkTimerTick >= IkToneDurationTicks )
		{
			ES0 = 0;	// Disable serial port interrupt.

// BUZZER MOD BEGIN
//			IkShadowSensorSel &= 0x8F;	// Shut off analog switch to buzzer.
			IkShadowSensorSel |= 0x40;	// Shut off analog switch to buzzer.
// BUZZER MOD END

			IK_SENSOR_SEL( IkShadowSensorSel );
		}
	}

	// Only load event into buffer if IN1BSY = 0 (not busy).
	//
	//if(( IN1CS & 0xFD ) == 0 )
	//  instead check the flag set by the interrupt routine
	if(bEP1Complete)
	{
		sendEvent = FALSE;

		if( IkEventPolledFlag )
		{
			IkEventPolledFlag = FALSE;

			if( IkEventTail != IkEventHead )
				sendEvent = TRUE;
			else
			{
				IN1BUF[ 0 ] = IK_EVENT_ACK;

#ifdef SEQUENCE
				nsend++;
				IN1BUF[5] = nsend/256;
				IN1BUF[6] = nsend - (nsend/256)*256;
#endif

				bEP1Complete = 0;  // FWR
				IN1BC = 8; // Arm endpoint by writing byte count.
			}
		}
		else if( IkEventMode == IK_EVENT_MODE_AUTO )
		{
			if( IkEventTail != IkEventHead )
				sendEvent = TRUE;
		}

		if( sendEvent)
		{
			*(IK_EVENT *)IN1BUF = IkEvents[ IkEventTail ];

			if( ++IkEventTail == IK_MAX_EVENTS )
				IkEventTail = 0;

#ifdef SEQUENCE
			nsend++;
			IN1BUF[5] = nsend/256;
			IN1BUF[6] = nsend - (nsend/256)*256;
#endif

			bEP1Complete = 0;  //  FWR set not complete
			IN1BC = 8;	// Arm endpoint by writing byte count.
		}
	}
}


void ReportOnOffSwitchStatus()
{
	if (IkDeviceSwitchedOn)
		IkEventLog ( IK_EVENT_ONOFFSWITCH, 1, 0);
	else
		IkEventLog ( IK_EVENT_ONOFFSWITCH, 0, 0);
}


void Correct()
{
	BYTE i, row, col;

#ifdef AUTOPILOT
//  fwr begin autopilot
	//  do nothing if we're in autopilot
	if (autopilotSupported==1 && autoPilotState==1)
		return;
//  fwr end autopilot
#endif

	//  do nothing if device switched off
	if (!IkDeviceSwitchedOn)
		return;

	//  do nothing if there are queue entries so data is current when the host
	//  gets it
	if (IkEventTail!=IkEventHead)
		return;

	//  queue event for each membrane presses
	for (i=0;i<IkPressBufNewIdx;i++)
	{
		col = IkPressBufNew[i] >> 8;
		row = IkPressBufNew[i] & 0xFF;
		IkEventLog ( IK_EVENT_CORRECT_MEMBRANE, col, row);
	}

	//  queue event for each switch press
	if( !(PINSC & 0x08) )
		IkEventLog( IK_EVENT_CORRECT_SWITCH, 1, TRUE );
	if( !(PINSC & 0x04) )
		IkEventLog( IK_EVENT_CORRECT_SWITCH, 2, TRUE );

	//  signal done
	IkEventLog( IK_EVENT_CORRECT_DONE, 2, TRUE );
}


// -------------------------------------------------------------- IkCmdProcess()
// FUNCTION:	IkCmdProcess()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkCmdProcess( void )
{
	int i;

	switch( IK_OUT_REPORT[ 0 ])
	{

	case IK_CMD_ALL_SENSORS:
		IkSensorScan ( TRUE );
		break;

	case IK_CMD_START_OUTPUT:
		bDoAnyOutput = TRUE;
		break;

	case IK_CMD_STOP_OUTPUT:
		// Flush event buffer.
		//
		IkEventHead = 0;
		IkEventTail = 0;
		bDoAnyOutput = FALSE;
		break;

	case IK_CMD_GET_EVENT:
		IkEventPolledFlag = TRUE;
		break;

	case IK_CMD_ONOFFSWITCH:
		ReportOnOffSwitchStatus();
		break;

	case IK_CMD_TONE:
		IkCmdTone();
		break;

	case IK_CMD_LED:
		IkCmdLed( IK_OUT_REPORT[ 1 ], IK_OUT_REPORT[ 2 ]);
		break;

	case IK_CMD_ALL_LEDS:
		for (i=0;i<9;i++)
			IkCmdLed ( i+1, IK_OUT_REPORT[ 1 ] );
		break;
	
	case IK_CMD_SCAN:
		if( IK_OUT_REPORT[ 1 ])
			IkCmdScanStart();
		else
			IkScanEnabledFlag = FALSE;
		break;
	
	case IK_CMD_INIT:
		bDoAnyOutput = TRUE;
		IkFunctionInit();
		IkEventMode = IK_OUT_REPORT[ 1 ];
		break;
	
	case IK_CMD_GET_VERSION:
		IkEventLog( IK_EVENT_VERSION, IK_VERSION_MAJOR, IK_VERSION_MINOR );
		break;

	case IK_CMD_EEPROM_READ:
		IkCmdEepromRead();
		break;

	case IK_CMD_EEPROM_READBYTE:
		IkCmdEepromReadByte();
		break;

	case IK_CMD_EEPROM_WRITE:
		IkCmdEepromWrite();
		break;

	case IK_CMD_CORRECT:
		Correct();
		break;

	case IK_CMD_RESET_DEVICE:
		// Flush event buffer.
		//
		IkEventHead = 0;
		IkEventTail = 0;
		bDoAnyOutput = FALSE;
		EZUSB_Discon(TRUE);
		break;

#ifdef AUTOPILOT
//  fwr begin autopilot
	case IK_CMD_START_AUTO:
		autopilotSupported = 1;
		autopilotTimeUp   = IK_OUT_REPORT[ 1 ]*256 + IK_OUT_REPORT[ 2 ];
		autopilotTimeDown = IK_OUT_REPORT[ 3 ]*256 + IK_OUT_REPORT[ 4 ];
		break;

	case IK_CMD_STOP_AUTO:
		autopilotSupported = 0;
		break;
//  fwr end autopilot
#endif

#if JWH
	case IK_CMD_REFLECT_KEYSTROKE:
        //IkCmdLed(1,1);
// Load endpoint 3
		for (i=0; i<7; i++) IN3BUF[i] = IK_OUT_REPORT[i+1];
		IN3BUF[7] = 0;
// And enable EZ-USB to supply data on next poll
		IN3BC = 8;
		break;

	case IK_CMD_REFLECT_MOUSE_MOVE:
        //IkCmdLed(4,1);
// Load endpoint 4
		for (i=0; i<3; i++) IN4BUF[i] = IK_OUT_REPORT[i+1];
		for (i=3; i<8; i++) IN4BUF[i] = 0;
// And enable EZ-USB to supply data on next poll
		IN4BC = 3;
		break;

#endif

	}

	IkCmdPending = FALSE;

	// Arm the OUT endpoint so it can receive the next packet. (Write any value.)
	//
	OUT2BC = 0;
}

// ------------------------------------------------------------ IkCmdScanStart()
// FUNCTION:	IkCmdScanStart()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkCmdScanStart( void )
{
	BYTE i;
	
	IkPressBufStoreIdx = 0;
	IkPressBufNewIdx = 0;

	IkSwitch1State = FALSE;
	IkSwitch2State = FALSE;
		
	IkLastMatrixScanTick = 0;
	IkLastSwitchScanTick = 0;
	IkLastSensorScanTick = 0;
	
	for( i = 0; i < IK_MAX_SENSORS; i++ )
		IkSensorState[ i ] = 0;

	IkEventHead = 0;
	IkEventTail = 0;
		
	IkScanEnabledFlag = TRUE;

	ReportOnOffSwitchStatus();
}

// ------------------------------------------------------------------ IkCmdLed()
// FUNCTION:	IkCmdLed()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkCmdLed( BYTE ledNum, BYTE onOrOff )
{
	BYTE mask;
	
	if( ledNum == 1 )
	{
		if( onOrOff )
			IkShadowSensorSel &= 0x7F;
		else
			IkShadowSensorSel |= 0x80;

		IK_SENSOR_SEL( IkShadowSensorSel );
	}
	else
	{
		switch( ledNum )
		{
		case 2:
			mask = 0x40;
			break;
		case 3:
			mask = 0x80;
			break;
		case 4:
			mask = 0x01;
			break;
		case 5:
			mask = 0x10;
			break;
		case 6:
			mask = 0x20;
			break;
		case 7:
			mask = 0x02;
			break;
		case 8:
			mask = 0x04;
			break;
		case 9:
			mask = 0x08;
			break;
		}
	
		if( onOrOff )
		{
			if(( ledNum == 4 ) || ( ledNum == 7 ))
			{
				mask = ~mask;
				IkShadowLedSel &= mask;
			}
			else
				IkShadowLedSel |= mask;
		}
		else
		{
			if(( ledNum == 4 ) || ( ledNum == 7 ))
				IkShadowLedSel |= mask;
			else
			{
				mask = ~mask;
				IkShadowLedSel &= mask;
			}
		}

		IK_LED_SEL( IkShadowLedSel );
	}
}

void IkCMdToneOff()
{
	ES0 = 0;	// Disable serial port interrupt.
// BUZZER MOD BEGIN
	IkShadowSensorSel |= 0x40;	// Shut off analog switch to buzzer.
// BUZZER MOD END
	IK_SENSOR_SEL( IkShadowSensorSel );
}

// ----------------------------------------------------------------- IkCmdTone()
// FUNCTION:	IkCmdTone()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkCmdTone( void )
{
	// If volume is not 0, set tone freq, duration and enable.
	//
	if( IK_OUT_REPORT[ 2 ])
	{
		// Tone frequency as determined by:
		// TH1 = 48,000,000 / (384 * desired frequency)
		//
		TH1 = IK_OUT_REPORT[ 1 ];

// BUZZER MOD BEGIN
		// Set volume.
		//
		IkShadowSensorSel &= 0x8F;	// Enable buzzer and default to max volume.
		switch( IK_OUT_REPORT[ 2 ])
		{
		case 1:
			IkShadowSensorSel |= 0x30;
			break;
		case 2:
			IkShadowSensorSel |= 0x20;
			break;
		case 3:
			IkShadowSensorSel |= 0x10;
			break;
		}
// BUZZER MOD END
		IK_SENSOR_SEL( IkShadowSensorSel );
		
		// A Duration value of 0 indicates a tone that will sound until it
		// is commanded to shut off. Otherwise, set the tone duration in
		// 10ms increments.
		//
		if( IK_OUT_REPORT[ 3 ])
			IkToneDurationTicks = IkTimerTick + (DWORD)IK_OUT_REPORT[ 3 ];
		else
			IkToneDurationTicks = 0xFFFFFFFF;
			
		ES0 = 1;		// Enable serial port interrupt.
		SBUF0 = 0x55;	// Get tone started.
	}
	else
	{
		IkCMdToneOff();
	}
}

// ---------------------------------------------------------- IkSerialPort0Int()
// FUNCTION:	IkSerialPort0Int()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkSerialPort0Int( void ) interrupt 4
{
	SCON0 &= 0xFD;	// Clear interrupt flag.

	SBUF0 = 0x55;	// Send pulse train of alternating 1's and 0's.
}

// --------------------------------------------------------------- IkTimer2Int()
// FUNCTION:	IkTimer2Int()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkTimer2Int ( void ) interrupt 5
{
    TF2 = 0;   // Clear timer overflow flag.

    IkTimerTick++;
}

// ----------------------------------------------------------- EepromWriteByte()
// FUNCTION:	EepromWriteByte()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 5-01-00	Created.
// -----------------------------------------------------------------------------
//
void EepromWriteByte( WORD addr, BYTE value )
{
	BYTE xdata 	ee_str[3];

	ee_str[ 0 ] = MSB( addr );
	ee_str[ 1 ] = LSB( addr );
	ee_str[ 2 ] = value;

	EZUSB_WriteI2C( IK_I2C_EEPROM_ADDR, 3, ee_str );
	EZUSB_WaitForEEPROMWrite( IK_I2C_EEPROM_ADDR );
}

// --------------------------------------------------------------- EepromWrite()
// FUNCTION:	EepromWrite()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 5-01-00	Created.
// -----------------------------------------------------------------------------
//
void EepromWrite( WORD addr, BYTE length, BYTE xdata *buf )
{
	BYTE	i;
	for( i = 0; i < length; ++i )
		EepromWriteByte( addr++, buf[ i ]);
}

// ---------------------------------------------------------------- EepromRead()
// FUNCTION:	3Read()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 5-01-00	Created.
// -----------------------------------------------------------------------------
//
void EepromRead( WORD addr, BYTE length, BYTE xdata *buf )
{
	BYTE  	xdata ee_str[ 2 ];

	ee_str[ 0 ] = MSB( addr );
	ee_str[ 1 ] = LSB( addr );

	EZUSB_WriteI2C( IK_I2C_EEPROM_ADDR, 2, ee_str );
	EZUSB_ReadI2C( IK_I2C_EEPROM_ADDR, length, buf );
}

// -------------------------------------------------------------- IkMatrixScan()
// FUNCTION:	IkMatrixScan()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//

void IkMatrixScan( void )
{
	BYTE rowChip, colChip, reg;
	BYTE i, row, col, nPressesInCol;

	IkPressBufNewIdx = 0;
	
	for( rowChip = 0; rowChip < 3; rowChip++ )
	{
		switch( rowChip )
		{
		case 0:
			IK_ROW_SEL_0( 0x00 );		// Pull all row lines low.
			IkShadowSensorSel &= 0xF4;	// Enable output of ROW chip.
			IK_SENSOR_SEL( IkShadowSensorSel );
			break;
		case 1:
			IK_ROW_SEL_1( 0x00 );		// Pull all row lines low.
			IkShadowSensorSel &= 0xF5;	// Enable output of ROW chip.
			IK_SENSOR_SEL( IkShadowSensorSel );
			break;
		case 2:
			IK_ROW_SEL_2( 0x00 );		// Pull all row lines low.
			IkShadowSensorSel &= 0xF6;	// Enable output of ROW chip.
			IK_SENSOR_SEL( IkShadowSensorSel );
			break;
		}

		for( colChip = 0; colChip < 3; colChip++ )
		{
			switch( colChip )
			{
			case 0:
				OUTC &= 0x7F;
				OUTC |= 0xE0;
				OUTC &= 0x7F;
				break;
			case 1:
				OUTC &= 0x9F;
				OUTC |= 0xE0;
				OUTC &= 0x9F;
				break;
			case 2:
				OUTC &= 0xBF;
				OUTC |= 0xE0;
				OUTC &= 0xBF;
				break;
			}

			reg = PINSB;

			OUTC |= 0xE0;

			if( reg != 0xFF )
				IkMatrixScanGetPress( rowChip, colChip );
		}

		// Disable outputs of all row chips.
		//
		IkShadowSensorSel |= 0x0F;
		IK_SENSOR_SEL( IkShadowSensorSel );
	}

	//  fwr 1/8/00 determine if the device is "off" by looking at
	//  current presses on column 0

	//  count the presses in column 0
	nPressesInCol = 0;
	for (i=0;i<IkPressBufNewIdx;i++)
	{
		col = IkPressBufNew[i] >> 8;
		row = IkPressBufNew[i] & 0xFF;
		if(col==0)
			nPressesInCol++;
	}

	if(nPressesInCol>=8)  //  fwr 1/8/01  12 works but not 24.  Hmmm......
	{
		//  condition is off.
		if (IkDeviceSwitchedOn)
		{	
			//  first off since on
			IkEventLog ( IK_EVENT_ONOFFSWITCH, 0, 0);
		}
		IkDeviceSwitchedOn = FALSE;
	}
	else
	{
		//  condition is on.
		if (!IkDeviceSwitchedOn)
		{
			// first on since off.
			IkCmdScanStart();
			IkEventLog ( IK_EVENT_ONOFFSWITCH, 1, 0);
		}
		IkDeviceSwitchedOn = TRUE;
	}

#ifdef AUTOPILOT
//  fwr begin autopilot
	//  count current presses
	numPress = 0;
	for (i=0;i<IkPressBufNewIdx;i++)
		numPress++;

	//  if just releasing
	if (numPress==0 && numPressLast>0)
	{
		//  increment counter if this one was "fast" compared to the last one
		if (IkTimerTick-timePressLast<35 && IkTimerTick-timePressLast>5)  //  350 msec between
			numFastPress++;
		else
			numFastPress = 0;
		timePressLast = IkTimerTick;

		// if three fast in a row, toggle autopilot state
		//  and act on it
		if (numFastPress==2) // 2 is 2 fast intervals, or three fast presses
			ToggleAutoPilot ();
	}
	numPressLast = numPress;

	DoAutoPilot ();
//  fwr end autopilot
#endif

	//  fwr 1/8/00  don't generate events if the device is "off" or if scanning is disabled
	if( IkScanEnabledFlag && IkDeviceSwitchedOn )
		IkMatrixScanCreateEvents();
}

// ------------------------------------------------------ IkMatrixScanGetPress()
// FUNCTION:	IkMatrixScanGetPress()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkMatrixScanGetPress( BYTE rowChip, BYTE colChip )
{
	BYTE rowMask, row, i, j, reg, x;
	WORD xy;

	rowMask = 0xFE;
	for( row = 0; row < 8; row++ )
	{
		switch( rowChip )
		{
		case 0:
			IK_ROW_SEL_0( rowMask );
			break;
		case 1:
			IK_ROW_SEL_1( rowMask );
			break;
		case 2:
			IK_ROW_SEL_2( rowMask );
			break;
		}

		switch( colChip )
		{
		case 0:
			OUTC &= 0x7F;
			OUTC |= 0xE0;
			OUTC &= 0x7F;
			break;
		case 1:
			OUTC &= 0x9F;
			OUTC |= 0xE0;
			OUTC &= 0x9F;
			break;
		case 2:
			OUTC &= 0xBF;
			OUTC |= 0xE0;
			OUTC &= 0xBF;
			break;
		}

		reg = PINSB;

		OUTC |= 0xE0;

		if( reg != 0xFF )
		{
			for( i = 0; i < 8; i++ )
			{
				if(( reg & 0x01 ) == 0 )
				{
					// We've got a membrane press.
					
					if(( IkPressBufNewIdx ) < IK_MAX_PRESS_BUF_LEN )
					{
						if(( colChip == 0 ) || (( colChip == 1 ) && ( i < 4 )))
							x = ( colChip * 8 ) + i;
						else if( colChip == 1 )
							x = 27 - i;
						else
							x = 19 - i;

						IkPressBufNew[ IkPressBufNewIdx++ ] =
							((WORD)x << 8 ) + ( rowChip * 8 ) + row;
					}
				}
				
				reg >>= 1;
			}
		}

		rowMask = ( rowMask << 1 ) | 0x01;
	}

	// ----------------------------------------------------
	// ----------------------------------------------------
	//
	rowMask = 0xFC;
	for( row = 0; row < 7; row++ )
	{
		switch( rowChip )
		{
		case 0:
			IK_ROW_SEL_0( rowMask );
			break;
		case 1:
			IK_ROW_SEL_1( rowMask );
			break;
		case 2:
			IK_ROW_SEL_2( rowMask );
			break;
		}

		switch( colChip )
		{
		case 0:
			OUTC &= 0x7F;
			OUTC |= 0xE0;
			OUTC &= 0x7F;
			break;
		case 1:
			OUTC &= 0x9F;
			OUTC |= 0xE0;
			OUTC &= 0x9F;
			break;
		case 2:
			OUTC &= 0xBF;
			OUTC |= 0xE0;
			OUTC &= 0xBF;
			break;
		}

		reg = PINSB;

		OUTC |= 0xE0;

		if( reg != 0xFF )
		{
			for( i = 0; i < 8; i++ )
			{
				if(( reg & 0x01 ) == 0 )
				{
					// We've got a membrane press.

					if(( colChip == 0 ) || (( colChip == 1 ) && ( i < 4 )))
						x = ( colChip * 8 ) + i;
					else if( colChip == 1 )
						x = 27 - i;
					else
						x = 19 - i;

					xy = ((WORD)x << 8 ) + ( rowChip * 8 ) + row;
					for( j = 0; j < IkPressBufNewIdx; j++ )
					{
						if(( IkPressBufNew[ j ] == xy ) ||
								( IkPressBufNew[ j ] == ( xy + 1 )))
							break;
					}
					if( j == IkPressBufNewIdx )
					{
						if(( IkPressBufNewIdx + 2 ) <= IK_MAX_PRESS_BUF_LEN )
						{
							IkPressBufNew[ IkPressBufNewIdx++ ] = xy;
							IkPressBufNew[ IkPressBufNewIdx++ ] = xy + 1;
						}
					}
				}
				
				reg >>= 1;
			}
		}

		rowMask = ( rowMask << 1 ) | 0x01;
	}
}

// -------------------------------------------------- IkMatrixScanCreateEvents()
// FUNCTION:	IkMatrixScanCreateEvents()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkMatrixScanCreateEvents( void )
{
	BYTE i,j;
	BYTE found;

	//  generate a press for each new press not on the old list
	for (i=0;i<IkPressBufNewIdx;i++)
	{
		found = 0;
		for (j=0;j<IkPressBufStoreIdx;j++)
		{
			if (IkPressBufNew[ i ] == IkPressBufStore[ j ])
				found = 1;
		}
		if (found==0)
		{
			//  new press not found in old list
			IkEventLog( IK_EVENT_MEMBRANE_PRESS, IkPressBufNew[ i ] >> 8,
				IkPressBufNew[ i ]);
		}
	}

	//  generate a release for each old press not found in new list
	for (j=0;j<IkPressBufStoreIdx;j++)
	{
		found = 0;
		for (i=0;i<IkPressBufNewIdx;i++)
		{
			if (IkPressBufNew[ i ] == IkPressBufStore[ j ])
				found = 1;
		}
		if (found==0)
		{
			IkEventLog( IK_EVENT_MEMBRANE_RELEASE, IkPressBufStore[ j ] >> 8,
				IkPressBufStore[ j ]);
		}
	}

	//  copy the new array to the old
	for( i = 0; i < IkPressBufNewIdx; i++ )
		IkPressBufStore[ i ] = IkPressBufNew[ i ];
	IkPressBufStoreIdx = IkPressBufNewIdx;

}

// ---------------------------------------------------------------- IkEventLog()
// FUNCTION:	IkEventLog()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//

void IkEventLog( BYTE eventType, BYTE data1, BYTE data2 )
{
	if (!bDoAnyOutput)
		return;

	if((( IkEventHead + 1 ) == IkEventTail ) ||
			((( IkEventHead + 1 ) == IK_MAX_EVENTS ) && ( IkEventTail == 0 )))
	{
		return;	// Buffer is full.
	}

	IkEvents[ IkEventHead ].eventType = eventType;
	IkEvents[ IkEventHead ].data1 = data1;
	IkEvents[ IkEventHead ].data2 = data2;
	IkEvents[ IkEventHead ].timeStamp = (WORD)IkTimerTick;

	if( ++IkEventHead == IK_MAX_EVENTS )
		IkEventHead = 0;

	numEventsQueued++;
}

// -------------------------------------------------------------- IkSwitchScan()
// FUNCTION:	IkSwitchScan()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkSwitchScan( void )
{
	BOOL state;

	if( PINSC & 0x08 )
		state = FALSE;
	else
		state = TRUE;

	if( IkSwitch1State != state )
	{
		IkEventLog( IK_EVENT_SWITCH, 1, state );
		IkSwitch1State = state;
	}

	if( PINSC & 0x04 )
		state = FALSE;
	else
		state = TRUE;

	if( IkSwitch2State != state )
	{
		IkEventLog( IK_EVENT_SWITCH, 2, state );
		IkSwitch2State = state;
	}
}

// -------------------------------------------------------------- IkSensorScan()
// FUNCTION:	IkSensorScan(BOOL bForce)
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00	Created.
// -----------------------------------------------------------------------------
//
void IkSensorScan( BOOL bForce )
{
	BYTE sensor, i, shiftOut, shiftIn;

	for( sensor = 0; sensor < ( IK_MAX_SENSORS + 1 ); sensor++ )
	{
		IkShadowSensorSel &= 0xF0 | sensor;		// Drive sensor LED.
		IK_SENSOR_SEL( IkShadowSensorSel );
	
		OEB = 0x03;	// Make PB0 and PB1 output pins.

		OUTB &= 0xFC; // Start with I/O CLK and ADD IN low.
		OUTA &= 0xEF; // Drive the ADC chip select low.

		shiftOut = sensor;
		shiftIn = 0;
		for( i = 0; i < 8; i++ )
		{
			shiftIn <<= 1;
			if( PINSB & 0x04 )
				shiftIn |= 0x01;

			if( shiftOut & 0x08 )
				OUTB |= 0x02;
			else
				OUTB &= 0xFD;
			shiftOut <<= 1;

			OUTB |= 0x01;	// Clock the ADC.
			OUTB &= 0xFE;
		}
		
		if( sensor )
		{
			if( IkSensorState[ sensor - 1 ] > shiftIn )
				i = IkSensorState[ sensor - 1 ] - shiftIn;
			else
				i = shiftIn - IkSensorState[ sensor - 1 ];

			if( (i >= IkSensorEventDelta) || bForce )
			{
				IkEventLog( IK_EVENT_SENSOR_CHANGE, sensor - 1, shiftIn );
				IkSensorState[ sensor - 1 ] = shiftIn;
			}
		}
		
		while(( PINSA & 0x20 ) == 0 );  // Wait for EOC to go high, indicating
										// conversion is complete.
		
		OUTA |= 0x10; // De-select ADC.
		OEB = 0x00; // Return port B pins to input.

		IkShadowSensorSel |= 0x0F;
		IK_SENSOR_SEL( IkShadowSensorSel );		// Shut off sensor LED.
	}
}

// ----------------------------------------------------------- IkCmdEepromRead()
// FUNCTION:	IkCmdEepromRead()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 10-05-00	Created.
// -----------------------------------------------------------------------------
//


void IkCmdEepromReadByte ( void)
{
	WORD add;

	// Only do this if IN1BSY = 0 (not busy).
	//
	//if(( IN1CS & 0xFD ) != 0 )
	if(!bEP1Complete ) //  FWR
		return;

	add = ((WORD)IK_OUT_REPORT[ 2 ] << 8 ) + IK_OUT_REPORT[ 1 ];

	IN1BUF[ 0 ] = IK_EVENT_EEPROM_READBYTE;

	EepromRead( add, 1, &IN1BUF[ 1 ]);

	IN1BUF[2] = IK_OUT_REPORT[ 1 ]; // lsb
	IN1BUF[3] = IK_OUT_REPORT[ 2 ]; // msb

#ifdef SEQUENCE
	nsend++;
	IN1BUF[5] = nsend/256;
	IN1BUF[6] = nsend - (nsend/256)*256;
#endif

	bEP1Complete = 0;  // FWR
	IN1BC = 8; // Arm endpoint by writing byte count.
}

void IkCmdEepromRead( void )
{
	WORD add;

	// Only do this if IN1BSY = 0 (not busy).
	//
	//if(( IN1CS & 0xFD ) != 0 )
	if(!bEP1Complete ) //  FWR
		return;

	add = ((WORD)IK_OUT_REPORT[ 2 ] << 8 ) + IK_OUT_REPORT[ 1 ];

	IN1BUF[ 0 ] = IK_EVENT_EEPROM_READ;

	EepromRead( add, 7, &IN1BUF[ 1 ]);

#ifdef SEQUENCE
	nsend++;
	IN1BUF[5] = nsend/256;
	IN1BUF[6] = nsend - (nsend/256)*256;
#endif

	bEP1Complete = 0;  // FWR
	IN1BC = 8; // Arm endpoint by writing byte count.
}

// ---------------------------------------------------------- IkCmdEepromWrite()
// FUNCTION:	IkCmdEepromWrite()
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 10-05-00	Created.
// -----------------------------------------------------------------------------
//
void IkCmdEepromWrite( void )
{
	WORD add;	
	add = ((WORD)IK_OUT_REPORT[ 2 ] << 8 ) + IK_OUT_REPORT[ 1 ];

#ifndef SCRIBBLER
	//  protect first 7 bytes
	if (add <=6)
		return;
#endif

	EepromWrite( add, IK_OUT_REPORT[ 3 ], &IK_OUT_REPORT[ 4 ]);
}
