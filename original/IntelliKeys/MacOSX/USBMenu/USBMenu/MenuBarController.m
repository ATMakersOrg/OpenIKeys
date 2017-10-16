//
//  MenuBarController.m
//  USBMenu
//
//  Created by fred ross-perry on Wed Jun 04 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//


//  IKSupport.cpp contains some C-style functions
//  that wrap C++ code that can be called from Obj-C.

void IKMBInitialize ();
int  IKMBIsConnectedAndOn ();
void IKMBLaunchControlPanel ();
void IKMBOnTimer();
void IKMBGetLocalizedString ( char *tag, char *output );
void IKMBGetCurrentAppPath  ( char *output );
void IKMBGetCurrentAppOwner  ( char *app, char *output );
void IKMBGetCurrentAppMenuTitle  ( char *path, char *output );
void IKMBLaunchControlPanelForApp ( char *app );
void IKMBOnUpdateMenu ();
bool IKMBIsIntelliSwitchInstalled ();
void IKMBTroubleshooting();
void IKMBReloadStudent();
void IKMBLoadCurrentSettings ();
char * IKMBGetGroup ();
char * IKMBGetStudent ();
int IKMBGetMode ();
void IKMBSetMode (int mode);
void IKMBStripFileName ( char *path, bool b1, bool b2 );
char * IKMBGetToolTip ();
int IKMBCompareNoCase ( const char *s1, const char *s2 );
bool IKMBIsAppControlPanel(char *app);
bool IKMBAppIsRunning(char *app);
void IKMBSendAttachedOverlay();
char * IKMBGetSwitchSettingName (int nSetting);
char * IKMBGetOverlaySettingName ();
void IKMBSaveCurrentSettings ();
bool IKMBGetButAllowOverlays ();
void IKMBSetButAllowOverlays (bool bSet);
int IKMBgetNumSwitchSettings();
void IKMBSetSwitchSetting(int i);
void IKMBFixSwitchSetting();


//  a bunch of AppLib functions

#define TCHAR char

int		AppLibAddSystem				( TCHAR * SystemName, TCHAR * IdentifyingFileName );
int		AppLibRemoveSystem			( TCHAR * SystemName );
int		AppLibCountSystems			();
TCHAR *	AppLibGetNthSystem			( int n );
int		AppLibAddApplication		( TCHAR * AppPath );
int		AppLibRemoveApplication		( TCHAR * AppPath, TCHAR * SystemName );
int		AppLibTakeOwnership			( const TCHAR * AppPath, const TCHAR * SystemName );
int		AppLibHasOwnership			( TCHAR * AppPath, TCHAR * SystemName);
TCHAR *	AppLibGetOwner				( TCHAR * AppPath );
int		AppLibReleaseOwnership		( const TCHAR * AppPath, const TCHAR * SystemName);
int		AppLibCountApplications		();
TCHAR *	AppLibGetNthApplication		( int n );
bool	AppLibLock					( int timeout );
bool	AppLibUnlock				();
TCHAR *	AppLibGetAppFriendlyName	( TCHAR * AppPath );
int		AppLibMaintenance			();

