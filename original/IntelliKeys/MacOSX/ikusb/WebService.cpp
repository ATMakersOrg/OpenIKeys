/*
 *  WebService.cpp
 *
 *  Created by fred ross-perry on 6/16/09.
 *  Copyright 2009 IntelliTools, Inc. All rights reserved.
 *
 */
 
#ifdef PLAT_MACINTOSH
#include <curl/curl.h>
#else
#include "curl.h"
#endif

#include <string.h>
#include <assert.h>

#include "WebService.h"

//---------------------------------------------------------------------------------------------------------------------
//
//  template data for GetContent

const char *templateGetContentHeaders[] = 
{
	"Content-Type: text/xml; charset=utf-8",
	"SOAPAction: \"http://www.kurzweiledu.com/overlayweb/GetContent\""
};

const char *templateGetContentBody = "\
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:over=\"http://www.kurzweiledu.com/overlayweb\">\
<soapenv:Header/>\
<soapenv:Body>\
<over:GetContent>\
<over:key>guid</over:key>\
<over:data>base64Binary</over:data>\
</over:GetContent>\
</soapenv:Body>\
</soapenv:Envelope>\
";

//---------------------------------------------------------------------------------------------------------------------
//
//  template data for StoreContent

const char *templateStoreContentHeaders[] = 
{
	"Content-Type: text/xml; charset=utf-8",
	"SOAPAction: \"http://www.kurzweiledu.com/overlayweb/StoreContent\""
};

const char *templateStoreContentBody = "\
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:over=\"http://www.kurzweiledu.com/overlayweb\">\
<soapenv:Header/>\
<soapenv:Body>\
<over:StoreContent>\
<over:key>guid</over:key>\
<over:data>base64Binary</over:data>\
<over:overwrite>true</over:overwrite>\
</over:StoreContent>\
</soapenv:Body>\
</soapenv:Envelope>\
";

//---------------------------------------------------------------------------------------------------------------------
//
//  template data for DeleteAll

const char *templateDeleteAllHeaders[] = 
{
	"Content-Type: text/xml; charset=utf-8",
	"SOAPAction: \"http://www.kurzweiledu.com/overlayweb/DeleteAll\""
};

const char *templateDeleteAllBody = "\
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:over=\"http://www.kurzweiledu.com/overlayweb\">\
<soapenv:Header/>\
<soapenv:Body>\
<over:DeleteAll/>\
</soapenv:Body>\
</soapenv:Envelope>\
";

//---------------------------------------------------------------------------------------------------------------------
//
//  template data for DataExists

const char *templateDataExistsHeaders[] = 
{
	"Content-Type: text/xml; charset=utf-8",
	"SOAPAction: \"http://www.kurzweiledu.com/overlayweb/DataExists\""
};

const char *templateDataExistsBody = "\
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:over=\"http://www.kurzweiledu.com/overlayweb\">\
<soapenv:Header/>\
<soapenv:Body>\
<over:DataExists>\
<over:key>guid</over:key>\
<over:exist>true</over:exist>\
</over:DataExists>\
</soapenv:Body>\
</soapenv:Envelope>\
";

//---------------------------------------------------------------------------------------------------------------------
//
//  template data for DeleteKey

const char *templateDeleteKeyHeaders[] = 
{
	"Content-Type: text/xml; charset=utf-8",
	"SOAPAction: \"http://www.kurzweiledu.com/overlayweb/DeleteKey\""
};

const char *templateDeleteKeyBody = "\
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:over=\"http://www.kurzweiledu.com/overlayweb\">\
<soapenv:Header/>\
<soapenv:Body>\
<over:DeleteKey>\
<over:key>guid</over:key>\
</over:DeleteKey>\
</soapenv:Body>\
</soapenv:Envelope>\
";

//---------------------------------------------------------------------------------------------------------------------
//
//  template data for ListKeys

const char *templateListKeysHeaders[] = 
{
	"Content-Type: text/xml; charset=utf-8",
	"SOAPAction: \"http://www.kurzweiledu.com/overlayweb/ListKeys\""
};

const char *templateListKeysBody = "\
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:over=\"http://www.kurzweiledu.com/overlayweb\">\
<soapenv:Header/>\
<soapenv:Body>\
<over:ListKeys>\
<over:msg>message</over:msg>\
</over:ListKeys>\
</soapenv:Body>\
</soapenv:Envelope>\
";

