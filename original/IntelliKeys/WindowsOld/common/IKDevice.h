// IKDevice.h: interface for the IKDevice class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKDEVICE_H__4B588751_F075_44F4_BE1D_5B3E7EA7D079__INCLUDED_)
#define AFX_IKDEVICE_H__4B588751_F075_44F4_BE1D_5B3E7EA7D079__INCLUDED_

#include "IKString.h"	
#include "QueueAr.h"
#include "IKOverlay.h"
#include "IKSettings.h"


//  write data function prototype

typedef void (*WriteDataProc)(int,BYTE *);

//  size of data going to/from the device
#define IK_REPORT_LEN 8

//  resolution of the device
#define IK_RESOLUTION_X 24
#define IK_RESOLUTION_Y 24

//  number of switches
#define IK_NUM_SWITCHES  6

//  number of sensors
#define IK_NUM_SENSORS   3

//  some internal commands that don't make it to the device
#define COMMAND_BASE 100
#define IK_CMD_DELAY				COMMAND_BASE+1 // (msec)
#define IK_CMD_MOUSE_MOVE			COMMAND_BASE+2 // (x, y)
#define IK_CMD_MOUSE_BUTTON			COMMAND_BASE+3 // (left/right, down/up)
#define IK_CMD_KEYBOARD				COMMAND_BASE+4 // (keycode, down/up)
#define IK_CMD_KEY_DONE				COMMAND_BASE+5 // signal downstream that a key is done
#define IK_CMD_KEY_START			COMMAND_BASE+6 // signal downstream that a key is starting
#define IK_CMD_KEY_REPEAT			COMMAND_BASE+7 // signal downstream that a key is repeating

#define IK_CMD_CP_HELP				COMMAND_BASE+8
#define IK_CMD_CP_LIST_FEATURES		COMMAND_BASE+9
#define IK_CMD_CP_REFRESH			COMMAND_BASE+10
#define IK_CMD_CP_TOGGLE			COMMAND_BASE+11

#define IK_CMD_KEYBOARD_UNICODE		COMMAND_BASE+12 // lead, trail, down/up

#define IK_CMD_LIFTALLMODIFIERS     COMMAND_BASE+13

#define IK_CMD_CP_REPORT_REALTIME	COMMAND_BASE+14

//  for keystrokes and mouse buttons
enum {IK_DOWN=1,IK_UP,IK_TOGGLE};
enum {IKUSB_LEFT_BUTTON=0,IKUSB_RIGHT_BUTTON,IKUSB_MIDDLE_BUTTON,IKUSB_BUTTON_4,IKUSB_BUTTON_5,IKUSB_BUTTON_6,IKUSB_BUTTON_7,IKUSB_BUTTON_8};

enum {
	kModifierStateOff = 0,
	kModifierStateLatched,
	kModifierStateLocked
};

