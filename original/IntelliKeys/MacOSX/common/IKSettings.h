// IKSettings.h: interface for the IKSettings class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKSETTINGS_H__2529EB67_16DF_4B22_B49F_7BE997C36C53__INCLUDED_)
#define AFX_IKSETTINGS_H__2529EB67_16DF_4B22_B49F_7BE997C36C53__INCLUDED_


#include "IKString.h"
#include "IKPrefs.h"

enum {
	kSettingsRateLow = 1,
	kSettingsRateHigh = 15
};

enum {
	kSettingsShiftLatching=0,
	kSettingsShiftLocking,
	kSettingsShiftNoLatch
};

enum {
	kSettings3lights=0,
	kSettings6lights
};

enum {
	kSettingsKeysoundOff = 0,
	kSettingsKeysound1,
	kSettingsKeysound2,
	kSettingsKeysound3,
	kSettingsKeysound4
};

enum {
	kSettingsModeLastSentOverlay = 0,
	kSettingsModeThisOverlay,
	kSettingsModeSwitch,
	kSettingsModeDiscover
};

class IKSettings  
{
public:

	IKSettings();
	virtual ~IKSettings();

	static IKSettings * GetSettings ();

	void Write ( IKString filename );
	void Write ( );
	bool Read ( IKString filename );
	void SetToDefault(bool bFeatureReset=false);
	IKSettings & operator = ( const IKSettings &rhs );
	bool operator == ( const IKSettings &rhs );
	bool operator != ( const IKSettings &rhs );
	int  GetIntValue  (const TCHAR * pKey);
	void SetIntValue  (const TCHAR * pKey, int value);
	bool GetBoolValue (const TCHAR * pKey);
	void SetBoolValue (const TCHAR * pKey, bool value);
	IKString GetStringValue(const TCHAR * pKey);
	void SetStringValue ( const TCHAR * pKey, const TCHAR * pValue);
	IKSettings ( const IKSettings& src);  //  copy ctor

	int  m_iResponseRate;
	bool m_bRequiredLiftOff;
	int  m_iRepeatRate;
	bool m_bRepeat;
	bool m_bRepeatLatching;
	int  m_iShiftKeyAction;
	int  m_iMouseSpeed;
	bool m_bSmartTyping;
	int  m_iDataSendRate;
	int  m_iMakeBreakRate;
	int  m_iIndicatorLights;
	int  m_iKeySoundVolume;
	int  m_iUseThisSwitchSetting;
	bool m_bUseSystemRepeatSettings;
	IKString m_sUseThisOverlay;
	int m_iMode;
	IKPrefs m_keyMap;
	IKString m_sLastSent;
	IKString m_sLastSentBy;
	bool m_bShowModeWarning;
	bool m_bButAllowOverlays;

private:
	void StoreValues ();
};

#endif // !defined(AFX_IKSETTINGS_H__2529EB67_16DF_4B22_B49F_7BE997C36C53__INCLUDED_)
