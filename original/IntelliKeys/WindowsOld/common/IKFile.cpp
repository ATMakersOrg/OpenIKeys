// IKFile.cpp: implementation of the IKFile class.
//
//////////////////////////////////////////////////////////////////////

#include "IKCommon.h"
#include "IKFile.h"
#include "IKUtil.h"

#ifdef PLAT_WINDOWS
#include <sys/stat.h>
#include <aclapi.h>
#include <lmerr.h>
#endif

static BYTE UnicodeBOM[2] = {0xff, 0xfe};
static BYTE UTF8BOM[3] = {0xef, 0xbb, 0xbf};

IKFile::IKFile()
:
	m_hFile(hFileNull),
	m_bCloseOnDelete(FALSE), m_bUnicode(false), m_bUtf8(false)
{
	m_hFile = (UINT) hFileNull;

	m_bCloseOnDelete = FALSE;

	m_bUnicode = false;
	m_bUtf8 = false;

	m_strFileName.Empty();
}

IKFile::~IKFile()
{
	if ((m_hFile != (UINT)hFileNull) && m_bCloseOnDelete)
	{
		Close();
	}
}

unsigned int IKFile::GetPosition() const
{
	DWORD dwPos = SetFilePointer((HANDLE)m_hFile, 0, NULL, FILE_CURRENT);
	return dwPos;
}

IKString IKFile::GetFileName() const
{
	return m_strFileName;
}


bool IKFile::Open(TCHAR * lpszFileName, unsigned int nOpenFlags)
{
		//  bad filename
	if ( lpszFileName == NULL )	
	{
		return FALSE;
	}

	IKString s = lpszFileName;
	if (s.Compare(TEXT("")) == 0)
	{
		return FALSE;
	}
		

	// CFile objects are always binary and CreateFile does not need flag
	nOpenFlags &= ~(UINT)typeBinary;

	m_bCloseOnDelete = FALSE;
	m_hFile = (UINT)hFileNull;
	m_strFileName.Empty();

	m_strFileName = lpszFileName;

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
			break;
	}

	// map share mode
	DWORD dwShareMode = 0;
	switch (nOpenFlags & 0x70)    
	{
		// map compatibility mode to exclusive
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
		default:
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
		{
			dwCreateFlag = OPEN_ALWAYS;
		}
		else
		{
			dwCreateFlag = CREATE_ALWAYS;
		}
	}
	else
	{
		dwCreateFlag = OPEN_EXISTING;
	}

	//  attempt file creation
	//  use a Unicode or non-Unicode flavor, depending.

	HANDLE hFile = INVALID_HANDLE_VALUE;

	if (DATAI(TEXT("Unicode_File_Names"), 0) != 0)
	{
		int len = IKString::strlen(lpszFileName);
		WCHAR wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,						// utf-8
							0,								// character-type options
							(const char *) lpszFileName,	// string to map
							len,							// number of bytes in string
							wide,							// wide-character buffer
							255								// size of buffer
							);
		wide[result] = 0;


		hFile = CreateFileW(wide, dwAccess, dwShareMode, &sa,
				dwCreateFlag, FILE_ATTRIBUTE_NORMAL, NULL);	}
	else
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

	//  start file from beginning
	Seek ( 0, IKFile::begin );

	//  read the first 2 bytes
	BYTE b1 = 0;
	BYTE b2 = 0;
	BYTE b3 = 0;
	Read(&b1,1);
	Read(&b2,1);
	Read(&b3,1);

	//  check for Unicode
	m_bUnicode = false;
	if ((b1 == UnicodeBOM[0]) && (b2 == UnicodeBOM[1]))
	{
		m_bUnicode = true;
	}
	else if ((b1 == UTF8BOM[0]) && (b2 == UTF8BOM[1]) && (b3 == UTF8BOM[2]))
	{
		//DebugLogToFile("IKFile::Open - File=[%s] Is UTF-8 Encoded", lpszFileName);

		m_bUtf8 = true;
	}
	//  back to the top
	Seek ( 0, IKFile::begin );

	return TRUE;
}

unsigned int IKFile::Read(void* lpBuf, unsigned int nCount)
{
	if (nCount == 0)
	{
		return 0;   // avoid Win32 "null-read"
	}

	DWORD dwRead;
	bool b = !!ReadFile((HANDLE)m_hFile, lpBuf, nCount, &dwRead, NULL);

	return (UINT)dwRead;
}