class IKDevice
{
public:
	void KeySound2 ( int msLength, int freq );
	bool IsIntelliSwitchV1 ();
	int CountRawEvents ();
	void ShortKeySound();
	void LongKeySound();
	bool GetRawEvent ( BYTE *bytes );
	void PurgeRawQueue();
	void InterpretRaw ();
	void SetDevType (int devType) {m_devType = devType;}
	int  GetDevType () {return m_devType;}
	void Disconnect();
	int GetCurrentOverlayNumber() {return m_currentOverlay;}
	void PostReportDataToControlPanel(bool bForce=false);
	IKString GetKeyName ( int code );
	int GetRepeatDelayMS();
	void DoSetLED ( int number, int on );
	void ReportDataToControlPanel(bool bForce = false);
	void PostLiftAllModifiers();
	void KickDLL();
	void PostKeyUnicode ( BYTE lead, BYTE trail, int direction );
	void PostKeyStroke ( int code );
	void PostCPRefresh();
	IKString GetSerialNumber();
	int GetLevel();
	IKOverlay * GetCurrentOverlay();
	int GetIndex();
	void GetMembraneStatus ( BYTE *data, int *ndata );
	void GetSwitchStatus ( BYTE *data, int nswitch );
	void GetSensorArray ( BYTE *data );
	void UserReset(bool bFromCP);
	void LiftNonModifiers();
	void SetLevel(int level);
	void OverlaySendingFeedback();
	bool HasStandardOverlay();
	void SetCode ( BYTE code );
	IKDevice();
	virtual ~IKDevice();
	void Periodic ();
	void DoKey(int code, int direction);
	void DoKeyUnicode(BYTE lead, BYTE trail, int direction);
	static IKString GetCommandName ( BYTE command );
	bool IsSwitchedOn();
	void Reset();
	bool IsOpen() {return m_bOpen;}
	void SetOpen(bool bOpen) {m_bOpen = bOpen;}
	void SetIndex(int index) {m_index = index;}
	void SetWriteProc (WriteDataProc proc) {m_writeProc = proc;}
	void Start();
	void OnDataReceived ( BYTE *data );
	void PostKey(int code, int direction, int delayAfter=0);
	void KeySound(int msLength);
	void PostDelay(int ms);
	void PostMouseButtonToggle(int whichButton);
	void PostMouseButtonClick(int whichButton);
	void PostMouseButtonDoubleClick(int whichButton);
	void PostMouseButton(int whichButton, int direction);
	void PostMouseMove(int x, int y);
	void ResetMouse();
	void ResetKeyboard();
	
	void RawNotify ( int eventType, int i1, int i2, int time );
	// June 2012 - JR - added for retrieval of firmware version
	IKString FirmwareVersion();

private:
	bool m_bCapsLock;
	bool m_bNumLock;
	void PurgeToLastStart();
	static void SaveKeyboardSettings();
	static void RestoreKeyboardSettings();
	bool IsNumLockOn();
	bool IsCapsLockOn();
	bool IsMouseDown();
	unsigned int m_lastLEDTime;
	void SetLEDs();
	void DoKeyRepeat();
	void DoKeyStart();
	void DoKeyDone();
	void LiftAllLatchedModifiers();
	void LiftAllModifiers();
	void PostKeyDone();
	void PostKeyStart();
	void PostKeyRepeat();
	bool IsAnyModifierDown();
	bool IsMouseAction(BYTE code);
	void HackKeyContent ( BYTE *data );
	void ResetSound();
	void DeleteLastSentOverlay();
	int ComputeMouseInc(int nPar);
	bool IsIdle();
	int  CountRealCodes (BYTE *data);
	bool HasMouseClicks (BYTE *data);
	bool HasMouseActions (BYTE *data);
	bool IsModifier (BYTE data);
	bool IsMouseMove (BYTE data);
	bool IsMouseClick (BYTE data);
	bool IsSetupCode (BYTE data);
	bool HasSetupCodes (BYTE *data);
	bool HasOnlyMouseMoveActions (BYTE *data);
	bool HasNonRepeating(BYTE *data);
	bool DoSetupKey ( BYTE * data );

	//  type definition of an entry in a queue
	typedef struct
	{
		BYTE buffer[IK_REPORT_LEN];
	} queueEntry;

	//  this is the structure of data found in the intellikeys eeprom
	#define IK_EEPROM_SN_SIZE 29
	typedef struct
	{
		BYTE serialnumber[IK_EEPROM_SN_SIZE];
		BYTE sensorBlack[IK_NUM_SENSORS];
		BYTE sensorWhite[IK_NUM_SENSORS];
	} eeprom;

	void FindDomain ( IKOverlay *pOverlay, int *domainNumber, bool *bPressAnywhere, int *switchNumber );
	void OverlayRecognitionFeedback();
	void OnStdOverlayChange ();

	bool IsKeystroke(BYTE code);
	void SettleOverlay();

	void OnMembranePress ( int x, int y );
	void OnMembraneRelease ( int x, int y );
	void OnCorrectSwitch ( int switchnum );
	void DoCorrect();
	void OnCorrectMembrane(int x, int y);
	void OnCorrectDone();
	void PostCommand ( BYTE *command );
	void SendCommand ( BYTE *command );

