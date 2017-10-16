// WinHeader.h
//
// This file declares macros and functions needed for Win32 compilation
// of plugins using the REALbasic Plugin SDK.  This file should be
// set as the prefix file for your Windows (x86) target.  In addition,
// you'll need to add the "Win32-x86 Support" folder to the system
// access paths.
//
// WinHeader.cpp does not include the C++ precompiled headers, 
// but WinHeader++.cpp does.  The only noticeable difference is 
// if you are using COM.  If you include WinHeader.cpp you have
// to access a COM class' member functions via the vtable pointer,
// but with WinHeader++.cpp it is seamless.
//
// © 1997-2002 REAL Software Inc. -- All Rights Reserved
// See file "Plug-in License SDK.txt" for details.
#ifndef WINHEADER_H
#define WINHEADER_H
#include <Win32Headers.mch>
namespace QT {
	#include <QTML.h>
	#include <Movies.h>
}
typedef unsigned char Boolean;
typedef char *Ptr;
struct Rect
{
	short top, left, bottom, right;
};
struct Point
{
	short v, h;
};
#ifdef nil
  #undef nil
#endif
#define nil 0
#ifdef NULL
  #undef NULL
#endif
#define NULL 0
#ifndef true
	#define true 1
	#define false 0
#endif
long TickCount(void);
Boolean PtInRect(const Point &pt, Rect *rBounds);
Boolean SectRect(Rect *rect1, Rect *rect2, Rect *resultRect);
Ptr NewPtr(int len);
void DisposePtr(Ptr p);
#endif
