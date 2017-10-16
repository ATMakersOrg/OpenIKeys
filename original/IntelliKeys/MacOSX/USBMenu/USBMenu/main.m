//
//  main.m
//  USBMenu
//
//  Created by fred ross-perry on Wed Jun 04 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MenuBarController.h"
#import "USBMenuAppController.h"
#import <Security/Security.h>

extern MenuBarController *pController;
USBMenuAppController *myAppController;

@implementation USBMenuAppController

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    [[NSDistributedNotificationCenter defaultCenter] 
        addObserver:self selector:@selector(handleOverlayChangedNotification:) 
            name:@"com.cambium.usbmenu.notifications.overlayalert" object:nil];
}

-(void)handleOverlayChangedNotification:(NSNotification *)notification;
{
    // NSLog (@"0. handleOverlayChangedNotification");
    NSString *overlayMessage = [[notification userInfo] objectForKey:@"overlayMessage"];
    [self displayOverlayAlert: overlayMessage];
}

- (void)awakeFromNib
{
    // Make us window's delegate, so we receive windowShouldClose:.
    [window setDelegate:self];
    myAppController = self;
    progressTimer = nil;
}


- (BOOL)windowShouldClose:(id)sender
{
    // Set up our timer to periodically call the fade: method.
    //fadeTimer = [[NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(fade:) userInfo:nil repeats:YES] retain];
    // Don't close just yet.
    return NO;
}

- (void)updateProgress:(NSTimer *)theTimer
{
    double p = [progress doubleValue] + 10.0;
    [progress setDoubleValue: p];
    if (p >= [progress maxValue]) {
        [progressTimer invalidate];
        [progressTimer release];
        progressTimer = nil;
        // Set up our timer to periodically call the fade: method.
        // NSLog (@"2. At progress maxValue");

        fadeTimer = [[NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(fade:) userInfo:nil repeats:YES] retain];
    }
}

- (void)fade:(NSTimer *)theTimer
{
    if ([window alphaValue] > 0.0) {
        // If window is still partially opaque, reduce its opacity.
        [window setAlphaValue:[window alphaValue] - 0.05];
    } 
    else 
    {
        // Otherwise, if window is completely transparent, destroy the timer and close the window.
        [fadeTimer invalidate];
        [fadeTimer release];
        fadeTimer = nil;
        
        //[window close];
        [window orderOut:self];
        
        // Make the window fully opaque again for next time.
        [window setAlphaValue:0.0];
        [progress stopAnimation:self];
        // NSLog (@"3. progress stopAnimation/window hidden");
    }
}

-(void)displayOverlayAlert:(NSString *)overlayMessage
{
    if (progressTimer) {
        [progressTimer invalidate];
        [progressTimer release];
        progressTimer = nil;
    }
    
    [progress setIndeterminate:NO];
    [progress setMaxValue:100.0];
    [progress setMinValue:0.0];
    [progress setDoubleValue:0.0];
    
    [alertText setStringValue: overlayMessage];
    [window setAlphaValue:1.0];
    [window makeKeyAndOrderFront:self];
    [NSApp unhideWithoutActivation];
    // NSLog (@"1. NSApp unhideWithoutActivation");

    progressTimer = [[NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(updateProgress:) userInfo:nil repeats:YES] retain];
}

@end

//-------------------------------------------------------------------------
//
//  MyNSApplication is a subclass of NSApplication for the purpose
//  of overriding sendEvent.
@interface MyNSApplication : NSApplication
{
    USBMenuAppController *appController;
}

-(void)sendEvent:(NSEvent *)event;
-(void)displayOverlayAlert:(NSString *)overlayPath;

@end

@implementation MyNSApplication

-(void)sendEvent:(NSEvent *)event
{
	//  on mouse down, update the menu to reflect current circumstances.
	NSEventType type = [event type];
	if (type == NSLeftMouseDown)
		if ( pController != Nil )
			[pController updateMenu];
    
	//  now let the super handle it.
	[super sendEvent:event];
}

