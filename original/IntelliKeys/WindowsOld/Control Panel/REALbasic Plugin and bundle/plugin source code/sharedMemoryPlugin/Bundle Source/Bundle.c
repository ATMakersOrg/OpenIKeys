/*
 *  Bundle.c
 *  by Josef W. Wankerl
 *  12/04/00
 */
#include <MSL MacHeadersMach-O.h>
#include <Carbon/Carbon.h>
#include "sharedMemory.h"
enum
{
	kAlertID = 128
};
#pragma export on
void SayHello(void)
{
	short theRefNum;
	CFBundleRef theBundle;
	
	theBundle = CFBundleGetBundleWithIdentifier(CFSTR("com.IntelliTools.USB.ControlPanel"));
	
	if (theBundle != NULL)
	{
		theRefNum = CFBundleOpenBundleResourceMap(theBundle);
		
		if (theRefNum != 0)
		{
			
			CFBundleCloseBundleResourceMap(theBundle, theRefNum);
		}
	}
}
#pragma export off