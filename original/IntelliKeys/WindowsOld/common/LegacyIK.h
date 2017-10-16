/*************************************************************************************************************************
// Copyright (c) IntelliTools Inc., 2001. All Rights Reserved.
//
// Class:		LegacyIK.h
//
// Purpose:		defs for Legacy IntelliKeys communications
//
// 06/18/01 fwr, sss	initial implementation	
//
**************************************************************************************************************************/

#ifndef __LEGACYIK_H__
#define __LEGACYIK_H__

#ifndef BYTE
	typedef unsigned char BYTE;
#endif

typedef	struct
{
	BYTE	Custom_ovl_lockout;    
	BYTE	Photo_level_change;    
	BYTE	Keypress_index;   
	BYTE	Lift_off_flag;    
	BYTE	Sound;    
	BYTE	Repeat_enable; 
	BYTE	Repeat_index;  
	BYTE	Repeat_latching;
	BYTE	Mod_latch_mode; 
	BYTE	LED_pairing;    
	BYTE	Mouse_arrows;   
	BYTE	Mouse_speed_index;
	BYTE	Smart_type_flag;  
	BYTE	Custom_level;    
	BYTE	Data_rate_index; 
	BYTE	Second_keyboard; 
	BYTE	Force_cable_val;
	BYTE	AT_multi_arrows;
	BYTE	Coord_mode;
	BYTE	Reserved[5];    
	BYTE	ROMversion2;    
	BYTE	ROMversion1;    
	BYTE	Keyboard_type;  
	BYTE	Photo_val;   
	BYTE	CustOvlName[32];
} LegacyIKSettings;

#define GET_ADDR	1
#define UPLD_SET 	2
#define DNLD_SET	3
#define UPLD_OVL 	4
#define DNLD_OVL	5
#define UPLD_LVN 	6
#define DNLD_LVN	7

#define NO_ERR		 0
#define CHKSUM_ERR	-1
#define TIME_OUT	-2
#define BAD_CMD		-3
#define LOW_MEM		-4
#define NO_INTELL	-5

#endif // __LEGACYIK_H__
