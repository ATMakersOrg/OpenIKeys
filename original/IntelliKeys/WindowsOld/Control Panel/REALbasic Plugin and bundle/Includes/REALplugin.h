// REALplugin.h
//
//	This file is part of the REALbasic plugin API.  It's included automatically
//	by "rb_plugin.h", so normally you'll just include the latter.
//
// © 1997-2000 REAL Software Inc. -- All Rights Reserved
// See file "Plug-in License SDK.txt" for details.
#ifndef REALPLUGIN_H
#define REALPLUGIN_H
#if TARGET_CPU_68K
	// Compile with Metrowerks-ish/Think C calling conventions:
	// Turn "MPW C" option off, and return pointers in D0 rather than A0.
	#pragma d0_pointers on
	#pragma mpwc off
#endif
// define some shorter macros for the four kinds of targets we support
#if WIN32
	#define TARGET_WIN32 1
	typedef QT::Movie Movie;
#elif TARGET_OS_MAC
	#define TARGET_MACOS 1
#endif
#if TARGET_CPU_68K
	#define TARGET_68K 1
#endif
#if TARGET_CPU_PPC
	#if TARGET_API_MAC_CARBON
		#define TARGET_CARBON 1
	#else
		#define TARGET_PPC 1
	#endif
#endif
// define some constants for use with REALGetControlPosition etc.
#define kREALLeft 0
#define kREALTop 1
#define kREALWidth 2
#define kREALHeight 3
struct REALstringStruct
{
// This structure is for examination purposes only!
// Do not construct them yourself - use REALBuildString!
private:
	long mPrivateUsageCount;
	unsigned char *mPrivateStringData;
	long mPrivateAllocLength;
	long mPrivateLength;
public:
	long Length(void);
	const char *CString();
	const unsigned char *PString();
};
typedef REALstringStruct *REALstring;
typedef void (*REALproc)(void);
struct REALdbDatabaseStruct;
typedef REALdbDatabaseStruct *REALdbDatabase;
struct REALdbCursorStruct;
typedef REALdbCursorStruct *REALdbCursor;
struct REALcontrolInstanceStruct;
typedef REALcontrolInstanceStruct *REALcontrolInstance;
struct REALgraphicsStruct;
typedef REALgraphicsStruct *REALgraphics;
struct REALobjectStruct;
typedef REALobjectStruct *REALobject;
struct REALfolderItemStruct;
typedef REALfolderItemStruct *REALfolderItem;
struct REALpictureStruct;
typedef REALpictureStruct *REALpicture;
struct REALsoundStruct;
typedef REALsoundStruct *REALsound;
struct REALappleEventStruct;
typedef REALappleEventStruct *REALappleEvent;
struct REALwindowStruct;
typedef REALwindowStruct *REALwindow;
struct REALpopupMenuStruct;
typedef REALpopupMenuStruct *REALpopupMenu;
struct REALmemoryBlockStruct;
typedef REALmemoryBlockStruct *REALmemoryBlock;
struct REALmovieStruct;
typedef REALmovieStruct *REALmovie;
struct REALmoviePlayerStruct;
typedef REALmoviePlayerStruct *REALmoviePlayer;
struct REALsocketStruct;
typedef REALsocketStruct *REALsocket;
struct REALintArrayStruct;
typedef REALintArrayStruct *REALintArray;
struct REALstringArrayStruct;
typedef REALstringArrayStruct *REALstringArray;
struct REALobjectArrayStruct;
typedef REALobjectArrayStruct *REALobjectArray;
struct REALclassRefStruct;
typedef REALclassRefStruct *REALclassRef;
typedef unsigned long REALDBConnectionDialogRef;
#define REALnoImplementation ((REALproc) nil)
struct REALmethodDefinition
{
	REALproc function;
	REALproc setterFunction;
	const char *declaration;
};
#define REALpropInvalidate 1
#define REALpropRuntimeOnly 2
#define REALstandardGetter ((REALproc) -1)
#define REALstandardSetter ((REALproc) -1)
struct REALproperty
{
	const char *group;
	const char *name;
	const char *type;
	int flags;
	REALproc getter;
	REALproc setter;
	int param;
	REALproc editor;
	int enumCount;
	const char **enumEntries;
};
struct REALevent
{
	const char *declaration;
	int forSystemUse;
};
struct REALeventInstance
{
	const char *name;
	REALproc implementation;
};
struct REALbindingDescription
{
	const char *szInterface;
	const char *szDescription;
	const char *szPart;
};
struct REALconstant
{
	const char *declaration;
	void* reserved1;
	int reserved2;
};
enum REALControlStateChangedBits {
	kActivationChanged = 1,
	kEnabledChanged = 2,
	kVisibilityChanged = 4,
	kBoundsChanged = 8
};
struct REALcontrolBehaviour
{
	void (*constructorFunction)(REALcontrolInstance);
	void (*destructorFunction)(REALcontrolInstance);
#if TARGET_OS_MAC
	void (*redrawFunction)(REALcontrolInstance);
#else
	void (*redrawFunction)(REALcontrolInstance, REALgraphics context);
#endif
	Boolean (*clickFunction)(REALcontrolInstance, int x, int y, int modifiers);
	void (*mouseDragFunction)(REALcontrolInstance, int x, int y);
	void (*mouseUpFunction)(REALcontrolInstance, int x, int y);
	void (*gainedFocusFunction)(REALcontrolInstance);
	void (*lostFocusFunction)(REALcontrolInstance);
	Boolean (*keyDownFunction)(REALcontrolInstance, int charCode, int keyCode, int modifiers);
	void (*openFunction)(REALcontrolInstance);
	void (*closeFunction)(REALcontrolInstance);
	void (*backgroundIdleFunction)(REALcontrolInstance);
	void (*drawOffscreenFunction)(REALcontrolInstance, REALgraphics context);
	void (*setSpecialBackground)(REALcontrolInstance);
	void (*constantChanging)(REALcontrolInstance, REALstring);
	void (*droppedNewInstance)(REALcontrolInstance);
	void (*mouseEnterFunction)(REALcontrolInstance);
	void (*mouseExitFunction)(REALcontrolInstance);
	void (*mouseMoveFunction)(REALcontrolInstance, int x, int y);
	void (*stateChangedFunction)(REALcontrolInstance, unsigned long changedField);
	void (*actionEventFunction)(REALcontrolInstance, unsigned long reserved);
	unsigned long (*controlHandleGetter)(REALcontrolInstance);
	Boolean (*mouseWheelFunction)(REALcontrolInstance, long x, long y, long delta);
	REALproc unusedFunction1;
	REALproc unusedFunction2;
};
#define kCurrentREALControlVersion 6
#define REALcontrolAcceptFocus 			1
#define REALcontrolFocusRing 			2
#define REALinvisibleControl 			4
#define REALopaqueControl 				8
#define REALenabledControl 				16
#define REALcontrolOwnsCursor 			32
#define REALcontrolIsHIViewCompatible	64 // This means that it doesn't use things that are incompatible with HIView's like Draw1Control
#define REALdontTryHIViewize		   128 // REALbasic will not create a HIView pane for this control, use
										   // if you are making a plugin for a OS Control, and Apple has already done the work
										   // or you provide your own HIView subclass.  Otherwise you shouldn't use this flag.
										   // This flag has no meaning for controls that are not contained on a Mac OS X composited window.