bool	AppLibHasClickitAdaptation			( TCHAR * AppPath );
TCHAR *	AppLibGetStudentAttachedOverlay		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
int		AppLibCountAttachedOverlays			( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
int		AppLibGetSelectedOverlay			( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
TCHAR *	AppLibGetNumberedOverlay			( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, int OverlayNumber);
int		AppLibAddApplicationForStudent		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
int		AppLibSetSelectedOverlay			( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, int OverlayNumber);
int		AppLibAttachOverlayForStudent		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, TCHAR *OverlayPath );
int		AppLibDetachOverlayForStudent		( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName, TCHAR *OverlayPath );
bool	AppLibIsSystemPresent				( TCHAR * SystemName );
void	AppLibAddPreinstalledOverlays		( TCHAR * GroupName, TCHAR * StudentName );
void	AppLibFloatingMessage				( TCHAR * Message );
bool	AppLibIsAppAllowed					( TCHAR * AppPath );
void	AppLibSetAppAllowed					( TCHAR * AppPath, bool bAllowed );
int		AppLibRemoveApplicationForStudent	( TCHAR * AppPath, TCHAR * GroupName, TCHAR * StudentName );
int		AppLibCountStandardApps				( );
TCHAR *	AppLibGetNthStandardApp				( int n );
TCHAR * AppLibGetTruePath					( TCHAR * AppPath );
void	AppLibSetSystemInterest				( TCHAR * AppPath, TCHAR * SystemName, bool bInterest );
bool	AppLibHasSystemInterest				( TCHAR * AppPath, TCHAR * SystemName );
void	AppLibAddDisallowedApps				();
void	AppLibDeferSaving					( bool bDefer );
bool	AppLibIsOverlayPreinstalled			( TCHAR * OverlayPath );
int		AppLibUpdateAppPath					( TCHAR * oldPath, TCHAR * newPath );
bool	AppLibIsIntelliSwitchInstalled		( );
bool	AppLibIsIntellikeysInstalled		( );

bool	AppLibShowDiscoverAnyway			( );


//  The one and only menu bar controller.

#import "MenuBarController.h"
MenuBarController *pController = Nil;


@implementation MenuBarController

//-------------------------------------------------------------------------
//
//

-(id)init
{   
	//NSLog(@"init");

	if (self=[super init])
	{
	}

	IKMBInitialize();

	pController = self;

	return self;
}

//-------------------------------------------------------------------------
//
//

- (void)awakeFromNib
{
    mainTimer = [[NSTimer scheduledTimerWithTimeInterval:(1.0)
								target:self
								selector:@selector(timer:)
								userInfo:nil
								repeats:YES] retain];

    statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength] retain];
    [statusItem setHighlightMode:YES];
    [statusItem setImage:[NSImage imageNamed:@"redicon"]];
    [statusItem setEnabled:YES];
	
	[statusItem setMenu:theMenu];

    [mainTimer fire];
}

typedef struct
{
	int type;        //  0=ignore, 1=Launch, 2=launchfor, 3=none, 4=overlay, 5=system
	char sdata[256];
	int idata;
} menuItem;

static menuItem items[256];

//-------------------------------------------------------------------------
//
//

