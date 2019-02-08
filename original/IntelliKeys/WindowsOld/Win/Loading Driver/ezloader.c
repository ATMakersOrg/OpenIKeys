//////////////////////////////////////////////////////////////////////
//
// File:      ezloader.c
// $Archive: /USB/Drivers/ezloader/ezloader.c $
//
// Purpose:
//    driver for downloading firmware to pre-renumerated ezusb devices.
//
// Note:
//    derived from ezusbsys.c ver 15

// Environment:
//    kernel mode
//
// $Author: bwilson $
//
// $History: ezloader.c $           
//  
//  *****************  Version 1  *****************
//  User: Tpm          Date: 6/09/00    Time: 6:30p
//  Created in $/USB/Drivers/ezloader
//  
//  *****************  Version 7  *****************
//  User: Markm        Date: 4/12/99    Time: 1:17p
//  Updated in $/EzUsb/Drivers/ezloader
//  
//  *****************  Version 6  *****************
//  User: Markm        Date: 4/12/99    Time: 1:16p
//  Updated in $/EzUsb/Drivers/ezloader
//  
//  *****************  Version 5  *****************
//  User: Markm        Date: 4/12/99    Time: 1:00p
//  Updated in $/EzUsb/Drivers/ezloader
//  minor changes to get rid of compiler warnings.
//  
//  *****************  Version 4  *****************
//  User: Markm        Date: 3/26/99    Time: 2:59p
//  Updated in $/EzUsb/Drivers/ezloader
//  Fixed a bug in the surprise removal code I just added.  I was returning
//  from the PnP dispatch function without unlocking the device object.
//  
//  *****************  Version 3  *****************
//  User: Markm        Date: 3/25/99    Time: 2:05p
//  Updated in $/EzUsb/Drivers/ezloader
//  Added code to allow unplugs (surprise removal) under NT5 without
//  notifying the user.
//  
//  *****************  Version 2  *****************
//  User: Markm        Date: 4/10/98    Time: 2:06p
//  Updated in $/EZUSB/ezloader
//  modified to download intel hex records and to download the loader
//  firmware for downloading to external RAM
//  
//  *****************  Version 1  *****************
//  User: Markm        Date: 2/24/98    Time: 5:26p
//  Created in $/EZUSB/ezloader
//  
//  
// Copyright (c) 1997 Anchor Chips, Inc.  May not be reproduced without
// permission.  See the license agreement for more details.
//
//////////////////////////////////////////////////////////////////////

//
// Include files needed for WDM driver support
//
#include <wdm.h>
#include "stdarg.h"
#include "stdio.h"

//
// Include files needed for USB support
//
#include "usbdi.h"
#include "usbdlib.h"

//
// Include file for the Ezusb Device
//
#include "ezloader.h"

//
// Disable compiler warning about unreferenced function parameters
//
#pragma warning (disable : 4100)

//
// this file contains an image of the device firmware
//
extern INTEL_HEX_RECORD firmware[];
extern INTEL_HEX_RECORD loader[];

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:
   
Arguments:
   DriverObject - pointer to the driver object
   RegistryPath - pointer to a unicode string representing the path
                  to driver-specific key in the registry

Return Value:
   STATUS_SUCCESS if successful,
   STATUS_UNSUCCESSFUL otherwise

--*/
{
   NTSTATUS ntStatus = STATUS_SUCCESS;

   Ezusb_KdPrint (("entering (Ezusb) DriverEntry (Build: %s/%s\n",__DATE__,__TIME__));

   DriverObject->DriverUnload = Ezusb_Unload;

   //
   // POWER and PNP IRPs go to the same dispatch function.  Under
   // Win95, there is just a single IRP for both, called
   // IRP_MJ_PNP_POWER.  This is assigned the same value as
   // IRP_MJ_PNP has under Win98 and NT5.  I'm only concerned
   // with basic PNP stuff, like START and REMOVE.  All other
   // PNP and POWER IRPs will simply be passed down the driver
   // stack.  This driver won't be around like enough to worry
   // about POWER IRPs.  That is, as soon as code is downloaded
   // to the device, the device will remove itself and the driver
   // will go away.
   //
   DriverObject->MajorFunction[IRP_MJ_PNP] =
   DriverObject->MajorFunction[IRP_MJ_POWER] = Ezusb_DispatchPnp;

   DriverObject->DriverExtension->AddDevice = Ezusb_PnPAddDevice;

   Ezusb_KdPrint (("exiting (Ezusb) DriverEntry (%x)\n", ntStatus));

   return ntStatus;
}

