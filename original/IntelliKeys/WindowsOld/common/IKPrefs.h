// IKPrefs.h: interface for the IKPrefs class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKPREFS_H__FD0D207D_8997_4F48_8F0E_B9CAAD102D4B__INCLUDED_)
#define AFX_IKPREFS_H__FD0D207D_8997_4F48_8F0E_B9CAAD102D4B__INCLUDED_


#include "IKMap.h"
#include "IKString.h"	// Added by ClassView

class IKPrefs
{
public:
	IKString GetPath();

	void		Add				( IKString key, TCHAR *bval);
	void		SetValue		( IKString key, int val );		
	void		SetValue		( IKString key, float val );		
	void		SetValue		( IKString key, TCHAR * val );
	int			GetValueInt		( IKString key, int defVal = 0 );		
	float		GetValueFloat	( IKString key, float defVal = 0 );
	IKString	GetValueString	( IKString key, IKString defVal = TEXT("") );
	bool		Read			( IKString filePath );
	void		Write			( IKString filePath );
	void		Write			( void );
	void		RemoveAll		( void );

	IKPrefs & operator = ( const IKPrefs &rhs );

	IKPrefs ();  //  copy ctor
	IKPrefs ( const IKPrefs& src);  //  copy ctor
	~IKPrefs ();
	
private:
	
	IKMap		m_map;

};

#endif // !defined(AFX_IKPREFS_H__FD0D207D_8997_4F48_8F0E_B9CAAD102D4B__INCLUDED_)
