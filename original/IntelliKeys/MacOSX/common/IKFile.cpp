// IKFile.cpp: implementation of the IKFile class.
//
//////////////////////////////////////////////////////////////////////

#include "IKCommon.h"
#include "IKFile.h"
#include "IKUtil.h"

#ifdef PLAT_MACINTOSH_X
//#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef PLAT_WINDOWS
#include <sys/stat.h>
#include <aclapi.h>
#include <lmerr.h>
#endif

//#ifdef PLAT_MACINTOSH_CLASSIC
#ifdef PLAT_MACINTOSH
#include <time.h>
#ifndef PLAT_MACINTOSH_X
#include <stat.h>
#endif
#endif

#ifdef PLAT_MACINTOSH
#ifdef BUILD_CW
  #ifdef BUILD_CARBON
    #include <Carbon.h>
  #else
    //  TODO
  #endif
#else

#endif
  #include <string.h>
  #include <stdio.h>
#endif

static BYTE UnicodeBOM[2] = {0xff, 0xfe};
static BYTE UTF8BOM[3] = {0xef, 0xbb, 0xbf};

#ifdef PLAT_MACINTOSH
//#include "Files.h"
//#include "CFString.h"
//#include "FileCopy.h"
#endif

#ifdef PLAT_MACINTOSH

/*
**  Deny mode permissions for use with the HOpenAware, HOpenRFAware,
**  FSpOpenAware, and FSpOpenRFAware functions.
**  Note: Common settings are the ones with comments.
*/

enum {
  dmNone                        = 0x0000,
  dmNoneDenyRd                  = fsRdDenyPerm,
  dmNoneDenyWr                  = fsWrDenyPerm,
  dmNoneDenyRdWr                = (fsRdDenyPerm + fsWrDenyPerm),
  dmRd                          = fsRdPerm, /* Single writer, multiple readers; the readers */
  dmRdDenyRd                    = (fsRdPerm + fsRdDenyPerm),
  dmRdDenyWr                    = (fsRdPerm + fsWrDenyPerm), /* Browsing - equivalent to fsRdPerm */
  dmRdDenyRdWr                  = (fsRdPerm + fsRdDenyPerm + fsWrDenyPerm),
  dmWr                          = fsWrPerm,
  dmWrDenyRd                    = (fsWrPerm + fsRdDenyPerm),
  dmWrDenyWr                    = (fsWrPerm + fsWrDenyPerm),
  dmWrDenyRdWr                  = (fsWrPerm + fsRdDenyPerm + fsWrDenyPerm),
  dmRdWr                        = fsRdWrPerm, /* Shared access - equivalent to fsRdWrShPerm */
  dmRdWrDenyRd                  = (fsRdWrPerm + fsRdDenyPerm),
  dmRdWrDenyWr                  = (fsRdWrPerm + fsWrDenyPerm), /* Single writer, multiple readers; the writer */
  dmRdWrDenyRdWr                = (fsRdWrPerm + fsRdDenyPerm + fsWrDenyPerm) /* Exclusive access - equivalent to fsRdWrPerm */
};

static void MyPLstrcat ( unsigned char *dst, unsigned char *src )
{
    unsigned int ld = dst[0];
    unsigned int ls = src[0];
    for (int i=0;i<ls;i++)
        dst[ld+i+1] = src[i+1];
    dst[0] = ld + ls;
}

#endif

IKFile::IKFile()
:
#ifdef PLAT_WINDOWS
	m_hFile(hFileNull),
#endif

#ifdef PLAT_MACINTOSH
	m_fileRef(0),
#endif
	m_bCloseOnDelete(FALSE), m_bUnicode(false)
{
#ifdef PLAT_WINDOWS
	m_hFile = (UINT) hFileNull;
#endif

#ifdef PLAT_MACINTOSH
	m_fileRef = 0;
#endif

	m_bCloseOnDelete = FALSE;

	m_bUnicode = false;

	m_strFileName.Empty();
}

IKFile::~IKFile()
{
#ifdef PLAT_WINDOWS
	if (m_hFile != (UINT)hFileNull && m_bCloseOnDelete)
		Close();
#endif

#ifdef PLAT_MACINTOSH
	if (m_fileRef != 0 && m_bCloseOnDelete)
		Close();
#endif
}

unsigned int IKFile::GetPosition() const
{
#ifdef PLAT_WINDOWS

	DWORD dwPos = SetFilePointer((HANDLE)m_hFile, 0, NULL, FILE_CURRENT);
	return dwPos;

#endif

#ifdef PLAT_MACINTOSH
	
    long int newPos;
	GetFPos ( m_fileRef, &newPos );
    return (unsigned int) newPos;
        
#endif
}

IKString IKFile::GetFileName() const
{
	return m_strFileName;
}


