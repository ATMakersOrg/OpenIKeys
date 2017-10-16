/*
	File:		Optimization.h
	Contains:	Defines that let you make MoreFiles code more efficient.
	Version:	MoreFiles
	Copyright:	© 1992-2002 by Apple Computer, Inc., all rights reserved.
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.
				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.
				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.
				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	File Ownership:
		DRI:				Apple Macintosh Developer Technical Support
		Other Contact:		Apple Macintosh Developer Technical Support
							<http://developer.apple.com/bugreporter/>
		Technology:			DTS Sample Code
	Writers:
		(JL)	Jim Luther
	Change History (most recent first):
		 <3>	 8/23/02	JL		[2853901]  Updated standard disclaimer.
		 <2>	  7/6/01	JL		[2661720]  Make comments ANSI C compliant.
		 <1>	  2/7/01	JL		first checked in
*/
/*
	The Optimization changes to MoreFiles source and header files, along with
	this file and OptimizationEnd.h, let you optimize the code produced
	by MoreFiles in several ways.
	
	1 -- MoreFiles contains extra code so that many routines can run under
	Mac OS systems back to System 6. If your program requires a specific
	version of Mac OS and your program checks for that version before
	calling MoreFiles routines, then you can remove a lot of compatibility
	code by defining one of the following to 1:
	
		__MACOSSEVENFIVEONEORLATER	// assume Mac OS 7.5.1 or later
		__MACOSSEVENFIVEORLATER		// assume Mac OS 7.5 or later
		__MACOSSEVENORLATER			// assume Mac OS 7.0 or later
	
	If you're compiling 68K code, the default is to include all compatibility code.
	If you're compiling PowerPC code (TARGET_RT_MAC_CFM), the default is __MACOSSEVENORLATER
	If you're compiling for Carbon code (TARGET_API_MAC_CARBON), the default is __MACOSSEVENFIVEONEORLATER
	
	2 -- You may disable Pascal calling conventions in all MoreFiles routines
	except for system callbacks that require Pascal calling conventions.
	This will make 68K C programs both smaller and faster. 
	(PowerPC compilers ignore pascal calling conventions.)
	Just define __WANTPASCALELIMINATION to be 1 to turn this optimization on
	when building MoreFiles for use from C programs (you'll need to keep
	Pascal calling conventions when linking MoreFiles routines with Pascal
	programs).
	
	3 -- If Metrowerks compiler is used, "#pragma internal on" may help produce
	better code. However, this option can also cause problems if you're
	trying to build MoreFiles as a shared library, so it is by default not used.
	Just define __USEPRAGMAINTERNAL to be 1 to turn this optimization on.
	
	Original changes supplied by Fabrizio Oddone
*/
#include "ConditionalMacros.h"
/* if we're compiling for Carbon, then we're running on Mac OS 8.1 or later */
#ifndef __MACOSSEVENFIVEONEORLATER
	#define __MACOSSEVENFIVEONEORLATER TARGET_API_MAC_CARBON
#endif
#ifndef __MACOSSEVENFIVEORLATER
	#define __MACOSSEVENFIVEORLATER __MACOSSEVENFIVEONEORLATER
#endif
#ifndef __MACOSSEVENORLATER
	#if TARGET_RT_MAC_CFM
		#define __MACOSSEVENORLATER 1
	#else
		#define __MACOSSEVENORLATER __MACOSSEVENFIVEORLATER
	#endif
#endif
#ifndef	__WANTPASCALELIMINATION
	#define	__WANTPASCALELIMINATION	0
#endif
#if	__WANTPASCALELIMINATION
	#define pascal	
#endif
#ifndef __USEPRAGMAINTERNAL
	#define	__USEPRAGMAINTERNAL	0
#endif
#if	__USEPRAGMAINTERNAL
	#if defined(__MWERKS__)
		#pragma internal on
	#endif
#endif
