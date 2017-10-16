
#include <IOKit/IOMessage.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <unistd.h>

//  define this to skip interaction with the engine
//#define NO_ENGINE  1

//  define this to get no reading and no asynch event source
//#define NO_READING 1  

//  define this to use Apple's firmware from USBNotification Example
//#define USE_APPLE_FIRMWARE 1

//  define this to create a simple test program
//#define SIMPLE_TEST 1

//  define this to run the engine out of a run loop timer instead of
//  in a separate thread.
//#define ENGINE_IN_TIMER 1

//  app-specific includes
#include "IKCommon.h"
#include "IKEngine.h"
#include "IKUtil.h"
#include "IKFile.h"

#ifdef NO_ENGINE
  //#define BYTE unsigned char
  //#define MAX_INTELLIKEYS 10
  //#define TRACE0(f)             {char m[255]; sprintf(m,f);            printf(m); printf("\n");}
  //#define TRACE1(f,p1)          {char m[255]; sprintf(m,f,p1);         printf(m); printf("\n");}
  //#define TRACE2(f,p1,p2)       {char m[255]; sprintf(m,f,p1,p2);      printf(m); printf("\n");}
  //#define TRACE3(f,p1,p2,p3)    {char m[255]; sprintf(m,f,p1,p2,p3);   printf(m); printf("\n");}
  //#define TRACE4(f,p1,p2,p3,p4) {char m[255]; sprintf(m,f,p1,p2,p3,p4);printf(m); printf("\n");}
  //#define IK_REPORT_LEN 8
  
  //#define TEXT(a) a
  //#define IKTRACE(a)
  
#else
  //  one and only instance of our engine
  static IKEngine theEngine;
#endif

//  firmware
#define MAX_INTEL_HEX_RECORD_LENGTH 16
typedef struct _INTEL_HEX_RECORD
{
   UInt32  	Length;
   UInt32 	Address;
   UInt32  	Type;
   UInt8  	Data[MAX_INTEL_HEX_RECORD_LENGTH];
} INTEL_HEX_RECORD, *PINTEL_HEX_RECORD;
extern INTEL_HEX_RECORD firmware[];

//  for firmware downloading
#define	k8051_USBCS		0x7f92

//  our vendor and product IDs
#define kOurVendorIDRaw	2398
#define kOurProductIDRaw	256
#ifdef USE_APPLE_FIRMWARE
  #include "AppleFirmware.c"
  #define kOurVendorIDLoaded	1351
  #define kOurProductIDLoaded	4098
#else
  #include "EzLoader_Firmware.c"
  #define kOurVendorIDLoaded	2398
  #define kOurProductIDLoaded	257
  static int IK_VID = kOurVendorIDLoaded;
  static int IK_PID = kOurProductIDLoaded;
  static int IS_VID = 0;
  static int IS_PID = 0;
#endif

// globals
static 	IONotificationPortRef	gNotifyPort;
static 	io_iterator_t		gRawAddedIter;
static 	io_iterator_t		gRawRemovedIter;
static 	io_iterator_t		gLoadedAddedIter;
static 	io_iterator_t		gLoadedRemovedIter;
static 	io_iterator_t		gIntelliSwitchAddedIter;
static 	io_iterator_t		gIntelliSwitchRemovedIter;
static	pthread_t		thread;

static mach_port_t 		gMasterPort;

typedef struct {
    int inPipe;
    int outPipe;
    IOUSBInterfaceInterface **ppif;
    int numBytesRead;
    BYTE readBuffer[IK_REPORT_LEN];
    UInt32 locationID;
    io_object_t notification;
	int devType;
} device;

static device devices[MAX_INTELLIKEYS];


void DeviceNotification(void *refCon, io_service_t service, natural_t messageType, void *messageArgument)
{
    if (messageType==kIOMessageServiceIsTerminated)
    {
	UInt32 locationID = (UInt32) refCon;
	IKTRACE(( TEXT("main: DeviceNotification for %d"), locationID ));
	
	//  find us in the table
	int index = -1;
	for (int i=0;i<MAX_INTELLIKEYS;i++)
	{
	    if (locationID == devices[i].locationID)
	    {
		IKTRACE((TEXT("main: DeviceNotification found location %d at index %d"),locationID,i));
		index = i;
		break;
	    }
	}
	
	if (index!=-1)
	{
	    //  tell engine to forget us
	    #ifndef NO_ENGINE
	    theEngine.RemoveDevice ( index );
	    #endif
	
	    //  kill outstanding i/o ?
	    if (devices[index].inPipe != 0)
	    {
		(void) (*devices[index].ppif)->AbortPipe(devices[index].ppif,devices[index].inPipe);
	    }
	    if (devices[index].outPipe != 0)
	    {
		(void) (*devices[index].ppif)->AbortPipe(devices[index].ppif,devices[index].outPipe);
	    }
	
	    //  close interface ?
	    if (devices[index].ppif)
	    {
		(void) (*devices[index].ppif)->USBInterfaceClose(devices[index].ppif);
		(void) (*devices[index].ppif)->Release(devices[index].ppif);
	    }
	    
	    //  release notification object
	    IOObjectRelease(devices[index].notification);
	
	    //  clear the device list entry
	    devices[index].inPipe	= 0;
	    devices[index].outPipe	= 0;
	    devices[index].ppif		= 0;
	    devices[index].numBytesRead = 0;
	    devices[index].locationID   = 0;
	    devices[index].notification = 0;

	}
	else
	{
	    IKTRACE((TEXT("main: DeviceNotification did not find location %d"),locationID));
	}
	
    }
}

