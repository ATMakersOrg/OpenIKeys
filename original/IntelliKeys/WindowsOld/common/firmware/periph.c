#pragma NOIV					// Do not generate interrupt vectors
//-----------------------------------------------------------------------------
//	File:		periph.c
//	Contents:	Hooks required to implement USB peripheral function.
//
//	Copyright (c) 1997 AnchorChips, Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "ezusb.h"
#include "ezregs.h"

// DBC
#include "Ik.h"
// DBC

extern BOOL	GotSUD;			// Received setup data flag
extern BOOL	Sleep;
extern BOOL	Rwuen;
extern BOOL	Selfpwr;

BYTE	Configuration;		// Current configuration
BYTE	AlternateSetting;	// Alternate settings

// DBC: HID code start
BYTE InReportBytes;
void *dscr_ptr;
// DBC: HID code end

//-----------------------------------------------------------------------------
// Task Dispatcher hooks
//	The following hooks are called by the task dispatcher.
//-----------------------------------------------------------------------------

// DBC
/*
void TD_Init(void) 				// Called once at startup
{
	Rwuen = TRUE;				// Enable remote-wakeup
}

void TD_Poll(void) 				// Called repeatedly while the device is idle
{
}
*/
// DBC

BOOL TD_Suspend(void) 			// Called before the device goes into suspend mode
{
	int i;

	//  stop any sound
	IkCmdToneOff();

	//  stop all the lights
	for (i=0;i<9;i++)
		IkCmdLed ( i+1, 0);

	// turn off the 5 volts
	OUTC &= 0xEF;
	EZUSB_Delay1ms();

	return(TRUE);
}

BOOL TD_Resume(void) 			// Called after the device resumes
{
	OUTC |= 0x10;  //  turn on 5V
	EZUSB_Delay1ms();

	return(TRUE);
}


//-----------------------------------------------------------------------------
// Device Request hooks
//	The following hooks are called by the end point 0 device request parser.
//-----------------------------------------------------------------------------

BOOL DR_GetDescriptor(void)
{
	return(TRUE);
}

BOOL DR_SetConfiguration(void)	// Called when a Set Configuration command is received
{
	Configuration = SETUPDAT[2];
	return(TRUE);				// Handled by user code
}

BOOL DR_GetConfiguration(void)	// Called when a Get Configuration command is received
{
	IN0BUF[0] = Configuration;
	EZUSB_SET_EP_BYTES(IN0BUF_ID,1);
	return(TRUE);				// Handled by user code
}

BOOL DR_SetInterface(void) 		// Called when a Set Interface command is received
{
	AlternateSetting = SETUPDAT[2];
	return(TRUE);				// Handled by user code
}

BOOL DR_GetInterface(void) 		// Called when a Set Interface command is received
{
	IN0BUF[0] = AlternateSetting;
	EZUSB_SET_EP_BYTES(IN0BUF_ID,1);
	return(TRUE);				// Handled by user code
}

BOOL DR_GetStatus(void)
{
	return(TRUE);
}

BOOL DR_ClearFeature(void)
{
	return(TRUE);
}

BOOL DR_SetFeature(void)
{
	return(TRUE);
}

BOOL DR_VendorCmnd(void)
{
	return(TRUE);
}

//-----------------------------------------------------------------------------
// USB Interrupt Handlers
//	The following functions are called by the USB interrupt jump table.
//-----------------------------------------------------------------------------

// Setup Data Available Interrupt Handler
void ISR_Sudav(void) interrupt 0
{
	GotSUD = TRUE;				// Set flag
	EZUSB_IRQ_CLEAR();
	USBIRQ = bmSUDAV;			// Clear SUDAV IRQ
}

// Setup Token Interrupt Handler
void ISR_Sutok(void) interrupt 0
{
	EZUSB_IRQ_CLEAR();
	USBIRQ = bmSUTOK;			// Clear SUTOK IRQ
}

void ISR_Sof(void) interrupt 0
{
	EZUSB_IRQ_CLEAR();
	USBIRQ = bmSOF;				// Clear SOF IRQ
}

//
//  fwr 10-29-02
//  arming of the in endpoint is 
//

#if JWH
// Don't need this hack
#else
void ArmOnce()
{
	REPORTDSCR *rdp;
	dscr_ptr = (REPORTDSCR xdata *) pReportDscr;
	rdp = dscr_ptr;
	InReportBytes = rdp -> Report_Count_In_Value;
	EPIO[IN1BUF_ID].bytes = InReportBytes;
}
#endif

void ISR_Ures(void) interrupt 0
{
#if JWH
// Following a USB Reset the device should be DISABLED
// We should NOT be loading any data endpoints!!!
#else
// DBC: HID code start

	// Arm the HID endpoints by loading the byte count registers.
	// Get the number of bytes the In report will send.
	// This information is in Report Count in the report descriptor,
	// and in Report_Count_In_Value in the REPORTDSCR structure.
	// (Assumes that ReportSize (specified in the report descriptor) = 8).
	// To do: calculate the number of bytes to send for reports with any ReportSize.

#ifndef NO_EARLY_CHATTER2
	ArmOnce();
	// The value for Out reports can be anything.
	EPIO[OUT2BUF_ID].bytes = 64;
#endif

// DBC: HID code end
#endif

	EZUSB_IRQ_CLEAR();
	USBIRQ = bmURES;			// Clear URES IRQ
}

void ISR_IBN(void) interrupt 0
{
}

void ISR_Susp(void) interrupt 0
{
	Sleep = TRUE;

	EZUSB_IRQ_CLEAR();
	USBIRQ = bmSUSP;
}

void ISR_Ep0in(void) interrupt 0
{
}

void ISR_Ep0out(void) interrupt 0
{
}

extern BOOL bEP1Complete;
extern BYTE	KeyboardLEDs;

void ISR_Ep1in(void) interrupt 0
{
// FWR
	//  Advertise that this is complete
	bEP1Complete = 1;
// DBC
	// Clear the IRQ.
	//
	EZUSB_IRQ_CLEAR();
	IN07IRQ = bmEP1;
// DBC
}

void ISR_Ep1out(void) interrupt 0
{
}

void ISR_Ep2in(void) interrupt 0
{
}

void ISR_Ep2out(void) interrupt 0
{
// DBC	
	IkCmdPending = TRUE;
	
	// Clear the IRQ.
	//
	EZUSB_IRQ_CLEAR();
	OUT07IRQ = bmEP2;
// DBC
}

void ISR_Ep3in(void) interrupt 0
{
}

void ISR_Ep3out(void) interrupt 0
{
// Just received a Keyboard Output Report
	KeyboardLEDs = OUT3BUF[0];
	EZUSB_IRQ_CLEAR();
	OUT07IRQ = bmEP3;
	OUT3BC = 0;		// Rearm endpoint to receive next report
}

void ISR_Ep4in(void) interrupt 0
{
}

void ISR_Ep4out(void) interrupt 0
{
}

void ISR_Ep5in(void) interrupt 0
{
}

void ISR_Ep5out(void) interrupt 0
{
}

void ISR_Ep6in(void) interrupt 0
{
}

void ISR_Ep6out(void) interrupt 0
{
}

void ISR_Ep7in(void) interrupt 0
{
}

void ISR_Ep7out(void) interrupt 0
{
}
