// ------------------------------------------------------------------ FILE: Ik.H
//      Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//     FILE:	Ik.H
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 8-01-00		Created.
//
// fwr   12/01/00   version 0.4, 10 mSec polling, 100 mAmp power
// fwr   01/05/01   version 0.5, workaround to Cypress errata regarding
//                               register polling
// fwr   01/08/01   version 0.6, first on/off implementation
//
// JWH   11/02/02   Added two additional HID descriptors and associated code
// Disable by setting JHW to 0
#define JWH 1
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// DEFINITIONS														 DEFINITIONS
// -----------------------------------------------------------------------------
//

#include "firmware.h"

#define IK_VERSION_MAJOR			2
#define IK_VERSION_MINOR			0

//
//  fwr 10-29-02
//  I think we can get away with fewer here
//
//#define IK_MAX_EVENTS				300
#define IK_MAX_EVENTS				100

#define IK_MAX_PRESS_BUF_LEN		16	// = max membrane presses that can be
										// handled simultaneously.

// This is the address of an 8K x 8 (24LC64) device on the I2C bus.
//
#define IK_I2C_EEPROM_ADDR			0x51

#define IK_OUT_REPORT				OUT2BUF

#define IK_DEFAULT_SENSOR_EVENT_DELTA	1 //  fwr 2/14/00 TODO: testing ONLY!!!

// DBC: HID code start
#define GD_HIDDESC				0x21	// Get HID descriptor
#define GD_REPORT				0x22	// Get HID report descriptor
// DBC: HID code end

#define NO_EARLY_CHATTER  1
//#define NO_EARLY_CHATTER2 1
#define SEQUENCE

// -----------------------------------------------------------------------------
// TYPE DEFINITIONS												TYPE DEFINITIONS
// -----------------------------------------------------------------------------
//
typedef struct
{
	BYTE eventType;
	BYTE data1;
	BYTE data2;
	WORD timeStamp;
} IK_EVENT;

#if JWH
// These two structs are wrong/not used
#else

// HID code start
typedef struct
{
	BYTE	length;			// HID descriptor length ( = sizeof(HIDDSCR) )
	BYTE	type;			// Descriptor type (HID = 21h)
	BYTE	specl;			// HID spec # low byte
	BYTE	spech;			// HID spec # high byte
	BYTE	country;		// Country localization (0 = none) 
	BYTE	num_dscr;		// Number of descriptors to follow
	BYTE	dscr_type;		// Descriptor type to follow (22h = report)
	BYTE	reportlength;		// Report descriptor length
}HIDDSCR;

typedef struct				// HID Report Descriptor for this application
{
	BYTE	Usage_Page_Code1;	
	BYTE	Usage_Page_LSB1;
	BYTE	Usage_Page_MSB1;
	BYTE	Usage_Code1;
	BYTE	Usage_Value1;
	BYTE	Collection_Code1;
	BYTE	Collection_Value1;
	BYTE	Usage_Code2;
	BYTE	Usage_Value2;
	BYTE	Collection_Code2;
	BYTE	Collection_Value2;
	BYTE	Usage_Page_Code2;
 	BYTE	Usage_Page_LSB2;
	BYTE	Usage_Page_MSB2;
// Input report
	BYTE	Usage_Code3;
	BYTE	Usage_Value3;
	BYTE	Usage_Code4;
	BYTE	Usage_Value4;
	BYTE	Logical_Min_In_Code;
	BYTE	Logical_Min_In_Value;
	BYTE	Logical_Max_In_Code;
	BYTE	Logical_Max_In_Value;
	BYTE	Physical_Min_In_Code;
	BYTE	Physical_Min_In_Value;
	BYTE	Physical_Max_In_Code;
	BYTE	Physical_Max_In_Value;
	BYTE	Report_Size_In_Code;
	BYTE	Report_Size_In_Value;
	BYTE	Report_Count_In_Code;
	BYTE	Report_Count_In_Value;
	BYTE	Input_Code;
	BYTE	Input_Value;
// Output report
	BYTE	Usage_Code5;
	BYTE	Usage_Value5;
	BYTE	Usage_Code6;
	BYTE	Usage_Value6;
	BYTE	Logical_Min_Out_Code;
	BYTE	Logical_Min_Out_Value;
	BYTE	Logical_Max_Out_Code;
	BYTE	Logical_Max_Out_Value;
	BYTE	Physical_Min_Out_Code;
	BYTE	Physical_Min_Out_Value;
	BYTE	Physical_Max_Out_Code;
	BYTE	Physical_Max_Out_Value;
	BYTE	Report_Size_Out_Code;
	BYTE	Report_Size_Out_Value;
	BYTE	Report_Count_Out_Code;
	BYTE	Report_Count_Out_Value;
	BYTE	Output_Code;
	BYTE	Output_Value;

	BYTE	End_Collection1;
	BYTE	End_Collection2;
}REPORTDSCR;

