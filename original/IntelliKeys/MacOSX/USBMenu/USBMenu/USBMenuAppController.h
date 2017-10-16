//
//  USBMenuAppController.h
//  USBMenu
//
//  Created by Ken Rorick on 8/9/12.
//  Copyright 2012 RORICK.NET. All rights reserved.
//
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

@interface USBMenuAppController : NSObject {
    IBOutlet NSWindow *window;
    NSTimer *fadeTimer;
    NSTimer *progressTimer;
    IBOutlet NSTextField *alertText;
    IBOutlet NSProgressIndicator *progress;
}
- (void)awakeFromNib;
- (BOOL)windowShouldClose:(id)sender;
- (void)fade:(NSTimer *)theTimer;
- (void)displayOverlayAlert:(NSString *)overlayPath;
- (void)updateProgress:(NSTimer *)theTimer;
- (void)applicationDidFinishLaunching:(NSNotification *)notification;
- (void)handleOverlayChangedNotification:(NSNotification *)notification;

@end

