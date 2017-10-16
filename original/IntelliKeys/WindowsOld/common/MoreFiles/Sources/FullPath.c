/*
	File:		FullPath.c
	Contains:	Routines for dealing with full pathnames... if you really must.
	Version:	MoreFiles
	Copyright:	© 1995-2002 by Apple Computer, Inc., all rights reserved.
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
		 <4>	 8/23/02	JL		[2853901]  Updated standard disclaimer.
		 <3>	  7/6/01	JL		[2661720]  Make comments ANSI C compliant.
		 <2>	  2/7/01	JL		Added standard header. Updated names of includes.
		<1>		12/06/99	JL		MoreFiles 1.5.
*/
#include <MacTypes.h>
#include <MacErrors.h>
#include <MacMemory.h>
#include <Files.h>
#include <TextUtils.h>
#include <Aliases.h>
#define	__COMPILINGMOREFILES
#include "FSpCompat.h"
#include "FullPath.h"
/*
	IMPORTANT NOTE:
	
	The use of full pathnames is strongly discouraged. Full pathnames are
	particularly unreliable as a means of identifying files, directories
	or volumes within your application, for two primary reasons:
	
	• 	The user can change the name of any element in the path at virtually
		any time.
	•	Volume names on the Macintosh are *not* unique. Multiple
		mounted volumes can have the same name. For this reason, the use of
		a full pathname to identify a specific volume may not produce the
		results you expect. If more than one volume has the same name and
		a full pathname is used, the File Manager currently uses the first
		mounted volume it finds with a matching name in the volume queue.
	
	In general, you should use a file’s name, parent directory ID, and
	volume reference number to identify a file you want to open, delete,
	or otherwise manipulate.
	
	If you need to remember the location of a particular file across
	subsequent system boots, use the Alias Manager to create an alias record
	describing the file. If the Alias Manager is not available, you can save
	the file’s name, its parent directory ID, and the name of the volume on
	which it’s located. Although none of these methods is foolproof, they are
	much more reliable than using full pathnames to identify files.
	
	Nonetheless, it is sometimes useful to display a file’s full pathname to
	the user. For example, a backup utility might display a list of full
	pathnames of files as it copies them onto the backup medium. Or, a
	utility might want to display a dialog box showing the full pathname of
	a file when it needs the user’s confirmation to delete the file. No
	matter how unreliable full pathnames may be from a file-specification
	viewpoint, users understand them more readily than volume reference
	numbers or directory IDs. (Hint: Use the TruncString function from
	TextUtils.h with truncMiddle as the truncWhere argument to shorten
	full pathnames to a displayable length.)
	
	The following technique for constructing the full pathname of a file is
	intended for display purposes only. Applications that depend on any
	particular structure of a full pathname are likely to fail on alternate
	foreign file systems or under future system software versions.
*/
/*****************************************************************************/
pascal	OSErr	GetFullPath(short vRefNum,
							long dirID,
							ConstStr255Param name,
							short *fullPathLength,
							Handle *fullPath)
{
	OSErr		result;
	FSSpec		spec;
	
	*fullPathLength = 0;
	*fullPath = NULL;
	
	result = FSMakeFSSpecCompat(vRefNum, dirID, name, &spec);
	if ( (result == noErr) || (result == fnfErr) )
	{
		result = FSpGetFullPath(&spec, fullPathLength, fullPath);
	}
	
	return ( result );
}
/*****************************************************************************/
pascal	OSErr	FSpGetFullPath(const FSSpec *spec,
							   short *fullPathLength,
							   Handle *fullPath)
{
	OSErr		result;
	OSErr		realResult;
	FSSpec		tempSpec;
	CInfoPBRec	pb;
	
	*fullPathLength = 0;
	*fullPath = NULL;
	
	
	/* Default to noErr */
	realResult = result = noErr;
	
	/* work around Nav Services "bug" (it returns invalid FSSpecs with empty names) */
	if ( spec->name[0] == 0 )
	{
		result = FSMakeFSSpecCompat(spec->vRefNum, spec->parID, spec->name, &tempSpec);
	}
	else
	{
		/* Make a copy of the input FSSpec that can be modified */
		BlockMoveData(spec, &tempSpec, sizeof(FSSpec));
	}
	
	if ( result == noErr )
	{
		if ( tempSpec.parID == fsRtParID )
		{
			/* The object is a volume */
			
			/* Add a colon to make it a full pathname */
			++tempSpec.name[0];
			tempSpec.name[tempSpec.name[0]] = ':';
			
			/* We're done */
			result = PtrToHand(&tempSpec.name[1], fullPath, tempSpec.name[0]);
		}
		else
		{
			/* The object isn't a volume */
			
			/* Is the object a file or a directory? */
			pb.dirInfo.ioNamePtr = tempSpec.name;
			pb.dirInfo.ioVRefNum = tempSpec.vRefNum;
			pb.dirInfo.ioDrDirID = tempSpec.parID;
			pb.dirInfo.ioFDirIndex = 0;
			result = PBGetCatInfoSync(&pb);
			/* Allow file/directory name at end of path to not exist. */
			realResult = result;
			if ( (result == noErr) || (result == fnfErr) )
			{
				/* if the object is a directory, append a colon so full pathname ends with colon */
				if ( (result == noErr) && (pb.hFileInfo.ioFlAttrib & kioFlAttribDirMask) != 0 )
				{
					++tempSpec.name[0];
					tempSpec.name[tempSpec.name[0]] = ':';
				}
				
				/* Put the object name in first */
				result = PtrToHand(&tempSpec.name[1], fullPath, tempSpec.name[0]);
				if ( result == noErr )
				{
					/* Get the ancestor directory names */
					pb.dirInfo.ioNamePtr = tempSpec.name;
					pb.dirInfo.ioVRefNum = tempSpec.vRefNum;
					pb.dirInfo.ioDrParID = tempSpec.parID;
					do	/* loop until we have an error or find the root directory */
					{
						pb.dirInfo.ioFDirIndex = -1;
						pb.dirInfo.ioDrDirID = pb.dirInfo.ioDrParID;
						result = PBGetCatInfoSync(&pb);
						if ( result == noErr )
						{
							/* Append colon to directory name */
							++tempSpec.name[0];
							tempSpec.name[tempSpec.name[0]] = ':';
							
							/* Add directory name to beginning of fullPath */
							(void) Munger(*fullPath, 0, NULL, 0, &tempSpec.name[1], tempSpec.name[0]);
							result = MemError();
						}
					} while ( (result == noErr) && (pb.dirInfo.ioDrDirID != fsRtDirID) );
				}
			}
		}
	}
	
	if ( result == noErr )
	{
		/* Return the length */
		*fullPathLength = GetHandleSize(*fullPath);
		result = realResult;	/* return realResult in case it was fnfErr */
	}
	else
	{
		/* Dispose of the handle and return NULL and zero length */
		if ( *fullPath != NULL )
		{
			DisposeHandle(*fullPath);
		}
		*fullPath = NULL;
		*fullPathLength = 0;
	}
	
	return ( result );
}
/*****************************************************************************/
pascal OSErr FSpLocationFromFullPath(short fullPathLength,
									 const void *fullPath,
									 FSSpec *spec)
{
	AliasHandle	alias;
	OSErr		result;
	Boolean		wasChanged;
	Str32		nullString;
	
	/* Create a minimal alias from the full pathname */
	nullString[0] = 0;	/* null string to indicate no zone or server name */
	result = NewAliasMinimalFromFullPath(fullPathLength, fullPath, nullString, nullString, &alias);
	if ( result == noErr )
	{
		/* Let the Alias Manager resolve the alias. */
		result = ResolveAlias(NULL, alias, spec, &wasChanged);
		
		/* work around Alias Mgr sloppy volume matching bug */
		if ( spec->vRefNum == 0 )
		{
			/* invalidate wrong FSSpec */
			spec->parID = 0;
			spec->name[0] =  0;
			result = nsvErr;
		}
		DisposeHandle((Handle)alias);	/* Free up memory used */
	}
	return ( result );
}
/*****************************************************************************/
pascal OSErr LocationFromFullPath(short fullPathLength,
								  const void *fullPath,
								  short *vRefNum,
								  long *parID,
								  Str31 name)
{
	OSErr	result;
	FSSpec	spec;
	
	result = FSpLocationFromFullPath(fullPathLength, fullPath, &spec);
	if ( result == noErr )
	{
		*vRefNum = spec.vRefNum;
		*parID = spec.parID;
		BlockMoveData(&spec.name[0], &name[0], spec.name[0] + 1);
	}
	return ( result );
}
/*****************************************************************************/