bool IKFile::Open(TCHAR * lpszFileName, unsigned int nOpenFlags)
{
        //  bad filename
	if ( lpszFileName == NULL )	
		return FALSE;

	IKString s = lpszFileName;
	if (s.Compare(TEXT(""))==0)
		return FALSE;
		
#ifdef PLAT_WINDOWS

	//ASSERT_VALID(this);
	//ASSERT(AfxIsValidString(lpszFileName));
	//ASSERT(pException == NULL ||
	//	AfxIsValidAddress(pException, sizeof(CFileException)));
	//ASSERT((nOpenFlags & typeText) == 0);   // text mode not supported

	// CFile objects are always binary and CreateFile does not need flag
	nOpenFlags &= ~(UINT)typeBinary;

	m_bCloseOnDelete = FALSE;
	m_hFile = (UINT)hFileNull;
	m_strFileName.Empty();

	m_strFileName = lpszFileName;

	//ASSERT(sizeof(HANDLE) == sizeof(UINT));
	//ASSERT(shareCompat == 0);

	// map read/write mode
	//ASSERT((modeRead|modeWrite|modeReadWrite) == 3);

	DWORD dwAccess = 0;
	switch (nOpenFlags & 3)
	{
	case modeRead:
		dwAccess = GENERIC_READ;
		break;
	case modeWrite:
		dwAccess = GENERIC_WRITE;
		break;
	case modeReadWrite:
		dwAccess = GENERIC_READ|GENERIC_WRITE;
		break;
	default:
		; //ASSERT(FALSE);  // invalid share mode
	}

	// map share mode
	DWORD dwShareMode = 0;
	switch (nOpenFlags & 0x70)    // map compatibility mode to exclusive
	{
	default:
		; // ASSERT(FALSE);  // invalid share mode?
	case shareCompat:
	case shareExclusive:
		dwShareMode = 0;
		break;
	case shareDenyWrite:
		dwShareMode = FILE_SHARE_READ;
		break;
	case shareDenyRead:
		dwShareMode = FILE_SHARE_WRITE;
		break;
	case shareDenyNone:
		dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ;
		break;
	}

	// Note: typeText and typeBinary are used in derived classes only.

	// map modeNoInherit flag
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = (nOpenFlags & modeNoInherit) == 0;

	// map creation flags
	DWORD dwCreateFlag;
	if (nOpenFlags & modeCreate)
	{
		if (nOpenFlags & modeNoTruncate)
			dwCreateFlag = OPEN_ALWAYS;
		else
			dwCreateFlag = CREATE_ALWAYS;
	}
	else
		dwCreateFlag = OPEN_EXISTING;

	//  attempt file creation
	//  use a Unicode or non-Unicode flavor, depending.

	HANDLE hFile = INVALID_HANDLE_VALUE;

	if (DATAI(TEXT("Unicode_File_Names"),0) != 0)
	{
		int len = IKString::strlen(lpszFileName);
		unsigned short wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,				// utf-8
							0,						// character-type options
							(const char *) lpszFileName, // string to map
							len,				// number of bytes in string
							wide,				// wide-character buffer
							255		// size of buffer
							);
		wide[result] = 0;


		hFile = CreateFileW(wide, dwAccess, dwShareMode, &sa,
				dwCreateFlag, FILE_ATTRIBUTE_NORMAL, NULL);	}
	else
	//if (hFile == INVALID_HANDLE_VALUE)
	{
		hFile = CreateFile(lpszFileName, dwAccess, dwShareMode, &sa,
			dwCreateFlag, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	m_hFile = (HFILE)hFile;
	m_bCloseOnDelete = TRUE;


#endif

#ifdef PLAT_MACINTOSH

	//  decode primary access flags
	unsigned int primaryOpenFlags;
	unsigned int accessFlags = nOpenFlags & dmRdWr;
	switch (accessFlags)
	{
		case modeRead:
			primaryOpenFlags = dmRd;
			break;
		case modeWrite:
			primaryOpenFlags = dmWr;
			break;
		case modeReadWrite:
			primaryOpenFlags = dmRdWr;
			break;
		default:
			primaryOpenFlags = dmNone;
			break;
	}
	        
	//  create the file if it's not there and if we're allowed to
	//  according to the access flags.
	FSSpec spec;
	TCHAR macFileName[255];
	IKString::strcpy(&(macFileName[1]),lpszFileName);
	macFileName[0] = IKString::strlen(&(macFileName[1]));
	
	OSErr err = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &spec );
    bool bCreated = false;
	
    if ((err == fnfErr) || (err == afpItemNotFound))
    {
		//  if we're not allowed to attempt file creation,
		//  exit here.
		bool bCreate = ( (nOpenFlags & modeCreate) != 0);
		if ( ( (primaryOpenFlags) == dmRd ) || (!bCreate) )
			return FALSE;
                    
        //  create the file
        int type = 'TEXT';
        int creator = '    ';
	
        err = FSpCreate(&spec,type,creator,smSystemScript);
	
        if (err != noErr)
                return FALSE;
        bCreated = true;
                
        //  re-make the spec
        err = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &spec );
        if (err != noErr)
        {
            //  error re-making the spec.
            //  delete the file, 'cause we just created it.
            Remove(lpszFileName);

            return FALSE; 
        }               
    }
        
	//  handle some more access flags
	if (nOpenFlags & shareDenyRead)
                primaryOpenFlags |= dmNoneDenyRd;
	if (nOpenFlags & shareDenyWrite)
                primaryOpenFlags |= dmNoneDenyWr;
	if (nOpenFlags & shareExclusive)
	{
		primaryOpenFlags |= dmNoneDenyRd;
		primaryOpenFlags |= dmNoneDenyWr;
	}
	if (nOpenFlags & shareDenyNone)
		primaryOpenFlags |= dmNone;
        
    //  open file
    short int flags = primaryOpenFlags;

    err = FSpOpenDF ( &spec, flags, &m_fileRef );

    if (err!=noErr)
    {
        if (bCreated)
        {
            Remove(lpszFileName);
        }
        return FALSE;
    }

	m_strFileName = lpszFileName;
	m_bCloseOnDelete = TRUE;
        
#endif

	//  start file from beginning
	Seek ( 0, IKFile::begin );

	//  read the first 2 bytes
	BYTE b1=0,b2=0;
	Read(&b1,1);
	Read(&b2,1);

	//  check for Unicode
	m_bUnicode = false;
	if (b1==UnicodeBOM[0] && b2==UnicodeBOM[1])
		m_bUnicode = true;

	//  back to the top
	Seek ( 0, IKFile::begin );

	return TRUE;
}

unsigned int IKFile::Read(void* lpBuf, unsigned int nCount)
{
#ifdef PLAT_WINDOWS

	if (nCount == 0)
		return 0;   // avoid Win32 "null-read"

	DWORD dwRead;
	bool b = !!ReadFile((HANDLE)m_hFile, lpBuf, nCount, &dwRead, NULL);

	return (UINT)dwRead;

#endif

#ifdef PLAT_MACINTOSH
	
        long len = nCount;
	OSErr err = 
            FSRead ( m_fileRef, &len, lpBuf );
        return len;
        
#endif
}

