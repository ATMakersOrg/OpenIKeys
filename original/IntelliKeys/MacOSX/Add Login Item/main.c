#include <CoreFoundation/CoreFoundation.h>

#include "LoginItemAPI.h"

static char *path = "/Applications/IntelliTools/IntelliKeys USB/Private/USBMenu.app";

int main (int argc, const char * argv[]) 
{
	//  first see if it's already there
	int count,i;
	bool bPresent = false;
	
	count = GetCountOfLoginItems(kAllUsers);
	for (i=0;i<count;i++)
	{
		char *val = ReturnLoginItemPropertyAtIndex ( kAllUsers, kFullPathInfo, i );
		CFStringRef a = CFStringCreateWithCString(NULL,val ,kCFStringEncodingASCII);
		CFStringRef b = CFStringCreateWithCString(NULL,path,kCFStringEncodingASCII);
		if (CFStringCompare(a,b,kCFCompareCaseInsensitive)==kCFCompareEqualTo)
			bPresent = true;
	}
	
	//  if it's not there, add it.
	if (!bPresent)
		AddLoginItemWithPropertiesToUser ( kAllUsers, path, false );
			
	return 0;
}
