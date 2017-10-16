// REALplugin.h
//
//	This file is part of the REALbasic plugin API.  It's included automatically
//	by "rb_plugin.h", so normally you'll just include the latter.
//
// © 1997-2006 REAL Software Inc. -- All Rights Reserved
// See file "Plug-in License SDK.txt" for details.
//
//	Jun 23 2006 -- AJB (1)	Added support for currency, structures, enumerations, 
//							module properties, shared methods and shared properties

#ifndef REALPLUGIN_H
#define REALPLUGIN_H

// define some shorter macros for the four kinds of targets we support
#if WIN32
	#define TARGET_WIN32 1
	typedef QT::Movie Movie;
#elif TARGET_OS_MAC
	#define TARGET_MACOS 1
#endif

#if TARGET_MACOS
	#if COCOA
		#define TARGET_COCOA 1
		#if ALLOW_CARBON_IN_COCOA
			#ifdef TARGET_CARBON
				#undef TARGET_CARBON
			#endif
			#define TARGET_CARBON 1
		#endif
	#elif TARGET_API_MAC_CARBON
		#define TARGET_CARBON 1
	#elif TARGET_CPU_PPC
		#define TARGET_PPC 1
	#endif
#endif

#ifdef _MSC_VER
	#if _MSC_VER <= 1200
		// Older versions of Visual Studio don't have long long support
		#define RBInt64	__int64
	#else
		// But newer versions certainly do
		#define RBInt64	long long
	#endif
#else
	#define RBInt64	long long
#endif

// The Currency data type is a signed 64-bit integer value that
// is expressed in "mils."  This means that if you would like to
// convert from a currency value into a regular Int64 value, you 
// would simply divide by 10000.  To convert an Int64 into a
// currency value, you would multiply by 10000. -- Jun 23 2006 -- AJB (1)
typedef RBInt64 REALcurrency;

typedef enum REALArrayType {
	kTypeSInt32 = 0,					// 32-bit integer, or color
	kTypeFloat32 = 1,					// 32-bit float type (single)
	kTypeFloat64 = 2,					// 64-bit float type (double)
	kTypeString = 3,					// string (32 bit pointer, refcounted)
	kTypeObject = 4,					// object (32 bit pointer, refcounted)
	kTypeUInt8 = 5,						// 8-bit unsigned integer, or boolean
	kTypeSInt8 = 6,						// 8-bit signed integer
	kTypeUInt16 = 7,					// 16-bit unsigned integer
	kTypeSInt16 = 8,					// 16-bit signed integer
	kTypeUInt32 = 9,					// 32-bit unsigned integer
	kTypeUInt64 = 10,					// 64-bit unsigned integer
	kTypeSInt64 = 11,					// 64-bit signed integer, or Currency
	kTypeStructure = 12,				// Arbitrary-sized structure
	kTypePtr = 13,						// 32-bit signed integer
	kTypeColor = 14,					// 32-bit signed integer
	kTypeCurrency = 15,					// 64-bit currency integer
	kTypeBoolean = 16,					// 8-bit signed integer
} REALArrayType;

struct REALstringStruct;
typedef struct REALstringStruct *REALstring;

typedef void (*REALproc)(void);

struct REALdbDatabaseStruct;
typedef struct REALdbDatabaseStruct *REALdbDatabase;

struct REALdbCursorStruct;
typedef struct REALdbCursorStruct *REALdbCursor;

struct REALcontrolInstanceStruct;
typedef struct REALcontrolInstanceStruct *REALcontrolInstance;

struct REALobjectStruct;
typedef struct REALobjectStruct *REALobject;

struct REALpictureStruct;
typedef struct REALpictureStruct *REALpicture;

struct REALgraphicsStruct;
typedef struct REALgraphicsStruct *REALgraphics;

struct REALfolderItemStruct;
typedef struct REALfolderItemStruct *REALfolderItem;

struct REALsoundStruct;
typedef struct REALsoundStruct *REALsound;

struct REALappleEventStruct;
typedef struct REALappleEventStruct *REALappleEvent;

struct REALmovieStruct;
typedef struct REALmovieStruct *REALmovie;

struct REALarrayStruct;
typedef struct REALarrayStruct *REALarray;

struct REALclassRefStruct;
typedef struct REALclassRefStruct *REALclassRef;

typedef unsigned long REALDBConnectionDialogRef;

#define REALnoImplementation ((REALproc) nil)