void IKFile::Write(const void* lpBuf, unsigned int nCount)
{
#ifdef PLAT_WINDOWS

	if (nCount == 0)
		return;     // avoid Win32 "null-write" option

	DWORD nWritten;
	bool b= !!WriteFile((HANDLE)m_hFile, lpBuf, nCount, &nWritten, NULL);

#endif

#ifdef PLAT_MACINTOSH
	long len = nCount;
        // OSErr err =
	     FSWrite ( m_fileRef, &len, lpBuf );
#endif
}

void IKFile::Rename(TCHAR * lpszOldName,	TCHAR * lpszNewName)
{
#ifdef PLAT_WINDOWS

	bool bOK = false;

	if (DATAI(TEXT("Unicode File Names"),0) != 0)
	{
		int lenold = IKString::strlen(lpszOldName);
		unsigned short oldw[255];
		int resultold = MultiByteToWideChar (
							CP_UTF8,				// utf-8
							0,						// character-type options
							(const char *) lpszOldName, // string to map
							lenold,				// number of bytes in string
							oldw,				// wide-character buffer
							255		// size of buffer
							);
		oldw[resultold] = 0;

		int lennew = IKString::strlen(lpszNewName);
		unsigned short neww[255];
		int resultnew = MultiByteToWideChar (
							CP_UTF8,				// utf-8
							0,						// character-type options
							(const char *) lpszNewName, // string to map
							lennew,				// number of bytes in string
							neww,				// wide-character buffer
							255		// size of buffer
							);
		neww[resultnew] = 0;


		bOK = !!MoveFileW( oldw, neww );
	}
	else
	//if (!bOK)
	{
		bOK = !!MoveFile((LPTSTR)lpszOldName, (LPTSTR)lpszNewName);
	}

#endif

#ifdef PLAT_MACINTOSH
	
	//  spec the old name.  Must exist.
	FSSpec spec;
    TCHAR macFileName[255];
    IKString::strcpy(&(macFileName[1]),lpszOldName);
    macFileName[0] = IKString::strlen(&(macFileName[1]));
	OSErr err = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &spec );
	if (err!=noErr)
		return;
		
	//  rename it.
    IKString::strcpy(&(macFileName[1]),lpszNewName);
    macFileName[0] = IKString::strlen(&(macFileName[1]));
    err = FSpRename ( &spec, (unsigned TCHAR *)macFileName );
	
#endif
}

void IKFile::Remove(TCHAR * lpszFileName)
{

#ifdef PLAT_WINDOWS

	bool bOK = false;

	if (DATAI(TEXT("Unicode File Names"),0) != 0)
	{
		int len = IKString::strlen(lpszFileName);
		unsigned short wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,				// utf-8
							0,						// character-type options
							(const char *) lpszFileName, // string to map
							len,				// number of bytes in string
							wide,				// wide-character buffer
							255		// size of buffer
							);
		wide[result] = 0;


		bOK = !!DeleteFileW ( wide );
	}
	else
	//if (!bOK)
	{
		bOK = !!DeleteFile(lpszFileName);
	}

#endif

#ifdef PLAT_MACINTOSH
	TCHAR macfilename[1024];
	FSSpec fs;
	OSErr err;
        int i;
	
	strcpy(&(macfilename[1]),lpszFileName);
	
	//  if this is a unix-style name, convert to original
	if (macfilename[1]==('/'))
	{
		//  strip off "/Volumes"
		if (strstr(&(macfilename[1]),TEXT("/Volumes"))==&(macfilename[1]))
			strcpy(&(macfilename[1]),&(macfilename[9+1]));
		
		//  convert '/' to ':'
	    int l = IKString::strlen(&(macfilename[1]));
	    for (i=0;i<l;i++)
			if (macfilename[i+1]==('/'))
				macfilename[i+1] = (':');
	}

	macfilename[0] = IKString::strlen(&(macfilename[1]));

	err = FSMakeFSSpec((short)0, (long)0, (unsigned TCHAR *)macfilename,&fs);
	if (err == noErr)
	{
		err = FSpDelete(&fs);
	}

#endif

}

unsigned int IKFile::SeekToEnd()
{
	return Seek(0, IKFile::end);
}


void IKFile::SeekToBegin()
{
	Seek(0,IKFile::begin);
}

unsigned int  IKFile::Seek(unsigned int lOff, unsigned int nFrom)
{
#ifdef PLAT_WINDOWS

	DWORD dwNew = 0;
	switch (nFrom)
    {
    case IKFile::begin:
		dwNew = SetFilePointer((HANDLE)m_hFile, lOff, NULL, (DWORD)FILE_BEGIN);
        break;
    case IKFile::current:
		dwNew = SetFilePointer((HANDLE)m_hFile, lOff, NULL, (DWORD)FILE_CURRENT);
        break;
    case IKFile::end:
		dwNew = SetFilePointer((HANDLE)m_hFile, lOff, NULL, (DWORD)FILE_END);
        break;
    }

	return dwNew;

#endif

#ifdef PLAT_MACINTOSH

    switch (nFrom)
    {
    case IKFile::begin:
	    {
	        SetFPos ( m_fileRef, fsFromStart, lOff ); 
	    }
        break;
        
    case IKFile::current:
	    {
	        SetFPos ( m_fileRef, fsAtMark, lOff ); 
	    }
        break;
        
    case IKFile::end:
	    {
	        SetFPos ( m_fileRef, fsFromLEOF, lOff ); 
	    }
        break;
    }
    
    long int newPos;

	GetFPos ( m_fileRef, &newPos );
        
	return (unsigned int) newPos;

#endif
}