int AllocateDeviceIndex (IOUSBDeviceInterface **dev, io_service_t usbDevice, int devType)
{
    //  get the location ID
    UInt32 locationID;
    IOReturn kr = (*dev)->GetLocationID(dev, &locationID);
    if (kr != KERN_SUCCESS)
    {
	IKTRACE((TEXT("main: error %d getting location ID for allocation"),kr));
	return 0;
    }
    IKTRACE((TEXT("main: allocated for loaction id=%d"),locationID));
    
    //  find an empty slot
    int slot = -1;
    for (int i=0;i<MAX_INTELLIKEYS;i++)
    {
		if (devices[i].locationID==0)
		{
			//  use this slot
			slot = i;
			break;
		}
    }
    if (slot!=-1)
    {
		IKTRACE((TEXT("main: allocated location %d at index %d"),locationID,slot));
		devices[slot].locationID = locationID;
		devices[slot].devType = devType;
		//  add an interest notification
		kr = IOServiceAddInterestNotification 
			( gNotifyPort, usbDevice, kIOGeneralInterest, DeviceNotification,
			  (void *)locationID, &(devices[slot].notification) );
		return slot;
    }

    IKTRACE((TEXT("main: error allocating index, no free slot.")));
    return 0;
}


void WriteRoutine ( int index, BYTE *data )
{
    if (devices[index].outPipe==0)
    {
	IKTRACE((TEXT("main: error writing, pipe is zero")));
	return;
    }
    
    if (devices[index].ppif==0)
    {
	IKTRACE((TEXT("main: error writing, interface is zero")));
	return;
    }
    
    IOReturn kr;
	IOReturn status = (*devices[index].ppif)->GetPipeStatus (devices[index].ppif, devices[index].outPipe);
	if (status == kIOUSBPipeStalled) {
		(*devices[index].ppif)->ClearPipeStall (devices[index].ppif, devices[index].outPipe);
	}
    kr = (*devices[index].ppif)->WritePipe(devices[index].ppif, 
	    devices[index].outPipe, (char *) data, IK_REPORT_LEN);
    if (kr != kIOReturnSuccess)
    {
	IKTRACE((TEXT("main: error writing, kr=%d"), kr));
    }

}

#ifdef SIMPLE_TEST

#define JWH  1
#include "firmware.h"
#include "IKUniversal.h"
#include "IKUniversalToHID.h"

//  for keyboard and mouse reflection
static BYTE m_KeyBoardReport[7] = {0,0,0,0,0,0,0};
static BYTE m_MouseReport[3] = {0,0,0};


static void CvtUniversalToHID(int universalCode, int *usagePage, int *usageID)
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

static void DoKey(int index, int code, int direction)
{
	//  convert to HID
	int usagePage, usageID;
	CvtUniversalToHID ( code, &usagePage, &usageID );
	
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
		}
	}
	
	BYTE msg[IK_REPORT_LEN] = {IK_CMD_REFLECT_KEYSTROKE,0,0,0,0,0,0,0};
	for (unsigned int j=0;j<sizeof(m_KeyBoardReport);j++)
		msg[j+1] = m_KeyBoardReport[j];
	
	WriteRoutine ( index, msg );
}

static void TypeKey ( int index, int code )
{
	DoKey ( index, code, IK_DOWN);
	usleep ( 10*1000 );
	DoKey ( index, code, IK_UP);
	usleep ( 10*1000 );
}

static void OnDataReceived(int index,BYTE *buffer)
{
	if (buffer[0]==IK_EVENT_SWITCH && buffer[2]==1)  //  switch down
	{
		char message[100];
		
		TypeKey ( index, UNIVERSAL_S );
		TypeKey ( index, UNIVERSAL_W );
		TypeKey ( index, UNIVERSAL_I );
		TypeKey ( index, UNIVERSAL_T );
		TypeKey ( index, UNIVERSAL_C );
		TypeKey ( index, UNIVERSAL_H );
		
		TypeKey ( index, UNIVERSAL_SPACE );
		
		TypeKey ( index, UNIVERSAL_0 + buffer[1] );
		
		TypeKey ( index, UNIVERSAL_ENTER );
	}
}
#endif

