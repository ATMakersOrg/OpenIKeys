#include "IKCommon.h"
#include "IKFileDialog.h"

#if 0

#ifdef PLAT_MACINTOSH
	#include "MacUtilities.h"
	#include "XIntelliKeys_Mac.h"
	#include <UStandardDialogs.h>
	#include <UNavServicesDialogs.h>
	#define kMaxFileTypes 16

	// If on, a block of memory is allocated and released before invoking dialogs
	#define PREFLIGHT_DIALOGS 0
	
#if TARGET_API_MAC_CARBON
	#define USE_NAV_DIALOGS 1
	//#define USE_CLASSIC_DIALOGS 1
#else
	#define USE_CLASSIC_DIALOGS 1
#endif

	#if USE_CLASSIC_DIALOGS
		#include "UClassicDialogs.h"
	#endif
	
	#if USE_NAV_DIALOGS
		#include <UNavServicesDialogs.h>
	#endif
	
	static pascal Boolean MyNavObjectFilter(
			AEDesc*			inItem,
			void*			inInfo,
			void*			inUserData,
			NavFilterModes	inFilterMode);
	static IKFileDialog	*	g_pFileDialog = NULL;
#endif

#endif


//----------------------------------------------------------------
//----------------------------------------------------------------
IKFileDialog :: IKFileDialog( OpenSave os, IKStringArray &typeArray, bool bAllowMultiOpen /* = FALSE */ ) 
	:	m_openOrSave ( os ),
		m_typeArray ( typeArray ),
		m_bAllowMultiOpen ( bAllowMultiOpen )
#ifdef PLAT_WINDOWS
	//,	m_fileDlg(os == Open)
#endif		
{
	Initialize();
}
	
IKFileDialog :: IKFileDialog( OpenSave os ) 
	:	m_openOrSave ( os ),
		//m_typeArray ( XApplication::GetApplication()->GetFileTypeArray() ),
		m_bAllowMultiOpen ( FALSE )
#ifdef PLAT_WINDOWS
	,	m_fileDlg(os == Open)

#endif		
{
	Initialize();
}

void IKFileDialog::SetName ( CString name )
{
	m_title = name;
}



void IKFileDialog :: Initialize ( void )
{
#ifdef PLAT_WINDOWS
	bool bInitedDefault = false;
	
	for ( int i = 0; i < m_typeArray.GetSize(); i++ )
	{
		const XFileTypeDescription & type = m_typeArray[i];
		if ( type.GetMacUseFileTypeAsFilter() == false )
		{
			CString strExtension = /* "*." + */ type.GetFileExtension();
			if ( ! bInitedDefault )
			{
				bInitedDefault = true;
				m_strDefaultExtension = strExtension;
			}
			CString strDescription ( type.GetTypeDescription() );
			if ( strExtension.CompareNoCase ( "txt" ) == 0 )
				strDescription = _M ( S_TEXT_TYPE );

			if ( strDescription.GetLength() == 0 )
			{
				// What to do if no description?
			}

			m_strFilter += strDescription + "|";
			int semiPos = strExtension.Find ( ';' );
			if ( semiPos != -1 )
			{
				m_strDefaultExtension = m_strDefaultExtension.Left ( semiPos );
				
				while ( true )
				{
					int semiPos = strExtension.Find ( ';' );
					if ( semiPos == -1 )
						break;

					m_strFilter += "*." + strExtension.Left ( semiPos );
					strExtension = strExtension.Right ( strExtension.GetLength() - semiPos - 1 );
					if ( strExtension.Find ( ';' ) != -1 )
						m_strFilter += "; ";
					else
					{
						if ( strExtension.GetLength() )
							m_strFilter += "; *." + strExtension;
						break;
					}
				}
			}
			else
				m_strFilter += "*." + strExtension;

			m_strFilter += '|';
		}
	}
	m_strFilter += '|';

	m_fileDlg.m_ofn.lpstrDefExt = m_strDefaultExtension;
#endif
}


//----------------------------------------------------------------
//----------------------------------------------------------------
void IKFileDialog :: SetDialogTitle ( LPCSTR lpTitle )
{
#ifdef PLAT_WINDOWS
		m_fileDlg.m_ofn.lpstrTitle = lpTitle;
#endif
}


//----------------------------------------------------------------
//----------------------------------------------------------------
void IKFileDialog :: SetInitialDirectory ( CString strDir )
{
	m_strInitialDir = strDir;
}