void IKFile::SetLength(unsigned int dwNewLen)
{
#ifdef PLAT_WINDOWS

	Seek((LONG)dwNewLen, (UINT)begin);

	bool b = !!SetEndOfFile((HANDLE)m_hFile);

#endif

#ifdef PLAT_MACINTOSH
	//  TODO: implement
	UNUSED(dwNewLen)
#endif
}

unsigned int  IKFile::GetLength() const
{
	unsigned int dwLen, dwCur;

	// Seek is a non const operation
	IKFile* pFile = (IKFile*)this;

	dwCur = pFile->Seek(0L, current);
	dwLen = pFile->SeekToEnd();
	pFile->Seek(dwCur, begin);

	return dwLen;
}

void IKFile::Abort()
{
#ifdef PLAT_WINDOWS

	//ASSERT_VALID(this);
	if (m_hFile != (UINT)hFileNull)
	{
		// close but ignore errors
		CloseHandle((HANDLE)m_hFile);
		m_hFile = (UINT)hFileNull;
	}
	m_strFileName.Empty();

#endif

#ifdef PLAT_MACINTOSH
	//  TODO: implement
#endif
}

void IKFile::Flush()
{
#ifdef PLAT_WINDOWS

	if (m_hFile == (UINT)hFileNull)
		return;

	bool b = !!FlushFileBuffers((HANDLE)m_hFile);

#endif

#ifdef PLAT_MACINTOSH
	//  TODO: implement
#endif
}

void IKFile::Close()
{
#ifdef PLAT_WINDOWS

	if (m_hFile != (UINT)hFileNull)
		CloseHandle((HANDLE)m_hFile);

	m_hFile = (UINT) hFileNull;
#endif

#ifdef PLAT_MACINTOSH
    // WriteUnlock(m_fileRef);
	if (m_fileRef != 0)
            FSClose(m_fileRef);
        m_fileRef = 0;
#endif
	m_bCloseOnDelete = FALSE;
	m_strFileName.Empty();
}

void IKFile::Write(IKString string)
{
	TCHAR *address = (TCHAR *)string;
	int nBytes = string.GetLength() * sizeof(TCHAR);
	
	Write (address, nBytes);
	
#ifdef PLAT_MACINTOSH
	if (DATAI(TEXT("Save_Text_As_Unicode"),0)==1)
	{
		TCHAR b = CHAR(' ');
		Write(&b,sizeof(TCHAR));
	}
#endif

}

void IKFile::WriteLine(IKString string)
{
	Write ( string );

#ifdef PLAT_WINDOWS
	Write ( TEXT("\r\n") );
#endif
#ifdef PLAT_MACINTOSH
	Write ( ("\r") );
#endif

}

bool IKFile::NewFolder( IKString pFolderPath)
{
#ifdef PLAT_MACINTOSH
	
	FSSpec spec;
        TCHAR macFileName[255];
        strcpy(&(macFileName[1]),(TCHAR *)pFolderPath);
        macFileName[0] = IKString::strlen(&(macFileName[1]));
	OSErr err = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &spec );
	if (err!=noErr && err!=fnfErr)
	{
	    return false;
	}
	    
	SInt32 did;
	err = FSpDirCreate ( &spec, smSystemScript, &did );
	if (err!=noErr)
	{
	    return false;
	}
	
	return true;
	
#endif

#ifdef PLAT_WINDOWS

	bool bCreated = false;

	if (DATAI(TEXT("Unicode File Names"),0) != 0)
	{
		int len = IKString::strlen(pFolderPath);
		unsigned short wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,				// utf-8
							0,						// character-type options
							(const char *) pFolderPath, // string to map
							len,				// number of bytes in string
							wide,				// wide-character buffer
							255		// size of buffer
							);
		wide[result] = 0;

		bCreated = !!CreateDirectoryW ( wide, NULL );
	}
	else
	//if (!bCreated)
	{
		bCreated = !!CreateDirectory ( (TCHAR *) pFolderPath, NULL );
	}

	return bCreated;

#endif

}


bool IKFile::Copy(IKString src, IKString dst)
{
#ifdef PLAT_WINDOWS

#ifdef PLUGIN

	//  open source
	IKFile fsrc;
	if (!fsrc.Open((TCHAR *)src,IKFile::modeRead|IKFile::shareDenyWrite))
	{
		return false;
	}

	//  open destination
	IKFile fdst;
	if (!fdst.Open((TCHAR *)dst,IKFile::modeWrite|IKFile::modeCreate))
	{
		fsrc.Close();
		return false;
	}

	//  copy the bytes
	while (1==1)
	{
		//  read a chunk
		BYTE data[1000];
		int nread = fsrc.Read((void *)data, 1000);
		if (nread==0)
			break;

		//  write a chunk
		fdst.Write((void *)data, nread);

	}

	//  close files and return goodness.
	fsrc.Close();
	fdst.Close();

	return true;
	
#else

	bool bResult = false;

	if (DATAI(TEXT("Unicode File Names"),0) != 0)
	{
		int lensrc = IKString::strlen(src);
		wchar_t srcw[255];
		int srcEnc = CP_ACP;
		if (src.IsUTF8())
			srcEnc = CP_UTF8;
		int resultsrc = MultiByteToWideChar ( srcEnc, 0, (const char *) src, lensrc, srcw, 255 );
		srcw[resultsrc] = 0;

		int lendst = IKString::strlen(dst);
		wchar_t dstw[255];
		int dstEnc = CP_ACP;
		if (dst.IsUTF8())
			dstEnc = CP_UTF8;
		int resultdst = MultiByteToWideChar ( dstEnc, 0, (const char *) dst, lendst, dstw, 255 );
		dstw[resultdst] = 0;

		bResult = !!::CopyFileW ( srcw, dstw, false );
	}
	else
	{
		bResult = !!::CopyFile ( src, dst, false );
	}

	return bResult;

#endif

#else

#if 0

	//  open source
	IKFile fsrc;
	if (!fsrc.Open((TCHAR *)src,IKFile::modeRead))
	{
		return false;
	}

	//  open destination
	IKFile fdst;
	if (!fdst.Open((TCHAR *)dst,IKFile::modeWrite|IKFile::modeCreate))
	{
		fsrc.Close();
		return false;
	}

	//  copy the bytes
	while (1==1)
	{
		//  read a chunk
		BYTE data[1000];
		int nread = fsrc.Read((void *)data, 1000);
		if (nread==0)
			break;

		//  write a chunk
		fdst.Write((void *)data, nread);

	}

	//  close files and return goodness.
	fsrc.Close();
	fdst.Close();

	return true;
        
#else

    //  spec the source name.  Must exist.
    FSSpec srcSpec;
    TCHAR macFileName[255];
    IKString::strcpy(&(macFileName[1]),src);
    macFileName[0] = IKString::strlen(&(macFileName[1]));
    OSErr err = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &srcSpec );