static void StartAsyncRead(int index);

static void ReadCompletion ( void *refCon, IOReturn result, void *arg0 )
{
    //  what's our index?
    int index = (int) refCon;
    
    //  check read success
    if (result != kIOReturnSuccess)
    {
        IKTRACE((TEXT("main: error %d in read completion"),result));
        return;
        
        //(void) (*devices[index].ppif)->USBInterfaceClose(devices[index].ppif);
        //(void) (*devices[index].ppif)->Release(devices[index].ppif);
        //devices[index].ppif = 0;
    }
    
    //TRACE1 ( "main: ReadCompletion %d", devices[index].readBuffer[0]);
	
#ifdef SIMPLE_TEST
	OnDataReceived(index,devices[index].readBuffer);
    StartAsyncRead(index);
	return;
#endif

    //  do something with the data
	
#ifndef NO_ENGINE
    theEngine.OnDataReceived(index,devices[index].readBuffer);
#endif
    
    //  hang another one
    StartAsyncRead(index);
}


static void StartAsyncRead(int index)
{        
    if (!devices[index].ppif)
    {
        IKTRACE((TEXT("main: ppif is null in StartAsyncRead")));
        return;
    }
    
    if (devices[index].inPipe==0)
    {
        IKTRACE((TEXT("main: inPipe is 0 in StartAsyncReadMac")));
        return;
    }
        
    IOReturn result = 98765;
    devices[index].numBytesRead = IK_REPORT_LEN;
    int n = 0;
    while (result != kIOReturnSuccess)
    {
        result = (*devices[index].ppif)->ReadPipeAsync(devices[index].ppif, devices[index].inPipe, devices[index].readBuffer, 
                devices[index].numBytesRead, ReadCompletion, (void *)index);
                
        if ( result != kIOReturnSuccess )
        {
            IKTRACE((TEXT("main: unable to start async read (%08x)"), result));
            usleep(1000);
            n = n + 1;
        }
                
        //if (kIOReturnSuccess != result)
        //{
            //IKTRACE((TEXT("main: unable to start async read (%08x)"), result));
            //(void) (*devices[index].ppif)->USBInterfaceClose(devices[index].ppif);
            //(void) (*devices[index].ppif)->Release(devices[index].ppif);
            //devices[index].ppif = 0;
        // return;
        //}   
    
    }     
}


static IOReturn FindInterfaces(IOUSBDeviceInterface **dev, int index)
{
    IOReturn			kr;
    IOUSBFindInterfaceRequest	request;
    io_iterator_t		iterator;
    io_service_t		usbInterface;
    IOCFPlugInInterface 	**plugInInterface = NULL;
    IOUSBInterfaceInterface 	**intf = NULL;
    HRESULT 			res;
    SInt32 			score;
    UInt8			intfClass;
    UInt8			intfSubClass;
    UInt8			intfNumEndpoints;
    int				pipeRef;
    CFRunLoopSourceRef		runLoopSource;
    
    
    request.bInterfaceClass = kIOUSBFindInterfaceDontCare;
    request.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
    request.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
    request.bAlternateSetting = kIOUSBFindInterfaceDontCare;
   
    kr = (*dev)->CreateInterfaceIterator(dev, &request, &iterator);
    
    while ((usbInterface = IOIteratorNext(iterator)))
    {
        IKTRACE((TEXT("main: Interface found.")));
       
        kr = IOCreatePlugInInterfaceForService(usbInterface, kIOUSBInterfaceUserClientTypeID, 
            kIOCFPlugInInterfaceID, &plugInInterface, &score);
        (void) IOObjectRelease(usbInterface);  // done with the usbInterface object now that I have the plugin
        if ((kIOReturnSuccess != kr) || !plugInInterface)
        {
            IKTRACE((TEXT("main: unable to create a plugin (%08x)"), kr));
            break;
        }
		IKTRACE((TEXT("main: got plugin interface")));
            
        // I have the interface plugin. I need the interface interface
        res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID), 
                /*(LPVOID)*/ (void **) &intf);
        (*plugInInterface)->Release(plugInInterface);			// done with this
        if (res || !intf)
        {
            IKTRACE((TEXT("main: couldn't create an IOUSBInterfaceInterface (%08x)"), (int) res));
            break;
        }
		IKTRACE((TEXT("main: created IOUSBInterfaceInterface.")));
        
        kr = (*intf)->GetInterfaceClass(intf, &intfClass);
        kr = (*intf)->GetInterfaceSubClass(intf, &intfSubClass);
        
        IKTRACE((TEXT("main: Interface class %d, subclass %d"), intfClass, intfSubClass));
        
        // Now open the interface. This will cause the pipes to be instantiated that are 
        // associated with the endpoints defined in the interface descriptor.
        kr = (*intf)->USBInterfaceOpen(intf);
        if (kIOReturnSuccess != kr)
        {
            IKTRACE((TEXT("main: unable to open interface (%08x)"), kr));
            (void) (*intf)->Release(intf);
            break;
        }
		IKTRACE((TEXT("main: opened interface")));
        
    	kr = (*intf)->GetNumEndpoints(intf, &intfNumEndpoints);
        if (kIOReturnSuccess != kr)
        {
            IKTRACE((TEXT("main: unable to get number of endpoints (%08x)"), kr));
            (void) (*intf)->USBInterfaceClose(intf);
            (void) (*intf)->Release(intf);
            break;
        }
        IKTRACE((TEXT("main: Interface has %d endpoints."), intfNumEndpoints));
        
        //  figure out which pipe is which.
        devices[index].inPipe  = 0;
        devices[index].outPipe = 0;
        for (pipeRef = 1; pipeRef <= intfNumEndpoints; pipeRef++)
        {
            IOReturn	kr2;
            UInt8	direction;
            UInt8	number;
            UInt8	transferType;
            UInt16	maxPacketSize;
            UInt8	interval;
            kr2 = (*intf)->GetPipeProperties(intf, pipeRef, &direction, &number, 
                &transferType, &maxPacketSize, &interval);
            if (kIOReturnSuccess == kr)
            {
                if (direction==kUSBIn && transferType==kUSBInterrupt)
                    devices[index].inPipe = pipeRef;
                else if (direction==kUSBOut && transferType==kUSBInterrupt)
                    devices[index].outPipe = pipeRef;
		else
		{
		    IKTRACE((TEXT("main: unsupported pipe ref=%d, direction=%d, transfer type=%d"),pipeRef,direction,transferType));
		}
            }
         }                 
        
        //  we got the interface.  Save it.
        devices[index].ppif = intf;

