// IKUtil.h: interface for the IKUtil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKUTIL_H__D5C23E77_A3BA_4B26_B508_BB7E25B7AAE9__INCLUDED_)
#define AFX_IKUTIL_H__D5C23E77_A3BA_4B26_B508_BB7E25B7AAE9__INCLUDED_

#include "IKString.h"
#include "IKStringArray.h"
#include "IKPrefs.h"

//  a way to get data items
#define DATAS(a,b) IKUtil::GetData()->GetValueString(a,b)
#define DATAI(a,b) IKUtil::GetData()->GetValueInt(a,b)

static const TCHAR * kFoldersType = TEXT('\0');
static const TCHAR * kAllFilesType = "*";

class IKUtil  
{
public:
	static void SetOneProcessor();
	static bool HasClickitAdaptation ( TCHAR * pAppPath );
	static IKString GetOMOverlaysFolder ();

	static IKString			GetOverlayMakerPath ( void );
	static bool             IsOverlayMakerInstalled ();

	static void BuildFileArray ( IKStringArray &array,
		const TCHAR * pInitialPath,
		const TCHAR * pType = kAllFilesType,
		int fileTypeOptions = -1 );
	static void BuildFolderArray ( IKStringArray &array, const TCHAR * pInitialPath );

	static bool IsPrimaryMouseDown(void);
	static void GetTrueFileName ( IKString & filename );
	static IKString GetParentApp ();
	static void DisplayAlert ( IKString title, IKString message );
	static unsigned int GetSysTimeSecs1970();
	static bool IsParentClickit ();
	static IKString GetAppFriendlyName ( IKString &path );
	static bool IsAppRunning ( char *appname );
#ifdef PLAT_WINDOWS
	static int GetCodePage ();
#endif
	static bool IsWin2KOrGreater();
	static bool IsWinVistaOrGreater();
	static void MemoryCopy    ( BYTE *dst, BYTE *src, int nbytes);
	static bool MemoryCompare ( BYTE *dst, BYTE *src, int nbytes);
	static void LaunchFile ( IKString filename );
#ifdef PLAT_MACINTOSH
	static void ApplyFinderTypeAndCreator ( IKString strFile,TCHAR *type, TCHAR *creator);
	static SInt32  GetSystemVersion();
#endif
	static IKString GetThisAppPath ();
	static void FlushLog();
	static void Periodic();
	static IKString IntToString ( int num );
	static IKString IntToHexString ( int num );
	static IKString IntToString ( unsigned int num );
	static int StringToInt (IKString string);
	static int StringToInt (TCHAR * string);
	static float StringToFloat (IKString string);

	static IKString GetDateAndTime();
	static IKString GetDate();
	static IKString GetTime();
	static IKString MakeStudentPath ( IKString group, IKString student );
	static void MakeAllFolders ( IKString path );

	static TCHAR GetPathDelimiter();
	static TCHAR GetMacPathDelimiter();
	static TCHAR GetWinPathDelimiter();

	static void Sleep ( unsigned int waitMS );
	static unsigned int GetCurrentTimeMS ();

	static void LogString ( TCHAR *s );

	static IKString GetRootFolder();
	static IKString GetChannelsFolder();
	static IKString GetUsersFolder();
	static IKString GetPrivateFolder();
	static IKString GetAppOverlaysFolder();
	static IKString GetOverlaysFolder();
	static IKString GetSwitchSettingsFolder();
	static IKString GetEngineFolder();
	static IKString GetGuestFolder();

	static IKString GetHelpFile();

	static void Initialize();

	static IKString GetCurrentApplicationPath();

	static IKPrefs * GetData();
	static void StripFileName ( IKString &str,
							bool bStripPath = true ,
							bool bStripExtension = true  );
							
	static bool RunningUnderOSX ();
	static bool RunningUnderClassic ();

};


#endif // !defined(AFX_IKUTIL_H__D5C23E77_A3BA_4B26_B508_BB7E25B7AAE9__INCLUDED_)
