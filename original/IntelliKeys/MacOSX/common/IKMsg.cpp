// IKMsg.cpp: implementation of the IKMsg class.
//
//////////////////////////////////////////////////////////////////////

#include "IKCommon.h"
#include "IKFile.h"
#include "IKUtil.h"
#include "IKMsg.h"

static int defaultTimeout       = 1000;

static const TCHAR *strTransactLockFile = TEXT("transaction lock.dat");
static const TCHAR *strCommandFile		= TEXT("command.dat");
static const TCHAR *strResponseFile 	= TEXT("response.dat");

static void MakeChannelFileName(IKString channel, IKString name, IKString& filename)
{
	filename  = IKUtil::GetChannelsFolder();
	filename += channel;
	filename += IKUtil::GetPathDelimiter();
	filename += name;
}

static void CleanupChannel(IKString channel)
{
	IKString filename;

	//  command file
	MakeChannelFileName(channel,IKString(strCommandFile),filename);
	IKFile::Remove(filename);

	//  response file
	MakeChannelFileName(channel,IKString(strResponseFile),filename);
	IKFile::Remove(filename);

}

static int GetLock ( IKString channel, IKString lockName, int timeoutMS )
{
	IKString filename;
	MakeChannelFileName ( channel, lockName, filename );

	unsigned int start = IKUtil::GetCurrentTimeMS();
	while (1==1)
	{

#ifdef PLAT_WINDOWS
		HANDLE hFile = CreateFile ( filename, GENERIC_WRITE, 0, NULL,
			CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			return kResponseNoError;
		}
                
#else

                FSSpec spec;
                TCHAR macFileName[255];
                IKString::strcpy(&(macFileName[1]),filename);
                macFileName[0] = IKString::strlen(&(macFileName[1]));
                OSErr error = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &spec );

                if ((error == fnfErr) || (error == afpItemNotFound))
                {
                    error = FSpCreate(&spec,'TEXT','????',smSystemScript);
                    if (error==noErr)
                        return kResponseNoError;
                }

#endif
		//  if no timeout, just return error.
		if (timeoutMS<=0)
			return kResponseError;

		//  are we timed out?
		unsigned int now = IKUtil::GetCurrentTimeMS();
		if ((int)(now-start) > timeoutMS)
			return kResponseTimeout;
		IKUtil::Sleep(100);
	}

	return kResponseError;  //  we never get here
}


static int ReleaseLock ( IKString channel, IKString lockName )
{
	IKString filename;
	MakeChannelFileName ( channel, lockName, filename );

	IKFile::Remove(filename);

	return kResponseNoError;
}



static int ReadDataFile (IKString filename, int *response, int *datalength, void *data )
{
	IKFile f;

	if (f.Open(filename,IKFile::modeRead))
	{
		f.Read(response,4);
		f.Read(datalength,4);

		if(*datalength>0)
			f.Read(data,*datalength);

		f.Close();

		return kResponseNoError;
	}

	return kResponseError;
}


static int WriteDataFile (IKString filename, int command, int datalength, void *data )
{
	IKString tempfile = filename;
	tempfile += TEXT(".temp");

	IKFile f;

    if (f.Open(tempfile,IKFile::modeWrite|IKFile::modeCreate))
    {
        f.Write((void *)&command,   sizeof(command)   );
        f.Write((void *)&datalength,sizeof(datalength));
        if (data!=NULL && datalength>0)
                f.Write(data,datalength);
        f.Close();

		IKFile::Remove(filename);
		
#ifdef PLAT_WINDOWS
		IKFile::Rename(tempfile,filename);
#else
		//  spec the old name.  Must exist.
		FSSpec spec;
	    TCHAR macFileName[255];
	    IKString::strcpy(&(macFileName[1]),tempfile);
	    macFileName[0] = IKString::strlen(&(macFileName[1]));
		OSErr err = FSMakeFSSpec( 0, 0, (unsigned TCHAR *)macFileName, &spec );
		if (err==noErr)
		{
			int i = filename.ReverseFind(TCHAR(':'));
			IKString shortname = filename.Mid(i+1);
	    	IKString::strcpy(&(macFileName[1]),shortname);
	    	macFileName[0] = IKString::strlen(&(macFileName[1]));
			err = FSpRename ( &spec, (unsigned TCHAR *) macFileName );
		}
    
#endif

		return kResponseNoError;
    }

	return kResponseError;
}