void IKFile::Write(const void* lpBuf, unsigned int nCount)
{
	if (nCount == 0)
	{
		return;     // avoid Win32 "null-write" option
	}

	DWORD nWritten;
	bool b = !!WriteFile((HANDLE)m_hFile, lpBuf, nCount, &nWritten, NULL);

}

void IKFile::Rename(TCHAR * lpszOldName,	TCHAR * lpszNewName)
{
	bool bOK = false;
	if (DATAI(TEXT("Unicode_File_Names"),0) != 0)
	{
		int lenold = IKString::strlen(lpszOldName);
		WCHAR oldw[255];
		int resultold = MultiByteToWideChar (
							CP_UTF8,					// utf-8
							0,							// character-type options
							(const char *) lpszOldName, // string to map
							lenold,						// number of bytes in string
							oldw,						// wide-character buffer
							255							// size of buffer
							);
		oldw[resultold] = 0;

		int lennew = IKString::strlen(lpszNewName);
		WCHAR neww[255];
		int resultnew = MultiByteToWideChar (
							CP_UTF8,					// utf-8
							0,							// character-type options
							(const char *) lpszNewName, // string to map
							lennew,						// number of bytes in string
							neww,						// wide-character buffer
							255							// size of buffer
							);
		neww[resultnew] = 0;


		bOK = !!MoveFileW( oldw, neww );
	}
	else
	{
		bOK = !!MoveFile((LPTSTR)lpszOldName, (LPTSTR)lpszNewName);
	}
}

void IKFile::Remove(TCHAR * lpszFileName)
{
	bool bOK = false;
	if (DATAI(TEXT("Unicode_File_Names"), 0) != 0)
	{
		int len = IKString::strlen(lpszFileName);
		WCHAR wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,						// utf-8
							0,								// character-type options
							(const char *) lpszFileName,	// string to map
							len,							// number of bytes in string
							wide,							// wide-character buffer
							255								// size of buffer
							);
		wide[result] = 0;


		bOK = !!DeleteFileW(wide);
	}
	else
	{
		bOK = !!DeleteFile(lpszFileName);
	}
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
}

void IKFile::SetLength(unsigned int dwNewLen)
{
	Seek((LONG)dwNewLen, (UINT)begin);

	bool b = !!SetEndOfFile((HANDLE)m_hFile);
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
	if (m_hFile != (UINT)hFileNull)
	{
		// close but ignore errors
		CloseHandle((HANDLE)m_hFile);
		m_hFile = (UINT)hFileNull;
	}
	m_strFileName.Empty();
}

void IKFile::Flush()
{
	if (m_hFile == (UINT)hFileNull)
	{
		return;
	}

	bool b = !!FlushFileBuffers((HANDLE)m_hFile);
}

void IKFile::Close()
{
	if (m_hFile != (UINT)hFileNull)
	{
		CloseHandle((HANDLE)m_hFile);
	}

	m_hFile = (UINT) hFileNull;
	m_bCloseOnDelete = FALSE;
	m_strFileName.Empty();
}

void IKFile::Write(IKString string)
{
	TCHAR *address = (TCHAR *)string;
	int nBytes = string.GetLength() * sizeof(TCHAR);
	
	Write (address, nBytes);
}

void IKFile::WriteLine(IKString string)
{
	Write(string + TEXT("\r\n"));
}

bool IKFile::NewFolder(IKString pFolderPath)
{
	bool bCreated = false;

	if (DATAI(TEXT("Unicode_File_Names"), 0) != 0)
	{
		int len = IKString::strlen(pFolderPath);
		WCHAR wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,					// UTF-8
							0,							// character-type options
							(const char *) pFolderPath, // string to map
							len,						// number of bytes in string
							wide,						// wide-character buffer
							255							// size of buffer
							);
		wide[result] = 0;

		bCreated = !!CreateDirectoryW ( wide, NULL );
	}
	else
	{
		bCreated = !!CreateDirectory ( (TCHAR *) pFolderPath, NULL );
	}

	return bCreated;
}


