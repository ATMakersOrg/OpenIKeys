#define kCommandFile "\pIK Sending Command"
#define kResultFile "\pIK Sending Result"
#define GET_ADDR	1
#define UPLD_SET 	2
#define DNLD_SET	3
#define UPLD_OVL 	4
#define DNLD_OVL	5
#define NO_ERR		0
#define CHKSUM_ERR	-1
#define TIME_OUT	-2
#define BAD_CMD		-3
#define LOW_MEM		-4
#define NO_INTELL	-5
#define LOCKOUT 0
#define BYTE unsigned char
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
} myrawfeature, *myfeaturePtr,**myfeatureHdl;
//
//  Get the version of Mac OS we're running.
//
static unsigned int  MacGetOSVersion ()
{
	static   Boolean    sAlreadyChecked = false;
	static   SInt32     version = 0;
	if (!sAlreadyChecked) 
	{
		OSErr err = Gestalt (gestaltSystemVersion, &version);
		sAlreadyChecked = true;
	}
	return version;
}
//
//  Take a standard C string and convert it to Pascal form.
//
static void ConvertCToPStr255 ( const char *pSrc, unsigned char *pDest )
{
	int nLen = strlen ( pSrc );
	if ( nLen > 255 )
		nLen = 255;
	memcpy ( pDest+1, pSrc, nLen );
	*pDest = (unsigned char) nLen;
}
//
//  Make a spec for the command file
//
static OSErr GetCommandSpec ( FSSpec *spec )
{
	//  find the temp file folder
	short vRefNum;
	long dirID;
	OSErr err = FindFolder ( kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &vRefNum, &dirID );
	if (err != noErr)
		return err;
		
	//  find the command 
	err =  FSMakeFSSpec ( vRefNum, dirID, kCommandFile, spec ) ;
	return err;
}
//
//  Make a spec for the result file
//
static OSErr GetResultSpec ( FSSpec *spec )
{
	//  find the temp file folder
	short vRefNum;
	long dirID;
	OSErr err = FindFolder ( kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &vRefNum, &dirID );
	if (err != noErr)
		return err;
		
	//  find the command 
	err =  FSMakeFSSpec ( vRefNum, dirID, kResultFile, spec ) ;
	return err;
}
static unsigned int GetTicksMS ()
{
	// Get the microseconds
	UnsignedWide microsecondsInt;
	::Microseconds( &microsecondsInt );
	// Convert to milliseconds using floating point
	const double kTwoPower32 = double ( 2 << 32 );
	
	double microseconds =
		double ( microsecondsInt.hi ) * kTwoPower32 + double ( microsecondsInt.lo );
		
	unsigned int retval = microseconds / 1000.0;
		
	return  retval;
}
static void Sleep ( unsigned int dwTicksMS )
{
	// sleep for the specified ticks
	unsigned int origMS = GetTicksMS();
	while ( true )
	{
		unsigned int curMS = GetTicksMS();		
		if ( ( curMS - origMS ) > dwTicksMS )
			break;
	}
}
static bool IsProcessRunning ( ProcessSerialNumber *psn )
{
	ProcessSerialNumber iteratingPSN = {kNoProcess,kNoProcess};
	while (GetNextProcess(&iteratingPSN) == noErr) 
	{
		if (iteratingPSN.highLongOfPSN == psn->highLongOfPSN &&
			iteratingPSN.lowLongOfPSN  == psn->lowLongOfPSN     )
			return true;
	}
	return false;
}
static void MakeSendingTitle ( char *pFilePath, Str255 strTitle )
{
	//  get the simple file name
	int lastColon = -1;
	for (int i=0;i<strlen(pFilePath);i++)
		if (pFilePath[i]==':')
			lastColon = i;
	
	//  TODO: localizable.  Where are the resources?
	char message[255];
	strcpy(message,"Sending \"");
	strcat(message,&(pFilePath[lastColon+1]));
	strcat(message,"\" ...");
	ConvertCToPStr255 ( message, strTitle );
}