NTSTATUS
Ezusb_DefaultPnpHandler(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp
   )
{
   PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;

   IoSkipCurrentIrpStackLocation(Irp);
   return IoCallDriver(pdx->StackDeviceObject, Irp);
}

///////////////////////////////////////////////////////////////////////////////
// @func Handle completion of a request by a lower-level driver
// @parm Functional device object
// @parm I/O request which has completed
// @parm Context argument supplied to IoSetCompletionRoutine, namely address of
// KEVENT object on which ForwardAndWait is waiting
// @comm This is the completion routine used for requests forwarded by ForwardAndWait. It
// sets the event object and thereby awakens ForwardAndWait.
// @comm Note that it's *not* necessary for this particular completion routine to test
// the PendingReturned flag in the IRP and then call IoMarkIrpPending. You do that in many
// completion routines because the dispatch routine can't know soon enough that the
// lower layer has returned STATUS_PENDING. In our case, we're never going to pass a
// STATUS_PENDING back up the driver chain, so we don't need to worry about this.

NTSTATUS 
OnRequestComplete(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp,
   IN PKEVENT pev
   )
/*++

Routine Description:
   Handle completion of a request by a lower-level driver

Arguments:
   DriverObject -  Functional device object
   Irp - I/O request which has completed
   pev - Context argument supplied to IoSetCompletionRoutine, namely address of
         KEVENT object on which ForwardAndWait is waiting

Return Value:
   STATUS_MORE_PROCESSING_REQUIRED
--*/
{
   KeSetEvent(pev, 0, FALSE);
   return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
ForwardAndWait(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp
   )
/*++
Routine Description:
   Forward request to lower level and await completion

   The only purpose of this routine in this particular driver is to pass down
   IRP_MN_START_DEVICE requests and wait for the PDO to handle them.
   
   The processor must be at PASSIVE IRQL because this function initializes
   and waits for non-zero time on a kernel event object.

Arguments:
   fdo - pointer to a device object
   Irp          - pointer to an I/O Request Packet

Return Value:
   STATUS_SUCCESS if successful,
   STATUS_UNSUCCESSFUL otherwise
--*/
{
	KEVENT event;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;
	NTSTATUS ntStatus;

   ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	
	//
   // Initialize a kernel event object to use in waiting for the lower-level
	// driver to finish processing the object. 
   //
	KeInitializeEvent(&event, NotificationEvent, FALSE);

	IoCopyCurrentIrpStackLocationToNext(Irp);
	IoSetCompletionRoutine(Irp, (PIO_COMPLETION_ROUTINE) OnRequestComplete,
		(PVOID) &event, TRUE, TRUE, TRUE);

	ntStatus = IoCallDriver(pdx->StackDeviceObject, Irp);

	if (ntStatus == STATUS_PENDING)
	{
      KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
      ntStatus = Irp->IoStatus.Status;
   }

	return ntStatus;
}

NTSTATUS
CompleteRequest(
   IN PIRP Irp,
   IN NTSTATUS status,
   IN ULONG_PTR info
   )
/*++
Routine Description:
   Mark I/O request complete

Arguments:
   Irp - I/O request in question
   status - Standard status code
   info Additional information related to status code

Return Value:
   STATUS_SUCCESS if successful,
   STATUS_UNSUCCESSFUL otherwise
--*/
{
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

   return status;
}

NTSTATUS
Ezusb_DispatchPnp(
   IN PDEVICE_OBJECT fdo,
   IN PIRP           Irp
   )
/*++
Routine Description:
   Process Plug and Play IRPs sent to this device.

Arguments:
   fdo - pointer to a device object
   Irp          - pointer to an I/O Request Packet

Return Value:
   NTSTATUS
--*/
{
   PIO_STACK_LOCATION irpStack;
   ULONG fcn;
   NTSTATUS ntStatus;

   Ezusb_KdPrint (("Enter Ezusb_DispatchPnp\n"));

   if (!LockDevice(fdo))
		return CompleteRequest(Irp, STATUS_DELETE_PENDING, 0);

   //
   // Get a pointer to the current location in the Irp. This is where
   //     the function codes and parameters are located.
   //
   irpStack = IoGetCurrentIrpStackLocation (Irp);

   ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

   fcn = irpStack->MinorFunction;

   switch (fcn)
   {
      case IRP_MN_START_DEVICE:

         Ezusb_KdPrint (("IRP_MN_START_DEVICE\n"));

         ntStatus = Ezusb_HandleStartDevice(fdo,Irp);

         break; //IRP_MN_START_DEVICE

      case IRP_MN_REMOVE_DEVICE:

         Ezusb_KdPrint (("IRP_MN_REMOVE_DEVICE\n"))

         ntStatus = Ezusb_HandleRemoveDevice(fdo,Irp);

         break; //IRP_MN_REMOVE_DEVICE

      case IRP_MN_QUERY_CAPABILITIES:
      {
         //
         // This code swiped from Walter Oney.  Please buy his book!!
         //

      	PDEVICE_CAPABILITIES pdc = irpStack->Parameters.DeviceCapabilities.Capabilities;

         Ezusb_KdPrint (("IRP_MN_QUERY_CAPABILITIES\n"))

         // Check to besure we know how to handle this version of the capabilities structure

	      if (pdc->Version < 1)
         {
		      ntStatus = Ezusb_DefaultPnpHandler(fdo, Irp);
            break;
         }

         ntStatus = ForwardAndWait(fdo, Irp);
	      if (NT_SUCCESS(ntStatus))
   		{						// IRP succeeded
      		pdc = irpStack->Parameters.DeviceCapabilities.Capabilities;
            // setting this field prevents NT5 from notifying the user when the
            // device is removed.
		      pdc->SurpriseRemovalOK = TRUE;
   		}						// IRP succeeded

	      ntStatus = CompleteRequest(Irp, ntStatus, Irp->IoStatus.Information);
      }
         break; //IRP_MN_QUERY_CAPABILITIES


      //
      // All other PNP IRP's are just passed down the stack by the default handler
      //
      default:
        Ezusb_KdPrint (("Passing down unhandled PnP IOCTL MJ=0x%x MN=0x%x\n",
           irpStack->MajorFunction, irpStack->MinorFunction));
        ntStatus = Ezusb_DefaultPnpHandler(fdo, Irp);

   } // switch MinorFunction

	if (fcn != IRP_MN_REMOVE_DEVICE)
      UnlockDevice(fdo);

   Ezusb_KdPrint (("Exit Ezusb_DispatchPnp %x\n", ntStatus));
   return ntStatus;

}//Ezusb_Dispatch


VOID
Ezusb_Unload(
    IN PDRIVER_OBJECT DriverObject
    )
/*++
Routine Description:
    Free all the allocated resources, etc.
    TODO: This is a placeholder for driver writer to add code on unload

Arguments:
    DriverObject - pointer to a driver object

Return Value:
    None
--*/
{
    Ezusb_KdPrint (("enter Ezusb_Unload\n"));
    /*
    // TODO: Free any global resources allocated in DriverEntry
    */
    Ezusb_KdPrint (("exit Ezusb_Unload\n"));
}

NTSTATUS
Ezusb_HandleRemoveDevice(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp
   )
{
   NTSTATUS ntStatus;
   PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;
	pdx->removing = TRUE;
	UnlockDevice(fdo);			// once for LockDevice at start of dispatch
	UnlockDevice(fdo);			// once for initialization during AddDevice
	KeWaitForSingleObject(&pdx->evRemove, Executive, KernelMode, FALSE, NULL);

	// Let lower-level drivers handle this request. Ignore whatever
	// result eventuates.

//   ntStatus = Ezusb_DefaultPnpHandler(fdo, Irp);

//   Ezusb_Cleanup(fdo);

   // Remove the device object

	Ezusb_RemoveDevice(fdo);

   ntStatus = Ezusb_DefaultPnpHandler(fdo, Irp);

   return ntStatus;				// lower-level completed IoStatus already

}


NTSTATUS
Ezusb_HandleStartDevice(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp
   )
{
   NTSTATUS ntStatus;

   //
   // First let all lower-level drivers handle this request.
   //
   ntStatus = ForwardAndWait(fdo, Irp);
	if (!NT_SUCCESS(ntStatus))
		return CompleteRequest(Irp, ntStatus, Irp->IoStatus.Information);

   //
   // now do whatever we need to do to start the device
   //
   ntStatus = Ezusb_StartDevice(fdo);

	return CompleteRequest(Irp, ntStatus, 0);
}

NTSTATUS
Ezusb_StartDevice(
    IN  PDEVICE_OBJECT fdo
    )
/*++

Routine Description:
   Initializes a given instance of the Ezusb Device on the USB.

   Arguments:
      fdo - pointer to the device object for this instance of a
                      Ezusb Device

Return Value:
   NT status code
--*/
{

   Ezusb_KdPrint (("enter Ezusb_StartDevice\n"));

   //
   // First download loader firmware.  The loader firmware implements a vendor
   // specific command that will allow us to anchor load to external ram
   //
   Ezusb_8051Reset(fdo,1);
   Ezusb_DownloadIntelHex(fdo,loader);
   Ezusb_8051Reset(fdo,0);

   //
   // Now download the device firmware
   //
   Ezusb_DownloadIntelHex(fdo,firmware);
   Ezusb_8051Reset(fdo,1);
   Ezusb_8051Reset(fdo,0);

   Ezusb_KdPrint (("exit Ezusb_StartDevice\n"));

   return STATUS_SUCCESS;
}


NTSTATUS
Ezusb_RemoveDevice(
    IN  PDEVICE_OBJECT fdo
    )
/*++

Routine Description:
    Removes a given instance of a Ezusb Device device on the USB.

Arguments:
    fdo - pointer to the device object for this instance of a Ezusb Device

Return Value:
    NT status code

--*/
{
   PDEVICE_EXTENSION pdx;
   NTSTATUS ntStatus = STATUS_SUCCESS;

   Ezusb_KdPrint (("enter Ezusb_RemoveDevice\n"));

   pdx = fdo->DeviceExtension;

   IoDetachDevice(pdx->StackDeviceObject);

   IoDeleteDevice (fdo);

   Ezusb_KdPrint (("exit Ezusb_RemoveDevice (%x)\n", ntStatus));

   return ntStatus;
}

NTSTATUS
Ezusb_PnPAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
/*++
Routine Description:
    This routine is called to create a new instance of the device

Arguments:
    DriverObject - pointer to the driver object for this instance of Ezusb
    PhysicalDeviceObject - pointer to a device object created by the bus

Return Value:
    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise

--*/
{
   NTSTATUS                ntStatus = STATUS_SUCCESS;
   PDEVICE_OBJECT          fdo = NULL;
   PDEVICE_EXTENSION       pdx;

   Ezusb_KdPrint(("enter Ezusb_PnPAddDevice\n"));

   ntStatus = IoCreateDevice (DriverObject,
                              sizeof (DEVICE_EXTENSION),
                              NULL,
                              FILE_DEVICE_UNKNOWN,
                              0,
                              FALSE,
                              &fdo);

   if (NT_SUCCESS(ntStatus))
   {
      pdx = fdo->DeviceExtension;

      //
      // Non plug and play drivers usually create the device object in
      // driver entry, and the I/O manager autimatically clears this flag.
      // Since we are creating the device object ourselves in response to 
      // a PnP START_DEVICE IRP, we need to clear this flag ourselves.
      //
      fdo->Flags &= ~DO_DEVICE_INITIALIZING;

      //
      // This driver uses direct I/O for read/write requests
      //
      fdo->Flags |= DO_DIRECT_IO;
      //
      //
      // store away the Physical device Object
      //
      pdx->PhysicalDeviceObject=PhysicalDeviceObject;

      //
      // Attach to the StackDeviceObject.  This is the device object that what we 
      // use to send Irps and Urbs down the USB software stack
      //
      pdx->StackDeviceObject =
         IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);

      ASSERT (pdx->StackDeviceObject != NULL);

	   pdx->usage = 1;				// locked until RemoveDevice
	   KeInitializeEvent(&pdx->evRemove,
                        NotificationEvent,
                        FALSE);              // set when use count drops to zero
   }

   Ezusb_KdPrint(("exit Ezusb_PnPAddDevice (%x)\n", ntStatus));

   return ntStatus;
}

