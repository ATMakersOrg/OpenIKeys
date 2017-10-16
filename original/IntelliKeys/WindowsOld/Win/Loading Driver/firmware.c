//////////////////////////////////////////////////////////////////////
//
// File:      firmware.c
// $Archive: /USB/Drivers/ezloader/firmware.c $
//
// Purpose:
//
//    The firmware array below should be replaced with your own firmware.
//
//
// Environment:
//    kernel mode
//
// $Author: bwilson $
//
// $History: firmware.c $           
//  
//  *****************  Version 1  *****************
//  User: Tpm          Date: 6/09/00    Time: 6:30p
//  Created in $/USB/Drivers/ezloader
//  
//  *****************  Version 2  *****************
//  User: Markm        Date: 4/10/98    Time: 2:08p
//  Updated in $/EZUSB/ezloader
//  
//  
// Copyright (c) 1997 Anchor Chips, Inc.  May not be reproduced without
// permission.  See the license agreement for more details.
//
//////////////////////////////////////////////////////////////////////
#include <wdm.h>
#include "usbdi.h"
#include "usbdlib.h"
#include "ezloader.h"

#ifdef SHOP
	#include "EzLoader_Firmware_Scribbler.c"
#else
	#include "EzLoader_Firmware.c"
#endif