#ifndef NO_READING
        // Just like with service matching notifications, we need to create an event source and add it 
        //  to our run loop in order to receive async completion notifications.
        kr = (*intf)->CreateInterfaceAsyncEventSource(intf, &runLoopSource);
        if (kIOReturnSuccess != kr)
        {
            IKTRACE((TEXT("unable to create async event source (%08x)"), kr));
            (void) (*intf)->USBInterfaceClose(intf);
            (void) (*intf)->Release(intf);
            devices[index].ppif = 0;
            break;
        }
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
        IKTRACE((TEXT("main: Async event source added to run loop.")));
#endif
	
	//  now tell the engine about us.
#ifndef NO_ENGINE
	theEngine.AddDevice ( index, WriteRoutine, devices[index].devType );
#endif

        // We just want to use the first interface, so exit the loop.
        break;
    }
    
    return kr;
}


static IOReturn ConfigureDevice(IOUSBDeviceInterface **dev)
{
    //  do nothing here because the composite driver has already configured us.
    return kIOReturnSuccess;


}



static void OpenDevice ( io_service_t usbDevice, int devType )
{
    kern_return_t kr;

    //  get a plugin
    SInt32 score;
    IOCFPlugInInterface **plugInInterface=NULL;
    kr = IOCreatePlugInInterfaceForService ( usbDevice, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score);
    if (kr!=kIOReturnSuccess || !plugInInterface)
    {
	IKTRACE((TEXT("main: error getting plugin interface")));
	return;
    }
    
    //  get a device interface
    HRESULT res;
    IOUSBDeviceInterface **dev=NULL;
    res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (void **) &dev);
    (*plugInInterface)->Release(plugInInterface);  // done with this
    if (res || !dev)
    {
	IKTRACE((TEXT("main: error getting device interface")));
	return;
    }
    
    //  make sure this is the device we want
    UInt16 vendor=0, product=0;  //  , release=0;
    kr = (*dev)->GetDeviceVendor(dev, &vendor);
    kr = (*dev)->GetDeviceProduct(dev, &product);
    //kr = (*dev)->GetDeviceReleaseNumber(dev, &release);

	//  TODO:  remove this for now.
	//  we really should check for both IK and IS.
#if 0
    if ((vendor != IK_VID) || (product != IK_PID))
    {
	IKTRACE((TEXT("main: error - device mismatch %d %d"),vendor,product));
	(*dev)->Release(dev);
	return;
    }
#endif

    IKTRACE((TEXT("main: this device matches.")));
    
    // open the device 
    kr = (*dev)->USBDeviceOpen(dev);
    if (kIOReturnSuccess != kr)
    {
	IKTRACE((TEXT("unable to open device: %08x"), kr));
	(*dev)->Release(dev);
	return;
    }
    IKTRACE((TEXT("main: device opened.")));

    //  configure the device
    kr = ConfigureDevice(dev);
    if (kIOReturnSuccess != kr)
    {
	IKTRACE((TEXT("main: unable to configure device: %08x"), kr));
	(*dev)->USBDeviceClose(dev);
	(*dev)->Release(dev);
	return;
    }
    IKTRACE((TEXT("main: device configured.")));
    
    //  figure out which device this is
    int index = AllocateDeviceIndex(dev,usbDevice,devType);
    
    //  find interfaces
    kr = FindInterfaces(dev,index);
    if (kIOReturnSuccess != kr)
    {
	IKTRACE((TEXT("main: unable to find interfaces on device: %08x"), kr));
	(*dev)->USBDeviceClose(dev);
	(*dev)->Release(dev);
	return;
    }
    IKTRACE((TEXT("main: interfaces found.")));

