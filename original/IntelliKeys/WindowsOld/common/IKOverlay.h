// IKOverlay.h: interface for the IKOverlay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKOVERLAY_H__F1519DB7_7FFE_409A_9232_B45D208FAF92__INCLUDED_)
#define AFX_IKOVERLAY_H__F1519DB7_7FFE_409A_9232_B45D208FAF92__INCLUDED_


#include "IKString.h"


#define OVL_NAME_LEN	32			/* overlay name/descriptor */
#define NUM_MACHINES	16			/* number of possible Cable_vals */
#define NUM_LEVELS		16
#define MAX_OVL_SIZE	0x7200

class IKString;

class IKOverlay  
{
public:
	BYTE * GetRawSettings () {return rawsettings;}
	void DoctorData();
	void StoreData ( BYTE *data, int datalen );
	void StoreSettings ( BYTE *data );
	bool SaveToFile(IKString filename);
	IKString m_nameString;
	void SetName(IKString name);
	BYTE * GetUniversalCodesFromDomain(int theDomain, int level);
	IKString GetName();
	void MakeSetupOverlay();
	IKOverlay();
	virtual ~IKOverlay();
	bool LoadFromFile(IKString filename);
	int GetDomainFromSwitch( int nswitch, int level );
	int GetDomainFromMembrane ( int x, int y, int level );
	bool IsSetupOverlay();
	int GetNumLevels();
	void Unload();
        void SwapOffsetBytes();
	WORD ComputeChecksum();

private:
	typedef struct OverlayHeader
	 {		/* overlay header */
		BYTE name[ OVL_NAME_LEN ];		/* descriptor */
		BYTE number;					/* of overlay */
		BYTE num_levels;				/* in overlay */
		BYTE setup_flag;				/* 1 if setup type, 0 if other type */
		BYTE coord_mode;
		BYTE reserved[ 4 ];				/* for the future */
		BYTE base_level[ NUM_MACHINES ];	/* which level each machine starts at */
		BYTE mouse_level[ NUM_MACHINES ];	/* which level each machine starts at */
		WORD offset[ NUM_LEVELS ];		/* offset from start of data to each level */
	} OverlayHeader;						/* followed by compressed key tables, string data */

	
	bool m_bSetupOverlay;
	bool IsValid();
	bool m_bValid;
#ifdef PLAT_MACINTOSH
	bool LoadFromFileMac(IKString filename);
	bool MacExtractFromFile ( TCHAR *pFilePath, Handle * p_hOverlay, Handle * p_hSet );
#endif

	BYTE ovldata[MAX_OVL_SIZE];
	OverlayHeader header;
	BYTE rawsettings[24];

	int m_nbytes;

	WORD m_checksum;

	bool m_bMacOverlay;
};

#endif // !defined(AFX_IKOVERLAY_H__F1519DB7_7FFE_409A_9232_B45D208FAF92__INCLUDED_)