//---------------------------------------------------------------------------------------------------------------------

static void strsub ( char *string, const char *find, const char *replace )
{
	int lf = strlen(find);
	//int lr = strlen(replace);
	int ls = strlen(string);
	
	int nloop = ls - lf + 1;
	
	int i;
	for (i=0;i<nloop;i++)
	{
		bool bMatch = true;
		int j;
		for (j=0;j<lf;j++)
			if (find[j]!=string[i+j])
				bMatch = false;
		if (bMatch)
		{
			char *rest = new char[ls-i+1];
			strncpy ( rest, &(string[i+lf]), ls-i-lf);
			rest[ls-i-lf] = 0;
			
			string[i] = 0;
			strcat ( string, replace );
			strcat ( string, rest );
			
			delete []rest;
			return;
		}
	}
}

//------------------------------------------------------------------------------------------------
//
//  base64 stuff

//  Translation Table as described in RFC1113

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//  find the index that matches the given value.
static int find ( char c )
{
	int l = strlen(cb64);
	for (int i=0;i<l;i++)
	{
		if ((cb64[i])==(c))
			return i;
	}

	assert(false);
	return -1;
}

//  get the value of a certain bit in a bit array.
static unsigned int getbit(unsigned char *set, int number)
{
	unsigned char * pByte = &(set[number/8]);
	return (*pByte & (1 << (number % 8))) != 0;       /* 0 or 1       */
}

// JR - Dec 2012 - upgrade to xCode 4.4.1 and 10.8 SDK caused setbit to fail as
//   I believe it is declared elsewhere now, so I renamed it to setbit3.

//  set a bit in a bit array to a given value.
static void setbit3(unsigned char *set, int number, int value)
{
	unsigned char * pByte = &(set[number/8]);
	if (value)
		*pByte |= 1 << (number % 8);              /* set bit      */
	else    
		*pByte &= ~(1 << (number % 8));           /* clear bit    */
}

static void decode64 ( char *src, unsigned char**bytes, int *nbytes )
{
	int srclen = strlen(src);
	unsigned char *output = new unsigned char[((srclen-1)/3+1)*4+1];  //  caller must delete
	*bytes = output;
	int nb = 0;
	for (int i=0;i<srclen;i++)
	{
		char val = find (src[i]);
		if (val>=0)
		{
            unsigned char uval = val;
			setbit3 ( output, nb, getbit(&uval,0) );  nb++;
			setbit3 ( output, nb, getbit(&uval,1) );  nb++;
			setbit3 ( output, nb, getbit(&uval,2) );  nb++;
			setbit3 ( output, nb, getbit(&uval,3) );  nb++;
			setbit3 ( output, nb, getbit(&uval,4) );  nb++;
			setbit3 ( output, nb, getbit(&uval,5) );  nb++;
		}
	}
	
	*nbytes = ((nb-1)/8) + 1;

}

static void encode64 ( unsigned char *bytes, int nbytes, char **dst )
{
	//  make a copy of the input array padded to a multiple
	//  of three
	
	int nb = ((nbytes-1)/3+1)*3;
	unsigned char *input = new unsigned char[nb];
	int i;
	for (i=0;i<nb;i++)
	{
		input[i] = 0;
		if (i<nbytes)
			input[i] = bytes[i];
	}

	int nbits = nb*8;
	int ncodes = (nbits-1)/6+1;
	*dst = new char[ncodes*8/6+1];  //  caller must delete this

	for (i=0;i<ncodes;i++)
	{
		unsigned char val = 0;
		setbit3 ( &val, 0, getbit ( input, i*6+0 ));
		setbit3 ( &val, 1, getbit ( input, i*6+1 ));
		setbit3 ( &val, 2, getbit ( input, i*6+2 ));
		setbit3 ( &val, 3, getbit ( input, i*6+3 ));
		setbit3 ( &val, 4, getbit ( input, i*6+4 ));
		setbit3 ( &val, 5, getbit ( input, i*6+5 ));

		(*dst)[i] = cb64[val];
	}
	
	(*dst)[ncodes] = 0;
	
	delete []input;
}

//------------------------------------------------------------------------------------------------