struct REALcontrol
{
	int version;
	const char *name;
	int dataSize;
	int flags;
	int toolbarPICT, toolbarDownPICT;
	int defaultWidth, defaultHeight;
	REALproperty *properties;
	int propertyCount;
	REALmethodDefinition *methods;
	int methodCount;
	REALevent *events;
	int eventCount;
	REALcontrolBehaviour *behaviour;
	int forSystemUse;
	REALeventInstance *eventInstances;
	int eventInstanceCount;
	const char *interfaces;
	REALbindingDescription *bindDescriptions;
	int bindDescriptionCount;
	REALconstant *constants;
	int constantCount;
};
struct REALclassDefinition
{
	int version;
	const char *name;
	const char *superName;
	int dataSize;
	int forSystemUse;
	REALproc constructor;
	REALproc destructor;
	REALproperty *properties;
	int propertyCount;
	REALmethodDefinition *methods;
	int methodCount;
	REALevent *events;
	int eventCount;
	REALeventInstance *eventInstances;
	int eventInstanceCount;
	const char *interfaces;
	REALbindingDescription *bindDescriptions;
	int bindDescriptionCount;
	REALconstant *constants;
	int constantCount;
};
struct REALinterfaceDefinition
{
	int version;						// just pass kCurrentREALControlVersion
	const char* name;					// interface name
	REALmethodDefinition *methods;		// list of methods the interface requires
	int methodCount;					// how many methods there are
};
struct REALmoduleDefinition
{
    int version;						// use kCurrentREALControlVersion
    const char *name;					// name of the module
    REALmethodDefinition *methods;		// list of public module methods
    int methodCount;					// number of entries in the method list
    REALconstant* constants;			// list of constants
    int constantCount;					// number of constants in the list
    void* reserved1;					// must be NULL
    int reserved2;						// must be 0
    void* reserved3;					// must be NULL
    int reserved4;						// must be 0
    void* reserved5;					// must be NULL
    int reserved6;						// must be 0
};
#if TARGET_OS_MAC
	typedef void (*REALEventCallback)(EventRecord *event, long param);
