/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// file:		firmware.h
//
// Purpose:		Constants of interest to both the firmware and the software.
//
// 06/18/01 fwr, dgs initial implementation
// 11/02/02 JWH, two IK_CMDs added
//
**************************************************************************************************************************/

//
//  command codes sent to the device
//  see firmware documentation for details
//

#define CMD_BASE 0
#define IK_CMD_GET_VERSION			CMD_BASE+1
#define IK_CMD_LED					CMD_BASE+2
#define IK_CMD_SCAN					CMD_BASE+3
#define IK_CMD_TONE					CMD_BASE+4
#define IK_CMD_GET_EVENT			CMD_BASE+5
#define IK_CMD_INIT					CMD_BASE+6
#define IK_CMD_EEPROM_READ			CMD_BASE+7
#define IK_CMD_EEPROM_WRITE			CMD_BASE+8
#define IK_CMD_ONOFFSWITCH			CMD_BASE+9
#define IK_CMD_CORRECT				CMD_BASE+10
#define IK_CMD_EEPROM_READBYTE		CMD_BASE+11
#define IK_CMD_RESET_DEVICE         CMD_BASE+12
#define IK_CMD_START_AUTO			CMD_BASE+13
#define IK_CMD_STOP_AUTO            CMD_BASE+14
#define IK_CMD_ALL_LEDS				CMD_BASE+15
#define IK_CMD_START_OUTPUT			CMD_BASE+16
#define IK_CMD_STOP_OUTPUT			CMD_BASE+17
#define IK_CMD_ALL_SENSORS			CMD_BASE+18

#if JWH
#define IK_CMD_REFLECT_KEYSTROKE	CMD_BASE+21
#define IK_CMD_REFLECT_MOUSE_MOVE	CMD_BASE+22
#endif

//
//  result codes/data sent to the software
//  see firmware documentation for details
//
#define EVENT_BASE 50
#define IK_EVENT_ACK				EVENT_BASE+1
#define IK_EVENT_MEMBRANE_PRESS		EVENT_BASE+2
#define IK_EVENT_MEMBRANE_RELEASE	EVENT_BASE+3
#define IK_EVENT_SWITCH				EVENT_BASE+4
#define IK_EVENT_SENSOR_CHANGE		EVENT_BASE+5
#define IK_EVENT_VERSION			EVENT_BASE+6
#define IK_EVENT_EEPROM_READ		EVENT_BASE+7
#define IK_EVENT_ONOFFSWITCH	    EVENT_BASE+8
#define IK_EVENT_NOMOREEVENTS		EVENT_BASE+9
#define IK_EVENT_MEMBRANE_REPEAT    EVENT_BASE+10
#define IK_EVENT_SWITCH_REPEAT      EVENT_BASE+11
#define IK_EVENT_CORRECT_MEMBRANE   EVENT_BASE+12
#define IK_EVENT_CORRECT_SWITCH     EVENT_BASE+13
#define IK_EVENT_CORRECT_DONE		EVENT_BASE+14
#define IK_EVENT_EEPROM_READBYTE	EVENT_BASE+15
#define IK_EVENT_DEVICEREADY        EVENT_BASE+16  
#define IK_EVENT_AUTOPILOT_STATE    EVENT_BASE+17
#define IK_EVENT_DELAY              EVENT_BASE+18
#define IK_EVENT_ALL_SENSORS		EVENT_BASE+19


#define IK_FIRSTUNUSED_EVENTCODE    EVENT_BASE+20

//
//  number of light sensors for reading overlay bar codes
//
#define IK_MAX_SENSORS				3

//
//  event reporting mode
//    auto   = data sent to software without specific command
//    polled = data sent to software only when asked
//
#define IK_EVENT_MODE_AUTO			0
#define IK_EVENT_MODE_POLLED		1