NTSTATUS
Ezusb_CallUSBD(
    IN PDEVICE_OBJECT fdo,
    IN PURB Urb
    )
/*++

Routine Description:
   Passes a Usb Request Block (URB) to the USB class driver (USBD)

Arguments:
   fdo - pointer to the device object for this instance of an Ezusb Device
   Urb          - pointer to Urb request block

Return Value:
   STATUS_SUCCESS if successful,
   STATUS_UNSUCCESSFUL otherwise

--*/
{
   NTSTATUS ntStatus, status = STATUS_SUCCESS;
   PDEVICE_EXTENSION pdx;
   PIRP irp;
   KEVENT event;
   IO_STATUS_BLOCK ioStatus;
   PIO_STACK_LOCATION nextStack;

   Ezusb_KdPrint (("enter Ezusb_CallUSBD\n"));

   pdx = fdo->DeviceExtension;

   // issue a synchronous request (see notes above)
   KeInitializeEvent(&event, NotificationEvent, FALSE);

   irp = IoBuildDeviceIoControlRequest(
             IOCTL_INTERNAL_USB_SUBMIT_URB,
             pdx->StackDeviceObject,
             NULL,
             0,
             NULL,
             0,
             TRUE, /* INTERNAL */
             &event,
             &ioStatus);

   // Prepare for calling the USB driver stack
   nextStack = IoGetNextIrpStackLocation(irp);
   ASSERT(nextStack != NULL);

   // Set up the URB ptr to pass to the USB driver stack
   nextStack->Parameters.Others.Argument1 = Urb;

   Ezusb_KdPrint (("Calling USB Driver Stack\n"));

   //
   // Call the USB class driver to perform the operation.  If the returned status
   // is PENDING, wait for the request to complete.
   //
   ntStatus = IoCallDriver(pdx->StackDeviceObject,
                         irp);

   Ezusb_KdPrint (("return from IoCallDriver USBD %x\n", ntStatus));

   if (ntStatus == STATUS_PENDING)
   {
      Ezusb_KdPrint (("Wait for single object\n"));

      status = KeWaitForSingleObject(
                    &event,
                    Suspended,
                    KernelMode,
                    FALSE,
                    NULL);

      Ezusb_KdPrint (("Wait for single object, returned %x\n", status));
   }
   else
   {
      ioStatus.Status = ntStatus;
   }

   Ezusb_KdPrint (("URB status = %x status = %x irp status %x\n",
     Urb->UrbHeader.Status, status, ioStatus.Status));

   ntStatus = ioStatus.Status;


   Ezusb_KdPrint(("exit Ezusb_CallUSBD (%x)\n", ntStatus));

   return ntStatus;
}

