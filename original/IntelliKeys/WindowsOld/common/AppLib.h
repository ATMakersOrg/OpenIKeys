
#ifndef _AppLib_H_
#define _AppLib_H_

#ifdef __cplusplus
extern "C" {
#endif

//  according to the draft 2 spec:

#define APPLIB_ERROR					0
#define APPLIB_SYSTEM_ALREADY_ADDED		2
#define	APPLIB_SYSTEM_ADDED				1 
#define	APPLIB_SYSTEM_REMOVED			1
#define	APPLIB_SYSTEM_ALREADY_REMOVED	2
#define APPLIB_APP_ALREADY_ADDED		2
#define APPLIB_APP_ADDED				1
#define	APPLIB_APP_REMOVED				1
#define APPLIB_APP_NOT_OWNED			2
#define APPLIB_APP_OWNED				1
#define APPLIB_TOOK_OWNERSHIP			1
#define APPLIB_ALREADY_OWNED			2
#define APPLIB_RELEASE_NOT_OWNER		2
#define APPLIB_RELEASED_OWNERSHIP		1
#define APPLIB_PATH_UPDATED				1
#define APPLIB_SYSTEM_ACTIVE			1
#define APPLIB_SYSTEM_INACTIVE			2

int		AppLibAddSystem				( TCHAR * SystemName, TCHAR * IdentifyingFileName );
int		AppLibRemoveSystem			( TCHAR * SystemName );
int		AppLibCountSystems			();
TCHAR *	AppLibGetNthSystem			( int n );
int		AppLibAddApplication		( TCHAR * AppPath );
int		AppLibRemoveApplication		( TCHAR * AppPath, TCHAR * SystemName );
int		AppLibTakeOwnership			( TCHAR * AppPath, TCHAR * SystemName );
int		AppLibHasOwnership			( TCHAR * AppPath, TCHAR * SystemName);
TCHAR *	AppLibGetOwner				( TCHAR * AppPath );
int		AppLibReleaseOwnership		( TCHAR * AppPath, TCHAR * SystemName);
int		AppLibCountApplications		();
TCHAR *	AppLibGetNthApplication		( int n );
bool	AppLibLock					( int timeout );
bool	AppLibUnlock				();
TCHAR *	AppLibGetAppFriendlyName	( TCHAR * AppPath );
int		AppLibMaintenance			();

//  added since draft 2 spec

#define	APPLIB_CHANGED_CURRENT_OVERLAY  1
#define APPLIB_NO_CURRENT_OVERLAY       2
#define APPLIB_ATTACHED					1

TCHAR *	AppLibGetStudentAttachedOverlay		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
int		AppLibCountAttachedOverlays			( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
int		AppLibGetSelectedOverlay			( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
TCHAR *	AppLibGetNumberedOverlay			( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, int OverlayNumber);
int		AppLibAddApplicationForStudent		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
int		AppLibAddApplicationForCurrentStudent		( TCHAR * AppPath );
int		AppLibSetSelectedOverlay			( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, int OverlayNumber);
int		AppLibAttachOverlayForStudent		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, TCHAR *OverlayPath );
int		AppLibDetachOverlayForStudent		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, TCHAR *OverlayPath );
bool	AppLibIsSystemPresent				( TCHAR * SystemName );
void	AppLibAddPreinstalledOverlays		( TCHAR * GroupName, TCHAR * StudentName, bool bForce );
void	AppLibFloatingMessage				( TCHAR * Message );
bool	AppLibIsAppAllowed					( TCHAR * AppPath );
void	AppLibSetAppAllowed					( TCHAR * AppPath, bool bAllowed );
int		AppLibRemoveApplicationForStudent	( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
int		AppLibCountStandardApps				( );
TCHAR *	AppLibGetNthStandardApp				( int n );
TCHAR * AppLibGetTruePath					( TCHAR * AppPath );
void	AppLibSetSystemInterest				( TCHAR * AppPath, TCHAR * SystemName, bool bInterest );
bool	AppLibHasSystemInterest				( TCHAR * AppPath, TCHAR * SystemName );
void	AppLibAddDisallowedApps				();
void	AppLibDeferSaving					( bool bDefer );
bool	AppLibIsOverlayPreinstalled			( TCHAR * OverlayPath );
int		AppLibUpdateAppPath					( TCHAR * oldPath, TCHAR * newPath );
bool	AppLibIsIntelliSwitchInstalled		( );
bool	AppLibIsIntellikeysInstalled		( );

bool	AppLibCanListApplication			( TCHAR * AppName );

int		AppLibIsSystemActive				( TCHAR * SystemName );
int		AppLibSetSystemActive				( TCHAR * SystemName );
int		AppLibSetSystemInactive				( TCHAR * SystemName );

bool	AppLibShowDiscoverAnyway			();

void	AppLibKillFloatingMessage			();


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class AppLibDeferSavingBlock
{
public:
	AppLibDeferSavingBlock()
	{
		m_count++;
		if (m_count==1)
			AppLibDeferSaving(true);
	}
	~AppLibDeferSavingBlock()
	{
		if (m_count>0)
		{
			m_count--;
			if (m_count==0)
				AppLibDeferSaving(false);
		}
	}
private:
	static int m_count;
};

#endif


#endif  //  #ifndef _AppLib_H_
