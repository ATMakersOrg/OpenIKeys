/********************************************************************************
// Copyright (c) IntelliTools Inc., 1999. All Rights Reserved.
//
// Class	XIntelliKeys.h
//
// Purpose:	Declaration of generic static methods and constants used to
// 			send, edit (using Overlay Maker) and make overlays.
//
//
// 08/24/99	py,
//			ss	initial implementation toward xframe 1.0
// 12/08/99 dgs added CONTENT_OVERLAY_TYPE and GROUP_OVERLAY_TYPE so these are browseable and
//				attachable
// 05/12/00 sss Revised file types
*********************************************************************************/

#ifndef __XINTELLIKEYS_H__
#define __XINTELLIKEYS_H__

//#include "diblib.h"

#define STD_OVERLAY_EXTENSION TEXT("oms")
#define CONTENT_OVERLAY_EXTENSION TEXT("omc")
#define GROUP_OVERLAY_EXTENSION TEXT("omg")

#ifdef PLAT_MACINTOSH
	#define STD_OVERLAY_MACTYPE TEXT('Covl')
	#define CONTENT_OVERLAY_MACTYPE TEXT('Cpvl')
	#define GROUP_OVERLAY_MACTYPE TEXT('Cgrp')
#endif

	
///////////////////////////////////////////////////
// Forward declarations used for making overlays
class XSimpleKey;
typedef CArray<XSimpleKey *,XSimpleKey *> XSimpleKeyArray;
///////////////////////////////////////////////////

///////////////////////////////////////////////////
// Enumerated error codes
enum {
	kNoError = 0,
	kFileNotFound,
	kFileNotOverlay,
	kTransmitError,
	kIntelliKeysNotFound,
	kUnknownError
};

//enum the IkSpecialCharacters
enum IKSpecialCharacter {

IK_RETURN				=	204,			
IK_BACKSPACE			=	205,		
IK_SPACE				=	234,			
IK_TAB					=	206,			
IK_ESCAPE				=	207,			
IK_UP_ARROW				=	208,		
IK_DOWN_ARROW			=	209,		
IK_LEFT_ARROW			=	210,		
IK_RIGHT_ARROW			=	211,		
IK_LEFT_SHIFT			=	212,		
IK_RIGHT_SHIFT			=	213,		
IK_LEFT_CONTROL			=	214,		
IK_RIGHT_CONTROL		=	215,		
IK_LEFT_OPTION			=	216,		
IK_RIGHT_OPTION			=	217,		
IK_COMMAND				=	218,			
IK_OPTION				=	0,				
IK_CAPS_LOCK			=	219,		
IK_NP_PERIOD			=	194,		
IK_NP_PLUS				=	193,		
IK_NP_MINUS				=	202,			
IK_NP_TIMES				=	192,		
IK_NP_DIVIDE			=	191,		
IK_NP_EQUALS			=	190,		
IK_NUM_PAD_ENTER		=	201,		
IK_NUM_PAD				=	180,		
IK_NUM_LOCK				=	220,		
IK_F1					=	165,				
IK_F2					=	166,				
IK_F3					=	167,				
IK_F4					=	168,				
IK_F5					=	169,				
IK_F6					=	170,				
IK_F7					=	171,				
IK_F8					=	172,				
IK_F9					=	173,				
IK_F10					=	174,				
IK_F11					=	175,				
IK_F12					=	176,				
IK_F13					=	177,				
IK_F14					=	178,				
IK_F15					=	179,				
IK_INSERT				=	195,		
IK_DELETE				=	199,			
IK_HOME					=	196,				
IK_END					=	200,		
IK_PAGE_UP				=	197,			
IK_PAGE_DOWN			=	198,			
IK_RESET				=	221,				
	// Mouse Movements
	
IK_MOUSE_UP				=	222,		
IK_MOUSE_DOWN			=	223,		
IK_MOUSE_LEFT			=	224,		
IK_MOUSE_RIGHT			=	225,		
IK_MOUSE_UP_RIGHT		=	226,	
IK_MOUSE_UP_LEFT		=	227,	
IK_MOUSE_DOWN_RIGHT		=	228,	
IK_MOUSE_DOWN_LEFT		=	229,		
IK_BUTTON_CLICK			=	230,		
IK_BUTTON_DOUBLE_CLICK	= 	231,	
IK_BUTTON_DOWN			=	232,		
IK_BUTTON_UP			=	233,			
IK_BUTTON_TOGGLE		=	162,

// Miscellaneous
IK_NON_REPEATING		=	51	
};


///////////////////////////////////////////////////
class XIntelliKeys
{
public:
	static bool		IsIntelliKeysConnected ( void );
	static bool		IsOverlayMakerInstalled ( void );
	static bool		IsSendableOverlayFile ( LPCSTR pFilePath );
	static bool		IsEditableOverlayFile ( LPCSTR pFilePath );
	
	static int		SendOverlay ( LPCSTR pFilePath, bool bReportErrors = FALSE );
	static void		EditOverlay ( LPCSTR pFilePath );
	
	static bool		MakeOverlayFile ( 
		LPCSTR pOutFilePath,
		const XSimpleKeyArray &array,
		LPCSTR pSwitchContent1,
		LPCSTR pSwitchContent2,
		LPCSTR pPathToContentFile,
		bool bFitToKeyguard = TRUE );
				
	static void		AddSpecialCharacter ( CString& theString, 
										IKSpecialCharacter theIKchar);
};


class XSimpleKey
{
public:
	XSimpleKey ( LPCSTR pContent,
		LPCSTR pLabel,
		LPCSTR pPathToPicture = NULL,
		bool bAddNonRepeating = FALSE,
		bool bAddEnter = FALSE );

	virtual	bool	SerializeDIB ( CArchive &ar );

#ifdef PLAT_MACINTOSH
	virtual PicHandle	GetPicHandle( void ) { return NULL; }
#endif

	CString m_strContent;
	const CString m_strLabel;
	const CString m_strPathToPicture;
	const bool m_bAddEnter;				/* yes/no [Enter] */
	const bool m_bAddNonRepeating;		/* yes/no [Non-Repeating]  */
};

#define MAX_KEYGUARD_KEYS 20

#define MAX_OVL_SIZE 0x7200

#define OVL_NAME_LEN 32			/* overlay name/descriptor */
#define NUM_MACHINES 16			/* number of possible Cable_vals */
#define NUM_LEVELS 16

typedef struct OverlayHeader
 {		/* overlay header */
	BYTE name[ OVL_NAME_LEN ];		/* descriptor */
	BYTE number;					/* of overlay */
	BYTE num_levels;				/* in overlay */
	BYTE setup_flag;				/* 1 if setup type, 0 if other type */
	BYTE coord_mode;
	BYTE reserved[ 4 ];				/* for the future */
	BYTE base_level[ NUM_MACHINES ];	/* which level each machine starts at */
	BYTE mouse_level[ NUM_MACHINES ];	/* which level each machine starts at */
	WORD offset[ NUM_LEVELS ];		/* offset from start of data to each level */
} OverlayHeader;						/* followed by compressed key tables, string data */

#endif // __XINTELLIKEYS_H__