//  OSErr err = FSPathMakeRef((const UInt8) macFileName, NULL, false);
    if (err!=noErr)
            return false;
            
    //  spec the new directory
    FSSpec dstSpec;
    int dirlen = dst.ReverseFind(':');
    IKString::strncpy(&(macFileName[1]),dst,dirlen);
    macFileName[0] = dirlen;    
    err = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &dstSpec );
    if (err!=noErr)
            return false;
    //  get the filename part
    IKString name = dst.Mid(dirlen+1,999);
    IKString::strcpy(&(macFileName[1]),name);
    macFileName[0] = IKString::strlen(&(macFileName[1]));
            
    //  copy away.
	
#ifdef PLAT_MACINTOSH_X

    
    //// TODO replace FSpFileCopy
    //////    err = FSpFileCopy ( (const FSSpec *)&srcSpec, (const FSSpec *)&dstSpec, (const unsigned char *)macFileName, 0, 0, false );
    
    FSRef srcRef;
    FSpMakeFSRef (&srcSpec, &srcRef);
    
    FSRef dstRef;
    FSpMakeFSRef (&dstSpec, &dstRef);
    
    CFStringRef fileName = CFStringCreateWithPascalString(kCFAllocatorDefault, (ConstStr255Param)macFileName, kTextEncodingMacRoman);
    assert (fileName != NULL);
    if (fileName != NULL) {
        err = FSCopyObjectSync (&srcRef, &dstRef, fileName, NULL, kFSFileOperationDefaultOptions);
        CFRelease (fileName);
    }
    
#else
    err = FSpFileCopy ( (const FSSpec *)&srcSpec, (const FSSpec *)&dstSpec, (const unsigned char *)macFileName, 0, 0, false );

#endif

    return ( err==noErr );

#endif

#endif

}

unsigned int IKFile::GetModTimeSecs1970(IKString filename)
{

#ifdef PLAT_WINDOWS

	if (DATAI(TEXT("Unicode File Names"),0) != 0)
	{

		//  unicode file name
		int len = filename.GetLength();
		unsigned short wide[255];
		int widel = MultiByteToWideChar ( CP_UTF8,	0, (const char *) filename, len, wide, 255);
		wide[widel] = 0;

		//  get file info
		WIN32_FIND_DATAW findFileData;
		HANDLE hFind = FindFirstFileW(wide, &findFileData);
		if (hFind == INVALID_HANDLE_VALUE)
			return 0;
		FindClose(hFind);

		//  Get Janary 1, 1970 as a FILETIME.
		SYSTEMTIME stJan1970 = {1970,1,0,1,0,0,0,0};
		FILETIME ftJan1970;
		SystemTimeToFileTime(&stJan1970,&ftJan1970);

		//  make large ints for each
		LARGE_INTEGER liFileTime;
		liFileTime.LowPart = findFileData.ftLastWriteTime.dwLowDateTime;
		liFileTime.HighPart = findFileData.ftLastWriteTime.dwHighDateTime;
		LARGE_INTEGER li1970;
		li1970.LowPart = ftJan1970.dwLowDateTime;
		li1970.HighPart = ftJan1970.dwHighDateTime;

		//  subtract and divide by 10,000,000;
		LARGE_INTEGER liDiff;
		liDiff.QuadPart = liFileTime.QuadPart - li1970.QuadPart;
		liDiff.QuadPart /= 10000000I64;
		IKASSERT(liDiff.HighPart==0);

		//  return the low part
		unsigned int result = liDiff.LowPart;
		return result;

	}
	else
	{
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile((LPTSTR)filename, &findFileData);
		if (hFind == INVALID_HANDLE_VALUE)
			return 0;
		FindClose(hFind);

		//  Get Janary 1, 1970 as a FILETIME.
		SYSTEMTIME stJan1970 = {1970,1,0,1,0,0,0,0};
		FILETIME ftJan1970;
		SystemTimeToFileTime(&stJan1970,&ftJan1970);

		//  make large ints for each
		LARGE_INTEGER liFileTime;
		liFileTime.LowPart = findFileData.ftLastWriteTime.dwLowDateTime;
		liFileTime.HighPart = findFileData.ftLastWriteTime.dwHighDateTime;
		LARGE_INTEGER li1970;
		li1970.LowPart = ftJan1970.dwLowDateTime;
		li1970.HighPart = ftJan1970.dwHighDateTime;

		//  subtract and divide by 10,000,000;
		LARGE_INTEGER liDiff;
		liDiff.QuadPart = liFileTime.QuadPart - li1970.QuadPart;
		liDiff.QuadPart /= 10000000I64;
		IKASSERT(liDiff.HighPart==0);

		//  return the low part
		unsigned int result = liDiff.LowPart;
		return result;
	}



#else

	struct stat fileInfo;
	time_t modTime = 0;
	
#ifdef PLAT_MACINTOSH_X
	IKString filename2 = IKFile::HFSToUnix ( filename );
#else
	IKString filename2 = filename;
#endif
	if (!filename2.IsEmpty())
	{
		if (stat((char *)filename2, &fileInfo) >= 0) 
		{
			modTime = fileInfo.st_mtime;
		}
	}

	return modTime;

#endif

}