-(void)displayOverlayAlert:(NSString *)alertMessage
{
    [myAppController performSelectorOnMainThread:@selector(displayOverlayAlert:) withObject: alertMessage waitUntilDone:YES];
}

@end

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

void addAsGlobalLoginItem ()
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    if (!globalLoginItemExists ()) {
//        NSLog (@"Did not find login item for USBMenu.app");
        LSSharedFileListRef globalLoginItems = LSSharedFileListCreate(NULL, kLSSharedFileListGlobalLoginItems, NULL);
        
        AuthorizationRef auth = NULL;
        AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &auth);
        LSSharedFileListSetAuthorization(globalLoginItems, auth);
        
        AuthorizationItem right[1] = {{"system.global-login-items.", 0, NULL, 0}};
        AuthorizationRights setOfRights = {1, right};
            
        AuthorizationFlags authFlags = kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights;
        AuthorizationCopyRights(auth, &setOfRights, kAuthorizationEmptyEnvironment, authFlags, NULL);
        
        // This will retrieve the path for the application
        NSString * appPath = [[NSBundle mainBundle] bundlePath];
        CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:appPath];
        
        if (globalLoginItems) {
            //Insert an item to the list.
            LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(globalLoginItems, kLSSharedFileListItemLast, NULL, NULL,url, NULL, NULL);
            if (item) {
//                NSLog (@"Added login item for %@", appPath);
//                NSLog (@"Added login item for %@", item);
                CFRelease(item);
            }
        }  
        
        CFRelease(globalLoginItems);
    }
    
    [pool release];
}

void disableF12HotKeys ()
{	
    int kF12_CODE = 111;
    int updated = 0;
	
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	CFPreferencesAppSynchronize(CFSTR("com.apple.symbolichotkeys"));

    NSMutableDictionary *hotkeysMutable = nil;
	
//    NSLog(@"disableF12HotKeys: looking for F12 hot key assignments");
	NSDictionary *hotkeys = (NSDictionary *)CFPreferencesCopyAppValue(CFSTR("AppleSymbolicHotKeys"), CFSTR("com.apple.symbolichotkeys"));
    if (hotkeys) {
        hotkeysMutable = [NSMutableDictionary dictionaryWithDictionary:hotkeys];
        if (hotkeysMutable) {
            for (NSNumber *keyId in hotkeys) {
                NSMutableArray *hotKey = [hotkeys valueForKeyPath:[NSString stringWithFormat:@"%@.value.parameters", keyId]];
                int isEnabled = [[hotkeys valueForKeyPath:[NSString stringWithFormat:@"%@.enabled", keyId]] intValue];
                int keyCode = [[hotKey objectAtIndex:1] intValue];
                long modifiers = [[hotKey objectAtIndex:2] longValue];
                // disable F12 (unmodified, no shift, control, command, etc) if enabled...
                if (keyCode == kF12_CODE && isEnabled && (modifiers == 0 || modifiers == 131072)) {
//                    NSLog(@"disabled hot key %@", keyId);
                    NSMutableDictionary *newHotkey = [NSMutableDictionary dictionaryWithDictionary:[hotkeys objectForKey: keyId]];
                    [newHotkey setValue: [NSNumber numberWithBool: FALSE] forKey: @"enabled"];
                    [hotkeysMutable setObject: newHotkey forKey: keyId];
                    updated++;
                }
            }
        }
    }
    if (updated > 0) {
//        NSLog(@"disableF12HotKeys: found F12 hot key assignments");
        CFPreferencesSetAppValue (CFSTR("AppleSymbolicHotKeys"), (CFMutableDictionaryRef)hotkeysMutable, CFSTR("com.apple.symbolichotkeys"));
        CFPreferencesAppSynchronize(CFSTR("com.apple.symbolichotkeys"));
    }
    else {
//        NSLog(@"disableF12HotKeys: hot key check completed, no update required.");
    }

    [pool release];
}


//-------------------------------------------------------------------------
//
//  Main program.  Sttart here.

int main(int argc, const char *argv[])
{
    disableF12HotKeys ();
    addAsGlobalLoginItem ();
    return NSApplicationMain(argc, argv);	
}