	void ProcessInput();
	void PostSetLED(int number, int value);
	void ProcessCommands();
	void Interpret();
	void UniversalToHID ( int universalCode, int *usagePage, int *usageID );
	void DoMouseButton(int whichButton, int direction);
	void DoMouseMove(int x, int y);
	void PostSound ( int freq, int duration, int volume );

	void PurgeQueues();
	void OnToggle ( int newValue );
	void SweepSound(int iStartFreq, int iEndFreq, int iDuration);
	void OnSwitch ( int nSwitch, int state );
	void OnSensorChange ( int sensor, int value );

	Queue<queueEntry> m_inputQueue;
	Queue<queueEntry> m_commandQueue;
	Queue<queueEntry> m_rawQueue;

	int m_membrane[IK_RESOLUTION_X][IK_RESOLUTION_Y];

	unsigned int m_delayUntil;
	unsigned int m_nextCorrect;
	BYTE m_KeyBoardReport[7];
	BYTE m_MouseReport[3];
	int m_toggle;
	int m_switches[IK_NUM_SWITCHES];
	int m_sensors[IK_NUM_SENSORS];

	int m_currentLevel;

	//  overlay recognition
	int m_lastOverlay;
	unsigned int m_lastOverlayTime;
	int m_currentOverlay;

	//  reading the eeprom
	void StoreEEProm ( BYTE data, BYTE addlsb, BYTE addmsb );
	eeprom m_eepromData;
	unsigned int m_eepromRequestTime[sizeof(eeprom)];
	bool m_eepromDataValid[sizeof(eeprom)];
	bool m_bEepromValid;

	//  for correction
	BYTE m_membranePressedInCorrectMode[IK_RESOLUTION_X][IK_RESOLUTION_X];
	BYTE m_switchesPressedInCorrectMode[IK_NUM_SWITCHES];

	BYTE m_last_membrane[IK_RESOLUTION_X][IK_RESOLUTION_Y];
	BYTE m_last_switches[IK_NUM_SWITCHES];
	bool m_bLastInit;

	
	void KeySoundVol(int msLength,int volume=-1);

	int m_repeatDomain;
	int m_currentDomain;
	unsigned int  m_repeatAfter;
	bool m_bRepeatLatched ;
	bool m_bRepeatLatchReleased ;
	int  m_nrepeat ;
	unsigned int  m_timeToRespond;
	unsigned int  m_deadUntil ;
	int  m_ndown ;
	bool m_bDidSetup;

	int ExecuteUniversalData(BYTE *thedata, bool bForceKeyUp=false );

	//  track some times for liftoff.
	unsigned int m_lastPress;
	unsigned int m_lastRelease;
	unsigned int m_lastExecute;
	unsigned int m_startDomain;

	int m_counter ;
	int m_lastcode ;

	class IKModifier
	{

	public:
		IKModifier(){m_state = kModifierStateOff;}
		virtual ~IKModifier(){}
		void Execute(int code=0);
		int GetState(){return m_state;}
		void SetState(int state);
		void SetCode (BYTE code ){m_universalCode = code;}
		int m_state;
		int m_universalCode;
		int m_universalCodeOverride;
		void SetDevice(IKDevice *pDev){m_device = pDev;}
		IKDevice *m_device;
	};

	IKModifier m_modShift;
	IKModifier m_modAlt;
	IKModifier m_modControl;
	IKModifier m_modCommand;

	bool m_lights[9];

	bool m_bOpen;

	WriteDataProc m_writeProc;

	int m_index;

	int m_lastCodeUp;

	bool m_bShifted;

	BYTE * m_lastExecuted;

	int m_last5Overlays[5];

	int m_newLevel;

	int m_devType;

	int m_lastSwitch;

	// June 2012 - JR - added for holding Firmware Version info
	unsigned char m_firmwareVersionMajor;
	unsigned char m_firmwareVersionMinor;
};

#endif // !defined(AFX_IKDEVICE_H__4B588751_F075_44F4_BE1D_5B3E7EA7D079__INCLUDED_)