#ifndef NO_READING
    //  start reading
    StartAsyncRead(index);
    IKTRACE((TEXT("main: async reading begun.")));
#endif
    
    //  all done.  We're doing asynch i/o, so we do not close the device
    //  or release the device interface.  Why not?
    return;

}



//
//  called when the loaded device is added
//

static void LoadedDeviceAdded(void *refCon, io_iterator_t iterator)
{
    io_service_t		usbDevice;
    kern_return_t		kr;    

    while ((usbDevice = IOIteratorNext(iterator)))
    {
		IKTRACE((TEXT("main: LoadedDeviceAdded")));
		OpenDevice (usbDevice,1);
        kr = IOObjectRelease(usbDevice);
    }
}

//
//  called when IntelliSwitch is added
//

static void IntelliSwitchDeviceAdded(void *refCon, io_iterator_t iterator)
{
    io_service_t		usbDevice;
    kern_return_t		kr;    

    while ((usbDevice = IOIteratorNext(iterator)))
    {
		IKTRACE((TEXT("main: IntelliSwitchDeviceAdded")));
		OpenDevice (usbDevice,2);
        kr = IOObjectRelease(usbDevice);
    }

}

//
//  called when the loaded device is removed
//
static void LoadedDeviceRemoved(void *refCon, io_iterator_t iterator)
{
    kern_return_t	kr;
    io_service_t	usbDevice;
	
    while ((usbDevice = IOIteratorNext(iterator)))
    {
		IKTRACE((TEXT("main: LoadedDeviceRemoved")));
		//  CloseDevice ( usbDevice );
        kr = IOObjectRelease(usbDevice);
    }
	
}

//
//  called when IntelliSwitch is removed
//
static void IntelliSwitchDeviceRemoved(void *refCon, io_iterator_t iterator)
{
    kern_return_t	kr;
    io_service_t	usbDevice;
	
    while ((usbDevice = IOIteratorNext(iterator)))
    {
		IKTRACE((TEXT("main: IntelliSwitchDeviceRemoved")));
		//  CloseDevice ( usbDevice );
        kr = IOObjectRelease(usbDevice);
    }
	
}

//
//  write data to the raw device
//
IOReturn AnchorWrite(IOUSBDeviceInterface **dev, UInt16 anchorAddress, UInt16 count, UInt8 writeBuffer[])
{
    IOUSBDevRequest 		request;
    
    request.bmRequestType = USBmakebmRequestType(kUSBOut, kUSBVendor, kUSBDevice);
    request.bRequest = 0xa0;
    request.wValue = anchorAddress;
    request.wIndex = 0;
    request.wLength = count;
    request.pData = writeBuffer;

    return (*dev)->DeviceRequest(dev, &request);
}


//
//  called to download firmware to the raw device
//
static IOReturn DownloadToRawDevice(IOUSBDeviceInterface **dev)
{
    int		i;
    UInt8 	writeVal;
    IOReturn	kr;
    
    // Assert reset
    writeVal = 1;
    kr = AnchorWrite(dev, k8051_USBCS, 1, &writeVal);
    if (kIOReturnSuccess != kr) 
    {
        IKTRACE((TEXT("main: AnchorWrite reset returned err 0x%x!"), kr));
        (*dev)->USBDeviceClose(dev);
        (*dev)->Release(dev);
        return kr;
    }
    
    i = 0;
    // Download code
    while (firmware[i].Type == 0) 
    {
	//TRACE3("main: downloading block %d address %d length %d",i,firmware[i].Address, firmware[i].Length);
        kr = AnchorWrite(dev, firmware[i].Address, firmware[i].Length, firmware[i].Data);
        if (kIOReturnSuccess != kr) 
        {
            IKTRACE((TEXT("main: AnchorWrite download %i returned err 0x%x!"), i, kr));
            (*dev)->USBDeviceClose(dev);
            (*dev)->Release(dev);
            return kr;
        }
        i++;
    }

    // De-assert reset
    writeVal = 0;
    kr = AnchorWrite(dev, k8051_USBCS, 1, &writeVal);
    if (kIOReturnSuccess != kr) 
    {
        IKTRACE((TEXT("main: AnchorWrite run returned err 0x%x!"), kr));
    }
    
    return kr;
}