int IKMsg::Send ( IKString channel, int command, void *dataOut, int dataOutLen, void *dataIn,int *dataInLen )
{
	return IKMsg::SendTO ( channel, command, dataOut, dataOutLen, dataIn, dataInLen, defaultTimeout );
}


int IKMsg::SendTO     ( IKString channel, int command, void *dataOut, int dataOutLen, void *dataIn,int *dataInLen, int timeoutMS )
{
	//::MessageBox(NULL,"IKMsg::SendTO start","",MB_OK);

	//  take out a transaction lock.
	int err = GetLock ( channel, IKString(strTransactLockFile), timeoutMS );
	if (err != kResponseNoError)
	{
		return err;
	}

	//  clean out the channel.
	CleanupChannel(channel);

	//  send the command
	IKString filename;
	MakeChannelFileName(channel,IKString(strCommandFile),filename);
	err = WriteDataFile ( filename, command, dataOutLen, dataOut );

	//  get the response with timeout
	if (err == kResponseNoError)
	{
		unsigned int start = IKUtil::GetCurrentTimeMS();
		while (1==1)
		{
			// read the file
			IKString responsefile;
			int l = 0;
			MakeChannelFileName(channel,IKString(strResponseFile),responsefile);
			int response;
			BYTE mydata[4096];
			err = ReadDataFile ( responsefile, &response, &l, mydata);

			//  if success, give data to caller and break
			if (err == kResponseNoError)
			{
				if (dataInLen)
					*dataInLen = l;
				if (dataIn)
				{
					for (int i=0;i<l;i++)
						((BYTE *)dataIn)[i] = mydata[i];
					((BYTE *)dataIn)[l] = 0;
					((BYTE *)dataIn)[l+1] = 0;
				}

				err = response;
				break;
			}

			//  no time out.
			if (timeoutMS<=0)
			{
				err = kResponseError;
				break;
			}

			//  time out
			unsigned int now   = IKUtil::GetCurrentTimeMS();
			if (now>start+timeoutMS)
			{
				err = kResponseTimeout;
				break;
			}

			//  wait and try again
			IKUtil::Sleep(100);
		}
	}

	//  clean out the channel.
	CleanupChannel(channel);

	//  release the transaction lock.
	ReleaseLock ( channel, IKString(strTransactLockFile) );
	
	return err;

}

int IKMsg::Receive    ( IKString channel, int *command, void *data, int maxdata, int *dataread )
{
	int x = maxdata;
	
	IKString filename;
	MakeChannelFileName(channel,IKString(strCommandFile),filename);

	int err = ReadDataFile ( filename, command, dataread, data );

	if(err==kResponseError)
		err = kResponseNoCommand;

	IKFile::Remove(filename);

	return err;
}


int IKMsg::Respond    ( IKString channel, int response, void *data, int datalength )
{

	IKString filename;
	MakeChannelFileName(channel,IKString(strResponseFile),filename);

	int err = WriteDataFile ( filename, response, datalength, data );

	return err;
}




void IKMsg::CleanSendingChannel()
{
	IKString filename;

	MakeChannelFileName(TEXT("sending"),strTransactLockFile,filename);
	IKFile::Remove(filename);

	MakeChannelFileName(TEXT("sending"),strCommandFile,filename);
	IKFile::Remove(filename);
	filename += TEXT(".temp");
	IKFile::Remove(filename);

	MakeChannelFileName(TEXT("sending"),strResponseFile,filename);
	IKFile::Remove(filename);
	filename += TEXT(".temp");
	IKFile::Remove(filename);
}


void IKMsg::Initialize()
{
	IKString filename;

	MakeChannelFileName(TEXT("sending"),"a.a",filename);
	IKFile::MakeWritable(filename);

}