// HID code end
#endif

// -----------------------------------------------------------------------------
// MACROS																  MACROS
// -----------------------------------------------------------------------------
//
#define IK_SENSOR_SEL( x )	{ OEB = 0xFF; OUTB = x; OUTC &= 0xDF; OUTC |= 0x20; OEB = 0x00; }
#define IK_LED_SEL( x ) { OEB = 0xFF; OUTB = x; OUTC &= 0xFE; OUTC |= 0x01; OEB = 0x00; }
#define IK_ROW_SEL_0( x ) { OEB = 0xFF; OUTB = x; OUTC &= 0x1F; OUTC |= 0xE0; OEB = 0x00; }
#define IK_ROW_SEL_1( x ) { OEB = 0xFF; OUTB = x; OUTC &= 0x3F; OUTC |= 0xE0; OEB = 0x00; }
#define IK_ROW_SEL_2( x ) { OEB = 0xFF; OUTB = x; OUTC &= 0x5F; OUTC |= 0xE0; OEB = 0x00; }

// -----------------------------------------------------------------------------
// EXTERNS																 EXTERNS
// -----------------------------------------------------------------------------
//
extern BYTE	volatile IkCmdPending;
extern BOOL bEP1Complete;

#if JWH
// Use arrays to access three HID devices
extern WORD	code ReportDscr[3];
extern BYTE code ReportDscrLen[3];
extern WORD	code HIDDscr[3];
#else
extern WORD	pReportDscr;
extern code REPORTDSCR	ReportDscr;
#endif

// -----------------------------------------------------------------------------
// PROTOTYPES														  PROTOTYPES
// -----------------------------------------------------------------------------
//
void IkInit( void );
void IkFunctionInit( void );
void IkPoll( void );
void IkCmdProcess( void );
void IkCmdScanStart( void );
void IkCmdLed( BYTE ledNum, BYTE onOrOff );
void IkCmdTone( void );
void IkCmdToneOff( void );
void EepromWriteByte( WORD addr, BYTE value );
void EepromWrite( WORD addr, BYTE length, BYTE xdata *buf );
void EepromRead( WORD addr, BYTE length, BYTE xdata *buf );
void IkMatrixScan( void );
void IkMatrixScanGetPress( BYTE rowChip, BYTE colChip );
void IkMatrixScanCreateEvents( void );
void IkEventLog( BYTE eventType, BYTE data1, BYTE data2 );
void IkSwitchScan( void );
void IkSensorScan( BOOL bForce );
void IkCmdEepromRead( void );
void IkCmdEepromReadByte( void );
void IkCmdEepromWrite( void );

// ----------------------------------------------------------------------- FILE:
//     FILE:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 5-01-00		Created.
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// TYPE DEFINITIONS												TYPE DEFINITIONS
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// DEFINITIONS														 DEFINITIONS
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// MACROS																  MACROS
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// STRUCTURES														  STRUCTURES
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// EXTERNS																 EXTERNS
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// PROTOTYPES														  PROTOTYPES
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// GLOBALS															     GLOBALS
// -----------------------------------------------------------------------------
//
// -------------------------------------------------------------------------- ()
// FUNCTION:	
//	   DESC:	
//   AUTHOR:	D. Cooper
// REVISION:	1.0		 5-01-00	Created.
// -----------------------------------------------------------------------------
//