- (void)updateMenu
{		
	//  get current user prefs
	IKMBOnUpdateMenu();
	char *group = IKMBGetGroup();
	char *student = IKMBGetStudent();
	
	//  remove menu items from the sub-menu
	int n, i2;
	if (subMenu != nil)
	{
		n = [subMenu numberOfItems];
		for (i2=0;i2<n;i2++)
		{
			int j = n - i2 - 1;
			[subMenu removeItemAtIndex:j];
		}
	}

	//  remove other menu items
	n = [theMenu numberOfItems];
	for (i2=0;i2<n;i2++)
	{
		int j = n - i2 - 1;
		[theMenu removeItemAtIndex:j];
	}
		
	//  add current items to it
	NSMenuItem *item; 
	NSMenuItem *itemNone = NULL; 
	char title[256];
	int nItems = 0;
	
	NSMenuItem *itemToCheck = NULL;
	
	//  one for opening the panel
	items[nItems].type = 1;
	if (IKMBIsIntelliSwitchInstalled())
		IKMBGetLocalizedString ( (char*)"Systray_Menu_Switch", title );
	else
		IKMBGetLocalizedString ( (char*)"Systray_Menu", title );
    
    //NSLog(@"updateMenu - Systray_Menu - [%s]", title);
    
	item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
	[item setTarget:self];
	[item setTag:1000+nItems];
	nItems++;
	
	//  create items for the four modes
	
	item = [NSMenuItem separatorItem];  [theMenu addItem: item];  //  separator
		
	items[nItems].type = 0;
	IKMBGetLocalizedString ( (char*)"Mode:", title );

    //NSLog(@"updateMenu - Mode: - [%s]", title);

    item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
	[item setTarget:self];
	[item setTag:1000+nItems];
	nItems++;

	items[nItems].type = 8;
	strcpy(title, "   ");  
    IKMBGetLocalizedString ( (char*)"Overlays", &title[3] );
	
    //NSLog(@"updateMenu - Overlays - [%s]", title);

    item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
	[item setTarget:self];
	[item setTag:1000+nItems];
	if (IKMBGetMode()==0)
		[item setState:NSOnState];
	nItems++;

	items[nItems].type = 11;
	strcpy(title, "   ");  
    IKMBGetLocalizedString ( (char*)"One_Overlay_Only", &title[3] );
	
    //NSLog(@"updateMenu - One_Overlay_Only - [%s]", title);

    item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
	[item setTarget:self];
	[item setTag:1000+nItems];
	if (IKMBGetMode()==1)
		[item setState:NSOnState];
	nItems++;

	items[nItems].type = 9;
	strcpy(title, "   ");  
    IKMBGetLocalizedString ( (char*)"Switch_Presets", &title[3] );
	
    //NSLog(@"updateMenu - Switch_Presets - [%s]", title);

    item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
	[item setTarget:self];
	[item setTag:1000+nItems];
	if (IKMBGetMode()==2)
		[item setState:NSOnState];
	nItems++;

	if (AppLibIsSystemPresent((char*)"Discover") || AppLibShowDiscoverAnyway())  //  don't show Discover if it's not installed.
	{
		items[nItems].type = 10;
		strcpy(title, "   ");  
        IKMBGetLocalizedString ( (char*)"DiscoverName", &title[3] );

        //NSLog(@"updateMenu - DiscoverName - [%s]", title);

        item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
		[item setTarget:self];
		[item setTag:1000+nItems];
		if (IKMBGetMode()==3)
			[item setState:NSOnState];
		nItems++;
	}
		
	//  get most recent app and its owner.
	IKMBGetCurrentAppPath ( lastApp );
	IKMBGetCurrentAppOwner ( lastApp, lastOwner );

	// create items for overlay mode.
	if (IKMBGetMode()==0)
	{
		item = [NSMenuItem separatorItem];  [theMenu addItem: item];  //  separator
		
		//  determine whether to show an item for this app or not
		bool bItemForApp = true;
		if (IKMBIsAppControlPanel(lastApp))
			bItemForApp = false;
		if (!IKMBAppIsRunning(lastApp))
			bItemForApp = false;

		if (bItemForApp)
		{
			//  one for the current application
			IKMBGetCurrentAppMenuTitle ( lastApp, title );
			items[nItems].type = 2; 
            strcpy(items[nItems].sdata,lastApp);
			
            //NSLog(@"updateMenu - lastapp - [%s]", title);

            item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
			[item setTarget:self];
			[item setTag:1000+nItems];
			nItems++;
		}
		
		//  get a bunch of information about apps, overlays and systems
		int nSelected = AppLibGetSelectedOverlay ( lastApp, group, student );  // 1-based
		int nOverlays = AppLibCountAttachedOverlays ( lastApp, group, student );
		int nSys = AppLibCountSystems();
		//  lastOwner and lastApp gotten elsewhere
		int i;
		int nInterest = 0;
		for (i=0;i<nSys;i++)
		{
			char *sys = AppLibGetNthSystem(i);
			if (IKMBCompareNoCase(sys,(char*)"IntelliKeys USB")!=0)
				if (AppLibHasSystemInterest(lastApp,sys))
					nInterest++;
		}
		
		//  separate overlay choices from app choice
		if (bItemForApp && nOverlays>0)
		{
			item = [NSMenuItem separatorItem];  [theMenu addItem: item];  //  separator
		}

		if (nOverlays>0)
		{
			int i;
			for (i=0;i<nOverlays;i++)
			{
				const char *pOverlay = AppLibGetNumberedOverlay ( lastApp, group, student, i+1 );  // 1-based
				char strName[256];
				strcpy(strName,pOverlay);
				IKMBStripFileName ( strName, true, true );
				
				IKMBGetLocalizedString ( (char*)"Use_Attached_Overlay", title );
				strcat ( title, " " );
				strcat(title,"\"");
				strcat(title,strName);
				strcat(title,"\"");
				
				items[nItems].type = 4;  
                strcpy(items[nItems].sdata,pOverlay);  items[nItems].idata = i+1;
				
                //NSLog(@"updateMenu - Use_Attached_Overlay - [%s]", title);

                item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
				[item setTarget:self];
				[item setTag:1000+nItems];
				nItems++;
				
				//  check the menu item
				if (nSelected==i+1 && IKMBCompareNoCase(lastOwner, (char*)"IntelliKeys USB")==0)
						//[item setState:NSOnState];
						itemToCheck = item;
			}
		}

		if (nOverlays>0)
		{
			items[nItems].type = 3;
			IKMBGetLocalizedString ( (char*)"App_Will_Choose",title);
		
            //NSLog(@"updateMenu - App_Will_Choose - [%s]", title);

			item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
			[item setTarget:self];
			[item setTag:1000+nItems];
			itemNone = item;
			nItems++;	
		}

		//  if nobody checked yet, check none.
		if (itemToCheck==0)
			if (itemNone != 0)
				itemToCheck = itemNone;

		//  now really check an item
		if (itemToCheck != 0)
			[itemToCheck setState:NSOnState];
	}
	
    //NSLog(@"updateMenu - GetMode = %d NUmberSwitchSettings = %d", IKMBGetMode(), IKMBgetNumSwitchSettings());
    
	//  items for one overlay
	if (IKMBGetMode() == 1)
	{
		item = [NSMenuItem separatorItem];  [theMenu addItem: item];  //  separator
		
		IKMBGetLocalizedString( (char*)"Using_Overlay",title);
		strcat ( title, " " );
		strcat ( title, "\"" );
		strcat ( title, IKMBGetOverlaySettingName() );
		strcat ( title, "\"" );

        //NSLog(@"updateMenu - Using_Overlay - [%s]", title);

		item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
		[item setTarget:self];
		[item setTag:1000+nItems];
		items[nItems].type = 0;
		nItems++;
	}

	//  items for switch presets
	
	if (IKMBGetMode() == 2)
	{
        //  separator
		item = [NSMenuItem separatorItem];  
        [theMenu addItem: item];  

		IKMBGetLocalizedString( (char*)"Using_Switch_Setting",title);
		strcat ( title, " " );
		strcat ( title, "\"" );
		strcat ( title, IKMBGetSwitchSettingName(-1) );
		strcat ( title, "\"" );
		
        //NSLog(@"updateMenu - Using_Switch_Setting - [%s]", title);

		item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
		[item setTarget:self];
		[item setTag:1000+nItems];
        
		items[nItems].type = 0;
		nItems++;
		
		//  make a menu for the switch settings
		IKMBGetLocalizedString( (char*)"Choose_A_Switch_Setting",title);
        
        NSLog(@"updateMenu - Choose_A_Switch_Setting - %s", title);

		item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
		[item setTarget:self];
		[item setTag:1000+nItems];
		nItems++;
		
		NSMenu *newMenu;
		newMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:[NSString stringWithCString:title encoding:NSUTF8StringEncoding]];
		[item setSubmenu:newMenu];
		[newMenu release];
		
		//  add a menu of the switch settings
		int numSettings = IKMBgetNumSwitchSettings();
		int i;
		for (i=0;i<numSettings;i++)
		{
			items[nItems].type = 13;  
            items[nItems].idata = i+1;
			strcpy(title,IKMBGetSwitchSettingName(i+1));
            
            //NSLog(@"updateMenu - Switch_Setting_Name_%d - [%s]", (i + 1), title);
            
			item = [newMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
			[item setTarget:self];
			[item setTag:1000+nItems];
			nItems++;
		}
	}
	
	//  items for discover
	if (AppLibIsSystemPresent ( (char*)"Discover") || AppLibShowDiscoverAnyway())  //  but not if Discover is not installed.
	{
		if (IKMBGetMode() == 3)
		{
			item = [NSMenuItem separatorItem];  [theMenu addItem: item];  //  separator
			
			items[nItems].type = 12; 
			IKMBGetLocalizedString ( (char*)"Allow_Applications_To_Send_Overlays", title );
            
            //NSLog(@"updateMenu - Allow_Applications_To_Send_Overlays - [%s]", title);

			item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
			[item setTarget:self];
			[item setTag:1000+nItems];
			nItems++;
			
			if (IKMBGetButAllowOverlays())
				[item setState:NSOnState];
		}
	}

	//  item for Troubleshooting
	item = [NSMenuItem separatorItem];  [theMenu addItem: item];  //  separator
	IKMBGetLocalizedString ( (char*)"S_TROUBLESHOOTING", title );

    //NSLog(@"updateMenu - S_TROUBLESHOOTING - [%s]", title);

    items[nItems].type = 6;
	item = [theMenu addItemWithTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding] action:@selector(menuAction:) keyEquivalent:@""];
	[item setTarget:self];
	[item setTag:1000+nItems];
	nItems++;

}

