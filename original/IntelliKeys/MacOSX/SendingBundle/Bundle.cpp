

#define TCHAR char
#define BYTE unsigned char
#define WORD unsigned short
#define TEXT(a) a

#define UNUSED(a) {a;}

#include "MessageClient.h"
#include "IKUtil.h"
#include "IKFile.h"
#include "IKSettings.h"
#include "IKMessage.h"
//#include "FileCopy.h"

extern "C"
{
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sysctl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
}

static char command[1024];

static void* myThreadFunction(void* input)
{
	int result = system(command);
	return 0;
}

static int ReverseFind ( const char *string, char c )
{
	int l = strlen(string);
	while (l>=0 && string[l]!=c)
		l--;
	return l;
}

static void MyCtoPstr2 ( unsigned char *str)
{
    int i, l, j;
    
    l = 0;
    while (str[l]!=0)
		l++;
	
    for (i=0;i<l;i++)
    {
		j = l-i-1;
		str[j+1] = str[j];
    }
    str[0] = l;
}

static void MyPtoCstr2 ( unsigned char *str )
{
    int i, l;
    
    l = str[0];
    for (i=0;i<l;i++)
		str[i] = str[i+1];
    str[l] = 0;
}

static bool SameFile ( char *s1, char *s2 )
{		
	if (strlen(s1) != strlen(s2))
		return false;
	
	for (int i=0;i<strlen(s1);i++)
	{
		char c1 = s1[i];
		if ( c1>='A' && c1<='Z')
			c1 = c1 - 'A' + 'a';
		char c2 = s2[i];
		if ( c2>='A' && c2<='Z')
			c2 = c2 - 'A' + 'a';
		
		if (c1 != c2)
			return false;
	}
	
	return true;
}


static void GetFullPath (long DirID, short vRefNum, char *path)
{
	CInfoPBRec myPB;// parameter block for PBGetCatInfo
	Str255 dirNameBuf; //  a directory name
	char fullPath[255];//  full pathname being constructed
		OSErr myErr;
		
		myPB.dirInfo.ioNamePtr = dirNameBuf;
		myPB.dirInfo.ioVRefNum = vRefNum;    //indicate target volume
		myPB.dirInfo.ioDrParID = DirID;      //initialize parent directory ID
		myPB.dirInfo.ioFDirIndex = -1;       //get info about a directory
		
		strcpy(fullPath,"");
		strcpy(path,"");
		
		do
		{
			myPB.dirInfo.ioDrDirID = myPB.dirInfo.ioDrParID;
			myErr = PBGetCatInfo(&myPB, FALSE);
			if ( myErr )
				return;
			
			MyPtoCstr2 ( dirNameBuf );
			
			char temp[255];
			strcpy(temp,fullPath);
			strcpy(fullPath,(char *)dirNameBuf);
			strcat(fullPath,":");
			strcat(fullPath,temp);
			
		} while ( myPB.dirInfo.ioDrDirID != fsRtDirID );
		
		strcpy(path,fullPath);
		return;
		
}

static bool CopyFile ( const char *src, char *dst )
{
    unsigned char macFileName[255];

    //  spec the source name.  Must exist.
	strcpy((char *) macFileName, src);   
	MyCtoPstr2(macFileName);

    FSSpec srcSpec;
    OSErr err = FSMakeFSSpec( 0, 0, (unsigned char *)macFileName, &srcSpec );
    
    //NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    if (err == noErr) 
    {
        //  spec the new directory
        int dirlen = ReverseFind(dst,':');
        strncpy((char*) macFileName,dst,dirlen);   
        MyCtoPstr2(macFileName);

        FSSpec dstSpec;
        err = FSMakeFSSpec( 0, 0, (unsigned char *)macFileName, &dstSpec );
        if (err == noErr) 
        {       
            //  get the filename part
            strcpy ((char*) macFileName, &(dst[dirlen+1]) );   
            MyCtoPstr2(macFileName);
            
            //  copy away.
            
            FSRef srcRef;
            FSpMakeFSRef (&srcSpec, &srcRef);
            
            FSRef dstRef;
            FSpMakeFSRef (&dstSpec, &dstRef);

            CFStringRef fileName = CFStringCreateWithPascalString(kCFAllocatorDefault, (ConstStr255Param)macFileName, kTextEncodingMacRoman);
            assert (fileName != NULL);
            err = FSCopyObjectSync (&srcRef, &dstRef, fileName, NULL, kFSFileOperationDefaultOptions);
            
            //err = FSpFileCopy ( (const FSSpec *)&srcSpec, (const FSSpec *)&dstSpec, (const unsigned char *)macFileName, 0, 0, false );
        }
    }
    
    //[pool release];

    return (err == noErr);
	
}

static void DeleteFile ( char *lpszFileName )
{
	if ( lpszFileName == NULL )	
		return;
	
	unsigned char macFileName[255];  
	strcpy((char*)macFileName,lpszFileName);
	MyCtoPstr2(macFileName);
	
	FSSpec spec;
	if (FSMakeFSSpec( 0, 0, (unsigned char *)macFileName, &spec ) == noErr)
		FSpDelete ( &spec);
}


static bool DoTheCopy ( const char *inStr, char *outStr )
{
	//  find the temporary files folder
	short vRefNum;
	long dirID;
	OSErr err = FindFolder ( kOnSystemDisk, kTemporaryFolderType, kDontCreateFolder, &vRefNum, &dirID );
	if (err==noErr)
	{
		//  get the full path of the temp folder
		char hfs[255];
		GetFullPath ( dirID, vRefNum, hfs );
		
		//  tack on the name from the source file
		int i = ReverseFind ( inStr, ':' );
		if (i>=0)
		{
			strcat ( hfs, &(inStr[i+1]) );
			
			//  make the copy
			DeleteFile ( hfs );
			bool bCopied = CopyFile ( inStr, hfs );
			if (bCopied)
				strcpy ( outStr, hfs );
			return bCopied;
		}
	}
	
	return false;
}

