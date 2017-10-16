//-----------------------------------------------------------------------------
//	File:		fw.c
//	Contents:	Firmware frameworks task dispatcher and device request parser
//				source.
//
//	Copyright (c) 1997 AnchorChips, Inc. All rights reserved
//
// JWH - processing for two additional commands added
//-----------------------------------------------------------------------------
#include "ezusb.h"
#include "ezregs.h"

// DBC
#include "Ik.h"
// DBC


//-----------------------------------------------------------------------------
// Random Macros
//-----------------------------------------------------------------------------
#define	min(a,b) (((a)<(b))?(a):(b))
#define	max(a,b) (((a)>(b))?(a):(b))

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define	DELAY_COUNT				0x9248*8L		// Delay for 8 sec at 24Mhz,
												// 4 sec at 48

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
volatile	BOOL	GotSUD;
BOOL		Rwuen;
BOOL		Selfpwr;
volatile	BOOL	Sleep;	// Sleep mode enable flag

WORD	pDeviceDscr;		// Pointer to Device Descriptor.
							// Descriptors may be moved.
WORD	pConfigDscr;	
WORD	pStringDscr;	

// DBC: HID code start
WORD	pReportDscr;
BYTE	reportlen;
// DBC: HID code end
BYTE	KeyboardLEDs;	

WORD	pHIDDscr;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void SetupCommand(void);
void TD_Init(void);
void TD_Poll(void);
BOOL TD_Suspend(void);
BOOL TD_Resume(void);

BOOL DR_GetDescriptor(void);
BOOL DR_SetConfiguration(void);
BOOL DR_GetConfiguration(void);
BOOL DR_SetInterface(void);
BOOL DR_GetInterface(void);
BOOL DR_GetStatus(void);
BOOL DR_ClearFeature(void);
BOOL DR_SetFeature(void);
BOOL DR_VendorCmnd(void);

//-----------------------------------------------------------------------------
// Code
//-----------------------------------------------------------------------------