#endif
#define kCurrentREALDatabaseVersion 1
typedef void (*REALDataSourceInterfaceProc)(void);
typedef REALdbDatabase (*REALDataSourceProc)(Ptr data, int dataLen);
struct dbDatabase;
struct dbTable;
struct dbCursor;
struct dbDate
{
	short year;
	short month;
	short day;
};
struct dbTime
{
	short hour;
	short minute;
	short second;
};
struct dbTimeStamp
{
	short year;
	short month;
	short day;
	short hour;
	short minute;
	short second;
};
enum dbFieldType
{
	dbTypeNull = 0,		//  0
	dbTypeByte,			//  1
	dbTypeShort,		//  2
	dbTypeLong,			//  3
	dbTypeChar,			//  4
	dbTypeText,			//  5
	dbTypeFloat,		//  6
	dbTypeDouble,		//  7
	dbTypeDate,			//  8
	dbTypeTime,			//  9
	dbTypeTimeStamp,	// 10
	dbTypeCurrency,		// 11
	dbTypeBoolean,		// 12
	dbTypeDecimal,		// 13
	dbTypeBinary,		// 14
	dbTypeLongText,		// 15	// deprecated; use dbTypeText
	dbTypeLongBinary,	// 16	// deprecated; use dbTypeBinary
	dbTypeMacPICT,		// 17
	dbTypeREALstring,	// 18
	dbTypeUnknown = 255
};
struct REALnewColumn
{
	REALnewColumn *nextColumn;
	REALstring columnName;
	long columnType;
	long bAllowNULL;
};
struct REALcolumnValue
{
	REALcolumnValue *nextColumn;
	REALstring columnName;
	REALstring columnValue;
	int columnType;
};
enum REALcolumnOperation
{
	rcOpEquals,
	rcOpLessThan,
	rcOpGreaterThan,
	rcOpLessThanEqual,
	rcOpGreaterThanEqual,
	rcOpNotEqual,
	rcOpLike,
	rcOpNotLike,
	rcOpIsNull,
	rcOpIsNotNull,
	rcOpAnd = 64,
	rcOpOr
};
struct REALcolumnConstraints
{
	REALcolumnConstraints *left, *right;
	long columnOperation;
	long columnComparison;
	REALstring column;
	REALstring value;
};
struct REALgetColumn
{
	REALgetColumn *next;
	REALstring column;
};
// REALdbEngineDefinition Flags1
enum {
	dbEnginePrimaryKeySupported = 1,
	dbEngineAlterTableAddParens = 2,
	dbEngineDontUseBrackets = 4,
	dbEngineColumnDeleteSupported = 8,
	dbEngineColumnModificationSupported = 16,
	dbEngineColumnOptionsSupported = 32
};
struct REALfieldUpdate
{
	REALfieldUpdate *next;
	Ptr tableField;
	int tableFieldLen;
	Ptr recordKey;
	int recordKeyLen;
	REALstring value;
};
struct REALdbEngineDefinition
{
	int version;
	unsigned char forSystemUse;
	unsigned char flags1;
	unsigned char flags2;
	unsigned char flags3;
	void (*closeDatabase)(dbDatabase *); // void (*closeDatabase)(dbDatabase *);
	REALdbCursor (*getTableSchemaCursor)(dbDatabase *); // dbCursor *(*tableCursor)(dbDatabase *)) /* optional */
	REALdbCursor (*getFieldSchemaCursor)(dbDatabase *, REALstring); // dbCursor *(*fieldCursor)(dbDatabase *, REALstring)) /* optional */
	REALdbCursor (*directSQLSelect)(dbDatabase *, REALstring); // DatabaseCursorObject (*directSQLSelect)(dbDatabase *, REALstring selectString);
	void (*directSQLExecute)(dbDatabase *, REALstring); // void (*directSQLExecute)(dbDatabase *, REALstring executeString);
	void (*createTable)(dbDatabase *, REALstring, REALnewColumn *, unsigned char *, int); // void (*createTable)(dbDatabase *, REALstring name, REALnewColumn *columns, unsigned char *primaryKey, int primaryKeyCount);
	void (*addTableRecord)(dbDatabase *, REALstring, REALcolumnValue *); // void (*addTableRecord)(dbDatabase *, REALstring tableName, REALcolumnValue *values);
	REALdbCursor (*getTableCursor)(dbDatabase *, REALstring, REALgetColumn *, REALcolumnConstraints *); // DatabaseCursorObject *(*getTableCursor)(dbDatabase *, REALstring tableName, REALgetColumn *columns, REALcolumnConstraints *constraints);
	void (*updateFields)(dbDatabase *, REALfieldUpdate *fields);
	void (*addTableColumn)(dbDatabase *, REALstring, REALnewColumn *);
	REALdbCursor (*getDatabaseIndexes)(dbDatabase *, REALstring table);
	long (*getLastErrorCode)(dbDatabase *);
	REALstring (*getLastErrorString)(dbDatabase *);
	void (*commit)(dbDatabase *);
	void (*rollback)(dbDatabase *);
	REALstring (*getProperty)(dbDatabase *, REALstring propertyName);
	void (*getSupportedTypes)(long **dataTypes, char **dataNames, long *count);
	REALproc unused1;
	REALproc unused2;
	REALproc unused3;
	REALproc unused4;
	REALproc unused5;
	REALproc unused6;
	REALproc unused7;
	REALproc unused8;
	REALproc unused9;
};
struct REALdbTableDefinition
{
	int version;
	int forSystemUse;
	REALproc closeTable; // void (*closeTable)(dbTable *);
	REALproc tableCursor; // dbCursor *(*tableCursor)(dbTable *);
};
struct REALcursorUpdate
{
	REALcursorUpdate *next;
	int fieldIndex;
	REALstring columnValue;
};
struct REALdbCursorDefinition
{
	int version;
	int forSystemUse;
	void (*closeCursor)(dbCursor *);
	int (*cursorColumnCount)(dbCursor *); // int (*cursorColumnCount)(dbCursor *);
	REALstring (*cursorColumnName)(dbCursor *, int column); // REALstring (*cursorColumnName)(dbCursor *, int column); /* optional */
	int (*cursorRowCount)(dbCursor *); // int (*cursorRowCount)(dbCursor *); /* optional */
	void (*cursorColumnValue)(dbCursor *, int, Ptr *, unsigned char *, int *); // void (*cursorColumnValue)(dbCursor *, int column, Ptr *value, dbFieldType *type, int *length);
	void (*cursorReleaseValue)(dbCursor *); // void (*cursorReleaseValue)(dbCursor *); /* optional */
	Boolean (*cursorNextRow)(dbCursor *); // Boolean (*cursorNextRow)(dbCursor *);
	void (*cursorDelete)(dbCursor *); // void (*cursorDelete)(dbCursor *);
	void (*cursorDeleteAll)(dbCursor *); // void (*cursorDeleteAll)(dbCursor *);
	Boolean (*cursorFieldKey)(dbCursor *, int, Ptr *, int *, Ptr *, int *);
	void (*cursorUpdate)(dbCursor *, REALcursorUpdate *fields);
	void (*cursorEdit)(dbCursor *);			// called by dbCursor.Edit in RB
	void (*cursorPrevRow)(dbCursor *);
	void (*cursorFirstRow)(dbCursor *);
	void (*cursorLastRow)(dbCursor *);
	int (*cursorColumnType)(dbCursor *, int index);	// gets the column type (0 based index)
	void *dummy8;
	void *dummy9;
	void *dummy10;
};
#define kREALfontStyleVersion 1
struct REALfontStyle
{
	long version;				// this should always be kREALfontStyleVersion
	REALstring fontName;		// name of the font or metafont (e.g. "System"); may be nil
	long fontSize;				// font size in points
	long color;					// color in standard plugin form, i.e. 0x00RRGGBB
	Boolean bold;				// style attributes
	Boolean italic;
	Boolean underline;
	Boolean _reserved;			// (not used)
};
#define FieldOffset(type, field) (long)(&((type *) 0)->field)
extern "C" {
void PluginEntry(void);
}
#endif