bool OverlayWebService :: DataExists ( char *key )
{
	//  count headers
	int nheaders = sizeof(templateDataExistsHeaders)/sizeof(char *);
	
	//  set up body
	char *body = new char[strlen(templateDataExistsBody)+strlen(key)+1];
	strcpy ( body, templateDataExistsBody );
	strsub ( body, "guid", key );
	//strsub ( body, "true", "" );
	
	//  make the request, then toss the body
	//  allocate a minimal response, it will be re-allocated by MakeSoapRequest
	char *response = new char[1];  response[0]=0;
	MakeSoapRequest(templateDataExistsHeaders, nheaders, body, &response);
	delete []body;
	
	//  get the status out of the response, then toss it
	
	const char *tag1 = "<DataExistsResult>";
	const char *tag2 = "</DataExistsResult>";
	char *k1 = strstr(response,tag1);
	char *k2 = strstr(response,tag2);
	
	bool bStatus = false;
	if (k1 && k2)
	{
		char *start = k1 + strlen(tag1);
		if (strstr(start,"success")==start)
		{
			const char *tag3 = "<exist>";
			const char *tag4 = "</exist>";
			char *k3 = strstr(response,tag3);
			char *k4 = strstr(response,tag4);
			if (k3 && k4)
			{
				char *start2 = k3 + strlen(tag3);
				if (strstr(start2,"true")==start2)
					bStatus = true;
			}
		}
	}

	delete []response;

	return bStatus;
}

bool OverlayWebService :: DeleteAll ()
{	
	//  set up body
	char *body = new char[strlen(templateDeleteAllBody)+1];
	strcpy ( body, templateDeleteAllBody );
	
	//  make the request, then toss the body
	//  allocate a minimal response, it will be re-allocated by MakeSoapRequest
	int nheaders = sizeof(templateDeleteAllHeaders)/sizeof(char *);
	char *response = new char[1];  response[0]=0;
	MakeSoapRequest(templateDeleteAllHeaders, nheaders, body, &response);
	delete []body;
	
	//  get the status out of the response, then toss it
	
	bool bStatus = false;
	
	const char *tag1 = "<DeleteAllResult>";
	const char *tag2 = "</DeleteAllResult>";
	
	char *k1 = strstr(response,tag1);
	char *k2 = strstr(response,tag2);
	
	if (k1 && k2)
	{
		char *start = k1 + strlen(tag1);
		if (strstr(start,"success")==start)
			bStatus = true;
	}

	delete []response;

	return bStatus;
}

	
bool OverlayWebService :: GetContent ( char *key, unsigned char **bytes, int *nbytes )
{
	//  count headers
	int nheaders = sizeof(templateGetContentHeaders)/sizeof(char *);
	
	//  set up body
	char *body = new char[strlen(templateGetContentBody)+strlen(key)+1];
	strcpy ( body, templateGetContentBody );
	strsub ( body, "guid", key );
	strsub ( body, "base64Binary", "" );
	
	//  make the request, then toss the body
	//  allocate a minimal response, it will be re-allocated by MakeSoapRequest
	char *response = new char[1];  response[0]=0;
	MakeSoapRequest(templateGetContentHeaders, nheaders, body, &response);
	delete []body;
	
	//  get the status out of the response, then toss it
	
	const char *tag1 = "<GetContentResult>";
	const char *tag2 = "</GetContentResult>";
	char *k1 = strstr(response,tag1);
	char *k2 = strstr(response,tag2);
	
	bool bStatus = false;
	if (k1 && k2)
	{
		char *start = k1 + strlen(tag1);
		if (strstr(start,"success")==start)
		{
			const char *tag3 = "<data>";
			const char *tag4 = "</data>";
			char *k3 = strstr(response,tag3);
			char *k4 = strstr(response,tag4);
			if (k3 && k4)
			{
				//  how many bytes?
				char *start = k3 + strlen(tag3);
				char *end   = strstr (start,tag4);
				int l = end-start;
				
				//  copy encoded text
				char *base64 = new char[l+1];
				strncpy ( base64, start, l );
				base64[l] = 0;
				
				//  decode text
				unsigned char *decoded = 0;
				int nb;
				decode64 ( base64, &decoded, &nb );
				delete []base64;
				
				//  copy the decoded data
				*nbytes = nb;
				unsigned char *by = new unsigned char[nb];
				*bytes = by;
				for (int k=0;k<nb;k++)
					by[k] = decoded[k];
				
				delete []decoded;

				bStatus = true;
			}
		}
	}

	delete []response;

	return bStatus;
}

