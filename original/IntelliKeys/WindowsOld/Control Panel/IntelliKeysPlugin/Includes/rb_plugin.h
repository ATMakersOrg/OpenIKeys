// rb_plugin.h
//
// This file is part of the REALbasic plugin API.  Include this file
// at the top of your plugin source files.
//
// © 1997-2002 REAL Software Inc. -- All Rights Reserved
// See file "Plug-in License SDK.txt" for details.

#ifndef RB_PLUGIN_H
#define RB_PLUGIN_H

#if defined(MPW_CPLUS) || defined(MPW_C)
	#include <Types.h>
	#include <MacTypes.h>
	#include <QDOffscreen.h>
	#include <Events.h>
	#include <Files.h>
	#include <AppleEvents.h>
	#include <Controls.h>
	#include <Movies.h>
	#include <Appearance.h>
#endif // MPW

#if !WIN32 || !defined( __cplusplus )
	#define QT_NAMESPACE
#else
	#define QT_NAMESPACE QT::
#endif

#include "REALplugin.h"

// This will be used in stage 2 of phasing out the 
// deprecated functionality.  It's not used now because
// it would require users to have to modify existing 
// projects so they compile.
//#include "RBDeprecatedPluginSDK.h"

// picture types and description
enum {
	pictureUnknown,
	pictureMacintoshPICT,
	pictureMacintoshCICN,
	pictureMacintoshIconSuite,
	pictureMacintoshGWorld,
	pictureWin32DIB,		// a HDIB, use GlobalLock on it
	pictureGdkPixmap,
	pictureWin32Bitmap,		// a HBITMAP, but really a DIBSECTION, use GetObject on it
	pictureCGBitmapContext,
	pictureGDPtr,			// Console Only: Pointer to a copy of GD data, which is 11 bytes meta data then pixels follow in ARGB format
	pictureGDImagePtr,		// Console Only: Pointer to gdImageStruct
	pictureGDIPlusBitmap,	// a GpBitmap handle
	pictureCairoContext		// a cairo_t ptr
};

// Common text encoding values
const unsigned long kREALTextEncodingUnknown = 0xFFFF;
const unsigned long kREALTextEncodingASCII = 0x0600;
const unsigned long kREALTextEncodingUTF8 = 0x08000100;
const unsigned long kREALTextEncodingUTF16 = 0x0100;
const unsigned long kREALTextEncodingUTF32 = 0x0c000100;

struct REALpictureDescription
{
	long pictureType;
	void *pictureData;
	long width, height, depth;			// Plugin authors can now get depth and transparency info (5.0)
	Boolean transparent;
};
typedef struct REALpictureDescription REALpictureDescription;

// Sound types and description
enum {
	soundUnknown,
	soundMacintoshSnd
};

struct REALsoundDescription
{
	long soundType;
	void *soundData;
};
typedef struct REALsoundDescription REALsoundDescription;

// handy macros
#define ControlData(defn, instance, typeName, data) typeName *data = (typeName *) REALGetControlData(instance, &defn)
#define ClassData(defn, instance, typeName, data) typeName *data = (typeName *) REALGetClassData(instance, &defn)
#ifndef _countof
	#define _countof( x )	((x)?(sizeof( x ) / sizeof( x[ 0 ] )):0)
#endif

// pixel types
enum RBPixelType
{
	kRBPixelRGB24 = 1,			// 3 bytes/pixel: Red, Green, Blue
	kRBPixelBGR24,				// 3 bytes/pixel: Blue, Green, Red
	kRBPixelXRGB32,				// 4 bytes/pixel: Unused, Red, Green, Blue
	kRBPixelBGRX32				// 4 bytes/pixel: Blue, Green, Red, Unused
};
typedef enum RBPixelType RBPixelType;

struct REALwindowStruct;
typedef struct REALwindowStruct *REALwindow;

