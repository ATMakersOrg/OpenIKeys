//
// IKEngine.h: interface for the IKEngine class.
//
// A hosting application should instance IKEngine once
// and initialize it.  Then it is responsible for calling
// Periodic() at intervals of ENGINE_PERIOD msecs.
//
//////////////////////////////////////////////////////////////////////

#if !defined(IKENGINE_INCLUDED)
#define IKENGINE_INCLUDED

#include "IKDevice.h"
#include "IKOverlay.h"
#include "IKPrefs.h"
#include "IKString.h"	// Added by ClassView

//  maximum numbers
#define MAX_INTELLIKEYS 10
#define MAX_STANDARD_OVERLAYS 8
#define MAX_SWITCH_OVERLAYS 30

class IKEngine  
{
public:
	void CheckNewInstall ();
	void OnNewCustomOverlaySender ( IKString name, IKString sender );
	void GetCurrentGroupAndStudent(IKString &group, IKString &student);
	int CountOpenDevices();
	void LoadStudentData();
	void SwitchApplication(IKString &app);
	void SendOverlayForApp ( IKString &file, IKString &sender );
	void WatchApplications ();
	void DisconnectDevices();

#ifdef PLAT_WINDOWS
	void FixIkeysXfr ();
#endif

	void MakeAllFilesWriteable(IKString startingFolder);
	void OnNewOverlayData();
	void ReloadSwitchOverlay();
	void ConvertUser ( IKString oldfile, IKString newfile );
	void DeleteLastSentOverlay();
	IKOverlay * GetSwitchOverlay(int number);
	IKOverlay * GetIntelliSwitchOverlay() {return &m_defaultIntelliSwitchOverlay;}
	IKOverlay * GetUseThisOverlay();
	void ReloadCustomOverlayFile(IKString filename);
	void OnNewCustomOverlaySettings(BYTE *data, int datalen);
	void OnNewCustomOverlayData(BYTE *data, int datalen);
	void MakeDiagnosticBlock ( BYTE *data, int *ndata );
	void MakeGlobalData ( BYTE *data, int *ndata );

	IKDevice * GetFirstDevice();
	IKDevice * GetNumberedDevice(int ndev);
	IKString GetGroup();
	IKString GetStudent();
	void MakeIntellikeysArray ( BYTE *data, int *datalen );
	bool GetDiagnosticMode();
	void SetDiagnosticMode ( bool mode );
	bool GetRawMode();
	bool UseRawMode();
	void SetRawMode ( bool mode, int immunityTime=0 );
	void OnDataReceived ( int index, BYTE *data );
	void RemoveDevice ( int index );
	void AddDevice ( int index, WriteDataProc proc, int devType );
	void Terminate();
	void SetNotifyMode ( bool bMode );
	bool GetNotifyMode () {return m_bNotifyMode;}

	IKEngine();			//  construct
	virtual ~IKEngine();//  destruct

	void Initialize();	//  initialize
	void Periodic();	//  call this to keep things moving
	bool IsRunning()	{return m_bRunning;}
	void Stop()			{m_bRunning = false;}

	static IKEngine * GetEngine();

	IKOverlay * GetStandardOverlay(int index);
	IKOverlay * GetCustomOverlay () {return &m_customOverlay;}
	void LoadStandardOverlays	( );
	int NumDevicesWithStandardOverlay();
	int NumDevicesOn();
	int NumDevicesConnected();
	
	void PostKey					(int code, int direction);
	void PostDelay					(int delay);
	void PostMouseButtonToggle		(int whichButton);
	void PostMouseButtonClick		(int whichButton);
	void PostMouseButtonDoubleClick	(int whichButton);
	void PostMouseButton			(int whichButton, int direction);
	void PostMouseMove				(int x, int y);
	void ResetMouseInterface		();
	void ResetKeyboardInterface		();

private:
	void ProcessSentSettings ( BYTE *settings );

	void OnNewCustomOverlayFile ( IKString filename );
	void OnNewCustomOverlayName ( IKString name );
	void OnChangeStudent(IKString group, IKString student);
	void ProcessExternalCommands( );

	void ProcessExternalCommands2( );

	IKDevice m_devices[MAX_INTELLIKEYS];
	unsigned int m_nextExternalCommandTime;
	unsigned int m_nextExternalCommandTime2;
	unsigned int m_nextAppWatchTime;
	bool m_bRunning;

	IKOverlay m_standardOverlay[MAX_STANDARD_OVERLAYS];
	IKOverlay m_customOverlay;
	IKOverlay m_UseThisOverlay;
	IKOverlay m_defaultIntelliSwitchOverlay;

	int m_nSwitchOverlays;
	IKString m_switchOverlays[MAX_SWITCH_OVERLAYS];
	IKOverlay m_SwitchOverlay;

	IKString m_group;
	IKString m_student;
	IKPrefs m_preferences;

	bool m_bDiagnosticMode;
	bool m_bRawMode;
	unsigned int m_lastDiag;
	
	bool m_bForceSendAttachedOverlay;

	bool m_bRefreshCP;
	
	IKString m_currentApp;
	
	bool m_bNotifyMode;
};

#endif // !defined(IKENGINE_INCLUDED)