//----------------------------------------------------------------
//----------------------------------------------------------------
void IKFileDialog :: SetInitialFile ( CString strFile )
{
	m_strInitialFile = strFile;
}

#ifdef PLAT_MACINTOSH

class MyFileChooser : public PP_StandardDialogs::LFileChooser
{
public:
	void SetName ( Str255 name )
	{
		int n = name[0];
		for (int i=0;i<n+1;i++)
			mNavOptions.windowTitle[i] = name[i];
	}
};

#endif

//----------------------------------------------------------------
//----------------------------------------------------------------
bool IKFileDialog :: DoModal ( CString &f )
{
#ifdef PLAT_WINDOWS

	//  apply filter
	if (!m_strFilter.IsEmpty())
	{
		LPTSTR lpBuffer = m_strFilter.GetBuffer(1000);
		while ( *lpBuffer )
		{
			if ( *lpBuffer == '|' )
				*lpBuffer = 0;
			++lpBuffer;
		}
		m_fileDlg.m_ofn.lpstrFilter = m_strFilter;
	}

	//  apply initial directory
	if (!m_strInitialDir.IsEmpty())
	{
		LPCSTR lpDir = m_strInitialDir.GetBuffer(1000);
		m_fileDlg.m_ofn.lpstrInitialDir = lpDir;
	}

	//  apply initial file
	if (!m_strInitialFile.IsEmpty())
	{
		m_fileDlg.m_ofn.lpstrFile = m_strInitialFile.GetBuffer(1000);
	}

	//  apply title
	char title[_MAX_PATH];
	title[0] = 0;
	if (!m_title.IsEmpty())
	{
		strcpy(title,m_title);
		m_fileDlg.m_ofn.lpstrTitle = title;
	}

	bool bResult = m_fileDlg.DoModal() == IDOK;

	m_strInitialFile.ReleaseBuffer();
	m_strInitialDir.ReleaseBuffer();
	m_strFilter.ReleaseBuffer();


	if ( bResult )
		f = m_fileDlg.GetPathName();
 
	return bResult;
	
#else	// mac
	XVariableSetterBlock<IKFileDialog*> vsb ( g_pFileDialog, this );

#if PREFLIGHT_DIALOGS
	const int kPreflightSize = 100 * 1024;
	Handle h = NewHandle ( kPreflightSize );
	ASSERT ( h );
	if ( h )
		::DisposeHandle ( h );
#endif
		
	bool bResult = FALSE;
	if ( m_openOrSave == Open )
	{
		FSSpec fSpec;
		unsigned char		path[256];


#if USE_CLASSIC_DIALOGS
		UClassicDialogs::LFileChooser	chooser;
#else
 		// PP_StandardDialogs::LFileChooser	chooser;
 		MyFileChooser chooser;
#endif		
		chooser.SetObjectFilterProc ( MyNavObjectFilter );
				
		// if we got an initial directory, honor that here
		if( m_strInitialDir.GetLength() )
		{
			strcpy ( (char*) path, (const char*) m_strInitialDir );
			LString::CToPStr( (char*) path);  //  XCode port
			if( ::FSMakeFSSpec( 0, 0L, path, &fSpec ) == noErr )
				chooser.SetDefaultLocation( fSpec, false );
				
			m_strInitialDir.Empty();
		}
				

		// if we got an initial file, honor that here
		if( m_strInitialFile.GetLength() )
		{
			strcpy ( (char*) path, (const char*) m_strInitialFile );
			LString::CToPStr( (char*) path);  //  XCode port
			if( ::FSMakeFSSpec( 0, 0L, path, &fSpec ) == noErr )
				chooser.SetDefaultLocation( fSpec, true );
				
			m_strInitialFile.Empty();
		}
		
		//  set up the title
		//NavDialogOptions options;
		//OSErr err = NavGetDefaultDialogOptions(&options);
		//strcpy ( (char *)&(options.windowTitle[1]), (char *)"aaa");
		//chooser.GetDialogOptions(&options);
		Str255 title;
		if (!m_title.IsEmpty())
		{
			strcpy((char *)&(title[1]),m_title);
			title[0] = m_title.GetLength();
			chooser.SetName(title);
		}

		OSType fileTypes[1] = { 'TEXT' };
		PP_PowerPlant::LFileTypeList macTypeList( fileTypes_All );//0, NULL );

/*		
		// sss: a hack for now
		bool bOverlayFileType = ( ( fileTypes[0] == 'Covl' ) || ( fileTypes[0] == 'Cpvl' ) || ( fileTypes[0] == 'Cgrp' ) );
		if ( ( numTypes ) && ( bOverlayFileType ) )
			macTypeList.SetSignature ( 'Cmzr' );
*/			
		if (chooser.AskOpenFile( macTypeList ))
		{
			AEDescList		docList;
			chooser.GetFileDescList(docList);
	//		OpenOrPrintDocList(docList, PP_PowerPlant::ae_OpenDoc);

			SInt32		numDocs;
			OSErr 		err = ::AECountItems(&docList, &numDocs);
			ThrowIfOSErr_(err);
			
				// Loop through all items in the list
					// Extract descriptor for the document
					// Coerce descriptor data into a FSSpec
					// Tell Program object to open document
				
			for (SInt32 i = 1; i <= numDocs; i++)
			{
				AEKeyword	theKey;
				DescType	theType;
				FSSpec		theFileSpec;
				Size		theSize;
				err = ::AEGetNthPtr(&docList, i, typeFSS, &theKey, &theType,
									(Ptr) &theFileSpec, sizeof(FSSpec), &theSize);
				ThrowIfOSErr_(err);

	//			
	// TODO: Allow multiple opens, return a list...
				f = MacUtilities::GetFullPath( theFileSpec );
				bResult = TRUE;
			}
		}
	}
	else
	{
		bool		bReplacing;
		
#if USE_CLASSIC_DIALOGS
		UClassicDialogs::LFileDesignator	chooser;
#elif USE_NAV_DIALOGS
		UNavServicesDialogs::LFileDesignator	chooser;
		chooser.GetDialogOptions()->dialogOptionFlags &= ~kNavAllowStationery;
		chooser.GetDialogOptions()->dialogOptionFlags |= kNavNoTypePopup;
		chooser.GetDialogOptions()->dialogOptionFlags |= kNavDontAddTranslateItems;

#else
		PP_PowerPlant::PP_StandardDialogs::UConditionalDialogs::LFileDesignator	chooser;
		chooser.GetDialogOptions()->dialogOptionFlags &= ~kNavAllowStationery;
#endif
		FSSpec fSpec;
		unsigned char		path[256];

		// if we got an initial directory, honor that here
		if( m_strInitialDir.GetLength() )
		{
			strcpy ( (char*) path, (const char*) m_strInitialDir );
			LString::CToPStr( (char*) path);  //  XCode port
			if( ::FSMakeFSSpec( 0, 0L, path, &fSpec ) == noErr )
				chooser.SetDefaultLocation( fSpec, false );
				
			m_strInitialDir.Empty();
		}
		
		CString strFileName ( f );
		CString sDirPath;
		
		sDirPath = XUtilities::DirFromFileName(strFileName);
		
		if (!sDirPath.IsEmpty())
		{
			FSSpec dirFS;
			OSErr makeDirSpecResult;
			
			makeDirSpecResult = MacUtilities::GetFSSpec ( (LPCSTR) sDirPath, &dirFS );
			if (makeDirSpecResult == noErr)
			{
				chooser.SetDefaultLocation(dirFS, FALSE);
			}
		}
		
		//  title
		NavDialogOptions *pOptions = chooser.GetDialogOptions();
		if (pOptions && !m_title.IsEmpty())
		{
			char *title = (char *)pOptions->windowTitle;
			strcpy((char *)&(title[1]),m_title);
			title[0] = m_title.GetLength();
		}
		
		XUtilities::StripFileName ( strFileName, true, false );
//		LStr255 name ( strFileName );
		bool bDoAsk = TRUE;
		while (bDoAsk)
		{
			bDoAsk = FALSE;
			bResult = chooser.AskDesignateFile( strFileName );
				
			if ( bResult )
			{
				FSSpec		theFileSpec;
				FInfo fndrInfo;
				chooser.GetFileSpec ( theFileSpec );
				f = CString( MacUtilities::GetFullPath ( &theFileSpec ) );
				// check that it does not exit or may be written if it does
				bool bExists = ( ::FSpGetFInfo (&theFileSpec, &fndrInfo) == noErr );
				if ( bExists )
				{
				//2005-1191
					short refNum;
					OSErr err = ::FSpOpenDF(&theFileSpec, (SInt8) fsWrPerm, &refNum);
					if (err == noErr)
					{	
						::FSClose ( refNum );
					}
					else
					{
						CString sMsg;
						CString sFormat = _M(S_UNABLETOREPLACEFILE_TEXT);
						CString strNewFileName ( f );
						XUtilities :: StripFileName ( strNewFileName, true, false);
						sMsg.Format((LPCSTR)sFormat, (LPCSTR)strNewFileName);
						XMessageBox dlg((LPCSTR)sMsg,_M(S_UNABLETOREPLACEFILE_TITLE),MB_YESNO | MB_DEFBUTTON2);
						bDoAsk = (dlg.DoModal() == IDYES);
						if (!bDoAsk)
						{
							bResult = FALSE;
						}
					}
				}
			}
		}
	}

	return bResult;	
#endif
}