//
//  Called to configure the raw device
//
static IOReturn ConfigureRawDevice(IOUSBDeviceInterface **dev)
{
    UInt8				numConf;
    IOReturn				kr;
    IOUSBConfigurationDescriptorPtr	confDesc;
    
    kr = (*dev)->GetNumberOfConfigurations(dev, &numConf);
    if (!numConf)
        return -1;
    
    // get the configuration descriptor for index 0
    kr = (*dev)->GetConfigurationDescriptorPtr(dev, 0, &confDesc);
    if (kr)
    {
        IKTRACE((TEXT("main: \tunable to get config descriptor for index %d (err = %08x)"), 0, kr));
        return -1;
    }
    kr = (*dev)->SetConfiguration(dev, confDesc->bConfigurationValue);
    if (kr)
    {
        IKTRACE((TEXT("main: \tunable to set configuration to value %d (err=%08x)"), 0, kr));
        return -1;
    }
    
    return kIOReturnSuccess;
}

//
//  called when the raw device is added
//
static void RawDeviceAdded(void *refCon, io_iterator_t iterator)
{
    kern_return_t		kr;
    io_service_t		usbDevice;
    IOCFPlugInInterface 	**plugInInterface=NULL;
    IOUSBDeviceInterface 	**dev=NULL;
    HRESULT 			res;
    SInt32 			score;
    UInt16			vendor;
    UInt16			product;
    UInt16			release;
        
    while ((usbDevice = IOIteratorNext(iterator)))
    {
        IKTRACE((TEXT("main: Raw device added.")));
       
        kr = IOCreatePlugInInterfaceForService(usbDevice, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score);
        kr = IOObjectRelease(usbDevice);				// done with the device object now that I have the plugin
        if ((kIOReturnSuccess != kr) || !plugInInterface)
        {
            IKTRACE((TEXT("main: unable to create a plugin (%08x)"), kr));
            continue;
        }
            
        // I have the device plugin, I need the device interface
        res = (*plugInInterface)->QueryInterface(plugInInterface, 
            CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), /*(LPVOID)*/(void **)&dev);
        (*plugInInterface)->Release(plugInInterface);			// done with this
        if (res || !dev)
        {
            IKTRACE((TEXT("main: couldn't create a device interface (%08x)"), (int) res));
            continue;
        }
        // technically should check these kr values
        kr = (*dev)->GetDeviceVendor(dev, &vendor);
        kr = (*dev)->GetDeviceProduct(dev, &product);
        kr = (*dev)->GetDeviceReleaseNumber(dev, &release);
        //if ((vendor != kOurVendorIDRaw) || (product != kOurProductID) || (release != 1))
        if ((vendor != kOurVendorIDRaw) || (product != kOurProductIDRaw) )
        {
            // We should never get here because the matching criteria we specified above
            // will return just those devices with our vendor and product IDs
            IKTRACE((TEXT("main: found device i didn't want (vendor = %d, product = %d)"), vendor, product));
            (void) (*dev)->Release(dev);
            continue;
        }

        // need to open the device in order to change its state
        kr = (*dev)->USBDeviceOpen(dev);
        if (kIOReturnSuccess != kr)
        {
            IKTRACE((TEXT("main: unable to open device: %08x"), kr));
            (void) (*dev)->Release(dev);
            continue;
        }
        kr = ConfigureRawDevice(dev);
        if (kIOReturnSuccess != kr)
        {
            IKTRACE((TEXT("main: unable to configure device: %08x"), kr));
            (void) (*dev)->USBDeviceClose(dev);
            (void) (*dev)->Release(dev);
            continue;
        }

        kr = DownloadToRawDevice(dev);
        if (kIOReturnSuccess != kr)
        {
            IKTRACE((TEXT("main: unable to download to device: %08x"), kr));
            (void) (*dev)->USBDeviceClose(dev);
            (void) (*dev)->Release(dev);
            continue;
        }

        kr = (*dev)->USBDeviceClose(dev);
        kr = (*dev)->Release(dev);
    }
}

//
//  called when the raw device is removed
//
static void RawDeviceRemoved(void *refCon, io_iterator_t iterator)
{
    kern_return_t	kr;
    io_service_t	obj;

    while ((obj = IOIteratorNext(iterator)))
    {
        IKTRACE((TEXT("main: Raw device removed.")));
        kr = IOObjectRelease(obj);
    }

} 

