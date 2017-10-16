{
     File:       IterateDirectory.p
 
     Contains:   File Manager directory iterator routines.
 
     Version:    Technology: MoreFiles
                 Release:    1.5.4
 
     Copyright:  © 1995-2002 by Jim Luther and Apple Computer, Inc., all rights reserved.
 
     Bugs?:      For bug reports, consult the following page on
                 the World Wide Web:
 
                     http://developer.apple.com/bugreporter/
 
}
{
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
}
{$IFC UNDEFINED UsingIncludes}
{$SETC UsingIncludes := 0}
{$ENDC}
{$IFC NOT UsingIncludes}
 UNIT IterateDirectory;
 INTERFACE
{$ENDC}
{$IFC UNDEFINED __ITERATEDIRECTORY__}
{$SETC __ITERATEDIRECTORY__ := 1}
{$I+}
{$SETC IterateDirectoryIncludes := UsingIncludes}
{$SETC UsingIncludes := 1}
{$IFC UNDEFINED __MACTYPES__}
{$I MacTypes.p}
{$ENDC}
{$IFC UNDEFINED __FILES__}
{$I Files.p}
{$ENDC}
{$PUSH}
{$ALIGN MAC68K}
{$LibExport+}
{***************************************************************************}
TYPE
{$IFC TYPED_FUNCTION_POINTERS}
	IterateFilterProcPtr = PROCEDURE({CONST}VAR cpbPtr: CInfoPBRec; VAR quitFlag: BOOLEAN; yourDataPtr: UNIV Ptr);
{$ELSEC}
	IterateFilterProcPtr = ProcPtr;
{$ENDC}
	{	
	    This is the prototype for the IterateFilterProc function which is
	    called once for each file and directory found by IterateDirectory. The
	    IterateFilterProc gets a pointer to the CInfoPBRec that IterateDirectory
	    used to call PBGetCatInfo. The IterateFilterProc can use the read-only
	    data in the CInfoPBRec for whatever it wants.
	    
	    If the IterateFilterProc wants to stop IterateDirectory, it can set
	    quitFlag to true (quitFlag will be passed to the IterateFilterProc
	    false).
	    
	    The yourDataPtr parameter can point to whatever data structure you might
	    want to access from within the IterateFilterProc.
	
	    cpbPtr      input:  A pointer to the CInfoPBRec that IterateDirectory
	                        used to call PBGetCatInfo. The CInfoPBRec and the
	                        data it points to must not be changed by your
	                        IterateFilterProc.
	    quitFlag    output: Your IterateFilterProc can set quitFlag to true
	                        if it wants to stop IterateDirectory.
	    yourDataPtr input:  A pointer to whatever data structure you might
	                        want to access from within the IterateFilterProc.
	    
	    __________
	    
	    Also see:   IterateDirectory, FSpIterateDirectory
		}
	{	***************************************************************************	}
FUNCTION IterateDirectory(vRefNum: INTEGER; dirID: LONGINT; name: Str255; maxLevels: UInt16; iterateFilter: IterateFilterProcPtr; yourDataPtr: UNIV Ptr): OSErr;
{
    The IterateDirectory function performs a recursive iteration (scan) of
    the specified directory and calls your IterateFilterProc function once
    for each file and directory found.
    
    The maxLevels parameter lets you control how deep the recursion goes.
    If maxLevels is 1, IterateDirectory only scans the specified directory;
    if maxLevels is 2, IterateDirectory scans the specified directory and
    one subdirectory below the specified directory; etc. Set maxLevels to
    zero to scan all levels.
    
    The yourDataPtr parameter can point to whatever data structure you might
    want to access from within the IterateFilterProc.
    vRefNum         input:  Volume specification.
    dirID           input:  Directory ID.
    name            input:  Pointer to object name, or nil when dirID
                            specifies a directory that's the object.
    maxLevels       input:  Maximum number of directory levels to scan or
                            zero to scan all directory levels.
    iterateFilter   input:  A pointer to the routine you want called once
                            for each file and directory found by
                            IterateDirectory.
    yourDataPtr     input:  A pointer to whatever data structure you might
                            want to access from within the IterateFilterProc.
    
    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        paramErr            -50     No default volume or iterateFilter was NULL
        dirNFErr            -120    Directory not found or incomplete pathname
                                    or a file was passed instead of a directory
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname
        
    __________
    
    See also:   IterateFilterProcPtr, FSpIterateDirectory
}
{***************************************************************************}
FUNCTION FSpIterateDirectory({CONST}VAR spec: FSSpec; maxLevels: UInt16; iterateFilter: IterateFilterProcPtr; yourDataPtr: UNIV Ptr): OSErr;
{
    The FSpIterateDirectory function performs a recursive iteration (scan)
    of the specified directory and calls your IterateFilterProc function once
    for each file and directory found.
    
    The maxLevels parameter lets you control how deep the recursion goes.
    If maxLevels is 1, FSpIterateDirectory only scans the specified directory;
    if maxLevels is 2, FSpIterateDirectory scans the specified directory and
    one subdirectory below the specified directory; etc. Set maxLevels to
    zero to scan all levels.
    
    The yourDataPtr parameter can point to whatever data structure you might
    want to access from within the IterateFilterProc.
    spec            input:  An FSSpec record specifying the directory to scan.
    maxLevels       input:  Maximum number of directory levels to scan or
                            zero to scan all directory levels.
    iterateFilter   input:  A pointer to the routine you want called once
                            for each file and directory found by
                            FSpIterateDirectory.
    yourDataPtr     input:  A pointer to whatever data structure you might
                            want to access from within the IterateFilterProc.
    
    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        paramErr            -50     No default volume or iterateFilter was NULL
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname
        
    __________
    
    See also:   IterateFilterProcPtr, IterateDirectory
}
{***************************************************************************}
{$ALIGN RESET}
{$POP}
{$SETC UsingIncludes := IterateDirectoryIncludes}
{$ENDC} {__ITERATEDIRECTORY__}
{$IFC NOT UsingIncludes}
 END.
{$ENDC}
