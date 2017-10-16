// IKMsg.h: interface for the IKMsg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKMSG_H__0AA2221B_4FFF_42C7_90D8_D1A2EB5F2771__INCLUDED_)
#define AFX_IKMSG_H__0AA2221B_4FFF_42C7_90D8_D1A2EB5F2771__INCLUDED_

#include "IKString.h"
#include "Messages.h"

class IKMsg  
{
public:
//  Functions

	static void Initialize();
	static void CleanSendingChannel();

	static int Send       ( IKString channel, int command, void *dataOut, int dataOutLen, void *dataIn,int *dataInLen );
	static int SendTO     ( IKString channel, int command, void *dataOut, int dataOutLen, void *dataIn,int *dataInLen, int timeoutMS );
	static int Receive    ( IKString channel, int *command, void *data, int maxdata, int *dataread );
	static int Respond    ( IKString channel, int response, void *data, int datalength );
};

#endif // !defined(AFX_IKMSG_H__0AA2221B_4FFF_42C7_90D8_D1A2EB5F2771__INCLUDED_)