//
//  called to set up all of the notifications
//
static int SetupNotifications()
{
    CFMutableDictionaryRef 	matchingDict;
    CFRunLoopSourceRef		runLoopSource;
    kern_return_t		kr;
    SInt32			usbVendor = kOurVendorIDRaw;
    SInt32			usbProduct = kOurProductIDRaw;

    IKTRACE((TEXT("main: Looking for devices matching vendor ID=%ld and product ID=%ld"), usbVendor, usbProduct));

    // Set up the matching criteria for the devices we're interested in
    matchingDict = IOServiceMatching(kIOUSBDeviceClassName);	// Interested in instances of class IOUSBDevice and its subclasses
    if (!matchingDict)
    {
        IKTRACE((TEXT("main: Can't create a USB matching dictionary")));
        mach_port_deallocate(mach_task_self(), gMasterPort);
        return -1;
    }
    
    // Add our vendor and product IDs to the matching criteria
    CFDictionarySetValue( 
            matchingDict, 
            CFSTR(kUSBVendorID), 
            CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbVendor)); 
    CFDictionarySetValue( 
            matchingDict, 
            CFSTR(kUSBProductID), 
            CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbProduct)); 

    // Create a notification port and add its run loop event source to our run loop
    // This is how async notifications get set up.
    gNotifyPort = IONotificationPortCreate(gMasterPort);
    runLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);
    
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
    
    // Retain additional references because we use this same dictionary with four calls to 
    // IOServiceAddMatchingNotification, each of which consumes one reference.
    matchingDict = (CFMutableDictionaryRef) CFRetain( matchingDict ); 
    matchingDict = (CFMutableDictionaryRef) CFRetain( matchingDict ); 
    matchingDict = (CFMutableDictionaryRef) CFRetain( matchingDict ); 
    matchingDict = (CFMutableDictionaryRef) CFRetain( matchingDict ); 
    matchingDict = (CFMutableDictionaryRef) CFRetain( matchingDict ); 
    
    // Now set up two notifications, one to be called when a raw device is first matched by I/O Kit, and the other to be
    // called when the device is terminated.
    kr = IOServiceAddMatchingNotification(  gNotifyPort,
                                            kIOFirstMatchNotification,
                                            matchingDict,
                                            RawDeviceAdded,
                                            NULL,
                                            &gRawAddedIter );
                                            
    RawDeviceAdded(NULL, gRawAddedIter);	// Iterate once to get already-present devices and
                                                // arm the notification

    kr = IOServiceAddMatchingNotification(  gNotifyPort,
                                            kIOTerminatedNotification,
                                            matchingDict,
                                            RawDeviceRemoved,
                                            NULL,
                                            &gRawRemovedIter );
                                            
    RawDeviceRemoved(NULL, gRawRemovedIter);	// Iterate once to arm the notification
    
    // Change the USB product ID in our matching dictionary to the one the device will have once the
    // bulktest firmware has been downloaded.
    usbProduct = IK_PID;
    usbVendor  = IK_VID;
    
    CFDictionarySetValue( 
            matchingDict, 
            CFSTR(kUSBProductID), 
            CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbProduct)); 
    CFDictionarySetValue( 
            matchingDict, 
            CFSTR(kUSBVendorID), 
            CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbVendor)); 

    // Now set up two more notifications, one to be called when a bulk test device is first matched by I/O Kit, and the other to be
    // called when the device is terminated.
    kr = IOServiceAddMatchingNotification(  gNotifyPort,
                                            kIOFirstMatchNotification,
                                            matchingDict,
                                            LoadedDeviceAdded,
                                            NULL,
                                            &gLoadedAddedIter );
                                            
    LoadedDeviceAdded(NULL, gLoadedAddedIter);	// Iterate once to get already-present devices and
                                                        // arm the notification

    kr = IOServiceAddMatchingNotification(  gNotifyPort,
                                            kIOTerminatedNotification,
                                            matchingDict,
                                            LoadedDeviceRemoved,
                                            NULL,
                                            &gLoadedRemovedIter );
                                            
    LoadedDeviceRemoved(NULL, gLoadedRemovedIter); 	// Iterate once to arm the notification
    
	
    // now od for IntelliSwitch
    usbProduct = IS_PID;
    usbVendor  = IS_VID;
    
    CFDictionarySetValue( 
						  matchingDict, 
						  CFSTR(kUSBProductID), 
						  CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbProduct)); 
    CFDictionarySetValue( 
						  matchingDict, 
						  CFSTR(kUSBVendorID), 
						  CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbVendor)); 
	
    // Now set up two more notifications, one to be called when a bulk test device is first matched by I/O Kit, and the other to be
    // called when the device is terminated.
    kr = IOServiceAddMatchingNotification(  gNotifyPort,
                                            kIOFirstMatchNotification,
                                            matchingDict,
                                            IntelliSwitchDeviceAdded,
                                            NULL,
                                            &gIntelliSwitchAddedIter );
	
    IntelliSwitchDeviceAdded(NULL, gIntelliSwitchAddedIter);	// Iterate once to get already-present devices and
												// arm the notification
	
    kr = IOServiceAddMatchingNotification(  gNotifyPort,
                                            kIOTerminatedNotification,
                                            matchingDict,
                                            IntelliSwitchDeviceRemoved,
                                            NULL,
                                            &gIntelliSwitchRemovedIter );
	
    IntelliSwitchDeviceRemoved(NULL, gIntelliSwitchRemovedIter); 	// Iterate once to arm the notification

	
	// Now done with the master_port
	// mach_port_deallocate(mach_task_self(), gMasterPort);
	// gMasterPort = 0;

    return 0;
}

