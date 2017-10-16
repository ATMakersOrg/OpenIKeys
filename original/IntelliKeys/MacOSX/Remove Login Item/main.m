#include <CoreFoundation/CoreFoundation.h>
#import <Cocoa/Cocoa.h>
#import <Security/Security.h>

static char *path = "/Applications/IntelliTools/IntelliKeys USB/Private/USBMenu.app";

boolean_t globalLoginItemExists ()
{
    boolean_t found = NO;  
    UInt32 seedValue;
    CFURLRef thePath = NULL;
    
    LSSharedFileListRef globalLoginItems = LSSharedFileListCreate(NULL, kLSSharedFileListGlobalLoginItems, NULL);
    
    // We're going to grab the contents of the shared file list (LSSharedFileListItemRef objects)
    // and pop it in an array so we can iterate through it to find our item.
    CFArrayRef loginItemsArray = LSSharedFileListCopySnapshot(globalLoginItems, &seedValue);
    for (id item in (NSArray *)loginItemsArray) {    
        LSSharedFileListItemRef itemRef = (LSSharedFileListItemRef)item;
        if (LSSharedFileListItemResolve(itemRef, 0, (CFURLRef*) &thePath, NULL) == noErr) {
//            NSLog (@"Found login item: %@", itemRef);
//            NSLog (@"Found login item path is: %@", [(NSURL *)thePath path]);
            if ([[(NSURL *)thePath path] hasSuffix: @"USBMenu.app"]) {
                found = YES;
                break;
            }
            // Docs for LSSharedFileListItemResolve say we're responsible
            // for releasing the CFURLRef that is returned
            if (thePath != NULL) CFRelease(thePath);
        }
    }
    if (loginItemsArray != NULL) CFRelease(loginItemsArray);
    
    return found;
}

void removeUSBMenuGlobalLoginItem ()
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    if (globalLoginItemExists ()) {
//        NSLog (@"Found login item for USBMenu.app");
        LSSharedFileListRef globalLoginItems = LSSharedFileListCreate(NULL, kLSSharedFileListGlobalLoginItems, NULL);
        
        AuthorizationRef auth = NULL;
        AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &auth);
        LSSharedFileListSetAuthorization(globalLoginItems, auth);
        
        AuthorizationItem right[1] = {{"system.global-login-items.", 0, NULL, 0}};
        AuthorizationRights setOfRights = {1, right};
        
        AuthorizationFlags authFlags = kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights;
        AuthorizationCopyRights(auth, &setOfRights, kAuthorizationEmptyEnvironment, authFlags, NULL);
        
        // This will retrieve the path for the application
        CFURLRef url = (CFURLRef)[NSURL fileURLWithPath: @"/Applications/IntelliTools/IntelliKeys USB/Private/USBMenu.app"];
        
        if (globalLoginItems) {
            UInt32 seedValue;
            CFURLRef thePath = NULL;
            CFArrayRef loginItemsArray = LSSharedFileListCopySnapshot(globalLoginItems, &seedValue);
            for (id item in (NSArray *)loginItemsArray) {    
                LSSharedFileListItemRef itemRef = (LSSharedFileListItemRef)item;
                if (LSSharedFileListItemResolve(itemRef, 0, (CFURLRef*) &thePath, NULL) == noErr) {
//                    NSLog (@"Found login item: %@", itemRef);
//                    NSLog (@"Found login item path is: %@", [(NSURL *)thePath path]);
                    if ([[(NSURL *)thePath path] hasSuffix: @"USBMenu.app"]) {
//                        NSLog (@"Removed USBMenu login item: %@", itemRef);
                        LSSharedFileListItemRemove(globalLoginItems, itemRef); // Deleting the item
                        break;
                    }
                    // Docs for LSSharedFileListItemResolve say we're responsible
                    // for releasing the CFURLRef that is returned
                    if (thePath != NULL) CFRelease(thePath);
                }
            }
            if (loginItemsArray != NULL) CFRelease(loginItemsArray);
            CFRelease(globalLoginItems);
        }  
    }
    
    [pool release];
}

int main (int argc, const char * argv[]) 
{
    removeUSBMenuGlobalLoginItem ();				
	return 0;
}