#if FLAT_C_PLUGIN_HEADERS && defined( __cplusplus )
	extern "C" {
#endif

typedef void (*BackgroundTaskProc)(void *data);
long REALRegisterBackgroundTask( BackgroundTaskProc proc, unsigned long period, void *data );
void REALUnregisterBackgroundTask( long id );

Boolean REALinRuntime(void);

void REALRegisterControl(REALcontrol *defn);

void REALRegisterDBEngine(REALdbEngineDefinition *defn);

void REALRegisterDBTable(REALdbTableDefinition *defn);

void REALRegisterDBCursor(REALdbCursorDefinition *defn);

void REALRegisterClass(REALclassDefinition *defn);

REALstring REALDefaultControlFont(void);

unsigned long REALDefaultControlFontSize(void);

REALstring REALBuildString(const char *contents, int length);

#if FLAT_C_PLUGIN_HEADERS
	REALstring REALBuildStringWithEncoding( const char *contents, int byteCount, unsigned long encoding );
#else
	REALstring REALBuildString( const char *contents, int byteCount, unsigned long encoding );
#endif

void REALLockObject(REALobject obj);

void REALUnlockObject(REALobject obj);

void REALLockString(REALstring str);

void REALUnlockString(REALstring str);

void *REALGetStringContents( REALstring str, size_t *numBytes );

#if (TARGET_68K || TARGET_PPC || TARGET_CARBON)
REALpicture REALBuildPictureFromPicHandle(PicHandle pic, Boolean bPassOwnership);
#endif

REALpicture REALBuildPictureFromGWorld(void *world, Boolean bPassOwnership);

REALpicture REALBuildPictureFromPictureDescription(REALpictureDescription *description, Boolean bPassOwnership);

void REALUnlockPictureDescription(REALpicture pic);

void REALLockSoundDescription(REALsound sound, REALsoundDescription *description);

void REALUnlockSoundDescription(REALsound sound);

REALdbCursor REALdbCursorFromDBCursor(dbCursor *cursor, REALdbCursorDefinition *defn);

REALdbDatabase REALdbDatabaseFromDBDatabase(dbDatabase *database, REALdbEngineDefinition *defn);

void *REALGetEventInstance(REALcontrolInstance instance, REALevent *event);

void *REALGetControlData(REALcontrolInstance instance, REALcontrol *defn);

void *REALGetClassData(REALobject instance, REALclassDefinition *defn);

#if (TARGET_68K || TARGET_PPC || TARGET_CARBON) || COCOA
void REALSelectGraphics(REALgraphics context);
#endif

#if (TARGET_68K || TARGET_PPC || TARGET_CARBON) || COCOA
void REALGraphicsDrawOffscreenMacControl(REALgraphics context, ControlHandle mh);
#endif

#if (TARGET_68K || TARGET_PPC || TARGET_CARBON) && !TARGET_COCOA
REALsound REALBuildSoundFromHandle(Handle sound, Boolean bPassOwnership);
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON
REALappleEvent REALBuildAppleEvent(const AppleEvent *event, Boolean bPassOwnership);
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON
REALappleEvent REALBuildAEDescList(const AppleEvent *event, Boolean bPassOwnership);
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON
REALappleEvent REALBuildAEObjSpecifier(const AppleEvent *event, Boolean bPassOwnership);
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON
AppleEvent *REALAccessAppleEvent(REALappleEvent event);
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON
AppleEvent *REALAccessAppleEventReply(REALappleEvent event);
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON || TARGET_COCOA
REALmovie REALbuildMovie(QT_NAMESPACE Movie movie, int resRefNum, int bNew);
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON || TARGET_COCOA
void REALmarkMovieDirty(REALmovie movie);
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || TARGET_COCOA
int REALenterMovies(void);
#endif

void REALRegisterDataSourceInterface(const char *szMenuName, REALDataSourceInterfaceProc proc);

void REALRegisterDataSource(const char *szDatasourceName, REALDataSourceProc proc);

void REALDesignAddDataSource(const char *baseName, const char *szDataSourceName, Ptr data, int dataLen);

void REALRegisterDatabaseConnection(REALDatabaseConnectionDefinition *defn);

#if TARGET_WIN32
REALpicture REALBuildPictureFromDIB(HANDLE hDIB, Boolean bPassOwnership);
#endif

double REALGetRBVersion(void);

void REALRaiseException(REALobject exception);

int REALGetArrayUBound(void*array);

void REALGetArrayStructure( REALarray array, int index, void *structure );

Boolean REALGetVariantStructure( REALobject variant, void *buffer, unsigned long length );

void REALYieldToRB(void);

REALclassRef REALGetClassRef(const char *className);

#if FLAT_C_PLUGIN_HEADERS
	REALobject REALnewInstanceWithClass( REALclassRef classRef );
#else
REALobject REALnewInstance(REALclassRef classRef);
#endif

void REALRegisterInterface(REALinterfaceDefinition *defn);

REALstring REALGetDBHost(REALdbDatabase db);

REALstring REALGetDBDatabaseName(REALdbDatabase db);

REALstring REALGetDBPassword(REALdbDatabase db);

REALstring REALGetDBUserName(REALdbDatabase db);

dbDatabase *REALGetDBFromREALdbDatabase(REALdbDatabase db);

void REALConstructDBDatabase(REALdbDatabase db, dbDatabase *mydb, REALdbEngineDefinition *engine);

#if TARGET_PPC || TARGET_CARBON || TARGET_COCOA
Boolean REALFSRefFromFolderItem(REALfolderItem f, FSRef*outRef, HFSUniStr255*outName);
#endif

#if TARGET_PPC || TARGET_CARBON || TARGET_COCOA
	#if FLAT_C_PLUGIN_HEADERS
		REALfolderItem REALFolderItemFromParentFSRef(const FSRef *parent, const HFSUniStr255 *fileName);
	#else
REALfolderItem REALFolderItemFromParentFSRef(const FSRef& parent, const HFSUniStr255& fileName);
#endif
#endif

REALDBConnectionDialogRef REALDBConnectionDialogCreate(void);

void REALDBConnectionDialogAddField(REALDBConnectionDialogRef dialogRef, REALstring label, REALstring defaultText, Boolean maskField);

REALstring REALDBConnectionDialogShow(REALDBConnectionDialogRef dialogRef, REALstring title);

void REALDBConnectionDialogDelete(REALDBConnectionDialogRef dialogRef);

#if TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || X_WINDOW || TARGET_COCOA
REALpicture REALBuildPictureFromBuffer(long width, long height, RBPixelType pixelType, void*buffer, long rowBytes);
#endif

#if TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || X_WINDOW || TARGET_COCOA
Boolean REALInDebugMode(void);
#endif

#if TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || X_WINDOW || TARGET_COCOA
void REALStripAmpersands(REALstring*  ioString);
#endif

REALobject REALGetProjectFolder(void);

void REALRegisterModule(REALmoduleDefinition*defn);

void *REALLoadFrameworkMethod( const char *prototype );

void *REALLoadObjectMethod(REALobject object, const char *prototype);

#if FLAT_C_PLUGIN_HEADERS
	Boolean REALGetPropValueInt64(REALobject object, const char *propName, RBInt64 *value);
	Boolean REALGetPropValueUInt64(REALobject object, const char *propName, unsigned RBInt64 *value);
	Boolean REALGetPropValueInt32(REALobject object, const char *propName, long *outValue);
	Boolean REALGetPropValueUInt32(REALobject object, const char *propName, unsigned long *value);
	Boolean REALGetPropValueInt16(REALobject object, const char *propName, short *value);
	Boolean REALGetPropValueUInt16(REALobject object, const char *propName, unsigned short *value);
	Boolean REALGetPropValueInt8(REALobject object, const char *propName, char *value);
	Boolean REALGetPropValueUInt8(REALobject object, const char *propName, unsigned char *value);
	Boolean REALGetPropValueDouble(REALobject object, const char *propName, double *outValue);
	Boolean REALGetPropValueSingle(REALobject object, const char *propName, float *value);
	Boolean REALGetPropValueString(REALobject object, const char *propName, REALstring *outValue);
	Boolean REALGetPropValueObject(REALobject object, const char *propName, REALobject *outValue);
	Boolean REALGetPropValuePtr( REALobject object, const char *propName, void **outValue );
	Boolean REALGetPropValueCString( REALobject object, const char *propName, const char **value );
	Boolean REALGetPropValueWString( REALobject object, const char *propName, const wchar_t **value );
	Boolean REALGetPropValuePString( REALobject object, const char *propName, const unsigned char **value );
	#if TARGET_PPC || TARGET_CARBON || TARGET_COCOA
		Boolean REALGetPropValueCFStringRef( REALobject object, const char *propName, CFStringRef *value );
	#endif
#else
	Boolean REALGetPropValue(REALobject object, const char *propName, long *outValue);
	Boolean REALGetPropValue(REALobject object, const char *propName, REALstring *outValue);
	Boolean REALGetPropValue(REALobject object, const char *propName, double *outValue);
	Boolean REALGetPropValue(REALobject object, const char *propName, REALobject *outValue);
	Boolean REALGetPropValue(REALobject object, const char *propName, RBInt64 *value);
	Boolean REALGetPropValue(REALobject object, const char *propName, unsigned RBInt64 *value);
	Boolean REALGetPropValue(REALobject object, const char *propName, unsigned long *value);
	Boolean REALGetPropValue(REALobject object, const char *propName, unsigned short *value);
	Boolean REALGetPropValue(REALobject object, const char *propName, short *value);
	Boolean REALGetPropValue(REALobject object, const char *propName, unsigned char *value);
	Boolean REALGetPropValue(REALobject object, const char *propName, char *outValue);
	Boolean REALGetPropValue(REALobject object, const char *propName, float *value);

	Boolean REALGetPropValue( REALobject object, const char *propName, void **outValue );
	Boolean REALGetPropValue( REALobject object, const char *propName, const char **value );
	Boolean REALGetPropValue( REALobject object, const char *propName, const wchar_t **value );
	Boolean REALGetPropValue( REALobject object, const char *propName, const unsigned char **value );
	#if TARGET_PPC || TARGET_CARBON || TARGET_COCOA
		Boolean REALGetPropValue( REALobject object, const char *propName, CFStringRef *value );
	#endif

#endif

void REALSetDBIsConnected(REALdbDatabase database, Boolean connected);

REALobject REALNewVariantString(REALstring value);

REALobject REALNewVariantInteger(long value);

REALobject REALNewVariantDouble(double value);

REALobject REALNewVariantBoolean(Boolean value);

REALobject REALNewVariantColor(long value);

REALobject REALNewVariantStructure( const void *data, unsigned long len );

REALobject REALNewVariantPtr( void *value );
REALobject REALNewVariantCString( const char *value );
REALobject REALNewVariantWString( const wchar_t *value );
REALobject REALNewVariantPString( const unsigned char *value );
REALobject REALNewVariantOSType( unsigned long value );
#if TARGET_PPC || TARGET_CARBON || TARGET_COCOA
	REALobject REALNewVariantCFStringRef( CFStringRef value );
#endif


REALarray REALCreateArray( REALArrayType type, long bounds );

#if FLAT_C_PLUGIN_HEADERS
	Boolean REALSetPropValueInt32(REALobject object, const char *propName, long value);
	Boolean REALSetPropValueString(REALobject object, const char *propName, REALstring value);
	Boolean REALSetPropValueDouble(REALobject object, const char *propName, double value);
	Boolean REALSetPropValueObject(REALobject object, const char *propName, REALobject value);
	Boolean REALSetPropValueBoolean(REALobject object, const char *propName, Boolean value);
	Boolean REALSetPropValueUInt32(REALobject object, const char *propName, unsigned long value);
	Boolean REALSetPropValueUInt64(REALobject object, const char *propName, unsigned RBInt64 value);
	Boolean REALSetPropValueInt64(REALobject object, const char *propName, RBInt64 value);
	Boolean REALSetPropValueInt16(REALobject object, const char *propName, short value);
	Boolean REALSetPropValueUInt16(REALobject object, const char *propName, unsigned short value);
	Boolean REALSetPropValueInt8(REALobject object, const char *propName, char value);
	Boolean REALSetPropValueUInt8(REALobject object, const char *propName, unsigned char value);
	Boolean REALSetPropValueSingle(REALobject object, const char *propName, float value);
	Boolean REALSetPropValuePtr( REALobject object, const char *propName, void *value );
	Boolean REALSetPropValueCString( REALobject object, const char *propName, const char *value );
	Boolean REALSetPropValueWString( REALobject object, const char *propName, const wchar_t *value );
	Boolean REALSetPropValuePString( REALobject object, const char *propName, const unsigned char *value );
	#if TARGET_PPC || TARGET_CARBON || TARGET_COCOA
		Boolean REALSetPropValueCFStringRef( REALobject object, const char *propName, CFStringRef value );
	#endif

	void REALInsertArrayValueInt64( REALarray arr, long index, RBInt64 value );
	void REALInsertArrayValueInt32( REALarray arr, long index, long value );
	void REALInsertArrayValueInt16( REALarray arr, long index, short value );
	void REALInsertArrayValueInt8( REALarray arr, long index, char value );
	void REALInsertArrayValueUInt64( REALarray arr, long index, unsigned RBInt64 value );
	void REALInsertArrayValueUInt32( REALarray arr, long index, unsigned long value );
	void REALInsertArrayValueUInt16( REALarray arr, long index, unsigned short value );
	void REALInsertArrayValueUInt8( REALarray arr, long index, unsigned char value );
	void REALInsertArrayValueSingle( REALarray arr, long index, float value );
	void REALInsertArrayValueDouble( REALarray arr, long index, double value );
	void REALInsertArrayValueBoolean( REALarray arr, long index, Boolean value );
	void REALInsertArrayValueObject( REALarray arr, long index, REALobject value );
	void REALInsertArrayValueString( REALarray arr, long index, REALstring value );

	void REALGetArrayValueInt64( REALarray arr, long index, RBInt64 *value );
	void REALGetArrayValueInt32( REALarray arr, long index, long *value );
	void REALGetArrayValueInt16( REALarray arr, long index, short *value );
	void REALGetArrayValueInt8( REALarray arr, long index, char *value );
	void REALGetArrayValueUInt64( REALarray arr, long index, unsigned RBInt64 *value );
	void REALGetArrayValueUInt32( REALarray arr, long index, unsigned long *value );
	void REALGetArrayValueUInt16( REALarray arr, long index, unsigned short *value );
	void REALGetArrayValueUInt8( REALarray arr, long index, unsigned char *value );
	void REALGetArrayValueSingle( REALarray arr, long index, float *value );
	void REALGetArrayValueDouble( REALarray arr, long index, double *value );
	void REALGetArrayValueBoolean( REALarray arr, long index, Boolean *value );
	void REALGetArrayValueObject( REALarray arr, long index, REALobject *value );
	void REALGetArrayValueString( REALarray arr, long index, REALstring *value );

	void REALSetArrayValueInt64( REALarray arr, long index, RBInt64 value );
	void REALSetArrayValueInt32( REALarray arr, long index, long value );
	void REALSetArrayValueInt16( REALarray arr, long index, short value );
	void REALSetArrayValueInt8( REALarray arr, long index, char value );
	void REALSetArrayValueUInt64( REALarray arr, long index, unsigned RBInt64 value );
	void REALSetArrayValueUInt32( REALarray arr, long index, unsigned long value );
	void REALSetArrayValueUInt16( REALarray arr, long index, unsigned short value );
	void REALSetArrayValueUInt8( REALarray arr, long index, unsigned char value );
	void REALSetArrayValueSingle( REALarray arr, long index, float value );
	void REALSetArrayValueDouble( REALarray arr, long index, double value );
	void REALSetArrayValueBoolean( REALarray arr, long index, Boolean value );
	void REALSetArrayValueObject( REALarray arr, long index, REALobject value );
	void REALSetArrayValueString( REALarray arr, long index, REALstring value );

#else
	Boolean REALSetPropValue(REALobject object, const char *propName, long value);
	Boolean REALSetPropValue(REALobject object, const char *propName, REALstring value);
	Boolean REALSetPropValue(REALobject object, const char *propName, double value);
	Boolean REALSetPropValue(REALobject object, const char *propName, REALobject value);
	Boolean REALSetPropValue(REALobject object, const char *propName, unsigned long value);
	Boolean REALSetPropValue(REALobject object, const char *propName, unsigned RBInt64 value);
	Boolean REALSetPropValue(REALobject object, const char *propName, RBInt64 value);
	Boolean REALSetPropValue(REALobject object, const char *propName, short value);
	Boolean REALSetPropValue(REALobject object, const char *propName, unsigned short value);
	Boolean REALSetPropValue(REALobject object, const char *propName, Boolean value);
	Boolean REALSetPropValue(REALobject object, const char *propName, char value);
	Boolean REALSetPropValueUInt8(REALobject object, const char *propName, unsigned char value);
	Boolean REALSetPropValue(REALobject object, const char *propName, float value);
	Boolean REALSetPropValue(REALobject object, const char *propName, const char *value);
	Boolean REALSetPropValue(REALobject object, const char *propName, const wchar_t *value);
	Boolean REALSetPropValue(REALobject object, const char *propName, const unsigned char *value);
	#if TARGET_PPC || TARGET_CARBON || TARGET_COCOA
		Boolean REALSetPropValue(REALobject object, const char *propName, CFStringRef value);
	#endif

	void REALInsertArrayValue( REALarray arr, long index, RBInt64 value );
	void REALInsertArrayValue( REALarray arr, long index, long value );
	void REALInsertArrayValue( REALarray arr, long index, short value );
	void REALInsertArrayValue( REALarray arr, long index, char value );
	void REALInsertArrayValue( REALarray arr, long index, unsigned RBInt64 value );
	void REALInsertArrayValue( REALarray arr, long index, unsigned long value );
	void REALInsertArrayValue( REALarray arr, long index, unsigned short value );
	void REALInsertArrayValueUInt8( REALarray arr, long index, unsigned char value );
	void REALInsertArrayValue( REALarray arr, long index, float value );
	void REALInsertArrayValue( REALarray arr, long index, double value );
	void REALInsertArrayValue( REALarray arr, long index, Boolean value );
	void REALInsertArrayValue( REALarray arr, long index, REALobject value );
	void REALInsertArrayValue( REALarray arr, long index, REALstring value );

	void REALGetArrayValue( REALarray arr, long index, RBInt64 *value );
	void REALGetArrayValue( REALarray arr, long index, long *value );
	void REALGetArrayValue( REALarray arr, long index, short *value );
	void REALGetArrayValue( REALarray arr, long index, char *value );
	void REALGetArrayValue( REALarray arr, long index, unsigned RBInt64 *value );
	void REALGetArrayValue( REALarray arr, long index, unsigned long *value );
	void REALGetArrayValue( REALarray arr, long index, unsigned short *value );
	void REALGetArrayValueUInt8( REALarray arr, long index, unsigned char *value );
	void REALGetArrayValue( REALarray arr, long index, float *value );
	void REALGetArrayValue( REALarray arr, long index, double *value );
	void REALGetArrayValue( REALarray arr, long index, Boolean *value );
	void REALGetArrayValue( REALarray arr, long index, REALobject *value );
	void REALGetArrayValue( REALarray arr, long index, REALstring *value );

	void REALSetArrayValue( REALarray arr, long index, RBInt64 value );
	void REALSetArrayValue( REALarray arr, long index, long value );
	void REALSetArrayValue( REALarray arr, long index, short value );
	void REALSetArrayValue( REALarray arr, long index, char value );
	void REALSetArrayValue( REALarray arr, long index, unsigned RBInt64 value );
	void REALSetArrayValue( REALarray arr, long index, unsigned long value );
	void REALSetArrayValue( REALarray arr, long index, unsigned short value );
	void REALSetArrayValueUInt8( REALarray arr, long index, unsigned char value );
	void REALSetArrayValue( REALarray arr, long index, float value );
	void REALSetArrayValue( REALarray arr, long index, double value );
	void REALSetArrayValue( REALarray arr, long index, Boolean value );
	void REALSetArrayValue( REALarray arr, long index, REALobject value );
	void REALSetArrayValue( REALarray arr, long index, REALstring value );
#endif

dbCursor *REALGetCursorFromREALdbCursor(REALdbCursor cursor);

Boolean REALLockPictureDescription(REALpicture pic, REALpictureDescription *description, long picType);
#if !FLAT_C_PLUGIN_HEADERS
	void REALLockPictureDescription(REALpicture pic, REALpictureDescription *description);
#else
	void REALLockPictureDescriptionWithNativeType(REALpicture pic, REALpictureDescription *description);	
#endif

#if TARGET_CARBON || TARGET_WIN32 || X_WINDOW
void SetClassConsoleSafe(REALclassDefinition def);
#endif

REALobject REALNewVariantUInt32(unsigned long value);

REALobject REALNewVariantInt64(RBInt64 value);

REALobject REALNewVariantUInt64(unsigned RBInt64 value);

REALobject REALNewVariantSingle(float value);

REALobject REALNewVariantCurrency(REALcurrency value);

unsigned long REALstringToOSType(REALstring id);

unsigned long REALGetStringEncoding(REALstring str);
void REALSetStringEncoding(REALstring str, unsigned long encoding);
REALstring REALConvertString(REALstring str, unsigned long encoding);
unsigned long REALWin32CodePageToEncoding( unsigned long codePage );

void REALGetControlBounds(REALcontrolInstance instance, Rect *rBounds);
long REALGetControlPosition(REALcontrolInstance instance, long which);
void REALSetControlPosition(REALcontrolInstance instance, long which, long value);

long REALGetControlVisible(REALcontrolInstance instance);
void REALSetControlVisible(REALcontrolInstance instance, unsigned long visible);

Boolean REALGetControlEnabled(REALcontrolInstance instance);
void REALSetControlEnabled(REALcontrolInstance instance, long unused, Boolean enable);

#if TARGET_COCOA
	NSView *REALGetControlHandle(REALcontrolInstance control);
#elif TARGET_68K || TARGET_PPC || TARGET_CARBON
	ControlHandle REALGetControlHandle(REALcontrolInstance control);
#elif TARGET_WIN32
	HWND REALGetControlHandle(REALcontrolInstance control);
#elif X_WINDOW
	unsigned long REALGetControlHandle(REALcontrolInstance control);
#endif

REALwindow REALGetControlWindow(REALcontrolInstance instance);

#if TARGET_COCOA || TARGET_CARBON
CGImageRef REALCopyPictureCGImage(REALpicture pic);
#endif

#if FLAT_C_PLUGIN_HEADERS && defined(__cplusplus)
	}