void IKFile::SetFileLocked(IKString filename, bool bSet)
{

#ifdef PLAT_MACINTOSH

	FSSpec spec;
        TCHAR macFileName[255];
        strcpy(&(macFileName[1]),(TCHAR *)filename);
        macFileName[0] = IKString::strlen(&(macFileName[1]));

	if (::FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &spec )==noErr)
		if (bSet)
			;//::FSpSetFLockCompat(&spec);
		else
			;//::FSpRstFLockCompat(&spec);

#endif

#ifdef PLAT_WINDOWS

	//  TODO: Unicode this!!

	DWORD dwAttribute = GetFileAttributes((TCHAR *)filename);
	//IKASSERT(dwAttribute != 0xFFFFFFFF);

	if (bSet)
		dwAttribute |= FILE_ATTRIBUTE_READONLY;
	else
		dwAttribute &= (!FILE_ATTRIBUTE_READONLY);

	BOOL bResult = SetFileAttributes ( filename, dwAttribute );
	//IKASSERT(bResult);

#endif

	return;
}

void IKFile::MarkAsUnicode()
{
	bool bDoit = false;
#ifdef PLAT_WINDOWS
#ifdef UNICODE
	bDoit = true;
#endif
#endif

#ifdef PLAT_MACINTOSH
	if (DATAI(TEXT("Save_Text_As_Unicode"),0)==1)
		bDoit = true;
#endif

	if (bDoit)
	{
		Write(&(UnicodeBOM[0]),1);
		Write(&(UnicodeBOM[1]),1);
		WriteLine(TEXT(""));
	}

}

void IKFile::MarkAsUTF8()
{
	Write(&(UTF8BOM[0]),1);
	Write(&(UTF8BOM[1]),1);
	Write(&(UTF8BOM[2]),1);
	WriteLine(TEXT(""));
}

void IKFile::MakeWritable(IKString filenameIn, int nParents )
{

	IKString filename = filenameIn;

	for (int i = 0;i<nParents+1;i++)
	{
		if (i > 0)
		{
			//  do one folder up
			int j = filename.ReverseFind(IKUtil::GetPathDelimiter());
			if (j>=0)
			{
				filename = filename.Left(j);
			}
		}

#ifdef PLAT_MACINTOSH

		//  make FSSpec from the filename
		FSSpec spec;
		TCHAR macFileName[255];
		strcpy(&(macFileName[1]),(TCHAR *)filename);
		macFileName[0] = IKString::strlen(&(macFileName[1]));
		OSErr err = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &spec );
		if (err!=noErr)
			return;
		
		//  now make the FSRef
		FSRef ref;
		err = FSpMakeFSRef ( &spec, &ref );
		if (err!=noErr)
			return;

#ifdef PLAT_MACINTOSH_X

		CFURLRef url = CFURLCreateFromFSRef ( NULL, &ref );
		if (url!=NULL)
		{
			char path[256];
			if (CFURLGetFileSystemRepresentation(url,true,(UInt8 *)path,256))
			{
				uid_t uid = getuid();
				gid_t gid = getgid();
				int result;
            
				result = chown ( path, uid, gid );
				result = chmod (path, 0777);
			}
			CFRelease(url);
		}
    
#else

		//  get the current perms
		FSCatalogInfo catalogInfo;
		err = FSGetCatalogInfo( &ref, kFSCatInfoPermissions, &catalogInfo, 
								NULL, NULL, NULL );
		if (err!=noErr)
			return;

		//  set the new perms
		FSPermissionInfo    *permissions;
		permissions = (FSPermissionInfo*)(catalogInfo.permissions);
		permissions->mode    |= 0666; 
		err = FSSetCatalogInfo( &ref, kFSCatInfoPermissions, &catalogInfo );

#endif

#endif

#ifdef PLAT_WINDOWS

		//  if we're looking at the Windows directory, stop here.
		TCHAR wpath[255];
		::GetWindowsDirectory(wpath,255);
		int l = IKString::strlen(wpath)-1;
		if ( wpath[l]==TEXT('\\') )
			wpath[l] = 0;
		IKString s1(wpath);
		IKString s2(filename);
		if (s1.CompareNoCase(s2)==0)
			return;


		//  TODO: Unicode this!!

		//  for NTFS

		//LPTSTR FileName;

		LPTSTR TrusteeName = TEXT("EVERYONE");
		DWORD AccessMask   = GENERIC_ALL;
		DWORD InheritFlag  = NO_INHERITANCE;
		ACCESS_MODE option = GRANT_ACCESS;

		PACL ExistingDacl = NULL;
		PACL NewAcl = NULL;
		PSECURITY_DESCRIPTOR psd = NULL;
		DWORD dwError;
		EXPLICIT_ACCESS explicitaccess;


		// get current Dacl on specified file

		dwError = GetNamedSecurityInfo(
						(TCHAR *)filename,
						SE_FILE_OBJECT,
						DACL_SECURITY_INFORMATION,
						NULL,
						NULL,
						&ExistingDacl,
						NULL,
						&psd
						);

		if (dwError == ERROR_SUCCESS) 
		{

			// add specified access to the object

			BuildExplicitAccessWithName(
					&explicitaccess,
					TrusteeName,
					AccessMask,
					option,
					InheritFlag
					);

			dwError = SetEntriesInAcl(
					1,
					&explicitaccess,
					ExistingDacl,
					&NewAcl
					);

			if (dwError == ERROR_SUCCESS) 
			{

				//
				// apply new security to file
				//

				dwError = SetNamedSecurityInfo(
								(TCHAR *)filename,
								SE_FILE_OBJECT, // object type
								DACL_SECURITY_INFORMATION,
								NULL,
								NULL,
								NewAcl,
								NULL
								);
								
				if (dwError == ERROR_SUCCESS) 
				{
				}

				if( NewAcl != NULL ) 
					AccFree( NewAcl );
			}

			if( ExistingDacl != NULL ) 
				AccFree( ExistingDacl );

			if( psd != NULL ) 
				AccFree( psd );
		}
		
#endif

		SetFileLocked ( filename, false );
	}
}