//
//  USBNotifierThreadFunction
//  this function runs in a pthread and handles USB device
//  notifications
//
static void* RunLoopFunction(void* input)
{
    // first create a master_port
    kern_return_t	kr;
    kr = IOMasterPort(MACH_PORT_NULL, &gMasterPort);
    if (kr || !gMasterPort)
    {
        IKTRACE((TEXT("main: ERR: Couldn't create a master IOKit Port(%08x)"), kr));
        return 0;
    }
    
    //  establish the notifications
    SetupNotifications();
    
    // Start the run loop. Now we'll receive notifications.
    while(true)
    {
        CFRunLoopRun();
    }
    
    syslog(1,"USBNotifierThreadFunction terminated");
    
    //  never get here
    return 0;

}

//
//  CleanUp
//
static void CleanUp()
{
    //  terminate the engine.
#ifndef NO_ENGINE
    theEngine.Terminate();
#endif
    
    //  kill thread
    pthread_cancel(thread);//tell thread to stop
    pthread_join(thread,NULL);//wait for the thread to stop
    
    // Clean up here
    IONotificationPortDestroy(gNotifyPort);

    if (gRawAddedIter) 
    {
        IOObjectRelease(gRawAddedIter);
        gRawAddedIter = 0;
    }

    if (gRawRemovedIter) 
    {
        IOObjectRelease(gRawRemovedIter);
        gRawRemovedIter = 0;
    }
    
    if (gLoadedAddedIter) 
    {
        IOObjectRelease(gLoadedAddedIter);
        gLoadedAddedIter = 0;
    }

    if (gLoadedRemovedIter) 
    {
        IOObjectRelease(gLoadedRemovedIter);
        gLoadedRemovedIter = 0;
    }
    
    // Now done with the master_port
    mach_port_deallocate(mach_task_self(), gMasterPort);
    gMasterPort = 0;
}


//
//  SignalHandler is called when we're interrupted
//
static void SignalHandler(int sigraised)
{
    syslog(1,"SIGINT caught");
    CleanUp();
    exit(0);
}



#ifdef ENGINE_IN_TIMER
static void TimerCallBack ( CFRunLoopTimerRef /*inTimer*/, void* x)
{
    theEngine.Periodic();
}

#else
static void* EngineFunction(void* input)
{
    //  main loop
#ifdef NO_ENGINE
    while (true)
    {
        usleep(5*1000);
    }
#else
    while (theEngine.IsRunning())
    {
        theEngine.Periodic();
        usleep(DATAI("Engine_Period",5)*1000);
    }
#endif
	
    syslog(1,"engine termineted.");
	
    //  terminate the engine
#ifndef NO_ENGINE
    theEngine.Terminate();
#endif
	
}
#endif

int main (int argc, const char * argv[]) 
{	
    //  establish a signal handler
    sig_t oldHandler;
    oldHandler = signal(SIGINT, SignalHandler);
    if (oldHandler == SIG_ERR) {
        IKTRACE((TEXT("main: Could not establish new signal handler")));
    }
        
    //  run at high priority
    setpriority ( PRIO_PROCESS, 0, -20 );
	
    //  initialize device list
    for (int i=0;i<MAX_INTELLIKEYS;i++)
    {
		devices[i].inPipe	= 0;
		devices[i].outPipe	= 0;
		devices[i].ppif		= 0;
		devices[i].numBytesRead = 0;
		devices[i].locationID   = 0;
		devices[i].notification = 0;
    }
	
    //  initialize the engine
#ifndef NO_ENGINE
    theEngine.Initialize();
    
    IK_VID = DATAI(TEXT("IK_Vendor_ID"),0);
    IK_PID = DATAI(TEXT("IK_Product_ID"),0);
    IS_VID = DATAI(TEXT("IS_Vendor_ID"),0);
    IS_PID = DATAI(TEXT("IS_Product_ID"),0);
#endif

#ifdef ENGINE_IN_TIMER
    //  run engine out of a timer
    CFRunLoopTimerContext context = {0,0,NULL,NULL,NULL};
    float period = float(DATAI("Engine_Period",5))/float(1000);  //  seconds
    CFRunLoopTimerRef timer = CFRunLoopTimerCreate ( NULL, 0, period, 0, 0, (CFRunLoopTimerCallBack)TimerCallBack, &context );
    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    CFRunLoopAddTimer(runLoop, timer, kCFRunLoopCommonModes);
#else
    //  Put the Engine in a thread
    pthread_create ( &thread, NULL, EngineFunction, NULL );
#endif
	
    //  Now start the main loop
    RunLoopFunction ( 0 );

    //  clean up.  If we ever get here.
    CleanUp();	

    return 0;
}