///////////////////////////////////////////////////////////////////////////////
// @func Lock a SIMPLE device object
// @parm Address of our device extension
// @rdesc TRUE if it was possible to lock the device, FALSE otherwise.
// @comm A FALSE return value indicates that we're in the process of deleting
// the device object, so all new requests should be failed

BOOLEAN LockDevice(
   IN PDEVICE_OBJECT fdo
   )
{
   PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;

   // Increment use count on our device object
   LONG usage = InterlockedIncrement(&pdx->usage);

   // AddDevice initialized the use count to 1, so it ought to be bigger than
   // one now. HandleRemoveDevice sets the "removing" flag and decrements the
   // use count, possibly to zero. So if we find a use count of "1" now, we
   // should also find the "removing" flag set.

   ASSERT(usage > 1 || pdx->removing);

   // If device is about to be removed, restore the use count and return FALSE.
   // If we're in a race with HandleRemoveDevice (maybe running on another CPU),
   // the sequence we've followed is guaranteed to avoid a mistaken deletion of
   // the device object. If we test "removing" after HandleRemoveDevice sets it,
   // we'll restore the use count and return FALSE. In the meantime, if
   // HandleRemoveDevice decremented the count to 0 before we did our increment,
   // its thread will have set the remove event. Otherwise, we'll decrement to 0
   // and set the event. Either way, HandleRemoveDevice will wake up to finish
   // removing the device, and we'll return FALSE to our caller.
   // 
   // If, on the other hand, we test "removing" before HandleRemoveDevice sets it,
   // we'll have already incremented the use count past 1 and will return TRUE.
   // Our caller will eventually call UnlockDevice, which will decrement the use
   // count and might set the event HandleRemoveDevice is waiting on at that point.

   if (pdx->removing)
	{
	   if (InterlockedDecrement(&pdx->usage) == 0)
		   KeSetEvent(&pdx->evRemove, 0, FALSE);
	   return FALSE;
	}

   return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// @func Unlock a SIMPLE device object
// @parm Address of our device extension
// @comm If the use count drops to zero, set the evRemove event because we're
// about to remove this device object.

void UnlockDevice(
   PDEVICE_OBJECT fdo
   )
{
   PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;
   LONG usage = InterlockedDecrement(&pdx->usage);

   ASSERT(usage >= 0);

   if (usage == 0)
   {						// removing device
      ASSERT(pdx->removing);	// HandleRemoveDevice should already have set this
      KeSetEvent(&pdx->evRemove, 0, FALSE);
   }						// removing device
}

NTSTATUS Ezusb_8051Reset(
   PDEVICE_OBJECT fdo,
   UCHAR resetBit
   )
/*++

Routine Description:
   Uses the ANCHOR LOAD vendor specific command to either set or release the
   8051 reset bit in the EZ-USB chip.

Arguments:
   fdo - pointer to the device object for this instance of an Ezusb Device
   resetBit - 1 sets the 8051 reset bit (holds the 8051 in reset)
              0 clears the 8051 reset bit (8051 starts running)
              
Return Value:
   STATUS_SUCCESS if successful,
   STATUS_UNSUCCESSFUL otherwise

--*/
{
   NTSTATUS ntStatus;
   PURB urb = NULL;

   urb = ExAllocatePool(NonPagedPool, 
                       sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));

   if (urb)
   {
      RtlZeroMemory(urb,sizeof(struct  _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));

      urb->UrbHeader.Length = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
      urb->UrbHeader.Function = URB_FUNCTION_VENDOR_DEVICE;

      urb->UrbControlVendorClassRequest.TransferBufferLength = 1;
      urb->UrbControlVendorClassRequest.TransferBuffer = &resetBit;
      urb->UrbControlVendorClassRequest.TransferBufferMDL = NULL;
      urb->UrbControlVendorClassRequest.Request = ANCHOR_LOAD_INTERNAL;
      urb->UrbControlVendorClassRequest.Value = CPUCS_REG;
      urb->UrbControlVendorClassRequest.Index = 0;

      ntStatus = Ezusb_CallUSBD(fdo, urb);
   }
   else
   {
      ntStatus = STATUS_NO_MEMORY;
   }

   if (urb)
      ExFreePool(urb);

   return ntStatus;
}