// Task dispatcher
void main(void)
{
#if JWH
// Don't need these variables
#else
	DWORD	i;
	WORD	offset;
	DWORD	DevDescrLen;
	WORD	IntDescrAddr;
	WORD	ExtDescrAddr;
#endif
	DWORD	j=0;

	//  this is wierd.  Is it helping?
	EZUSB_Delay(1000);
	
	// Initialize Global States
	Sleep = FALSE;					// Disable sleep mode
	Rwuen = FALSE;					// Disable remote wakeup
	Selfpwr = FALSE;				// Disable self powered
	GotSUD = FALSE;					// Clear "Got setup data" flag

	// Initialize user device
// DBC
//	TD_Init();
	IkInit();
// DBC

	// The following section of code is used to relocate the descriptor table. 
	// Since the SUDPTRH and SUDPTRL are assigned the address of the descriptor 
	// table, the descriptor table must be located in on-part memory.
	// The 4K demo tools locate all code sections in external memory.
	// The descriptor table is relocated by the frameworks ONLY if it is found 
	// to be located in external memory.
	pDeviceDscr = (WORD)&DeviceDscr;
	pConfigDscr = (WORD)&ConfigDscr;
	pStringDscr = (WORD)&StringDscr;

#if JWH
// The following code can't possibly work - there is no external RAM to move from!
// I need to remove some code to make room, this is an obvious candidate
#else

// DBC: HID code start
	pReportDscr = (WORD)&ReportDscr;
// DBC: HID code end

	if ((WORD)&DeviceDscr & 0xe000)
	{
		IntDescrAddr = INTERNAL_DSCR_ADDR;
		ExtDescrAddr = (WORD)&DeviceDscr;
		DevDescrLen = (WORD)&UserDscr - (WORD)&DeviceDscr + 2;
		for (i = 0; i < DevDescrLen; i++)
			*((BYTE xdata *)IntDescrAddr+i) = 0xCD;
		for (i = 0; i < DevDescrLen; i++)
			*((BYTE xdata *)IntDescrAddr+i) = *((BYTE xdata *)ExtDescrAddr+i);
		pDeviceDscr = IntDescrAddr;
		offset = (WORD)&DeviceDscr - INTERNAL_DSCR_ADDR;
		pConfigDscr -= offset;
		pStringDscr -= offset;

// DBC: HID code start
		pReportDscr -= offset;
// DBC: HID code end
	}
#endif

	EZUSB_IRQ_ENABLE();				// Enable USB interrupt (INT2)
	EZUSB_ENABLE_RSMIRQ();				// Wake-up interrupt

	// The 8051 is responsible for all USB events, even those that have happened
	// before this point.  We cannot ignore pending USB interrupts.
	// The chip will come out of reset with the flags all cleared.
	//	USBIRQ = 0xff;				// Clear any pending USB interrupt requests

// DBC: This design does not use external memory.
//	PORTCCFG |= 0xc0;				// Turn on r/w lines for external memory 
// DBC

	USBBAV = USBBAV | 1 & ~bmBREAK;	// Disable breakpoints and autovectoring
	USBIEN |= bmSUDAV | bmSUTOK | bmSUSP | bmURES;	// Enable selected interrupts
	EA = 1;						// Enable 8051 interrupts

	// This loop waits until we receive a setup packet from the host.
	// NOTE: The device will continue to renumerate until it receives a setup
	// packet.  This fixes a microsoft USB bug that loses disconnect/reconnect 
	// events during initial USB device driver configuration dialog box.
	// B2 Load:  This code is not needed for B2 load, only for renumeration.
	#ifndef NO_RENUM
		while(!GotSUD)
		{
			if(!GotSUD)
				EZUSB_Discon(TRUE);	// renumerate until setup received
			for(j=0;(j<DELAY_COUNT) && (!GotSUD);++j);
		}
	#endif


	CKCON = (CKCON&(~bmSTRETCH)) | FW_STRETCH_VALUE; // Set stretch to 0 (after renumeration)

	// Task Dispatcher
	while(TRUE)					// Main Loop
	{
		if(GotSUD)				// Wait for SUDAV
		{
			SetupCommand();	 		// Implement setup command
  			GotSUD = FALSE;		   	// Clear SUDAV flag
		}

		// Poll User Device
		// NOTE: Idle mode stops the processor clock.  There are only two
		// ways out of idle mode, the WAKEUP pin, and detection of the USB
		// resume state on the USB bus.  The timers will stop and the
		// processor will not wake up on any other interrupts.
		if (Sleep)
		    {
    		if(TD_Suspend())
    		    { 
    		    Sleep = FALSE;	   		// Clear the "go to sleep" flag.  Do it here to prevent any race condition between wakeup and the next sleep.
    		    do
    		        {
       			    EZUSB_Susp();			// Place processor in idle mode.
    		        }
                while(!Rwuen && EZUSB_EXTWAKEUP());
                // Must continue to go back into suspend if the host has disabled remote wakeup
                // *and* the wakeup was caused by the external wakeup pin.
                
    			// 8051 activity will resume here due to USB bus or Wakeup# pin activity.
    			EZUSB_Resume();	// If source is the Wakeup# pin, signal the host to Resume.		
    			TD_Resume();
    		    }   
		    }

// DBC
//		TD_Poll();

		IkPoll();
// DBC
	}
}