bool OverlayWebService :: StoreContent ( char *key, unsigned char *bytes, int nbytes )
{
	//  count headers
	int nheaders = sizeof(templateStoreContentHeaders)/sizeof(char *);
	
	//  set up body
	char *body = new char[strlen(templateStoreContentBody)+strlen(key)+nbytes*4/3+1];
	strcpy ( body, templateStoreContentBody );
	strsub ( body, "guid", key );

	char *base64 = 0;
	encode64 ( bytes, nbytes, &base64 );
	strsub ( body, "base64Binary", base64 );
	delete []base64;

	//  make the request, then toss the body
	//  allocate a minimal response, it will be re-allocated by MakeSoapRequest
	char *response = new char[1];  response[0]=0;
	MakeSoapRequest(templateStoreContentHeaders, nheaders, body, &response);
	delete []body;
	
	//  get the status out of the response, then toss it
	
	bool bStatus = false;
	
	const char *tag1 = "<StoreContentResult>";
	const char *tag2 = "</StoreContentResult>";
	
	char *k1 = strstr(response,tag1);
	char *k2 = strstr(response,tag2);
	
	if (k1 && k2)
	{
		char *start = k1 + strlen(tag1);
		if (strstr(start,"success")==start)
			bStatus = true;
	}

	delete []response;

	return bStatus;
}


bool OverlayWebService :: DeleteKey ( char *key )
{
	//  count headers
	int nheaders = sizeof(templateDeleteKeyHeaders)/sizeof(char *);
	
	//  set up body
	char *body = new char[strlen(templateDeleteKeyBody)+strlen(key)+1];
	strcpy ( body, templateDeleteKeyBody );
	strsub ( body, (char*)"guid", key );
	//strsub ( body, "base64Binary", "" );
	
	//  make the request, then toss the body
	//  allocate a minimal response, it will be re-allocated by MakeSoapRequest
	char *response = new char[1];  response[0]=0;
	MakeSoapRequest(templateDeleteKeyHeaders, nheaders, body, &response);
	delete []body;
	
	//  get the status out of the response, then toss it
	
	bool bStatus = false;
	
	char *tag1 = (char*)"<DeleteKeyResult>";
	char *tag2 = (char*)"</DeleteKeyResult>";
	
	char *k1 = strstr(response,tag1);
	char *k2 = strstr(response,tag2);
	
	if (k1 && k2)
	{
		char *start = k1 + strlen(tag1);
		if (strstr(start,"success")==start)
			bStatus = true;
	}

	delete []response;

	return bStatus;
}

bool OverlayWebService :: ListKeys ()
{
	//  count headers
	int nheaders = sizeof(templateListKeysHeaders)/sizeof(char *);
	
	//  set up body
	char *body = new char[strlen(templateListKeysBody)+1];
	strcpy ( body, templateListKeysBody );
	strsub ( body, "message", "" );
	
	//  make the request, then toss the body
	//  allocate a minimal response, it will be re-allocated by MakeSoapRequest
	char *response = new char[1];  response[0]=0;
	MakeSoapRequest(templateListKeysHeaders, nheaders, body, &response);
	delete []body;
	
	//  get the status out of the response, then toss it
	//  TDB
	int nc = strlen(response);
	delete []response;

	return false;
}

//-----------------------------------------------------------------------------------------------------------

//
// callback routines for curllib
//

static size_t CurlWriteCallback( void * inBuffer, int inElementSize, int inElementCount, void * outString )
{
	//  outString is really a char **
	char **response = (char **)outString;
	
	//  how many chars do we already have?
	int numIn = strlen(*response);
	
	//  how many are we adding?
	int numAdd = inElementCount * inElementSize;
	
	//  make a new string large enough to hold it all
	char *newString = new char[numIn+numAdd+1];
	
	//  copy the existing part
	strcpy ( newString, *response );
	
	//  append the new part.  We assume it's a char string.
	int i;
	for( i = 0; i < numAdd; ++i )
		newString[numIn+i] = ((char *)inBuffer)[i];
	newString[numIn+numAdd] = 0;
	
	//  replace the old string with the new one
	delete [](*response);
	*response = newString;
	
	return numAdd;
}