NTSTATUS Ezusb_DownloadIntelHex(
   PDEVICE_OBJECT fdo,
   PINTEL_HEX_RECORD hexRecord
   )
/*++

Routine Description:
   This function downloads Intel Hex Records to the EZ-USB device.  If any of the hex records
   are destined for external RAM, then the caller must have previously downloaded firmware
   to the device that knows how to download to external RAM (ie. firmware that implements
   the ANCHOR_LOAD_EXTERNAL vendor specific command).

Arguments:
   fdo - pointer to the device object for this instance of an Ezusb Device
   hexRecord - pointer to an array of INTEL_HEX_RECORD structures.  This array
               is terminated by an Intel Hex End record (Type = 1).

Return Value:
   STATUS_SUCCESS if successful,
   STATUS_UNSUCCESSFUL otherwise

--*/
{
   NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
   PURB urb = NULL;
   PINTEL_HEX_RECORD ptr = hexRecord;

   urb = ExAllocatePool(NonPagedPool, 
                       sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));

   if (urb)
   {
      //
      // The download must be performed in two passes.  The first pass loads all of the
      // external addresses, and the 2nd pass loads to all of the internal addresses.
      // why?  because downloading to the internal addresses will probably wipe out the firmware
      // running on the device that knows how to receive external ram downloads.
      //
      //
      // First download all the records that go in external ram
      //
      while (ptr->Type == 0)
      {
         if (!INTERNAL_RAM(ptr->Address))
         {
            RtlZeroMemory(urb,sizeof(struct  _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));

            urb->UrbHeader.Length = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
            urb->UrbHeader.Function = URB_FUNCTION_VENDOR_DEVICE;
            urb->UrbControlVendorClassRequest.TransferBufferLength = ptr->Length;
            urb->UrbControlVendorClassRequest.TransferBuffer = ptr->Data;
            urb->UrbControlVendorClassRequest.Request = ANCHOR_LOAD_EXTERNAL;
            urb->UrbControlVendorClassRequest.Value = ptr->Address;
            urb->UrbControlVendorClassRequest.Index = 0;

            Ezusb_KdPrint (("Downloading %d bytes to 0x%x\n",ptr->Length,ptr->Address));

            ntStatus = Ezusb_CallUSBD(fdo, urb);

            if (!NT_SUCCESS(ntStatus))
               break;
         }
         ptr++;
      }

      //
      // Now download all of the records that are in internal RAM.  Before starting
      // the download, stop the 8051.
      //
      Ezusb_8051Reset(fdo,1);
      ptr = hexRecord;
      while (ptr->Type == 0)
      {
         if (INTERNAL_RAM(ptr->Address))
         {
            RtlZeroMemory(urb,sizeof(struct  _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));

            urb->UrbHeader.Length = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
            urb->UrbHeader.Function = URB_FUNCTION_VENDOR_DEVICE;
            urb->UrbControlVendorClassRequest.TransferBufferLength = ptr->Length;
            urb->UrbControlVendorClassRequest.TransferBuffer = ptr->Data;
            urb->UrbControlVendorClassRequest.Request = ANCHOR_LOAD_INTERNAL;
            urb->UrbControlVendorClassRequest.Value = ptr->Address;
            urb->UrbControlVendorClassRequest.Index = 0;

            Ezusb_KdPrint (("Downloading %d bytes to 0x%x\n",ptr->Length,ptr->Address));

            ntStatus = Ezusb_CallUSBD(fdo, urb);

            if (!NT_SUCCESS(ntStatus))
               break;
         }
         ptr++;
      }

   }
   else
   {
      ntStatus = STATUS_NO_MEMORY;
   }

   if (urb)
      ExFreePool(urb);

   return ntStatus;
}