#endif

/****************** DEPRECATED FUNCTIONALITY **********************/
// Anything below this point is to be considered deprecated 
// functionality, and should be avoided except in extreme cases.

#if __GNUC__
	// Compiling with GCC of some sort
	#define	DEPRECATED		__attribute__ ((deprecated))
#else
	#define	DEPRECATED
#endif

#if TARGET_CPU_68K
	// Compile with Metrowerks-ish/Think C calling conventions:
	// Turn "MPW C" option off, and return pointers in D0 rather than A0.
	#pragma d0_pointers on
	#pragma mpwc off
#endif

#if TARGET_CPU_68K
	#define TARGET_68K 1
#endif

// define some constants for use with REALGetControlPosition etc.
#define kREALLeft 0
#define kREALTop 1
#define kREALWidth 2
#define kREALHeight 3

#define REALpropInvalidate 				(1 << 0)

#define REALenabledControl 				(1 << 4)

extern struct REALstringStruct
{
// This structure is for examination purposes only!
// Do not construct them yourself - use REALBuildString!
#ifdef __cplusplus
private:
	long mPrivateUsageCount;
	unsigned char *mPrivateStringData;
	long mPrivateAllocLength;
	long mPrivateLength;
	unsigned long mEncoding;
public:
	long Length(void);
	const char *CString();
	const unsigned char *PString();
#endif
} REALstringStruct;

