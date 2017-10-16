// winheader.cpp
//
// This file defines functions needed for Win32 REALbasic plugins.
// This file should be included in your Windows (x86) target.
// Remember to set your prefix file to winheader.h.
//
// © 1997-2000 REAL Software Inc. -- All Rights Reserved
// See file "Plug-in License SDK.txt" for details.

#ifndef WINHEADER_H
#warning Set your prefix file to "winheader.h"
#endif

// The following stuff is definitely needed despite the fact that its defined
// in QTML; the Quicktime versions aren't compatible with RB code, and even
// if they were we can't count on QuickTime being available.

Boolean PtInRect(const Point &pt, Rect *rBounds)
{
	if (pt.h >= rBounds->left && pt.h < rBounds->right && pt.v >= rBounds->top
			&& pt.v < rBounds->bottom)
		return true;
	return false;
}

inline static short _min(short a, short b)
{
	return a < b ? a : b;
}

inline static short _max(short a, short b)
{
	return a > b ? a : b;
}

Boolean SectRect(Rect *rect1, Rect *rect2, Rect *resultRect)
{
	if (not rect1 or not rect2) return false;
	if (rect1->right < rect2->left
	 or rect2->right < rect1->left
	 or rect1->bottom < rect2->top
	 or rect2->bottom < rect1->top) {
		if (resultRect) {
			resultRect->left = resultRect->top = resultRect->right = resultRect->bottom = 0;
			return false;
		}
	}
	
	if (resultRect) {
		Rect r;
		r.left = _max(rect1->left, rect2->left);
		r.top = _max(rect1->top, rect2->top);
		r.right = _min(rect1->right, rect2->right);
		r.bottom = _min(rect1->bottom, rect2->bottom);
		*resultRect = r;
	}
	return true;
}

long TickCount(void)
{
	return GetTickCount() / 16;
}

Ptr NewPtr(int len)
{
	return (Ptr) malloc(len);
}

void DisposePtr(Ptr p)
{
	free(p);
}

/*void Debugger(void)
{
	_asm int 3
}

Ptr NewPtrClear(long size)
{
	Ptr p = (Ptr) malloc(size);
	if (!p)
		return nil;
	memset(p, 0, size);
	return p;
}*/

