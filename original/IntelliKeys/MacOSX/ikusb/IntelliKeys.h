#ifndef __INTELLIKEYS_H__
#define __INTELLIKEYS_H__

// Enumerated error codes

enum {
	kNoError = 0,
	kFileNotFound,
	kFileNotOverlay,
	kTransmitError,
	kIntelliKeysNotFound,
	kUnknownError
};

class IntelliKeys
{
public:
	static bool		IsIntelliKeysConnected ( void );
	static bool		IsSendableOverlayFile ( char * pFilePath );
	static int		SendOverlay ( char *  pFilePath, bool bReportErrors = false, char * pMessage=NULL, bool bWait=true, bool bBannerOnly=false );
};

#endif // __XINTELLIKEYS_H__