typedef struct 
{
	const char *szInterface;
	const char *szDescription;
	const char *szPart;
} REALbindingDescription;

struct REALpopupMenuStruct;
typedef struct REALpopupMenuStruct *REALpopupMenu;

struct REALmemoryBlockStruct;
typedef struct REALmemoryBlockStruct *REALmemoryBlock;

struct REALsocketStruct;
typedef struct REALsocketStruct *REALsocket;

struct REALintArrayStruct;
typedef struct REALintArrayStruct *REALintArray;

struct REALstringArrayStruct;
typedef struct REALstringArrayStruct *REALstringArray;

struct REALobjectArrayStruct;
typedef struct REALobjectArrayStruct *REALobjectArray;

struct REALmoviePlayerStruct;
typedef struct REALmoviePlayerStruct *REALmoviePlayer;

struct REALstructureArrayStruct;
typedef struct REALstructureArrayStruct *REALstructureArray;

enum REALControlStateChangedBits {
	kActivationChanged = 1,
	kEnabledChanged = 2,
	kVisibilityChanged = 4,
	kBoundsChanged = 8
};

#define kREALfontStyleVersion 2

typedef struct
{
	long version;				// this should always be kREALfontStyleVersion
	REALstring fontName;		// name of the font or metafont (e.g. "System"); may be nil
	long fontSize;				// font size in points
	long color;					// color in standard plugin form, i.e. 0x00RRGGBB
	Boolean bold;				// style attributes
	Boolean italic;
	Boolean underline;
	unsigned char fontUnit;		// 0 - Default, 1 - Pixel, 2 - Point, 3 - Inches, 4 - Millimeters
} REALfontStyle;