// You can use the following definitions when setting
// the "flags" property for methods, properties, classes and controls
#define REALpropRuntimeOnly				(1 << 1)
#define	REALconsoleSafe					(1 << 2)
#define REALconsoleOnly					(1 << 3)
#define REALwebOnly						(1 << 8)

// You can use the following definitions when setting
// the flags property for methods, properties, structures
// and enumerations.
#define	REALScopeGlobal					(1 << 4)
#define	REALScopePublic					(1 << 5)
#define	REALScopeProtected				(1 << 6)
#define	REALScopePrivate				(1 << 7)

typedef struct
{
	const char *mName;
	const char *mValue;
} REALattribute;

typedef struct
{
	REALproc function;
	REALproc setterFunction;
	const char *declaration;
	long mFlags;
#ifndef PRE_2011R4_SDK
	long attributeCount;
	REALattribute *attributes;
#endif
} REALmethodDefinition;

#define REALstandardGetter ((REALproc) -1)
#define REALstandardSetter ((REALproc) -1)

typedef struct
{
	const char *group;
	const char *name;
	const char *type;
	long flags;
	REALproc getter;
	REALproc setter;
	long param;
	REALproc editor;
	long enumCount;
	const char **enumEntries;
#ifndef PRE_2011R4_SDK
	long attributeCount;
	REALattribute *attributes;
#endif
} REALproperty;

typedef struct 
{
	const char *declaration;
	long forSystemUse;
#ifndef PRE_2011R4_SDK
	long attributeCount;
	REALattribute *attributes;
#endif
} REALevent;

typedef struct
{
	const char *name;
	REALproc implementation;
} REALeventInstance;

typedef struct 
{
	const char *declaration;
	void* reserved1;
	long mFlags;
#ifndef PRE_2011R4_SDK
	long attributeCount;
	REALattribute *attributes;
#endif
} REALconstant;

struct REALstructure	// Jun 23 2006 -- AJB (1)
{
	// Specifies the name of the structure
	const char *name;
	
	// Flags for the structure, such as scope
	long mFlags;
	
	// The fields of the structure as 
	// spelled out in REALbasic code
	// eg) "Foobar as Integer"
	const char **fields;
	
	// The number of fields in the 
	// structure, as specified by 
	// the fields member.
	long numFields;

#ifndef PRE_2011R4_SDK
	long attributeCount;
	REALattribute *attributes;
#endif
};

struct REALenum		// Jun 23 2006 -- AJB (1)
{
	// Specifies the name of the enumeration
	const char *name;
	
	// Specifies the type of the enumeration
	// as an RB intrinsic datatype.
	// eg) Integer or UInt8 (currently, integer
	// types are the only types supported).  If
	// you leave it blank, it means Integer
	const char *type;
	
	// Flags for the enumeration, such as scope
	long mFlags;
	
	// The fields of the enumeration
	// as spelled out in REALbasic code
	// eg) "Apple" or "Pear = 12"
	const char **fields;
	
	// The number of fields in the
	// enumeration, as specified by
	// the fields member.
	long numFields;

#ifndef PRE_2011R4_SDK
	long attributeCount;
	REALattribute *attributes;
#endif
};

typedef struct
{
	void (*constructorFunction)(REALcontrolInstance);
	void (*destructorFunction)(REALcontrolInstance);
#if TARGET_OS_MAC && !COCOA
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
	void (*enableMenuItemsFunction)(REALcontrolInstance);
	Boolean (*menuItemActionFunction)(REALcontrolInstance, REALobject menuitem);
	Boolean (*controlAcceptFocus)(REALcontrolInstance );
	Boolean (*keyUpFunction)(REALcontrolInstance, int charCode, int keyCode, int modifiers);
	
	REALproc unusedFunction1;
} REALcontrolBehaviour;

#ifndef PRE_2011R4_SDK
	#define kCurrentREALControlVersion 10
#else
	#define kCurrentREALControlVersion 9
#endif

#define REALcontrolAcceptFocus 			(1 << 0)
#define REALcontrolFocusRing 			(1 << 1)
#define REALinvisibleControl 			(1 << 2)
#define REALopaqueControl 				(1 << 3)
#define REALcontrolOwnsCursor 			(1 << 5)
#define REALcontrolIsHIViewCompatible	(1 << 6) // This means that it doesn't use things that are incompatible with HIView's like Draw1Control
#define REALdontTryHIViewize		   	(1 << 7) // REALbasic will not create a HIView pane for this control, use
										   		 // if you are making a plugin for a OS Control, and Apple has already done the work
											     // or you provide your own HIView subclass.  Otherwise you shouldn't use this flag.
											     // This flag has no meaning for controls that are not contained on a Mac OS X composited window.
