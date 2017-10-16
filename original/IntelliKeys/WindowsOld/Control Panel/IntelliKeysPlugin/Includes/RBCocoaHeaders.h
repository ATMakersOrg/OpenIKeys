#define COCOA 1
//#define FLAT_C_PLUGIN_HEADERS 1
#define ALLOW_CARBON_IN_COCOA 1

#ifdef RB_COCOA_PREFIX
	#define RB_COCOA_CLASS(x) RB_COCOA_PREFIX ## x
#else
	#define RB_COCOA_CLASS(x) x
#endif

#define TARGET_CARBON ALLOW_CARBON_IN_COCOA

#ifdef __OBJC__
    #import <Cocoa/Cocoa.h>
#else
	typedef void NSWindow;
	typedef void NSView;
	typedef void *id;
#endif

#if ALLOW_CARBON_IN_COCOA
	#import <Carbon/Carbon.h>
#endif

#import <CoreServices/CoreServices.h>
#import <QuickTime/QuickTime.h>

#include "rb_plugin.h"