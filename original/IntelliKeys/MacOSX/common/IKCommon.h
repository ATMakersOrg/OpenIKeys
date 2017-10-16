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
    #define TEXT(a) (const char *)a
    #define CHAR(a) (TCHAR)(a)
#endif
#endif


// JR - Dec 2012 - added common version macro for Software
#define INTELLIKEYS_USB_PRODUCT_VERSION  "3.5.2.19"

//  some trace functions for debuggging

#ifdef PLAT_WINDOWS

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