#define REALdontEraseBackground			(1 << 8)	 // Set this flag if you don't want REALbasic to automatically erase the control's background.
													 // Note, this only affects Windows, use it to reduce flicker if your control already handles background painting.
#define REALcontrolRequiresComposite    (1 << 9) // This flag designates that the control requires a composite window.

typedef struct
{
	long version;
	const char *name;
	long dataSize;
	long flags;
	long toolbarPICT, toolbarDownPICT;
	long defaultWidth, defaultHeight;
	REALproperty *properties;
	long propertyCount;
	REALmethodDefinition *methods;
	long methodCount;
	REALevent *events;
	long eventCount;
	REALcontrolBehaviour *behaviour;
	long forSystemUse;
	REALeventInstance *eventInstances;
	long eventInstanceCount;
	const char *interfaces;
	REALattribute *attributes;			// Re-using this in 2011r3 (control version 10 and later)
	long attributeCount;
	REALconstant *constants;
	long constantCount;
} REALcontrol;

typedef struct
{
	long version;
	const char *name;
	const char *superName;
	long dataSize;
	long forSystemUse;
	REALproc constructor;
	REALproc destructor;
	REALproperty *properties;
	long propertyCount;
	REALmethodDefinition *methods;
	long methodCount;
	REALevent *events;
	long eventCount;
	REALeventInstance *eventInstances;
	long eventInstanceCount;
	const char *interfaces;
	REALattribute *attributes;			// Re-using this in 2011r3 (control version 10 and later)
	long attributeCount;
	REALconstant *constants;
	long constantCount;
	long mFlags;
	REALproperty *sharedProperties;		// Begin Added in RB2006r4 (control version 9 and later) -- Jun 23 2006 -- AJB (1)
	long sharedPropertyCount;
	REALmethodDefinition *sharedMethods;
	long sharedMethodCount;				// End RB2006r4 stuff
} REALclassDefinition;

void SetClassConsoleSafe( REALclassDefinition *def );
void SetClassWebOnly( REALclassDefinition *def );

typedef struct
{
	long version;						// just pass kCurrentREALControlVersion
	const char* name;					// interface name
	REALmethodDefinition *methods;		// list of methods the interface requires
	long methodCount;					// how many methods there are
#ifndef PRE_2011R4_SDK
	REALattribute *attributes;
	long attributeCount;
#endif
} REALinterfaceDefinition;

typedef struct
{
	long version;						// use kCurrentREALControlVersion
	const char *name;					// name of the module
	REALmethodDefinition *methods;		// list of public module methods
	long methodCount;					// number of entries in the method list
	REALconstant* constants;			// list of constants
	long constantCount;					// number of constants in the list
	REALproperty *properties;			// Begin Added in RB2006r4 (control version 9 and later) -- Jun 23 2006 -- AJB (1)
	long propertyCount;					//
	REALstructure *structures;			//
	long structureCount;					// 
	REALenum *enums;					//
	long enumCount;						//
#ifndef PRE_2011R4_SDK
	REALattribute *attributes;
	long attributeCount;
#endif
} REALmoduleDefinition;

#if TARGET_OS_MAC && !COCOA
	typedef void (*REALEventCallback)(EventRecord *event, long param);
#endif

#define kCurrentREALDatabaseVersion 1

typedef void (*REALDataSourceInterfaceProc)(void);
typedef REALdbDatabase (*REALDataSourceProc)(Ptr data, int dataLen);

typedef struct dbDatabase dbDatabase;
typedef struct dbTable dbTable;
typedef struct dbCursor dbCursor;

typedef struct
{
	short year;
	short month;
	short day;
} dbDate;

typedef struct
{
	short hour;
	short minute;
	short second;
} dbTime;

typedef struct 
{
	short year;
	short month;
	short day;
	short hour;
	short minute;
	short second;
} dbTimeStamp;

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
	dbTypeInt64,		// 19
	dbTypeUnknown = 255
};