#ifdef PLAT_MACINTOSH
static pascal Boolean MyNavObjectFilter(
		AEDesc*			inItem,
		void*			inInfo,
		void*			inUserData,
		NavFilterModes	inFilterMode)
{
	ASSERT ( g_pFileDialog );
	if ( g_pFileDialog == NULL )
		return false;

	IKFileDialog::OpenSave openOrSave = g_pFileDialog->GetOpenSaveFlag();
	const XFileTypeArray & typeArray = g_pFileDialog->GetTypeArray();
		
	ResType fileType = 0;
	CString strPath;
	
	if (inItem != nil)
	{
	// Navigation Services Callback
		if ( inItem->descriptorType == typeFSS )
		{
			Size dataSize = AEGetDescDataSize( inItem );
			FSSpec fs;
			ASSERT ( dataSize == sizeof(fs) );

			NavFileOrFolderInfo* theInfo;
			theInfo = (NavFileOrFolderInfo*) inInfo;
			if ( theInfo->isFolder )
				return true;
			  
			if ( AEGetDescData ( inItem, &fs, sizeof ( fs ) ) != noErr )
				return false;
				
			strPath = MacUtilities::GetFullPath ( &fs );

			FInfo fInfo;
			if ( FSpGetFInfo (&fs, &fInfo) == noErr )
				fileType = fInfo.fdType;
		}
		else
			if ( inItem->descriptorType == typeFSRef )
			{
				FSRef r;
				AEGetDescData ( inItem, &r, sizeof ( r ) );
	
				char path [ _MAX_PATH ];
				FSRefMakePath ( &r, (StringPtr) path, sizeof (path) );
				LString::PToCStr ( (StringPtr) path );  //  XCode port
				strPath = path;
				
				FSSpec fs = MacUtilities :: GetFSSpec ( strPath );
				
				FInfo fInfo;
				if ( FSpGetFInfo (&fs, &fInfo) == noErr )
					fileType = fInfo.fdType;
			}
			else
				return false;
	}
	else
	{
		if (inUserData == nil)
			return false;
			
	// StandardFile Callback
		CInfoPBPtr thePB = (CInfoPBPtr) inUserData;
		HFileInfo & hFileInfo = thePB->hFileInfo;
		fileType = hFileInfo.ioFlFndrInfo.fdType;
		LStr255 macNameString ( hFileInfo.ioNamePtr );
		LString::PToCStr ( &macNameString[0] );  //  XCode port
		strPath = LPCSTR ( &macNameString[0] );
	}
	
	int lastPeriod = strPath.ReverseFind ( '.' );
	if ( lastPeriod != -1 )
	{
		strPath = strPath.Right( strPath.GetLength() - lastPeriod - 1 );
		strPath.MakeUpper();
	}
		
	for ( int i = 0; i < typeArray.GetSize(); i++ )
	{
	// Screen out files that are open or save only and don't agree with our current mode
		if ( ( openOrSave == IKFileDialog::Open ) &&
			 ( typeArray[i].GetFileTypeOptions() == kCanSaveSpecial ) )
			continue;
			
		if ( ( openOrSave == IKFileDialog::Save ) &&
			 ( typeArray[i].GetFileTypeOptions() == kCanOpenSpecial ) )
			continue;
			
	// If the type specifies that we should pay attention to the Mac file type, then do that.
		if ( typeArray[i].GetMacUseFileTypeAsFilter() )
		{
			if ( fileType == typeArray[i].GetMacOSType() )
				return true;
		}
		else
		{
			CString strExtension (".");
			strExtension += typeArray[i].GetFileExtension();

			if ( strExtension == ".*" )
				return true;
				
			strExtension.MakeUpper();
			
			if ( strExtension.Find ( strPath ) != -1 )
				return true;
		}
	}
		
	return false;
}
#endif