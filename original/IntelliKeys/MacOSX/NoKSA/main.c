#include <CoreFoundation/CoreFoundation.h>

int main (int argc, const char * argv[]) 
{
	//  name the preference file and key
	CFStringRef appID = CFSTR("com.apple.keyboardtype");
	CFStringRef key = CFSTR("keyboardtype");
	
	//  get the dictionary
	CFMutableDictionaryRef dict = CFPreferencesCopyValue ( key, appID,  
													kCFPreferencesAnyUser, 
													kCFPreferencesCurrentHost);
	//  create a new dictionary if there was not one before.
	if (!dict)
		dict = CFDictionaryCreateMutable(NULL,2,NULL,NULL);
	
	//  add our two entries
	int forty = 40;
	CFDictionaryAddValue ( dict, CFSTR("257-2398-0"),  CFNumberCreate(NULL,kCFNumberLongType,&forty));
	CFDictionaryAddValue ( dict, CFSTR("8-5195-0"),  CFNumberCreate(NULL,kCFNumberLongType,&forty));
	
	//  set the dictionary
	CFPreferencesSetValue ( key, dict, appID,
							kCFPreferencesAnyUser, 
							kCFPreferencesCurrentHost);

	// Write out the preference data.
	CFPreferencesSynchronize(appID,
							 kCFPreferencesAnyUser, 
							 kCFPreferencesCurrentHost);
	
	return 0;
}