// Device request parser
void SetupCommand(void)
{
	void	*dscr_ptr;
	DWORD	i;
	BYTE	j;

	//  handle some HID class stuff as a special case

	if( SETUPDAT[0] == 0x21 )	// Class Out request.
	{
		switch (SETUPDAT[1])
		{
			case 0x09:  //  Set Report
				OUT0BC = 0;  // Write any value to OUT0BC to arm the endpoint.
				while(( OUT07IRQ & 0x01 ) == 0 );  // Wait for the data to arrive.
				if (SETUPDAT[4] == 0)  //  custom
				{
					// Copy the command data to where it is expected by the rest of
					// the program.
					for( j = 0; j < 8; j++ )
						IK_OUT_REPORT[ j ] = OUT0BUF[ j ];
					OUT07IRQ = bmEP0; // Clear the soft interrupt.
					IkCmdPending = TRUE;
				}
				else if (SETUPDAT[4] == 1)  //  keyboard
				{
					KeyboardLEDs = OUT0BUF[0];
				}
				else
				{
					EZUSB_STALL_EP0();
				}
				break;

			default:
				EZUSB_STALL_EP0();
				break;
		}

		// Acknowledge handshake phase of device request.
		// Required for rev C does not effect rev B.
		EP0CS |= bmBIT1;

		return;
	}


	switch(SETUPDAT[1])
	{
		case SC_GET_DESCRIPTOR:						// *** Get Descriptor
			if(DR_GetDescriptor())
				switch(SETUPDAT[3])			
				{
					case GD_DEVICE:				// Device
						SUDPTRH = MSB(pDeviceDscr);
						SUDPTRL = LSB(pDeviceDscr);
						break;
					case GD_CONFIGURATION:			// Configuration
						if(dscr_ptr = (void *)EZUSB_GetConfigDscr(SETUPDAT[2]))
						{
							SUDPTRH = MSB(dscr_ptr);
							SUDPTRL = LSB(dscr_ptr);
						}
						else
							EZUSB_STALL_EP0(); 	// Stall End Point 0
						break;
					case GD_STRING:				// String
						if(dscr_ptr = (void *)EZUSB_GetStringDscr(SETUPDAT[2]))
						{
							// Workaround for rev D errata number 8
							// If you're certain that you will never run on rev D,
							// you can just do this:
							// SUDPTRH = MSB(dscr_ptr);
							// SUDPTRL = LSB(dscr_ptr);
							STRINGDSCR *sdp;
							BYTE len;

							sdp = dscr_ptr;

							len = sdp->length;
							if (len > SETUPDAT[6]) 
								len = SETUPDAT[6]; //limit to the requested length
							
							while (len)
							{
								for(i=0; i<min(len,64); i++)
									*(IN0BUF+i) = *((BYTE xdata *)sdp+i);

								//set length and arm Endpoint
								EZUSB_SET_EP_BYTES(IN0BUF_ID,min(len,64));	
								len -= min(len,64);

								// Wait for it to go out (Rev C and above)
								while(EP0CS & 0x04)
									;
							}

							// Arm a 0 length packet just in case.  There was some reflector traffic about
							// Apple hosts asking for too much data.  This will keep them happy and will
							// not hurt valid hosts because the next SETUP will clear this.
							EZUSB_SET_EP_BYTES(IN0BUF_ID,0);	
							// Clear the HS-nak bit
							EP0CS = bmHS;
						}
						else 
							EZUSB_STALL_EP0();	// Stall End Point 0
						break;
// DBC: HID code start
					case GD_REPORT:				
						//HID Report descriptor
						//Assumes there is one report descriptor.
						//To do: add the ability to return a specific report descriptor.
						//Can't use SUDPTRH and SUDPTRL because the report descriptor doesn't store 
						//its length in the first bytes.
						//Instead, adapt code from the String descriptor rev D errata code (above).
						{
#if JWH
// Allow for 3 HID devices, Setupdata[4] contains Interface Index
							BYTE code *rdp;
							rdp = (BYTE code *) ReportDscr[SETUPDAT[4]];
							reportlen = ReportDscrLen[SETUPDAT[4]];
							if (reportlen > SETUPDAT[6]) reportlen = SETUPDAT[6];
							for (i=0; i<reportlen; i++, rdp++) IN0BUF[i] = *rdp;
							IN0BC = reportlen;
							while(EP0CS & 0x04);
#else
							//rdp holds the address of a REPORTDSCR structure.
							//dscr_ptr is the address of the report descriptor
							REPORTDSCR *rdp;
							dscr_ptr = (REPORTDSCR xdata *) pReportDscr;
							reportlen = sizeof(REPORTDSCR);
							rdp = dscr_ptr;


							// If the descriptor is longer than the requested amount,
							// limit the data returned to the requested amount. 
							if (reportlen > SETUPDAT[6]) 
								reportlen = SETUPDAT[6]; 
							// If the host requests more bytes than the descriptor contains,
							// the device will send the descriptor only.
 							
							while (reportlen)
							{
								//Copy the data to send into Endpoint 0's IN buffer.
								//In each transaction, send the entire descriptor or 64 bytes, whichever is less.
								//The data to send begins at the address pointed to by *rdp.
								for(i=0; i<min(reportlen,64); i++)
									*(IN0BUF+i) = *((BYTE xdata *)rdp+i);

								//Set the amount of data to send and arm the endpoint
								EZUSB_SET_EP_BYTES(IN0BUF_ID,min(reportlen,64));	
								// If reportlen <= 64, all bytes have been copied, so set reportlen =0.
								// Else, set reportlen = number of bytes remaining to send.
								reportlen -= min(reportlen,64);

								// Wait for the data to go out (Rev C and above)
								while(EP0CS & 0x04);
							}
#endif							
                        }
						//else 
						//	EZUSB_STALL_EP0();	// Stall End Point 0

						break;
// DBC: HID code end
				
					case GD_HIDDESC:
						SUDPTRH = MSB(HIDDscr[SETUPDAT[4]]);
						SUDPTRL = LSB(HIDDscr[SETUPDAT[4]]);
						break;

					default:				// Invalid request
						EZUSB_STALL_EP0();		// Stall End Point 0
				}
			break;
		case SC_GET_INTERFACE:						// *** Get Interface
			DR_GetInterface();
			break;
		case SC_SET_INTERFACE:						// *** Set Interface
			DR_SetInterface();
			break;
		case SC_SET_CONFIGURATION:					// *** Set Configuration
			DR_SetConfiguration();
#if JWH
			if (SETUPDAT[2] == 0)	// Being de-configured. Disable Data Endpoints
			{
				IN07VAL &= ~(bmEP1 | bmEP3 | bmEP4);
				OUT07VAL &= ~(bmEP2 | bmEP3);
				IN07IEN &= ~bmEP1;
				OUT07IEN &= ~(bmEP2 | bmEP3);
			}
			else					// Being Configured. Enable Data Endpoints
			{
				IN07VAL |= bmEP1 | bmEP3 | bmEP4;
				OUT07VAL |= bmEP2 | bmEP3;
				IN07IEN |= bmEP1;
				OUT07IEN |= bmEP2 | bmEP3;
				OUT2BC = 0; 		// Prime the endpoint to enable reception
				OUT3BC = 0; 		// Prime the endpoint to enable reception
				//bEP1Complete = 0;	// Clear local flag
				bEP1Complete = 1;	//
			}
#endif
			break;
		case SC_GET_CONFIGURATION:					// *** Get Configuration
			DR_GetConfiguration();
			break;
		case SC_GET_STATUS:						// *** Get Status
			if(DR_GetStatus())
				switch(SETUPDAT[0])
				{
					case GS_DEVICE:				// Device
						IN0BUF[0] = ((BYTE)Rwuen << 1) | (BYTE)Selfpwr;
						IN0BUF[1] = 0;
						EZUSB_SET_EP_BYTES(IN0BUF_ID,2);
						break;
					case GS_INTERFACE:			// Interface
						IN0BUF[0] = 0;
						IN0BUF[1] = 0;
						EZUSB_SET_EP_BYTES(IN0BUF_ID,2);
						break;
					case GS_ENDPOINT:			// End Point
						IN0BUF[0] = EPIO[EPID(SETUPDAT[4])].cntrl & bmEPSTALL;
						IN0BUF[1] = 0;
						EZUSB_SET_EP_BYTES(IN0BUF_ID,2);
						break;
					default:				// Invalid Command
						EZUSB_STALL_EP0();		// Stall End Point 0
				}
			break;
		case SC_CLEAR_FEATURE:						// *** Clear Feature
			if(DR_ClearFeature())
				switch(SETUPDAT[0])
				{
					case FT_DEVICE:				// Device
						if(SETUPDAT[2] == 1)
							Rwuen = FALSE; 		// Disable Remote Wakeup
						else
							EZUSB_STALL_EP0();	// Stall End Point 0
						break;
					case FT_ENDPOINT:			// End Point
						if(SETUPDAT[2] == 0)
                  {
							EZUSB_UNSTALL_EP( EPID(SETUPDAT[4]) );
                     EZUSB_RESET_DATA_TOGGLE( SETUPDAT[4] );
                  }
						else
							EZUSB_STALL_EP0();	// Stall End Point 0
						break;
				}
			break;
		case SC_SET_FEATURE:						// *** Set Feature
			if(DR_SetFeature())
				switch(SETUPDAT[0])
				{
					case FT_DEVICE:				// Device
						if(SETUPDAT[2] == 1)
							Rwuen = TRUE;		// Enable Remote Wakeup
						else
							EZUSB_STALL_EP0();	// Stall End Point 0
						break;
					case FT_ENDPOINT:			// End Point
						if(SETUPDAT[2] == 0)
							EZUSB_STALL_EP( EPID(SETUPDAT[4]) );
						else
							EZUSB_STALL_EP0();	 // Stall End Point 0
						break;
				}
			break;
		default:							// *** Invalid Command
			if(DR_VendorCmnd())
				EZUSB_STALL_EP0();				// Stall End Point 0
	}

	// Acknowledge handshake phase of device request
	// Required for rev C does not effect rev B
	EP0CS |= bmBIT1;
}

// Wake-up interrupt handler
void resume_isr(void) interrupt WKUP_VECT
{
	EZUSB_CLEAR_RSMIRQ();
}