// socket constants
#define socketEventConnect 1
#define socketEventError 2
#define socketEventDataReceived 4
#define socketEventSendComplete 8


#if FLAT_C_PLUGIN_HEADERS && defined( __cplusplus )
	extern "C" {
#endif

void *REALLoadGlobalMethod( const char *module, const char *prototype ) DEPRECATED;

// You can use these methods to register a global structure
// or enumeration in much the same way you can register 
// global methods.  Note: be careful when adding anything
// to the global namespace as it could cause conflicts.  Use
// these methods with extreme caution.
void REALRegisterStructure( REALstructure *defn ) DEPRECATED;
void REALRegisterEnum( REALenum *defn ) DEPRECATED;

REALobject REALnewInstance(const char *className);

#if TARGET_68K || TARGET_PPC
QDGlobals *REALQDGlobals(void) DEPRECATED;
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON || COCOA
int REALallocateMenuID(void) DEPRECATED;
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON || COCOA
void REALreleaseMenuID(int id) DEPRECATED;
#endif

void GraphicsDrawLine(REALgraphics graphicsObject, int x1, int y1, int x2, int y2) DEPRECATED;
void REALRegisterMethod(REALmethodDefinition *defn) DEPRECATED;
void REALRegisterClassExtension(REALclassDefinition *defn) DEPRECATED;

const char *REALCString(REALstring str) DEPRECATED;
const unsigned char *REALPString(REALstring str) DEPRECATED;

REALstring REALInterpretConstantValue(REALstring value);
REALstring REALDefaultControlCaption(void);

#if (TARGET_68K || TARGET_PPC || TARGET_CARBON) && !COCOA
void REALRegisterEventFilter(REALEventCallback callback, long param) DEPRECATED;
#endif

REALproc REALInterfaceRoutine(REALobject obj, const char *interfaceName, const char *methodName) DEPRECATED;

void REALPictureClearCache(REALpicture pic) DEPRECATED;

#if TARGET_WIN32
void REALDrawPicturePrimitive(HDC hDC, REALpicture pic, const Rect *rBounds, int bTransparent) DEPRECATED;
#endif

#if (TARGET_68K || TARGET_PPC || TARGET_CARBON) || COCOA
void REALDrawPicturePrimitive(REALpicture pic, const Rect *rBounds, int bTransparent) DEPRECATED;
#endif

REALgraphics REALGetControlGraphics(REALcontrolInstance instance) DEPRECATED;

#if TARGET_WIN32
int REALGetWin32Charset(void) DEPRECATED;
#endif

#if TARGET_WIN32 || UNIX_ANSI
REALfolderItem REALFolderItemFromPath(const char *path) DEPRECATED;
#endif

#if TARGET_WIN32
HWND REALGetControlHWND(REALcontrolInstance control) DEPRECATED;
#endif

#if COCOA
CGContextRef REALGraphicsCGContext(REALgraphics context) DEPRECATED;
#endif

void REALInvalidateControl(REALcontrolInstance instance) DEPRECATED;

void REALInvalidateControlRect(REALcontrolInstance instance, int left, int top, int right, int bottom) DEPRECATED;

#if TARGET_WIN32
void REALSetSpecialBackground(REALcontrolInstance instance, COLORREF *pcolor) DEPRECATED;
#endif

#if (TARGET_68K || TARGET_PPC || TARGET_CARBON) || COCOA
void REALSetSpecialBackground(REALcontrolInstance instance) DEPRECATED;
#endif

#if TARGET_COCOA
	NSWindow *REALGetWindowHandle(REALwindow window) DEPRECATED;
#elif TARGET_68K || TARGET_PPC || TARGET_CARBON
WindowPtr REALGetWindowHandle(REALwindow window) DEPRECATED;
#endif

#if (TARGET_PPC || TARGET_CARBON) && !TARGET_COCOA
MenuHandle REALGetPopupMenuHandle(REALpopupMenu popup) DEPRECATED;
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || TARGET_COCOA
QT_NAMESPACE MovieController REALgetMoviePlayerController(REALmoviePlayer instance) DEPRECATED;
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || TARGET_COCOA
QT_NAMESPACE Movie REALgetMovieMovie(REALmovie instance) DEPRECATED;
#endif

void REALMarkSocketUsage(void) DEPRECATED;

void REALSocketConnect(REALsocket socket, REALstring address, int port) DEPRECATED;

void REALSocketClose(REALsocket socket) DEPRECATED;

REALstring REALSocketReadAll(REALsocket socket) DEPRECATED;

REALstring REALSocketRead(REALsocket socket, int count) DEPRECATED;

void REALSocketWrite(REALsocket socket, REALstring data) DEPRECATED;

int REALSocketLastErrorCode(REALsocket socket, int unused) DEPRECATED;

REALstring REALSocketLookahead(REALsocket socket, int unused) DEPRECATED;

REALstring REALSocketLocalAddressGetter(REALsocket socket) DEPRECATED;

void REALSocketPoll(REALsocket socket) DEPRECATED;

int REALSocketGetEvents(REALsocket socket, int unused) DEPRECATED;

void REALMessageBox(REALstring text) DEPRECATED;

#if (TARGET_68K || TARGET_PPC || TARGET_CARBON) && !TARGET_COCOA
void REALRefreshWindow(unsigned long macWindowPtr) DEPRECATED;
#endif

REALgraphics REALGetPictureGraphics(REALpicture picture) DEPRECATED;

REALpicture REALNewPicture(long width, long height, long depth) DEPRECATED;

REALmemoryBlock REALNewMemoryBlock(int bytes) DEPRECATED;

void*REALMemoryBlockGetPtr(REALmemoryBlock memBlock) DEPRECATED;

int REALMemoryBlockGetSize(REALmemoryBlock memBlock) DEPRECATED;

REALmemoryBlock REALPtrToMemoryBlock(void*data) DEPRECATED;

int REALGetArrayInt(REALintArray array, int index) DEPRECATED;

REALstring REALGetArrayString(REALstringArray array, int index) DEPRECATED;

REALobject REALGetArrayObject(REALobjectArray array, int index) DEPRECATED;

long REALGetTabPanelVisible(REALcontrolInstance instance) DEPRECATED;

#if TARGET_WIN32
HWND REALGetWindowHandle(REALwindow window) DEPRECATED;
#endif

Boolean REALGetControlFocus(REALcontrolInstance instance) DEPRECATED;

void REALSetControlFocus(REALcontrolInstance instance, Boolean focus) DEPRECATED;

REALcontrolInstance REALGetControlParent(REALcontrolInstance instance) DEPRECATED;

REALstring REALGetControlName(REALcontrolInstance control) DEPRECATED;

Boolean REALIsHIViewWindow(REALwindow window) DEPRECATED;

#if TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || X_WINDOW || TARGET_COCOA
unsigned long REALGetFontEncoding(const char *fontName) DEPRECATED;
#endif

#if TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || X_WINDOW || TARGET_COCOA
REALpicture REALGetPictureMask(REALpicture pict, Boolean createIfNil) DEPRECATED;
#endif

#if TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || X_WINDOW || TARGET_COCOA
void REALGraphicsDrawString(REALgraphics graphics, REALstring str, long x, long y, long width) DEPRECATED;
#endif

#if TARGET_CARBON || TARGET_COCOA
CFStringRef REALGetStringCFString(REALstring str, Boolean stripAmpersands) DEPRECATED;
#endif

#if TARGET_PPC || TARGET_CARBON || TARGET_COCOA
void REALGetStringSystemStr(REALstring str, Boolean stripAmpersands, Str255 outStr255) DEPRECATED;
#endif

#if TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || X_WINDOW || TARGET_COCOA
void REALGetGraphicsFontStyle(REALgraphics graphics, REALfontStyle*  outStyle) DEPRECATED;
#endif

#if TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || X_WINDOW || TARGET_COCOA
void REALSetGraphicsStyle(REALgraphics graphics, REALfontStyle*  styleInfo) DEPRECATED;
#endif

long REALGraphicsStringWidth(REALgraphics graphics, REALstring str) DEPRECATED;

long REALGraphicsStringHeight(REALgraphics graphics, REALstring str, long wrapWidth) DEPRECATED;

long REALGraphicsTextHeight(REALgraphics graphics) DEPRECATED;

long REALGraphicsTextAscent(REALgraphics graphics) DEPRECATED;

#if (TARGET_PPC || TARGET_CARBON) && !TARGET_COCOA
void REALReleasePopupMenuHandle(REALpopupMenu popup) DEPRECATED;
#endif

void REALSocketListen(REALsocket socket) DEPRECATED;

#if X_WINDOW
unsigned long REALGraphicsDC(REALgraphics context) DEPRECATED;
#endif

#if COCOA
		CGContextRef REALGraphicsDC(REALgraphics context) DEPRECATED;
#endif
		
#if X_WINDOW
void REALDrawPicturePrimitive(REALgraphics context, REALpicture pic, const Rect *rBounds, int bTransparent) DEPRECATED;
#endif

#if FLAT_C_PLUGIN_HEADERS
	#if TARGET_WIN32
		REALgraphics REALGetControlGraphicsWithDC(REALcontrolInstance instance, HDC dc) DEPRECATED;
	#endif
#else
	#if TARGET_WIN32
		REALgraphics REALGetControlGraphics(REALcontrolInstance instance, HDC dc) DEPRECATED;
	#endif
#endif

#if X_WINDOW
void *REALGraphicsGdkDrawable(REALgraphics context) DEPRECATED;
#endif

#if X_WINDOW
unsigned long REALGetWindowHandle(REALwindow window) DEPRECATED;
#endif

void REALGetGraphicsOrigin(REALgraphics context, long *originX, long *originY) DEPRECATED;

void REALSetGraphicsOrigin(REALgraphics context, long originX, long originY) DEPRECATED;

REALstring REALpathFromFolderItem(REALfolderItem item) DEPRECATED;

#if TARGET_WIN32
HDC REALGraphicsDC(REALgraphics context) DEPRECATED;
#endif

#if TARGET_WIN32 || X_WINDOW
void REALSetAccelerator(REALcontrolInstance instance, REALstring key) DEPRECATED;
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON || TARGET_WIN32 || TARGET_COCOA
void REALSetMovieMovie(REALmovie obj, Movie movie) DEPRECATED;
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON || TARGET_COCOA
REALfolderItem REALFolderItemFromFSSpec(const FSSpec *spec) DEPRECATED;
#endif

#if TARGET_68K || TARGET_PPC || TARGET_CARBON || TARGET_COCOA
Boolean REALFSSpecFromFolderItem(FSSpec *spec, REALfolderItem item) DEPRECATED;
#endif

#if FLAT_C_PLUGIN_HEADERS && defined(__cplusplus)
	}
