/*
	File:		IterateDirectory.c
	Contains:	File Manager directory iterator routines.
	Version:	MoreFiles
	Copyright:	© 1995-2002 by Jim Luther and Apple Computer, Inc., all rights reserved.
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
		 <2>	  2/7/01	JL		Added standard header. Updated names of includes.
		<1>		12/06/99	JL		MoreFiles 1.5.
*/
#ifndef BUILD_PB
   #include <MacTypes.h>
   #include <MacErrors.h>
   #include <Files.h>
#endif
#define	__COMPILINGMOREFILES
#include "MoreFilesExtras.h"
#include "IterateDirectory.h"
/*
**	Type definitions
*/
/* The IterateGlobals structure is used to minimize the amount of
** stack space used when recursively calling IterateDirectoryLevel
** and to hold global information that might be needed at any time.
*/
#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#endif
struct IterateGlobals
{
	IterateFilterProcPtr	iterateFilter;	/* pointer to IterateFilterProc */
	CInfoPBRec				cPB;			/* the parameter block used for PBGetCatInfo calls */
	Str63					itemName;		/* the name of the current item */
	OSErr					result;			/* temporary holder of results - saves 2 bytes of stack each level */
	Boolean					quitFlag;		/* set to true if filter wants to kill interation */
	unsigned short			maxLevels;		/* Maximum levels to iterate through */
	unsigned short			currentLevel;	/* The current level IterateLevel is on */
	void					*yourDataPtr;	/* A pointer to caller data the filter may need to access */
};
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif
typedef struct IterateGlobals IterateGlobals;
typedef IterateGlobals *IterateGlobalsPtr;
/*****************************************************************************/
/*	Static Prototype */
static	void	IterateDirectoryLevel(long dirID,
									  IterateGlobals *theGlobals);
/*****************************************************************************/
/*
**	Functions
*/
static	void	IterateDirectoryLevel(long dirID,
									  IterateGlobals *theGlobals)
{
	if ( (theGlobals->maxLevels == 0) ||						/* if maxLevels is zero, we aren't checking levels */
		 (theGlobals->currentLevel < theGlobals->maxLevels) )	/* if currentLevel < maxLevels, look at this level */
	{
		short index = 1;
		
		++theGlobals->currentLevel;	/* go to next level */
		
		do
		{	/* Isn't C great... What I'd give for a "WITH theGlobals DO" about now... */
		
			/* Get next source item at the current directory level */
			
			theGlobals->cPB.dirInfo.ioFDirIndex = index;
			theGlobals->cPB.dirInfo.ioDrDirID = dirID;
			theGlobals->result = PBGetCatInfoSync((CInfoPBPtr)&theGlobals->cPB);		
	
			if ( theGlobals->result == noErr )
			{
				/* Call the IterateFilterProc */
				CallIterateFilterProc(theGlobals->iterateFilter, &theGlobals->cPB, &theGlobals->quitFlag, theGlobals->yourDataPtr);
				
				/* Is it a directory? */
				if ( (theGlobals->cPB.hFileInfo.ioFlAttrib & kioFlAttribDirMask) != 0 )
				{
					/* We have a directory */
					if ( !theGlobals->quitFlag )
					{
						/* Dive again if the IterateFilterProc didn't say "quit" */
						IterateDirectoryLevel(theGlobals->cPB.dirInfo.ioDrDirID, theGlobals);
					}
				}
			}
			
			++index; /* prepare to get next item */
		} while ( (theGlobals->result == noErr) && (!theGlobals->quitFlag) ); /* time to fall back a level? */
		
		if ( (theGlobals->result == fnfErr) ||	/* fnfErr is OK - it only means we hit the end of this level */
			 (theGlobals->result == afpAccessDenied) ) /* afpAccessDenied is OK, too - it only means we cannot see inside a directory */
		{
			theGlobals->result = noErr;
		}
			
		--theGlobals->currentLevel;	/* return to previous level as we leave */
	}
}
/*****************************************************************************/
pascal	OSErr	IterateDirectory(short vRefNum,
								 long dirID,
								 ConstStr255Param name,
								 unsigned short maxLevels,
								 IterateFilterProcPtr iterateFilter,
								 void *yourDataPtr)
{
	IterateGlobals	theGlobals;
	OSErr			result;
	long			theDirID;
	short			theVRefNum;
	Boolean			isDirectory;
	
	/* Make sure there is a IterateFilter */
	if ( iterateFilter != NULL )
	{
		/* Get the real directory ID and make sure it is a directory */
		result = GetDirectoryID(vRefNum, dirID, name, &theDirID, &isDirectory);
		if ( result == noErr )
		{
			if ( isDirectory == true )
			{
				/* Get the real vRefNum */
				result = DetermineVRefNum(name, vRefNum, &theVRefNum);
				if ( result == noErr )
				{
					/* Set up the globals we need to access from the recursive routine. */
					theGlobals.iterateFilter = iterateFilter;
					theGlobals.cPB.hFileInfo.ioNamePtr = (StringPtr)&theGlobals.itemName;
					theGlobals.cPB.hFileInfo.ioVRefNum = theVRefNum;
					theGlobals.itemName[0] = 0;
					theGlobals.result = noErr;
					theGlobals.quitFlag = false;
					theGlobals.maxLevels = maxLevels;
					theGlobals.currentLevel = 0;	/* start at level 0 */
					theGlobals.yourDataPtr = yourDataPtr;
				
					/* Here we go into recursion land... */
					IterateDirectoryLevel(theDirID, &theGlobals);
					
					result = theGlobals.result;	/* set the result */
				}
			}
			else
			{
				result = dirNFErr;	/* a file was passed instead of a directory */
			}
		}
	}
	else
	{
		result = paramErr;	/* iterateFilter was NULL */
	}
	
	return ( result );
}
/*****************************************************************************/
pascal	OSErr	FSpIterateDirectory(const FSSpec *spec,
									unsigned short maxLevels,
									IterateFilterProcPtr iterateFilter,
									void *yourDataPtr)
{
	return ( IterateDirectory(spec->vRefNum, spec->parID, spec->name,
						maxLevels, iterateFilter, yourDataPtr) );
}
/*****************************************************************************/
