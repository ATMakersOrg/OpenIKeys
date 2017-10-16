// IKPrefs.cpp: implementation of the IKPrefs class.
//
//////////////////////////////////////////////////////////////////////

#include "IKCommon.h"
#include "IKPrefs.h"
#include "IKUtil.h"

#ifdef PLAT_MACINTOSH
#include <stdio.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void IKPrefs::SetValue ( IKString key, int val )	
{
	TCHAR buffer[500];
	MySprintf ( buffer, TEXT("%d"), val );
	Add ( key, buffer );
}

void IKPrefs::SetValue ( IKString key, float val )
{
	TCHAR buffer[500];
	MySprintf ( buffer, TEXT("%f"), val );
	Add ( key, buffer );
}

void IKPrefs::SetValue ( IKString key, TCHAR * val )
{
	TCHAR buffer[500];
	MySprintf ( buffer, TEXT("%s"), val );
	Add ( key, buffer );
}

int IKPrefs::GetValueInt ( IKString key, int defVal /* = 0 */ )
{
	IKString skey = key;
	skey.ReplaceChar(CHAR(' '),CHAR('_'));
	IKString sval = m_map.Lookup(skey);
	if (sval.Compare(TEXT(""))==0)
		return defVal;

	int val = IKUtil::StringToInt(sval);
	return val;
}

float IKPrefs::GetValueFloat ( IKString key, float defVal /* = 0 */ )
{
	IKString skey = key;
	skey.ReplaceChar(CHAR(' '),CHAR('_'));
	IKString sval = m_map.Lookup(skey);
	if (sval.Compare(TEXT(""))==0)
		return defVal;

	float val = IKUtil::StringToFloat(sval);
	return val;
}

IKString IKPrefs::GetValueString ( IKString key, IKString defVal /* = "" */ )
{
	IKString skey = key;
	skey.ReplaceChar(CHAR(' '),CHAR('_'));
	IKString sval = m_map.Lookup(skey);
	if (sval.Compare(TEXT(""))==0)
		return defVal;

	return sval;
}

bool IKPrefs::Read ( IKString filePath )
{
	return m_map.Read ( filePath );
}

void IKPrefs::Write ( IKString filePath )
{
	m_map.Write ( filePath );
}

void IKPrefs::Write ( )
{
	m_map.Write ( );
}

void IKPrefs::RemoveAll ( void )
{
	m_map.RemoveAll();
}


void IKPrefs::Add(IKString key, TCHAR *bval)
{
	IKString sval(bval);
	IKString skey = key;
	skey.ReplaceChar(CHAR(' '),CHAR('_'));
	m_map.Add(skey,sval);
}


IKString IKPrefs::GetPath()
{
	return m_map.GetPath();
}

IKPrefs & IKPrefs :: operator = ( const IKPrefs &rhs )
{
	m_map				= rhs.m_map;

	return *this;
}

IKPrefs::IKPrefs ()
{
}

IKPrefs::~IKPrefs ()
{
}