bool IKFile::FolderExists ( IKString foldername )
{
	//  see if a folder exists

#ifdef PLAT_WINDOWS

	WIN32_FIND_DATA findFileData;
	
	HANDLE h = ::FindFirstFile( foldername, &findFileData );
	
	if ( h == INVALID_HANDLE_VALUE )
		return false;
		
	::FindClose ( h );
	return true;

#else
	
	TCHAR path[255];
	IKString::strcpy(&(path[1]),(TCHAR *)foldername);
	path[0] = IKString::strlen(&(path[1]));	

	FSSpec theSpec;
	OSErr err = ::FSMakeFSSpec( 0, 0, (const unsigned char *)path, &(theSpec) );	// get spec for directory
	return ( err == noErr );

#endif
}

bool IKFile::FileExists ( IKString filename )
{
	//  see if a file exists

#ifdef PLAT_WINDOWS

	bool bExists = false;

	if (DATAI(TEXT("Unicode File Names"),0) != 0)
	{
		int len = IKString::strlen(filename);
		unsigned short wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,				// utf-8
							0,						// character-type options
							(const char *) filename, // string to map
							len,				// number of bytes in string
							wide,				// wide-character buffer
							255		// size of buffer
							);
		wide[result] = 0;

		WIN32_FIND_DATAW findFileData;

		HANDLE hFind = FindFirstFileW(wide, &findFileData);

		bExists = !(hFind == INVALID_HANDLE_VALUE);
		FindClose(hFind);

	}
	else
	{
		WIN32_FIND_DATA findFileData;

		HANDLE hFind = FindFirstFile((LPTSTR)filename, &findFileData);

		bExists = !(hFind == INVALID_HANDLE_VALUE);
		FindClose(hFind);
	}

	return bExists;

#else

	IKString newFileName = filename;
	if (newFileName.Find('/')>=0)
		newFileName = IKFile::UnixToHFS (newFileName);

	TCHAR macFileName[255];
	IKString::strcpy(&(macFileName[1]),(TCHAR *)newFileName);
	macFileName[0] = IKString::strlen(&(macFileName[1]));

	FSSpec spec;
	OSErr err = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &spec );
	if (err != noErr)
		return false;

#endif

	return true;
}

#ifdef PLAT_MACINTOSH
#ifndef CLASSIC_SHLIB


IKString IKFile::HFSToUnix ( IKString filename, bool bIsDirectory /*=false*/  )
{
	CFStringRef stringRef = CFStringCreateWithCString( kCFAllocatorDefault, (TCHAR *)filename, kCFStringEncodingMacRoman ); 
	
    //NSLog(@"IKFile::HFSToUnix stringRef %@", stringRef);

	CFURLRef urlRef = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, stringRef, kCFURLHFSPathStyle, bIsDirectory ); 

	stringRef = CFURLCopyFileSystemPath( urlRef, kCFURLPOSIXPathStyle ); 

	TCHAR cstring[256];
	CFStringGetCString( stringRef, cstring, 256, kCFStringEncodingMacRoman ); 
	
    //NSLog(@"IKFile::HFSToUnix cstring %s", cstring);

	IKString s = cstring;
	return s;

}

IKString IKFile::UnixToHFS ( IKString filename, bool bIsDirectory /*=false*/  )
{
	CFStringRef stringRef = CFStringCreateWithCString( kCFAllocatorDefault, (TCHAR *)filename, kCFStringEncodingMacHFS ); 
	
    //NSLog(@"IKFile::UnixToHFS stringRef %@", stringRef);

	CFURLRef urlRef = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, stringRef, kCFURLPOSIXPathStyle, bIsDirectory ); 

	stringRef = CFURLCopyFileSystemPath( urlRef, kCFURLHFSPathStyle ); 

	TCHAR cstring[256];
	CFStringGetCString( stringRef, cstring, 256, kCFStringEncodingMacHFS ); 
	
    //NSLog(@"IKFile::UnixToHFS cstring %s", cstring);

	IKString s = cstring;
	return s;
}


#endif   // CLASSIC_SHLIB
#endif   // PLAT_MACINTOSH_X


#ifdef PLAT_WINDOWS

//  this callback function centers the open file dialog on the screen.

static UINT_PTR CALLBACK OFNHookProc( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uiMsg )
	{
	case WM_NOTIFY:
		LPOFNOTIFY pofn = (LPOFNOTIFY) lParam;
		if ( pofn->hdr.code == CDN_INITDONE )
		{
			HWND hWnd = GetParent(hdlg);
			RECT rect;
			GetWindowRect(hWnd, &rect);

			/* calculate dimensions */
			int nWidth = rect.right - rect.left;
			int nHeight = rect.bottom - rect.top;

			/* find the coordinates of the upper-left corner */
			int x = (GetSystemMetrics(SM_CXSCREEN) - nWidth) / 2;
			int y = (GetSystemMetrics(SM_CYSCREEN) - nHeight) / 2;

			/* move it */
			SetWindowPos(hWnd, NULL, x, y, -1, -1, SWP_NOZORDER | SWP_NOSIZE);
		}
		break;
	}

	return 0;
}

