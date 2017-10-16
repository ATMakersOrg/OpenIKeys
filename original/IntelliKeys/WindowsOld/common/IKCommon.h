#if !defined(__COMMON_INCLUDED)
#define __COMMON_INCLUDED

#ifdef PLAT_WINDOWS
#include <windows.h>
#include <stdio.h>
#define MySprintf wsprintf
#endif

#ifdef PLAT_MACINTOSH
#define MySprintf sprintf
#endif

#ifdef PLAT_MACINTOSH
#ifndef UNICODE
  #define TCHAR char
  #define TEXT(a) a
#endif
#endif

//  some trace functions for debugging

#ifdef PLAT_WINDOWS

// JR - Added Oct 2012 for debugging of localized builds in the localized environments

#ifdef PLUGIN_LIB
#define DEBUG_LOG_PATH		TEXT("c:\\temp\\IKUSB_CP_Plugin_Lib.txt")
#elif SYSTRAY_LIB
#define DEBUG_LOG_PATH		TEXT("c:\\temp\\IKUSB_SysTray_Lib.txt")
#elif SERVICE_LIB 
#define DEBUG_LOG_PATH		TEXT("c:\\temp\\IKUSB_Service_Lib.txt")
#elif IKXFER_LIB 
#define DEBUG_LOG_PATH		TEXT("c:\\temp\\IKUSB_IKXfer_Lib.txt")
#endif


static void DebugLogToFile(TCHAR * lpszFormat, ...)
{
#ifdef DEBUG_LOG_PATH
	if (lpszFormat != NULL)
	{
		va_list args;
		va_start(args, lpszFormat);

		int nBuf = 0;
		TCHAR szBuffer[1024] = {0};

		nBuf = _vsnprintf(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), lpszFormat, args);

		if ((nBuf < 0) || (nBuf > 1022))
		{
			nBuf = 1022;
		}
		szBuffer[nBuf] = '\n';
		++nBuf;
		szBuffer[nBuf + 1] = 0;

		va_end(args);

		DWORD bytesWritten = 0;
		HANDLE logHandle = INVALID_HANDLE_VALUE;
		if (logHandle == INVALID_HANDLE_VALUE)
		{
			logHandle = ::CreateFileA(DEBUG_LOG_PATH, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (logHandle == INVALID_HANDLE_VALUE)
			{
				logHandle = ::CreateFileA(DEBUG_LOG_PATH, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			} 
			else 
			{
				::SetFilePointer(logHandle, 0, NULL, FILE_END);
			}
		}

		if (logHandle != INVALID_HANDLE_VALUE) 
		{
			::WriteFile(logHandle, szBuffer, nBuf, &bytesWritten, NULL);
			//::FlushFileBuffers(logHandle);
			::CloseHandle(logHandle);
		}
	}
#endif // DEBUG_LOG_PATH
}



static void _DbgPrint(TCHAR * lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vsnprintf(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), lpszFormat, args);

	szBuffer[nBuf] = TEXT('\n');
	nBuf++;
	szBuffer[nBuf] = 0;
	nBuf++;

	OutputDebugString(szBuffer);

	va_end(args);
}

#define IKTRACE(_x_) _DbgPrint _x_;

#else

#define IKTRACE(_x_)

#endif  //  PLAT_WINDOWS


#undef IKASSERT
#ifdef _DEBUG
	#define IKASSERT(exp) {if (!(exp)) IKTRACE((TEXT("IKASSERT %s in line %d of %s\n"),#exp,(TCHAR *)__LINE__,(TCHAR *)__FILE__));}
#else
	#define IKASSERT(a) (void(0))
#endif

#undef UNUSED
#define UNUSED(a) {}


#ifdef PLAT_MACINTOSH
typedef unsigned char BYTE;
#ifdef BUILD_PB
typedef short WORD;
#endif
#endif

#ifdef PLAT_MACINTOSH_CLASSIC
typedef short WORD;
#endif

#endif  //  __COMMON_INCLUDED

