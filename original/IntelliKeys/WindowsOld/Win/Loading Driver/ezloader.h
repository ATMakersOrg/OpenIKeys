//////////////////////////////////////////////////////////////////////
//
// File:      ezloader.h
// $Archive: /USB/Drivers/ezloader/ezloader.h $
//
// Purpose:
//    Header file for the Ezloader device driver
//
// Environment:
//    kernel mode
//
// $Author: bwilson $
//
// $History: ezloader.h $           
//  
//  *****************  Version 1  *****************
//  User: Tpm          Date: 6/09/00    Time: 6:30p
//  Created in $/USB/Drivers/ezloader
//  
//  *****************  Version 2  *****************
//  User: Markm        Date: 4/10/98    Time: 2:06p
//  Updated in $/EZUSB/ezloader
//  Support for downloading Intel Hex
//  
//  *****************  Version 1  *****************
//  User: Markm        Date: 2/24/98    Time: 5:26p
//  Created in $/EZUSB/ezloader
//  
// Copyright (c) 1997 Anchor Chips, Inc.  May not be reproduced without
// permission.  See the license agreement for more details.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BYTE_DEFINED
#define _BYTE_DEFINED
typedef unsigned char BYTE;
#endif // !_BYTE_DEFINED

#ifndef _WORD_DEFINED
#define _WORD_DEFINED
typedef unsigned short WORD;
#endif // !_WORD_DEFINED


#ifdef DRIVER

//
// Vendor specific request code for Anchor Upload/Download
//
// This one is implemented in the core
//
#define ANCHOR_LOAD_INTERNAL  0xA0

//
// This command is not implemented in the core.  Requires firmware
//
#define ANCHOR_LOAD_EXTERNAL  0xA3

//
// This is the highest internal RAM address for the AN2131Q
//
#define MAX_INTERNAL_ADDRESS  0x1B3F

#define INTERNAL_RAM(address) ((address <= MAX_INTERNAL_ADDRESS) ? 1 : 0)

//
// EZ-USB Control and Status Register.  Bit 0 controls 8051 reset
//
#define CPUCS_REG    0x7F92
//
// A structure representing the instance information associated with
// this particular device.
//
typedef struct _DEVICE_EXTENSION
{

   // physical device object
   PDEVICE_OBJECT PhysicalDeviceObject;        

   // Device object we call when submitting Urbs/Irps to the USB stack
   PDEVICE_OBJECT		StackDeviceObject;		

   // use counter for the device.  Gets incremented when the driver receives
   // a request and gets decremented when a request s completed.
   LONG usage;

   // this ev gets set when it is ok to remove the device
	KEVENT evRemove;

   // TRUE if we're trying to remove this device
   BOOLEAN removing;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define MAX_INTEL_HEX_RECORD_LENGTH 16

typedef struct _INTEL_HEX_RECORD
{
   BYTE  Length;
   WORD  Address;
   BYTE  Type;
   BYTE  Data[MAX_INTEL_HEX_RECORD_LENGTH];
} INTEL_HEX_RECORD, *PINTEL_HEX_RECORD;

#if DBG

#define Ezusb_KdPrint(_x_) 
#define Ezusb_KdPrint2(_x_) DbgPrint("Ezusb.SYS: "); \
                             DbgPrint _x_ ;
#define TRAP() DbgBreakPoint()
#else
#define Ezusb_KdPrint(_x_)
#define Ezusb_KdPrint2(_x_)
#define TRAP()
#endif


VOID
Ezusb_Unload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
Ezusb_StartDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
Ezusb_RemoveDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
Ezusb_CallUSBD(
    IN PDEVICE_OBJECT DeviceObject,
    IN PURB Urb
    );

NTSTATUS
Ezusb_PnPAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
ForwardAndWait(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );

NTSTATUS
Ezusb_DefaultPnpHandler(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp
   );

NTSTATUS
Ezusb_DispatchPnp(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   );

NTSTATUS
Ezusb_HandleStartDevice(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );

NTSTATUS
Ezusb_HandleRemoveDevice(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp
   );

NTSTATUS 
OnRequestComplete(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp,
   IN PKEVENT pev
   );

NTSTATUS
CompleteRequest(
   IN PIRP Irp,
   IN NTSTATUS status,
   IN ULONG_PTR info
   );

BOOLEAN LockDevice(
   IN PDEVICE_OBJECT fdo
   );

void UnlockDevice(
   PDEVICE_OBJECT fdo
   );

NTSTATUS Ezusb_8051Reset(
   PDEVICE_OBJECT fdo,
   UCHAR resetBit
   );

NTSTATUS Ezusb_DownloadIntelHex(
   PDEVICE_OBJECT fdo,
   PINTEL_HEX_RECORD hexRecord
   );

#endif      //DRIVER section


