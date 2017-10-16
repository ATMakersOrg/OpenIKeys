/*
 *  WebService.h
 *  Test Web Service
 *
 *  Created by fred ross-perry on 6/16/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

class OverlayWebService
{
public:
	
	bool GetContent ( char *key, unsigned char **bytes, int *nbytes );
	bool StoreContent ( char *key, unsigned char *bytes, int nbytes );
	bool DeleteAll ();
	bool DataExists ( char *key );
	bool DeleteKey ( char *key );
	bool ListKeys ();

private:
	
	bool MakeSoapRequest (const char **headers, int nheaders, char *body, char **result);
	const char *GetServerURL() {return "https://cltvar.kurzweiledu.com/overlayweb/OverlayService.asmx";}

private:
};