bool IKFile::Copy(IKString src, IKString dst)
{
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

	if (DATAI(TEXT("Unicode_File_Names"), 0) != 0)
	{
		int lensrc = IKString::strlen(src);
		wchar_t srcw[255];
		int srcEnc = CP_ACP;
		if (src.IsUTF8())
		{
			srcEnc = CP_UTF8;
		}
		int resultsrc = MultiByteToWideChar(srcEnc, 0, (const char *) src, lensrc, srcw, 255 );
		srcw[resultsrc] = 0;

		int lendst = IKString::strlen(dst);
		wchar_t dstw[255];
		int dstEnc = CP_ACP;
		if (dst.IsUTF8())
		{
			dstEnc = CP_UTF8;
		}
		int resultdst = MultiByteToWideChar(dstEnc, 0, (const char *) dst, lendst, dstw, 255 );
		dstw[resultdst] = 0;

		bResult = !!::CopyFileW ( srcw, dstw, false );
	}
	else
	{
		bResult = !!::CopyFile ( src, dst, false );
	}

	return bResult;

#endif
}

unsigned int IKFile::GetModTimeSecs1970(IKString filename)
{
	if (DATAI(TEXT("Unicode_File_Names"), 0) != 0)
	{
		//  unicode file name
		int len = filename.GetLength();
		WCHAR wide[255];
		int widel = MultiByteToWideChar(CP_UTF8, 0, (const char *) filename, len, wide, 255);
		wide[widel] = 0;

		//  get file info
		WIN32_FIND_DATAW findFileData;
		HANDLE hFind = FindFirstFileW(wide, &findFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return 0;
		}
		FindClose(hFind);

		//  Get January 1, 1970 as a FILETIME.
		SYSTEMTIME stJan1970 = {1970,1,0,1,0,0,0,0};
		FILETIME ftJan1970;
		SystemTimeToFileTime(&stJan1970,&ftJan1970);

		//  make large int for each
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

		//  Get January 1, 1970 as a FILETIME.
		SYSTEMTIME stJan1970 = {1970,1,0,1,0,0,0,0};
		FILETIME ftJan1970;
		SystemTimeToFileTime(&stJan1970,&ftJan1970);

		//  make large int for each
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
}

void IKFile::SetFileLocked(IKString filename, bool bSet)
{
	if (DATAI(TEXT("Unicode_File_Names"), 0) != 0)
	{
		int len = IKString::strlen(filename);
		WCHAR wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,					// utf-8
							0,							// character-type options
							(const char *) filename,	// string to map
							len,						// number of bytes in string
							wide,						// wide-character buffer
							255							// size of buffer
							);
		wide[result] = 0;

		DWORD dwAttribute = GetFileAttributesW(wide);
		if (bSet)
		{
			dwAttribute |= FILE_ATTRIBUTE_READONLY;
		}
		else
		{
			dwAttribute &= (!FILE_ATTRIBUTE_READONLY);
		}
		BOOL bResult = SetFileAttributesW(wide, dwAttribute);
	}
	else
	{
		DWORD dwAttribute = GetFileAttributesA((TCHAR *)filename);
		if (bSet)
		{
			dwAttribute |= FILE_ATTRIBUTE_READONLY;
		}
		else
		{
			dwAttribute &= (!FILE_ATTRIBUTE_READONLY);
		}
		BOOL bResult = SetFileAttributesA((TCHAR *)filename, dwAttribute);
	}


	//DebugLogToFile("IKFile::SetFileLocked: FileName=[%s] Make Read-Only=[%s] Result=[%s]", (LPSTR)filename, bSet ? "Yes" : "No", bResult == 0 ? "Failed" : "Success");

	return;
}

void IKFile::MarkAsUnicode()
{
#ifdef UNICODE
	Write(&(UnicodeBOM[0]),1);
	Write(&(UnicodeBOM[1]),1);
	WriteLine(TEXT(""));
#endif
}

void IKFile::MarkAsUTF8()
{
	Write(&(UTF8BOM[0]),1);
	Write(&(UTF8BOM[1]),1);
	Write(&(UTF8BOM[2]),1);
	WriteLine(TEXT(""));
}