#endif

// These methods are defined by PluginMain.cpp and are part
// of the standard plugin SDK
extern inline void* CallResolver(const char *entryName);
extern REALmethodDefinition *ConvertCurrentMethodsToV6(REALmethodDefinition *inMethods, long count);

#if _MSC_VER
	// Compiling with Visual Studio of some sort
	#pragma deprecated( GraphicsDrawLine, REALallocateMenuID, REALbuildMovie, REALDrawPicturePrimitive ) 
	#pragma deprecated( REALDrawPicturePrimitive, REALDrawPicturePrimitive,REALFolderItemFromPath )
	#pragma deprecated( REALGetArrayInt,REALGetArrayObject,REALGetArrayString )
	#pragma deprecated( REALGetControlFocus,REALGetControlGraphics,REALGetControlGraphics )
	#pragma deprecated( REALGetControlHWND )
	#pragma deprecated( REALGetControlName,REALGetControlParent )
	#pragma deprecated( REALGetFontEncoding,REALGetGraphicsFontStyle,REALGetGraphicsOrigin )
	#pragma deprecated( REALgetMovieMovie,REALgetMoviePlayerController,REALGetPictureGraphics )
	#pragma deprecated( REALGetPictureMask,REALGetPopupMenuHandle,REALGetStringSystemStr,REALGetTabPanelVisible )
	#pragma deprecated( REALGetWin32Charset,REALGetWindowHandle,REALGetWindowHandle )
	#pragma deprecated( REALGraphicsDC,REALGraphicsDC,REALGraphicsDrawString,REALGraphicsGdkDrawable )
	#pragma deprecated( REALGraphicsStringHeight,REALGraphicsStringWidth,REALGraphicsTextAscent )
	#pragma deprecated( REALGraphicsTextHeight,REALInterfaceRoutine,REALInvalidateControl,REALInvalidateControlRect )
	#pragma deprecated( REALIsHIViewWindow,REALMarkSocketUsage,REALMemoryBlockGetSize,REALMessageBox )
	#pragma deprecated( REALNewMemoryBlock,REALNewPicture,REALpathFromFolderItem,REALPictureClearCache )
	#pragma deprecated( REALRefreshWindow,REALRegisterClassExtension,REALRegisterEventFilter,REALRegisterMethod )
	#pragma deprecated( REALreleaseMenuID,REALReleasePopupMenuHandle,REALSetAccelerator )
	#pragma deprecated( REALSetControlFocus,REALSetGraphicsOrigin )
	#pragma deprecated( REALSetGraphicsStyle,REALSetMovieMovie,REALSetSpecialBackground,REALSetSpecialBackground )
	#pragma deprecated( REALSocketClose,REALSocketConnect,REALSocketGetEvents,REALSocketLastErrorCode )
	#pragma deprecated( REALSocketListen,REALSocketLocalAddressGetter,REALSocketLookahead,REALSocketPoll )
	#pragma deprecated( REALSocketRead,REALSocketReadAll,REALSocketWrite,REALMemoryBlockGetPtr )
	#pragma deprecated( REALPtrToMemoryBlock,REALQDGlobals,REALCString,REALPString,REALGetStringCFString )
	#pragma deprecated( REALRegisterEventFilter, REALFolderItemFromFSSpec, REALFSSpecFromFolderItem )
	#pragma deprecated( REALLoadGlobalMethod )
#endif

#endif