bool OverlayWebService :: MakeSoapRequest (const char **headers, int nheaders, char *body, char **response)
{
	CURLcode theErrorCode = CURLcode( 1 );
	
	//
	// use libcurl to make a web services transaction for hte license
	//
#ifdef PLAT_MACINTOSH
#if TARGET_CPU_PPC
	CURL *theCurlHandle = curl_easy_init( );
#else
	static CURL *theCurlHandle = NULL;
	if (theCurlHandle==NULL)
		theCurlHandle = curl_easy_init( );
	else
		curl_easy_reset(theCurlHandle);
#endif
#else
	static CURL *theCurlHandle = NULL;
	if (theCurlHandle==NULL)
		theCurlHandle = curl_easy_init( );
	else
		curl_easy_reset(theCurlHandle);
#endif
	
	if(theCurlHandle)
	{
		//
		// set all the options, then perform the request
		//
		
		//
		// set the URL of the service
		//
		//ASSERT( GetServerURL().GetLength() );
		theErrorCode =  curl_easy_setopt(theCurlHandle, CURLOPT_URL, (GetServerURL()));
		if( theErrorCode == 0 )
		{
			theErrorCode =  curl_easy_setopt(theCurlHandle, CURLOPT_SSL_VERIFYPEER, false);
			if( ! theErrorCode )
			{
				//
				// this is a post with content rather than fields
				//
				theErrorCode =  curl_easy_setopt(theCurlHandle, CURLOPT_POST, 1 );
				if( ! theErrorCode )
				{
					//
					// set the HTTP post headers
					//
					struct curl_slist * theHeaderList = NULL;
					if( true )
					{
						for( int headerIndex = 0; headerIndex < nheaders; ++headerIndex )
						{
							theHeaderList = curl_slist_append( theHeaderList, (headers[headerIndex]) );
						}
						if( theHeaderList ) 
						{
							theErrorCode =  curl_easy_setopt(theCurlHandle, CURLOPT_HTTPHEADER, theHeaderList );
						}
					}
					
					if( ! theErrorCode )
					{
						//
						// set the post field to our request XML
						//
						theErrorCode =  curl_easy_setopt(theCurlHandle, CURLOPT_POSTFIELDS, body );
						if( ! theErrorCode )
						{
							//
							// set the length of the request
							//
							theErrorCode =  curl_easy_setopt(theCurlHandle, CURLOPT_POSTFIELDSIZE, strlen(body) );
							if( ! theErrorCode )
							{
								//
								// set the callback function for when data is received from the service
								//
								theErrorCode =  curl_easy_setopt(theCurlHandle, CURLOPT_WRITEFUNCTION, CurlWriteCallback );
								if ( theErrorCode == 0 )
								{
									//
									// set the buffer that gets the result XML
									//
									//strcpy(response,"");  //  done by caller
									theErrorCode =  curl_easy_setopt(theCurlHandle, CURLOPT_FILE, (void *)response );
									if( theErrorCode == 0 )
									{
										//
										// now do it
										//
										try
										{
											theErrorCode = curl_easy_perform( theCurlHandle );
											if( theErrorCode != 0 )
											{
												//
												// explain the error to the user
												//
#if LIBCURL_VERSION_NUM >= 0x070C00
												//
												// curl_easy_strerror introduced in version 7.12 
												// provide a specific message.
												//
												
												//char message[1000];
												//strcpy(message, (char *)curl_easy_strerror( theErrorCode ));
												//DoMessageBox( curl_easy_strerror( theErrorCode ), _M( S_KEY_SERVER_COMMUNICATION_ERROR_TITLE ) );
												assert(false);
#endif
											}
										}
										catch(...)
										{
											theErrorCode = CURLcode(1);
										}
									}
								}
							}
						}
					}
					
					//
					// free the header list we allocated
					//
					if( theHeaderList ) curl_slist_free_all( theHeaderList );
				}
			}
		}

#ifdef PLAT_MACINTOSH
#if TARGET_CPU_PPC
		curl_easy_cleanup( theCurlHandle );
#endif
#endif

	}
	
	
	if( theErrorCode )
	{
		//
		// this will set the server state so that we don't attempt anymore communication with the server
		// provide generic message that tells user the can continue to work with open docs, but not open any more.
		//
		//DoMessageBox( _M( S_KEY_SERVER_COMMUNICATION_ERROR ), _M( S_KEY_SERVER_COMMUNICATION_ERROR_TITLE ) );
		//SetServerError( theErrorCode );
		//char message[1000];
		//strcpy(message, (char *)curl_easy_strerror( theErrorCode ));
		assert(false);
	}
	
	return theErrorCode == 0;
	
	
}