void IKFile::MakeWritable(IKString filenameIn, int nParents)
{
	//DebugLogToFile("IKFile::MakeWritable start. FileName=[%s] NbrParents=[%d]", (LPSTR)filenameIn, nParents);

	IKString filename = filenameIn;

	for (int i = 0; i < nParents + 1; i++)
	{
		if (i > 0)
		{
			//  do one folder up
			int j = filename.ReverseFind(IKUtil::GetPathDelimiter());
			if (j >= 0)
			{
				filename = filename.Left(j);
			}
		}

		//DebugLogToFile("IKFile::MakeWritable - processing [%s]", (LPSTR)filename);
	
		//  if we're looking at the Windows directory, stop here.
		TCHAR wpath[255];
		::GetWindowsDirectory(wpath, 255);
		int l = IKString::strlen(wpath) - 1;
		if (wpath[l] == TEXT('\\'))
		{
			wpath[l] = 0;
		}
		IKString winPath(wpath);
		IKString currentPath(filename);
		if (winPath.CompareNoCase(currentPath) == 0)
		{

			//DebugLogToFile("IKFile::MakeWritable - cannot make Windows Directory world-writable.");

			return;
		}

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

		//DebugLogToFile("IKFile::MakeWritable - calling GetNamedSecurityInfo");

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

		//DebugLogToFile("IKFile::MakeWritable - GetNamedSecurityInfo Result: %u",  dwError);

		if (dwError == ERROR_SUCCESS) 
		{

			//DebugLogToFile("IKFile::MakeWritable - calling BuildExplicitAccessWithName");

			// add specified access to the object

			BuildExplicitAccessWithName(
					&explicitaccess,
					TrusteeName,
					AccessMask,
					option,
					InheritFlag
					);

			//DebugLogToFile("IKFile::MakeWritable - calling SetEntriesInAcl");

			dwError = SetEntriesInAcl(
					1,
					&explicitaccess,
					ExistingDacl,
					&NewAcl
					);

			//DebugLogToFile("IKFile::MakeWritable - SetEntriesInAcl Result: %u ExistingDacl=%0x NewAcl=%0x pSD=%0x", dwError, ExistingDacl, NewAcl, psd);

			if (dwError == ERROR_SUCCESS) 
			{

				//DebugLogToFile("IKFile::MakeWritable - calling SetNamedSecurityInfo");

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
								
				//DebugLogToFile("IKFile::MakeWritable - SetNamedSecurityInfo Result: %u", dwError);

				if (dwError == ERROR_SUCCESS) 
				{
				}

				if (NewAcl != NULL) 
				{
					//DebugLogToFile("IKFile::MakeWritable - calling AccFree(NewAcl=%0x)", NewAcl);

					AccFree(NewAcl);
				}
			}

			// Note: July 2012 - JR - the MSDN clearly states this 'ExitingDacl' is just a pointer to the 
			//  to the DACL in the returned security descriptor (psd) and thus does not require
			//  a call to AccFree (LocalFree) explicitly. In fact calling AccFree on this pointer
			//  results in an exception on Windows 7.
			//if (ExistingDacl != NULL) 
			//{
			//	AccFree(ExistingDacl);
			//}

			if (psd != NULL)
			{
				//DebugLogToFile("IKFile::MakeWritable - calling AccFree(psd=%0x)", psd);

				AccFree(psd);
			}
		} // GetNamedSecurityInfo succeeded
		
		// remove any Read-Only attribute
		SetFileLocked(filename, false);
	}  // for loop

	//DebugLogToFile("IKFile::MakeWritable end. Input FileName: [%s]", (LPSTR)filenameIn);
}

bool IKFile::FolderExists(IKString foldername)
{
	//  see if a folder exists
	return FileExists(foldername);
	//WIN32_FIND_DATA findFileData;
	//HANDLE h = ::FindFirstFile( foldername, &findFileData );
	//
	//if ( h == INVALID_HANDLE_VALUE )
	//{
	//	return false;
	//}
	//else 
	//{
	//	::FindClose ( h );
	//	return true;
	//}
}

bool IKFile::FileExists(IKString filename)
{
	//  see if a file exists
	bool bExists = false;

	if (DATAI(TEXT("Unicode_File_Names"), 0) != 0)
	{
		int len = IKString::strlen(filename);
		WCHAR wide[255];
		int result = MultiByteToWideChar (
							CP_UTF8,					// UTF-8
							0,							// character-type options
							(const char *) filename,	// string to map
							len,						// number of bytes in string
							wide,						// wide-character buffer
							255							// size of buffer
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
}


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

bool IKFile::GetOneFile( IKString strTitle, IKString &filename, TCHAR *filter /* =NULL */ )
{
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
	WCHAR buffer2[1024];  //  for unicode
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
	WCHAR szFileName[MAX_PATH]; szFileName[0]=0;
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
	WCHAR buffer3[1024];
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
}

