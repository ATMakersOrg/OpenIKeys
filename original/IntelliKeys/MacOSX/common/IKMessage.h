// IKMessage.h: interface for the IKMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKMESSAGE_H__FDFD2DB4_68BB_44F5_ABF5_E1D8EAD20B26__INCLUDED_)
#define AFX_IKMESSAGE_H__FDFD2DB4_68BB_44F5_ABF5_E1D8EAD20B26__INCLUDED_

#include "IKString.h"
#include "Messages.h"

class IKMessage  
{
public:
	static void Initialize();
	static void CheckChannels();
	static bool IsOwnerAlive ( IKString channel );
	static IKString GetMessageName( int message );

//  Functions

	static int Send       ( IKString channel, int command, void *dataOut, int dataOutLen, void *dataIn,int *dataInLen, bool bCheckAlive=true );
	static int SendTO     ( IKString channel, int command, void *dataOut, int dataOutLen, void *dataIn,int *dataInLen, int timeoutMS, bool bCheckAlive=true );
	static int Receive    ( IKString channel, int *command, void *data, int maxdata, int *dataread );
	static int Respond    ( IKString channel, int response, void *data, int datalength );
};

#endif // !defined(AFX_IKMESSAGE_H__FDFD2DB4_68BB_44F5_ABF5_E1D8EAD20B26__INCLUDED_)
