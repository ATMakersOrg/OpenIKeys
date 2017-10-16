//
//  MenuBarController.h
//  USBMenu
//
//  Created by fred ross-perry on Wed Jun 04 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

@interface MenuBarController : NSObject {

    //  the status item that will be added to the system status bar
    NSStatusItem *statusItem;

    //  the menu attached to the status item
    IBOutlet NSMenu *theMenu;

    //  a timer which will let us check iTunes every 10 seconds
    NSTimer *mainTimer;
	
	//  last app and owner
	char lastApp[256];
	char lastOwner[256];
	
	//  a submenu
	NSMenu *subMenu;
}

//  called when mainTimer fires
-(void)timer:(NSTimer *)timer;

//  called when the icon is clicked to update the menu
-(void)updateMenu;

//  called when menu items are chosen
-(void)menuAction:(id)sender;

@end