struct REALnewColumn 
{
	struct REALnewColumn *nextColumn;
	REALstring columnName;
	long columnType;				// dbFieldType
	long bAllowNULL;				// Specifies whether or not this column allow NULL values
	REALstring columnTypeString;	// Since 6.0, so we can specify type constraints such as "Decimal(5,2)"
};
typedef struct REALnewColumn REALnewColumn;

struct REALcolumnValue
{
	struct REALcolumnValue *nextColumn;
	REALstring columnName;
	REALstring columnValue;
	long columnType;
};
typedef struct REALcolumnValue REALcolumnValue;

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
	struct REALcolumnConstraints *left, *right;
	long columnOperation;
	long columnComparison;
	REALstring column;
	REALstring value;
};
typedef struct REALcolumnConstraints REALcolumnConstraints;

struct REALgetColumn
{
	struct REALgetColumn *next;
	REALstring column;
};
typedef struct REALgetColumn REALgetColumn;

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
	struct REALfieldUpdate *next;
	Ptr tableField;
	long tableFieldLen;
	Ptr recordKey;
	long recordKeyLen;
	REALstring value;
};
typedef struct REALfieldUpdate REALfieldUpdate;

typedef struct
{
	long version;
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

	// Drops a table
	//  tableName -- the table to drop
	void (*dropTable)( dbDatabase *db, REALstring tableName );

	// Drops a column
	//  tableName -- table that the column exists in
	//  columnName -- column to drop
	void (*dropColumn)( dbDatabase *db, REALstring tableName, REALstring columnName );

	// Renames a table
	//  oldTableName -- old name of the table
	//  newTableName -- new name of the table
	void (*alterTableName)( dbDatabase *db, REALstring oldTableName, REALstring newTableName );

	// Renames a column
	//  tableName -- table that the column exists in
	//  oldColumnName -- old name of column
	//  newColumnName -- new name of column
	void (*alterColumnName)( dbDatabase *db, REALstring tableName, REALstring oldColumnName, REALstring newColumnName );

	// Changes column type
	//  tableName -- table that the column exists in
	//  columnName -- the column to modify
	//  oldType -- old column type
	//  newType -- new column type 
	void (*alterColumnType)( dbDatabase *db, REALstring tableName, REALstring columnName, REALstring oldType, REALstring newType );

	// Changes a column's constraint, such as Primary Key, Mandatory field, etc..
	//  tableName -- table that column exists in
	//  columnName -- the column to modify
	//  whichConstraint -- "Primary Key", "Mandatory", etc... same text that displays in the
	//                     properties listbox in the database schema editor
	//  oldConstraint -- the old value (if boolean, this string will be either "True" or "False")
	//  newConstraint -- the new value (if boolean, this string will be either "True" or "False")
	void (*alterColumnConstraint)( dbDatabase *db, REALstring tableName, REALstring columnName, REALstring whichConstraint, REALstring oldConstraint, REALstring newConstraint );

	REALobject (*PrepareStatement)( dbDatabase *db, REALstring statement );

	REALproc unused1;
	REALproc unused2;
} REALdbEngineDefinition;

typedef struct
{
	long version;
	long forSystemUse;

	REALproc closeTable; // void (*closeTable)(dbTable *);
	REALproc tableCursor; // dbCursor *(*tableCursor)(dbTable *);
} REALdbTableDefinition;

struct REALcursorUpdate
{
	struct REALcursorUpdate *next;
	long fieldIndex;
	REALstring columnValue;
};
typedef struct REALcursorUpdate REALcursorUpdate;

typedef struct
{
	long version;
	long forSystemUse;

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
	Boolean (*CursorIsBOF)(dbCursor *);	// Determines whether or not we're at the beginning of the record set
	Boolean (*CursorIsEOF)(dbCursor *);	// Determines whether or not we're at the end of the record set
	void *dummy10;
} REALdbCursorDefinition;

typedef struct {
	long mVersion;
	const char *mConnectionType;
	REALdbEngineDefinition *mDBEngine;

	void *(*Connect)( const char *connectionData, const unsigned long dataLength );
	void (*Disconnect)( void *db );
	const char *(*GetLastError)( void *db );
} REALDatabaseConnectionDefinition;

#ifndef FieldOffset
	#if __GNUC__
		#include <stddef.h>
		#define FieldOffset(type, field) offsetof( type, field )
	#else
		#define FieldOffset(type, field) (long)(&((type *) 0)->field)
	#endif
#endif

#ifdef __cplusplus
	extern "C" {
#endif
void PluginEntry(void);
#ifdef __cplusplus
}
#endif

#endif
