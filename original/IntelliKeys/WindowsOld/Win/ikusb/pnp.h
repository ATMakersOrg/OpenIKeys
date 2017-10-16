#ifndef _IKUSBPNP_H
#define _IKUSBPNP_H


typedef struct
{
	char data[255];
} mystr255;

#ifdef __cplusplus
extern "C" 
#endif
int FindMyHidDevices ( 
	ULONG vendorID,
	ULONG productID,
	int maxDevices,
	mystr255 *devNames
	);

#endif

#ifdef __cplusplus
extern "C" 
#endif
BOOL
IsDeviceInstallInProgress (VOID);

#ifdef __cplusplus
extern "C" 
#endif
BOOLEAN GetNumBuffers ( HANDLE h, PULONG pNumBuffers );

#ifdef __cplusplus
extern "C" 
#endif
BOOLEAN SetNumBuffers ( HANDLE h, ULONG NumBuffers );
