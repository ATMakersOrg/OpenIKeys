// LinuxHeader.h		|	Oct 28 2003 -- WYU
//
//	This is the prefix header for all linux plugins.
//	Use the gcc -include option to add this prefix when compiling.
//	Use -DWIDGET_GTK=1 when compiling plugins that use GTK.
//

#ifndef LINXUHEADER_H
#define LINUXHEADER_H

#define UNIX_ANSI 1
#define X_WINDOW 1

#ifndef NULL
	#define NULL 0
#endif

#ifndef assert
	#define assert(cond) if (!(cond));
	#define debugAssert(cond) assert(cond)
	#define assertMsg(cond, msg) assert(cond)
#endif

#include "macTypesForAnsi.h"
#include "CommonMacFunctions.h"
#include "PlatformHeaders.h"

namespace QT {
	typedef void *MovieController;
	typedef void *Movie;
};

using namespace RB;


#endif	// LINUXHEADER_H

