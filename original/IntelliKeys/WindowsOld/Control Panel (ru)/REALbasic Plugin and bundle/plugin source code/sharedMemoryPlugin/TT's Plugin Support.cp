/*
 * TT's Plugin Support.cp
 *
 * Support code for TT's Plugin Starter and other plugins:
 *
 *   Provides Startup and Termination handling for both Mac OS and Windows
 *
 * Brought to you for free by Thomas Tempelmann, http://www.tempel.org/rb/
 *
 * Updated March 19, 2003, with help from Peter Robinson: MPW support added, removed PluginInit()
 */
#if TARGET_OS_WIN32
	#define log(x) OutputDebugString(x)
#else
	#include <MacMemory.h>
	//#include "dcon.h"
	#define log(x) dprintf(x)
#endif
#if TARGET_OS_MAC and not TARGET_CARBON
	#include "QuitHandler.h"
#endif
#include "TT's Plugin Support.h"
//#include "IKUtil.h"
	// forward declarations
void InitTTsPluginSupport ();
#if TARGET_OS_WIN32
	// this is a Windows-only feature: We can get notified when our runtime environment starts and stops
	extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID fImpLoad);
	extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID fImpLoad)
	{
	    switch(fdwReason) {
	        case DLL_PROCESS_ATTACH: {
	            // A process has connected to the DLL.
	            // For this plugin, this means that the app has just started
	            
				//log ("[TT's Plugin Starter] DLL_PROCESS_ATTACH\n");
	            break;
	        }
	        case DLL_PROCESS_DETACH: {
	            // A process has released our DLL
	            // For this plugin, this means that the app is terminating
				
				//log ("[TT's Plugin Starter] DLL_PROCESS_DETACH\n");
	            break;
	        }
	    }
	    return TRUE;
	}
#endif
// To initialize the static C++ objects, we may need to call a special routine
// that the linker will generate. Here's the preparation for this:
#if defined(MPW_CPLUS) || defined(MPW_C)
	// MPW handling
	#if TARGET_OS_WIN32 || TARGET_CPU_68K
		#error MPW cannot build this Plugin Starter for Windows or 68k!
	#endif
	
	#include <MacRuntime.h>
	#define __InitCode__()  __init_lib(nil)
	#if !TARGET_CARBON
		#define __destroy_global_chain() __term_lib()
	#else
		// For Carbon, __init_lib and __term_lib aren't declared
		// in <MacRuntime.h>.  We'll respect that because the proper
		// PEF init/term routines should be called anyway.
		#define __destroy_global_chain()
		#define __init_lib(a)
	#endif
#else
	// CodeWarrior handling
	#if TARGET_OS_WIN32
		extern "C" far void _RunInit(void);
		#define __InitCode__ _RunInit
	#elif TARGET_CPU_PPC
		extern "C" void __sinit(void);
		#define __InitCode__ __sinit
	#elif TARGET_CPU_68K
		extern "C" far void __InitCode__ (void);
	#endif
	extern "C" void __destroy_global_chain (void);
#endif
static long gGlobalsInitedSig = 0;
static void quitHandler (void)
{
	//log ("[TT's Plugin Starter] quitHandler called\n");
	// dispose of all global C++ objects (calls their destructors)
	if (gGlobalsInitedSig == '<ok>') {
		gGlobalsInitedSig = 0;
		__destroy_global_chain ();
	}
}
	/*
	 * The following code shows how static con- and destructors get invoked.
	 *
	 * The result is this:
	 *   Under Windows, static constructors get called first, even before
	 * "PluginEntry" gets called. Static destructors are called at time
	 * the app terminates.
	 *   Under Mac OS, static constructors get called first, just as in
	 * Windows, but the destructors are _not_ called at program termination.
	 * However, this plugin support code installs a quit handler that takes
	 * care of the proper destruction of objects by manually calling the
	 * appropriate function.
	 */
static class globalsInitTracer
{
public:
	globalsInitTracer();
	~globalsInitTracer();
} globalsInitTracerObj;
#ifdef __MWERKS__	// --- mod PR ...
	#define kIDE			"\pCW"
#elif MPW_CPLUS
	#define kIDE			"\pMPW"
#else 
	#define kIDE			"\p??"
#endif
#if TARGET_CARBON
	#define kRuntime		"\pCarbon"
#elif TARGET_CPU_PPC
	#define kRuntime		"\pPPC"
#else
	#define kRuntime		"\p??"
#endif	// --- ... mod PR
globalsInitTracer::globalsInitTracer()
{
	//DebugStr("\pStarter: Static constructor. IDE: "kIDE", Runtime: "kRuntime);
	gGlobalsInitedSig = '<ok>';
	InitTTsPluginSupport ();
}
globalsInitTracer::~globalsInitTracer()
{
	//DebugStr("\pStarter: Static destructor. IDE: "kIDE", Runtime: "kRuntime);
	
	PluginExit ();
}
static Boolean gPluginInited = false;
void InitTTsPluginSupport ()
{	
	if (not gPluginInited) {
		gPluginInited = true;
		
		if (gGlobalsInitedSig != '<ok>') 
		{			
			// oops - RB's runtime forgot to init us - let's catch up now ourselves:
			//log ("[TT's Plugin Starter] Error: Globals did not get instantiated as expected.\n");
			__InitCode__ ();	// calls constructors for static and global C++ objects
		}
		
		#if TARGET_OS_MAC and not TARGET_CARBON
		
			// We want to get notified if the process ends that was running when
			// this plugin got initialized. Then, we can de-init the plugin again.
			// (We don't need this for Carbon and: the class destructor there
			// gets called already by other ways):
			InstallQuitHandler (quitHandler);
		
		#endif
	}
}
	
// EOF