typedef struct tagOFNx { 
  DWORD         lStructSize; 
  HWND          hwndOwner; 
  HINSTANCE     hInstance; 
  LPCTSTR       lpstrFilter; 
  LPTSTR        lpstrCustomFilter; 
  DWORD         nMaxCustFilter; 
  DWORD         nFilterIndex; 
  LPTSTR        lpstrFile; 
  DWORD         nMaxFile; 
  LPTSTR        lpstrFileTitle; 
  DWORD         nMaxFileTitle; 
  LPCTSTR       lpstrInitialDir; 
  LPCTSTR       lpstrTitle; 
  DWORD         Flags; 
  WORD          nFileOffset; 
  WORD          nFileExtension; 
  LPCTSTR       lpstrDefExt; 
  LPARAM        lCustData; 
  LPOFNHOOKPROC lpfnHook; 
  LPCTSTR       lpTemplateName; 
//#if (_WIN32_WINNT >= 0x0500)
  void *        pvReserved;
  DWORD         dwReserved;
  DWORD         FlagsEx;
//#endif // (_WIN32_WINNT >= 0x0500)
} OPENFILENAMEx, *LPOPENFILENAMEx;
#endif

#ifdef EXLUDE_FROM_LION_BUILD
#ifdef PLAT_MACINTOSH
#ifndef BUILD_DAEMON
#ifndef CLASSIC_SHLIB
static Boolean GetAFile( IKString title, FSSpec *fileSpec )
{
	NavReplyRecord		reply;
	OSErr				err;
	NavDialogOptions	options;
	Boolean				result = false;
	
	NavGetDefaultDialogOptions( &options );

	options.dialogOptionFlags &= ~kNavAllowMultipleFiles;
	
	// window title
    ::CopyCStringToPascal((TCHAR *)title, options.windowTitle); 

	err = NavGetFile( NULL, &reply, &options,
								NULL, NULL, NULL, NULL, NULL);
	
	if ( (err == noErr) && reply.validRecord )
	{
		AEDesc		fileSpecDesc = { typeNull, nil };
		AEKeyword	unusedKeyword;

		err = AEGetNthDesc( &reply.selection, 1, typeFSS, &unusedKeyword, &fileSpecDesc );

		if ( err == noErr )
			AEGetDescData(&fileSpecDesc, fileSpec, sizeof(FSSpec) );

		AEDisposeDesc( &fileSpecDesc );
	
		result = true;
	}

	NavDisposeReply( &reply );

	return result;
}
#endif
#endif
#endif
#endif

bool IKFile::GetOneFile( IKString strTitle, IKString &filename, TCHAR *filter /* =NULL */ )
{
#ifdef PLAT_WINDOWS

	//  set up an open file name structure;
	//  set the sizeof the structure differently
	//  depending on the OS
	OPENFILENAMEx ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAMEx));
	if (IKUtil::IsWin2KOrGreater())
		ofn.lStructSize = sizeof(OPENFILENAMEx);
	else
		ofn.lStructSize = sizeof(OPENFILENAME);

	//  no owner window
	ofn.hwndOwner = NULL;

	//  file type filter
	char buffer[1024];
	unsigned short buffer2[1024];  //  for unicode
	if (filter)
	{
		IKString strFilter(filter);
		if (!strFilter.IsEmpty())
		{
			strcpy(buffer,(TCHAR *)strFilter);
			int i = strFilter.Find('|');
			if (i>=0)
				buffer[i] = 0;
			buffer[strFilter.GetLength()+1] = 0;

			//  use UNICODE
			int len = IKString::strlen(buffer);
			int result = MultiByteToWideChar (CP_UTF8,0,(const char *) buffer,len,buffer2,1024);
			buffer2[result] = 0;
			ofn.lpstrFilter = (char *)buffer2;
		}
	}

	// resulting filename
	unsigned short szFileName[MAX_PATH]; szFileName[0]=0;
	ofn.lpstrFile = (LPTSTR)szFileName;
	ofn.nMaxFile = MAX_PATH;

	//  flags.
	ofn.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_FILEMUSTEXIST|OFN_ENABLESIZING|OFN_PATHMUSTEXIST;
	ofn.lpfnHook = (LPOFNHOOKPROC) OFNHookProc;
	ofn.Flags |= OFN_ENABLEHOOK;
	ofn.Flags |= OFN_EXPLORER;

	//  title
	//  use UNICODE
	int len = strTitle.GetLength();
	unsigned short buffer3[1024];
	int result = MultiByteToWideChar (CP_UTF8,0,(const char *) (TCHAR *)strTitle,len,buffer3,1024);
	buffer3[result] = 0;
	ofn.lpstrTitle = (char *)buffer3;

	//  do the ask
	if ( GetOpenFileNameW((LPOPENFILENAMEW)&ofn) )
	{
		int len=0;
		while(szFileName[len])
			len++;
		char narrow[1024];
		int result2 = WideCharToMultiByte ( CP_UTF8, 0, szFileName, len, narrow, 1024, 0, 0);
		narrow[result2] = 0;
		filename = IKString(narrow);
		return true;
	}
	
	return false;

#endif

#ifdef EXLUDE_FROM_LION_BUILD
#ifdef PLAT_MACINTOSH
#ifndef BUILD_DAEMON
#ifndef CLASSIC_SHLIB

	//  display a dialog to get a path to a file
	FSSpec fs;
	if (GetAFile((TCHAR *)strTitle,&fs))
	{
		char fullpath[255];
		void GetFullPath (long DirID, short vRefNum, TCHAR *path);
		GetFullPath ( fs.parID, fs.vRefNum, fullpath );
		
		//  tack on the file name
		int len = fs.name[0];
		fs.name[len+1] = 0;
		IKString::strcat(fullpath,(const char *)&(fs.name[1]));

		filename = IKString(fullpath);
		return true;
	}
	
	return false;
	
#endif
#endif
#endif
#endif
    
return false;

}