static bool MakeTempCopy ( const char *inStr, char *outStr )
{	
	//  find the boot drive 
	short bootvRefNum;
	long dirID;
	OSErr err = FindFolder ( kOnSystemDisk, kApplicationSupportFolderType, kDontCreateFolder, &bootvRefNum, &dirID );
	if (err==noErr)
	{
		//  get volume of the input file
	    unsigned char macFileName[255];
		strcpy ((char*) macFileName, inStr );
		MyCtoPstr2 ( macFileName );

		FSSpec fs;
		if (FSMakeFSSpec( 0, 0,(unsigned char *)macFileName, &fs )==noErr)
		{
			//  different volumes?
			if (bootvRefNum != fs.vRefNum)
			{
				//  make a copy
				char copyStr[255];
				bool bCopied = DoTheCopy ( inStr, copyStr );
				if (bCopied)
				{
					strcpy ( outStr, copyStr );
					return true;
				}
			}
		}
	}
	
	return false;  //  no copy made.
}

#pragma export on

extern "C" void BundleFloatingMessage ( const char *message )
{
#if 0  //  NO FLOATING MESSAGE 7/25/06

	//  create a command string
	strcpy(command,"");
	
	strcat(command,"\"");
	strcat(command,"/Applications/IntelliTools/IntelliKeys USB/Private/Balloon.app/Contents/MacOS/Balloon");
	strcat(command,"\"");
	
	strcat(command," ");
	strcat(command,"\"");
	strcat(command,message);
	strcat(command,"\"");
	
	//  create a thread for the system command
	pthread_t		myThread;
    pthread_create(&myThread,NULL,myThreadFunction,NULL);
#endif
}

extern "C" int BundleIsIntellikeysConnected ()
{
	//  initialize ikeys code
	IKUtil::Initialize();
	
	//  find out
	int response = IK2IsIntellikeysConnected();
	if (response==kResponseConnected)
		return 1;
	else
		return 0;
}

extern "C" int BundleIsIntellikeysOn ()
{
	//  initialize ikeys code
	IKUtil::Initialize();

	//  find out
	int response = IK2IsIntellikeysOn();
	if (response==kResponseOn)
		return 1;
	else
		return 0;
}

extern "C" int BundleTestOverlayFile ( const char *filename )
{
	UNUSED(filename);
	
	//  initialize ikeys code
	IKUtil::Initialize();
	
	return 333;
}

extern "C" int BundleSendOverlay ( const char *title, const char *filename )
{
	UNUSED(title);

	//  initialize ikeys code
	IKUtil::Initialize();
	
	//  copy the file if it's not on the local hard drive.
	char sendThis[255];
	bool bCopied = MakeTempCopy ( filename, sendThis );	
	
	//  send the overlay
	int response;
	if (bCopied)
	{
		response = IK2SendOverlay ((char *)sendThis);
		DeleteFile ( sendThis );
	}
	else
	{
		response = IK2SendOverlay ((char *)filename);
	}

	//  return results.
	if (response==kResponseNoError)
		return 1;
	else
		return 0;
}

static void LoadPreferences ()
{
	//  load preferences
	IKString prefsFile = IKUtil::GetPrivateFolder();
	prefsFile += DATAS(TEXT("Preferences_File"),TEXT("preferences.txt"));
	IKPrefs prefs;
	prefs.Read(prefsFile);
	
	//  get group and student name
	IKString group   = prefs.GetValueString(TEXT("Group"),  TEXT(""));
	IKString student = prefs.GetValueString(TEXT("Student"),TEXT(""));
	if (group.IsEmpty() && student.IsEmpty())
		group = DATAS(TEXT("Guest"),TEXT("Guest"));  //  localize!!
	
	//  make path to the settings files
	IKString path = IKUtil::MakeStudentPath ( group, student );
	IKString settingsFile = path;
	settingsFile += TEXT("settings.txt");
	
	//  read user settings
	IKSettings::GetSettings()->Read(settingsFile);
	
}

static void RegisterSender (const char *pFilePath)
{
	//  get current app name
	IKString appPath = IKUtil::GetCurrentApplicationPath();
	IKString appName = IKUtil::GetAppFriendlyName(appPath);
	
	//  get overlay name
	IKString ovlName = pFilePath;
	IKUtil::StripFileName(ovlName,true,true);
	
	//  modify settings
	LoadPreferences();
	IKSettings::GetSettings()->m_sLastSent   = ovlName;
	IKSettings::GetSettings()->m_sLastSentBy = appName;
	IKSettings::GetSettings()->Write();
	
	//  tell everyone
	IKMessage::Send(TEXT("engine"),kQueryReloadStudent, 0, 0, 0, 0);
}

extern "C" int BundleSendOverlay2 ( const char *title, const char *filename, const char *sender )
{
	//  copy the file if it's not on the local hard drive.
	char sendThis[255];
	bool bCopied = MakeTempCopy ( filename, sendThis );
	
	//  send the overlay.
	int result;
	if (bCopied)
	{
		result = BundleSendOverlay ( title, sendThis );
		DeleteFile ( sendThis );
	}
	else
	{
		result = BundleSendOverlay ( title, filename );
	}
	
	//  now register the sender
	RegisterSender ( filename );
	
	//  return result
	return result;
	
}


#pragma export off


