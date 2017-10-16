static void DisplayError ( char *text )
{
}

static char *devicenames[] = 
{
	"USB\\Vid_095e&Pid_0100&Rev_0000",
	"USB\\Vid_095e&Pid_0100",
	"USB\\Vid_095e&Pid_0101&Rev_0001",
	"USB\\Vid_095e&Pid_0101",
	"HID\\Vid_095e&Pid_0101&Rev_0001",
	"HID\\Vid_095e&Pid_0101",
	"USB\\Vid_0547&Pid_2131&Rev_0004",
	"USB\\Vid_0547&Pid_2131"
};
static int nDevices= 8;

static bool FindNoCase ( char *text, char *target )
{
	char MyText[1000];
	char MyTarget[1000];

	strcpy(MyText,text);
	strcpy(MyTarget,target);

	unsigned int j;
	for (j=0;j<strlen(MyText);j++)
		MyText[j] = toupper(MyText[j]);
	for (j=0;j<strlen(MyTarget);j++)
		MyTarget[j] = toupper(MyTarget[j]);

	if (strstr(MyText,MyTarget)==NULL)
		return false;

	return true;
}

bool IsOurLine ( char *text )
{
	int j;
	for (j=0;j<nDevices;j++)
	{
		if (FindNoCase(text,devicenames[j]))
			return true;
	}

	return false;
}

bool IsOurFile ( char * filename )
{
	bool bOurs = false;
	FILE *in;
	int result;
	char c;
	int nbytes;
	char bytes[1000];

	//  open the file
	nbytes = 0;
	bytes[0] = '\0';

	in = fopen(filename,"r");
	if(in!=NULL)
	{
		//  read the bytes
		while(1==1)
		{
			result = fread ( &c, 1, 1, in );
			if(result!=1)
				break;

			//  next byte.
			if (c==10 || c==13)
			{
				//  line completed.

				if (IsOurLine(bytes))
				{
					bOurs = true;
					break;
				}

				nbytes = 0;
				bytes[0] = '\0';
			}
			else
			{
				if (nbytes<999)
				{
					bytes[nbytes] = c;
					bytes[nbytes+1] = '\0';
				}
				nbytes++;
			}

		}

		//  close the file
		fclose(in);
	}

	return bOurs;
}

void Remove2K()
{

    HDEVINFO DeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD i,err;

    //
    // Verify the Arguments.
    //
    //if (argc < 2)
    //{
    //    _tprintf(TEXT("usage: remove <Hardware_ID>\n"));
    //    return 1; // Remove Failure
    //}

    //
    // Create a Device Information Set with all present devices.
    //
    DeviceInfoSet = SetupDiGetClassDevs(NULL, // All Classes
        0,
        0, 
        DIGCF_ALLCLASSES /* | DIGCF_PRESENT */ ); // All devices present on system
    if (DeviceInfoSet == INVALID_HANDLE_VALUE)
    {
        DisplayError(TEXT("GetClassDevs(All Present Devices)"));        
        return;
    }
    
    //
    //  Enumerate through all Devices.
    //
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i=0;SetupDiEnumDeviceInfo(DeviceInfoSet,i,&DeviceInfoData);i++)
    {
        DWORD DataT;
        LPTSTR p,buffer = NULL;
        DWORD buffersize = 0;
        
        //
        // We won't know the size of the HardwareID buffer until we call
        // this function. So call it with a null to begin with, and then 
        // use the required buffer size to Alloc the nessicary space.
        // Keep calling we have success or an unknown failure.
        //
        while (!SetupDiGetDeviceRegistryProperty(
            DeviceInfoSet,
            &DeviceInfoData,
            SPDRP_HARDWAREID,
            &DataT,
            (PBYTE)buffer,
            buffersize,
            &buffersize))
        {
            if (GetLastError() == ERROR_INVALID_DATA)
            {
                //
                // May be a Legacy Device with no HardwareID. Continue.
                //
                break;
            }
            else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                //
                // We need to change the buffer size.
                //
                if (buffer) 
                    LocalFree(buffer);
                buffer = (char *)LocalAlloc(LPTR,buffersize);
            }
            else
            {
                //
                // Unknown Failure.
                //
                DisplayError(TEXT("GetDeviceRegistryProperty"));
                goto cleanup_DeviceInfo;
            }            
        }

        if (GetLastError() == ERROR_INVALID_DATA) 
            continue;
        
        //
        // Compare each entry in the buffer multi-sz list with our HardwareID.
        //
        for (p=buffer;*p&&(p<&buffer[buffersize]);p+=lstrlen(p)+sizeof(TCHAR))
        {
            //_tprintf(TEXT("Compare device ID: [%s]\n"),p);

			//TRACE("comparing device ID %s\n",p);

			int j;
			bool bFound = false;
			for (j=0;j<nDevices;j++)
				if (!_tcscmp(devicenames[j],p))
					bFound= true;

            if (bFound)
            {
                //_tprintf(TEXT("Found! [%s]\n"),p);

				//TRACE("removing %s\n",p);

                //
                // Worker function to remove device.
                //
                if (!SetupDiCallClassInstaller(DIF_REMOVE,
                    DeviceInfoSet,
                    &DeviceInfoData))
                {
                    DisplayError(TEXT("CallClassInstaller(REMOVE)"));
                }
                break;
            }
        }

        if (buffer) LocalFree(buffer);
    }

    if ((GetLastError()!=NO_ERROR)&&(GetLastError()!=ERROR_NO_MORE_ITEMS))
    {
        DisplayError(TEXT("EnumDeviceInfo"));
    }
    
    //
    //  Cleanup.
    //    
cleanup_DeviceInfo:
    err = GetLastError();
    SetupDiDestroyDeviceInfoList(DeviceInfoSet);


	//  search OEM*.inf files for our device

	//  where is the INF folder?
	char path[256];
	GetWindowsDirectory(path,256);
	char infFolder[256];
	strcpy(infFolder,path);
	strcat(infFolder,"\\inf\\");

	char infWild[256];
	strcpy(infWild,infFolder);
	strcat(infWild,"oem*.inf");

	WIN32_FIND_DATA findData;
	HANDLE hFile = FindFirstFile( infWild, &findData);

	BOOL bContinue = (hFile != INVALID_HANDLE_VALUE) ? TRUE : FALSE;
	while (bContinue)
	{
		char fullPath[256];
		strcpy(fullPath,infFolder);
		strcat(fullPath,findData.cFileName);

		if (IsOurFile(fullPath))
		{
			char del1[256];
			strcpy(del1,infFolder);
			strcat(del1,findData.cFileName);

#if 1
			char del2[256];
			strcpy(del2,infFolder);
			strcat(del2,findData.cFileName);
			del2[strlen(del2)-4] =  '\0';
			strcat(del2,".pnf");

			char del3[256];
			strcpy(del3,infFolder);
			strcat(del3,findData.cFileName);
			del3[strlen(del3)-4] =  '\0';
			strcat(del3,".cat");

			SetFileAttributes ( del1, FILE_ATTRIBUTE_NORMAL );
			SetFileAttributes ( del2, FILE_ATTRIBUTE_NORMAL );
			SetFileAttributes ( del3, FILE_ATTRIBUTE_NORMAL );

			DeleteFile(del1);
			DeleteFile(del2);
			DeleteFile(del3);
#else

			BOOL b = SetupUninstallOEMInf ( del1, SUOI_FORCEDELETE, NULL );
#endif
		}

		bContinue = FindNextFile( hFile, &findData );
	}
    
    return ;
}
