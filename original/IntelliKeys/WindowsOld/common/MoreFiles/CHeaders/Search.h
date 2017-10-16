/*
     File:       Search.h
 
     Contains:   IndexedSearch and the PBCatSearch compatibility function.
 
     Version:    Technology: MoreFiles
                 Release:    1.5.4
 
     Copyright:  © 1992-2002 by Apple Computer, Inc., all rights reserved.
 
     Bugs?:      For bug reports, consult the following page on
                 the World Wide Web:
 
                     http://developer.apple.com/bugreporter/
 
*/
/*
    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
*/
#ifndef __SEARCH__
#define __SEARCH__
#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __FILES__
#include <Files.h>
#endif
#include "Optimization.h"
#if PRAGMA_ONCE
#pragma once
#endif
#ifdef __cplusplus
extern "C" {
#endif
#if PRAGMA_IMPORT
#pragma import on
#endif
#if PRAGMA_STRUCT_ALIGN
    #pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
    #pragma pack(2)
#endif
/*****************************************************************************/
EXTERN_API( OSErr )
IndexedSearch(
  CSParamPtr   pb,
  long         dirID);
/*
    The IndexedSearch function performs an indexed search in and below the
    specified directory using the same parameters (in pb) as is passed to
    PBCatSearch. See Inside Macintosh: Files for a description of the
    parameter block.
    
    pb          input:  A CSParamPtr record specifying the volume to search
                        and the search criteria.
                output: Fields in the parameter block are returned indicating
                        the number of matches found, the matches, and if the
                        search ended with noErr, the CatPosition record that
                        lets you resume a search where the last search left
                        off.
    dirID       input:  The directory to search. If fsRtDirID is passed,
                        the entire volume is searched.
    
    Note:   If you use a high-level debugger and use ioSearchTime to limit
            the length of time to run the search, you'll want to step over
            calls to IndexedSearch because it installs a Time Manager task.
            Most high-level debuggers don't deal gracefully with interrupt
            driven code.
    
    Result Codes
        noErr               0       No error
        nsvErr              -35     Volume not found
        ioErr               -36     I/O error
        eofErr              -39     End of catalog found (this is normal!)
        paramErr            -50     Parameter block has invalid parameters
                                    (see source for VerifyUserPB) or
                                    invalid catPosition record was passed
        extFSErr            -58     External file system error - no file
                                    system claimed this call.
        memFullErr          -108    Memory could not be allocated in heap
        catChangedErr       -1304   Catalog has changed and catalog position
                                    record may be invalid
    
    __________
    
    See also:   PBCatSearch, PBCatSearchSyncCompat
*/
/*****************************************************************************/
EXTERN_API( OSErr )
PBCatSearchSyncCompat(CSParamPtr paramBlock);
/*
    The PBCatSearchSyncCompat function uses PBCatSearch (if available) or
    IndexedSearch (if PBCatSearch is not available) to search a volume
    using a set of search criteria that you specify. It builds a list of all
    files or directories that meet your specifications.
    
    pb          input:  A CSParamPtr record specifying the volume to search
                        and the search criteria.
                output: Fields in the parameter block are returned indicating
                        the number of matches found, the matches, and if the
                        search ended with noErr, the CatPosition record that
                        lets you resume a search where the last search left
                        off.
    
    Note:   If you use a high-level debugger and use ioSearchTime to limit
            the length of time to run the search, you'll want to step over
            calls to PBCatSearchSyncCompat because it calls IndexedSearch
            which installs a Time Manager task. Most high-level debuggers
            don't deal gracefully with interrupt driven code.
    
    Result Codes
        noErr               0       No error
        nsvErr              -35     Volume not found
        ioErr               -36     I/O error
        eofErr              -39     End of catalog found (this is normal!)
        paramErr            -50     Parameter block has invalid parameters
                                    (see source for VerifyUserPB) or
                                    invalid catPosition record was passed
        extFSErr            -58     External file system error - no file
                                    system claimed this call.
        memFullErr          -108    Memory could not be allocated in heap
        catChangedErr       -1304   Catalog has changed and catalog position
                                    record may be invalid
        afpCatalogChanged   -5037   Catalog has changed and search cannot
                                    be resumed
    
    __________
    
    See also:   PBCatSearch, IndexedSearch
*/
/*****************************************************************************/
EXTERN_API( OSErr )
NameFileSearch(
  ConstStr255Param   volName,
  short              vRefNum,
  ConstStr255Param   fileName,
  FSSpecPtr          matches,
  long               reqMatchCount,
  long *             actMatchCount,
  Boolean            newSearch,
  Boolean            partial);
/*
    The NameFileSearch function searches for files with a specific file
    name on a volume that supports PBCatSearch.
    Note: A result of catChangedErr means the catalog has changed between
    searches, but the search can be continued with the possiblity that you
    may miss some matches or get duplicate matches.  For all other results
    (except for noErr), the search cannot be continued.
    volName         input:  A pointer to the name of a mounted volume
                            or nil.
    vRefNum         input:  Volume specification.
    fileName        input:  The name of the file to search for.
    matches         input:  Pointer to array of FSSpec where the match list is
                            returned.
    reqMatchCount   input:  Maximum number of matches to return (the number of
                            elements in the matches array).
    actMatchCount   output: The number of matches actually returned.
    newSearch       input:  If true, start a new search. If false and if
                            vRefNum is the same as the last call to
                            NameFileSearch, then start searching at the
                            position where the last search left off.
    partial         input:  If the partial parameter is false, then only files
                            that exactly match fileName will be found.  If the
                            partial parameter is true, then all file names that
                            contain fileName will be found.
    
    Result Codes
        noErr               0       No error
        nsvErr              -35     Volume not found
        ioErr               -36     I/O error
        eofErr              -39     End of catalog found (this is normal!)
        paramErr            -50     Parameter block has invalid parameters
                                    (see source for VerifyUserPB) or
                                    invalid catPosition record was passed
        extFSErr            -58     External file system error - no file
                                    system claimed this call.
        memFullErr          -108    Memory could not be allocated in heap
        catChangedErr       -1304   Catalog has changed and catalog position
                                    record may be invalid
        afpCatalogChanged   -5037   Catalog has changed and search cannot
                                    be resumed
    
    __________
    
    Also see:   CreatorTypeFileSearch
*/
/*****************************************************************************/
EXTERN_API( OSErr )
CreatorTypeFileSearch(
  ConstStr255Param   volName,
  short              vRefNum,
  OSType             creator,
  OSType             fileType,
  FSSpecPtr          matches,
  long               reqMatchCount,
  long *             actMatchCount,
  Boolean            newSearch);
/*
    The CreatorTypeFileSearch function searches for files with a specific
    creator or fileType on a volume that supports PBCatSearch.
    Note: A result of catChangedErr means the catalog has changed between
    searches, but the search can be continued with the possiblity that you
    may miss some matches or get duplicate matches.  For all other results
    (except for noErr), the search cannot be continued.
    volName         input:  A pointer to the name of a mounted volume
                            or nil.
    vRefNum         input:  Volume specification.
    creator         input:  The creator type of the file to search for.
                            To ignore the creator type, pass 0x00000000 in
                            this field.
    fileType        input:  The file type of the file to search for.
                            To ignore the file type, pass 0x00000000 in
                            this field.
    matches         input:  Pointer to array of FSSpec where the match list is
                            returned.
    reqMatchCount   input:  Maximum number of matches to return (the number of
                            elements in the matches array).
    actMatchCount   output: The number of matches actually returned.
    newSearch       input:  If true, start a new search. If false and if
                            vRefNum is the same as the last call to
                            CreatorTypeFileSearch, then start searching at the
                            position where the last search left off.
    
    Result Codes
        noErr               0       No error
        nsvErr              -35     Volume not found
        ioErr               -36     I/O error
        eofErr              -39     End of catalog found (this is normal!)
        paramErr            -50     Parameter block has invalid parameters
                                    (see source for VerifyUserPB) or
                                    invalid catPosition record was passed
        extFSErr            -58     External file system error - no file
                                    system claimed this call.
        memFullErr          -108    Memory could not be allocated in heap
        catChangedErr       -1304   Catalog has changed and catalog position
                                    record may be invalid
        afpCatalogChanged   -5037   Catalog has changed and search cannot
                                    be resumed
    
    __________
    
    Also see:   NameFileSearch
*/
/*****************************************************************************/
#include "OptimizationEnd.h"
#if PRAGMA_STRUCT_ALIGN
    #pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
    #pragma pack()
#endif
#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif
#ifdef __cplusplus
}
#endif
#endif /* __SEARCH__ */