//-------------------------------------------------------------------------
//
//

- (void)menuAction:(id)sender
{
	int tag = [sender tag];
	
	char *group = IKMBGetGroup();
	char *student = IKMBGetStudent();
	
    //NSLog(@"menuAction - Group = [%s] Student = [%s]", group, student);
    
	menuItem *pItem=NULL;
	if (tag>=1000)
		pItem = &(items[tag-1000]);
	
	if (pItem)
	{
        //NSLog(@"menuAction - MenuItem->type = [%d]", pItem->type);

		switch (pItem->type)
		{
			case 1:
				//  launch control panel
				IKMBLaunchControlPanel();
				break;
				
			case 2:
				//  launch control panel to the apps tab
				IKMBLaunchControlPanelForApp ( lastApp );
				break;
				
			case 3:
				//  release ownership
				AppLibReleaseOwnership(lastApp,lastOwner);
				//AppLibSetSelectedOverlay(lastApp, group, student, 0);
				IKMBSetMode(0);
				IKMBSaveCurrentSettings();
				IKMBReloadStudent();
				break;
				
			case 4:
				//  change an attached overlay
				AppLibTakeOwnership(lastApp,(char*)"IntelliKeys USB");
				AppLibSetSelectedOverlay(lastApp, group, student, pItem->idata);
				IKMBSetMode(0);
				IKMBSaveCurrentSettings();
				IKMBReloadStudent();
				
				//  on Mac, we need to send it now.
				IKMBSendAttachedOverlay();
				break;
				
			case 5:
				//  change ownership to a different system
				AppLibSetSelectedOverlay(lastApp, group, student, 0);
				AppLibTakeOwnership(lastApp,pItem->sdata);
				IKMBSetMode(0);
				IKMBSaveCurrentSettings();
				IKMBReloadStudent();
				break;
				
			case 6:
				//  launch control panel to the troubleshooting tab
				IKMBTroubleshooting();
				break;

			case 8:
				//  
				IKMBSetMode(0);
				IKMBSaveCurrentSettings();
				IKMBReloadStudent();
				break;
				
			case 9:
				//  
				IKMBSetMode(2);
				IKMBFixSwitchSetting();
				IKMBSaveCurrentSettings();
				IKMBReloadStudent();
				break;
				
			case 10:
				//  
				IKMBSetMode(3);
				IKMBSaveCurrentSettings();
				IKMBReloadStudent();
				break;
				
			case 11:
				//  
				IKMBSetMode(1);
				IKMBSaveCurrentSettings();
				IKMBReloadStudent();
				break;
				
			case 12:
				//  
				IKMBSetButAllowOverlays(!IKMBGetButAllowOverlays());
				IKMBSaveCurrentSettings();
				IKMBReloadStudent();
				break;

			case 13:
				//  
				IKMBSetSwitchSetting(pItem->idata);
				IKMBSaveCurrentSettings();
				IKMBReloadStudent();
				break;
		}
	}
	
}


//-------------------------------------------------------------------------
//
//

-(void)timer:(NSTimer *)timer
{   
   if (IKMBIsConnectedAndOn () ==1)
   {
        //  IK is connected and on
        [statusItem setImage:[NSImage imageNamed:@"blueicon"]];
   }
   else
   {
        //  IK is NOT connected and on
        [statusItem setImage:[NSImage imageNamed:@"redicon"]];
   }
   
   IKMBOnTimer();
   
   char *toolTip = IKMBGetToolTip();
   [statusItem setToolTip:[NSString stringWithCString:toolTip encoding:NSUTF8StringEncoding]];
}



//-------------------------------------------------------------------------
//
//

-(void)dealloc
{
    [statusItem release];
    [mainTimer invalidate];
    [mainTimer release];

    [super dealloc];
}

@end

