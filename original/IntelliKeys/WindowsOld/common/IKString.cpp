// IKString.cpp: implementation of the String class.
//
//////////////////////////////////////////////////////////////////////

#include "IKCommon.h"
#include "IKString.h"
#include "IKUtil.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
//  Copy ctor

IKString::IKString ( const IKString& src)
{
#if IKSTRING_DYNAMIC
	m_data = 0; m_size = 0;
#endif

	Empty();
	
	int i = IKString::strlen(src.m_data);
	
#if IKSTRING_DYNAMIC
	Allocate(i+1);
#else
	if (i>MAX_STRING-1)
		i = MAX_STRING-1;
#endif

	if(i>0)
		IKString::strncpy(m_data,src.m_data,i);

	m_data[i] = TEXT('\0');
}

//////////////////////////////////////////////////////////////////////

IKString::IKString()
{
#if IKSTRING_DYNAMIC
	m_data = 0; m_size = 0;
#endif

	Empty();  //  empty allocates at least one byte.
}

//////////////////////////////////////////////////////////////////////

IKString::IKString(TCHAR *psz)
{
#if IKSTRING_DYNAMIC
	m_data = 0; m_size = 0;
#endif

	Empty();

	int i = IKString::strlen(psz);
	
#if IKSTRING_DYNAMIC
	Allocate(i+1);
#else
	if (i>MAX_STRING-1)
		i = MAX_STRING-1;
#endif

	if(i>0)
		IKString::strncpy(m_data,psz,i);

	m_data[i] = TEXT('\0');
}

//////////////////////////////////////////////////////////////////////

IKString::IKString(BYTE *data, int len)
{
#if IKSTRING_DYNAMIC
	m_data = 0; m_size = 0;
#endif

#if IKSTRING_DYNAMIC
	Allocate(len+1);
#endif

	Empty();

	//  no string
	if (data==NULL)
	{
		return;
	}

	//  is it Unicode?
	int options;
	bool bUnicode = !!IsTextUnicode ( data, len, &options );

	if (bUnicode)
	{
		//  string is unicode.
#ifdef UNICODE
		int len2 = len/2;
		int i;
		for (i=0;i<len2;i++)
		{
			m_data[i] = (TCHAR) *(&(data[i*2]));
		}
		m_data[i] = TEXT('\0');
#else
		//  convert unicode to single
		//  TODO, but unlikely since we're always building
		//  Unicode.
		//IKASSERT(false);
#endif
	}
	else
	{
		//  string is not unicode.
#ifdef UNICODE
		//  convert single to unicode.
		int result = MultiByteToWideChar (
							CP_ACP,				// code page
							MB_PRECOMPOSED,		// character-type options
							(const char *) data,// string to map
							len,				// number of bytes in string
							m_data,				// wide-character buffer
							sizeof(m_data)		// size of buffer
							);

		if (result==0)
			m_data[0] = TEXT('\0');
		else
			m_data[len] = TEXT('\0');

#else
		int i;
		for (i=0;i<len;i++)
		{
			m_data[i] = (char) data[i];
		}
		m_data[i] = '\0';
#endif
	}
}


//////////////////////////////////////////////////////////////////////

IKString::~IKString()
{

#if IKSTRING_DYNAMIC
	Free();
#endif

}

//////////////////////////////////////////////////////////////////////

int IKString::strchr(const TCHAR *s1,const TCHAR c)
{	
	if (s1==NULL)
		return -1;

	for (int i=0;i<IKString::strlen(s1);i++)
	{
		if (c == s1[i])
			return i;
	}

	return -1;
}


//////////////////////////////////////////////////////////////////////

int IKString::strstr (const TCHAR *target, const TCHAR *candidate)
{
	//  bad params
	if (target==NULL)
		return -1;
	if (candidate==NULL)
		return -1;

	//  length of candidate
	int lc = IKString::strlen(candidate);

	//  candidate too long
	if(lc > IKString::strlen(target))
		return -1;

	int nloop = IKString::strlen(target) - lc + 1;
	for (int n=0;n<nloop;n++)
	{
		bool matched = true;
		for (int ic=0;ic<lc;ic++)
		{
			if (target[n+ic]!=candidate[ic])
			{
				matched = false;
				break;
			}
		}
		if (matched)
			return n;
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////

int IKString::stricmp(const TCHAR *s1,const TCHAR *s2)
{	
	if (s1==NULL)
		return -1;
	if (s2==NULL)
		return 1;

	int l1 = IKString::strlen(s1);
	int l2 = IKString::strlen(s2);

	int i = 0;
	while (true)
	{
		//  ran out of s1 first
		if (i>=l1 && i<l2)
			return -1;

		//  ran out of s2 first
		if (i>=l2 && i<l1)
			return 1;

		//  ran out of both at the same time,
		//  this means they are equal.
		if (i>=l1 && i>=l2)
			return 0;

		//  get both chars
		TCHAR c1 = s1[i];
		TCHAR c2 = s2[i];

		//  uppercase them
		if (c1>='a' && c1<='z')
			c1 = c1 - 'a' + 'A';
		if (c2>='a' && c2<='z')
			c2 = c2 - 'a' + 'A';

		//  this one is smaller
		if (c1<c2)
			return -1;

		//  this one is larger
		if (c1>c2)
			return 1;

		i++;
	}

	//  can't be here
	IKASSERT(false);
	return 0;
}

//////////////////////////////////////////////////////////////////////

int IKString::strcmp(const TCHAR *s1,const TCHAR *s2)
{
	if (s1==NULL)
		return -1;
	if (s2==NULL)
		return 1;

	int l1 = IKString::strlen(s1);
	int l2 = IKString::strlen(s2);

	int i = 0;
	while (true)
	{
		//  ran out of s1 first
		if (i>=l1 && i<l2)
			return -1;

		//  ran out of s2 first
		if (i>=l2 && i<l1)
			return 1;

		//  ran out of both at the same time,
		//  this means they are equal.
		if (i>=l1 && i>=l2)
			return 0;

		//  this one is smaller
		if (s1[i]<s2[i])
			return -1;

		//  this one is larger
		if (s1[i]>s2[i])
			return 1;

		i++;
	}

	//  can't be here
	IKASSERT(false);
	return 0;

}

//////////////////////////////////////////////////////////////////////

int IKString::strlen(const TCHAR *psz)
{
	if (psz==NULL)
		return 0;

	int i = 0;
	while (psz[i]!=TEXT('\0'))
		i++;

	return i;
}

//////////////////////////////////////////////////////////////////////

int IKString::strncpy(TCHAR *dst,const TCHAR *src, const int nchar)
{
	if (dst==NULL)
		return 0;
	if (src==NULL)
		return 0;

	int nc = nchar;
	int nsrc = IKString::strlen(src);
	if (nsrc<nc)
		nc = nsrc;

	int i;
	for (i=0;i<nc;i++)
		dst[i] = src[i];
	dst[i] = TEXT('\0');

	return nc;
}

//////////////////////////////////////////////////////////////////////

int IKString::strcpy(TCHAR *dst,const TCHAR *src)
{
	if (dst==NULL)
		return 0;
	if (src==NULL)
		return 0;

	int nc = IKString::strlen(src);

	int i;
	for (i=0;i<nc;i++)
		dst[i] = src[i];
	dst[i] = TEXT('\0');

	return nc;
}

//////////////////////////////////////////////////////////////////////

int IKString::strncat(TCHAR *dst,const TCHAR *src, const int nchar)
{
	if (dst==NULL)
		return 0;
	if (src==NULL)
		return 0;

	int nc = nchar;
	int nsrc = IKString::strlen(src);
	if (nsrc<nc)
		nc = nsrc;

	int l = IKString::strlen(dst);

	int i;
	for (i=0;i<nc;i++)
		dst[l+i] = src[i];
	dst[l+i] = TEXT('\0');

	return nc;
}

//////////////////////////////////////////////////////////////////////

int IKString::strcat(TCHAR *dst,const TCHAR *src)
{
	if (dst==NULL)
		return 0;
	if (src==NULL)
		return 0;

	int nc = IKString::strlen(src);

	int l = IKString::strlen(dst);

	int i;
	for (i=0;i<nc;i++)
		dst[l+i] = src[i];
	dst[l+i] = TEXT('\0');

	return nc;
}


//////////////////////////////////////////////////////////////////////

const IKString& IKString::operator =(const IKString& stringSrc)
{
	int i = 0;
	if (&stringSrc)
		i = IKString::strlen(stringSrc.m_data);
	
#if IKSTRING_DYNAMIC
	Allocate(i+1);
#else
	if (1>MAX_STRING-1)
		i = MAX_STRING-1;
#endif

	if(i>0)
		IKString::strncpy(m_data,stringSrc.m_data,i);
		
	m_data[i] = TEXT('\0');

	return *this;
}


//////////////////////////////////////////////////////////////////////

const IKString& IKString::operator =(const TCHAR * psz)
{	
	int i = 0;
	if (&psz)
		i = IKString::strlen(psz);

#if IKSTRING_DYNAMIC
	Allocate(i+1);
#else
	if (1>MAX_STRING-1)
		i = MAX_STRING-1;
#endif

	if(i>0)
		IKString::strncpy(m_data,psz,i);
		
	m_data[i] = TEXT('\0');
			
	return *this;
}

//////////////////////////////////////////////////////////////////////

const IKString& IKString::operator =(TCHAR ch)
{
#if IKSTRING_DYNAMIC
	Allocate(2);
#endif

	m_data[0] = ch;
	m_data[1] = TEXT('\0');
	return *this;
}

//////////////////////////////////////////////////////////////////////

int IKString::GetLength()
{
	return IKString::strlen(m_data);
}

//////////////////////////////////////////////////////////////////////

int IKString::GetByteCount()
{
#ifdef UNICODE
	return IKString::strlen(m_data) * 2;
#else
	return IKString::strlen(m_data);
#endif

}

//////////////////////////////////////////////////////////////////////

bool IKString::IsEmpty()
{
	return (IKString::strlen(m_data)==0);
}

//////////////////////////////////////////////////////////////////////

void IKString::Empty()
{
#if IKSTRING_DYNAMIC
	if (m_data==0)
		Allocate(1);
#endif

	m_data[0] = TEXT('\0');
}

//////////////////////////////////////////////////////////////////////

IKString::operator const TCHAR*() const
{ 
	return (const TCHAR*)m_data; 
}

//////////////////////////////////////////////////////////////////////

IKString::operator TCHAR*() const
{ 
	return (TCHAR*)m_data; 
}

//////////////////////////////////////////////////////////////////////

int IKString::Compare(const TCHAR* psz) const
{ 
	return IKString::strcmp(m_data, psz); 
}

//////////////////////////////////////////////////////////////////////

int IKString::CompareNoCase(const TCHAR* psz) const
{
	return IKString::stricmp(m_data, psz);
}

 bool operator==(const IKString& s1, const IKString& s2)
	{ return s1.Compare(s2) == 0; }
 bool operator==(const IKString& s1, const TCHAR* s2)
	{ return s1.Compare(s2) == 0; }
 bool operator==(const TCHAR* s1, const IKString& s2)
	{ return s2.Compare(s1) == 0; }
 bool operator!=(const IKString& s1, const IKString& s2)
	{ return s1.Compare(s2) != 0; }
 bool operator!=(const IKString& s1, const TCHAR* s2)
	{ return s1.Compare(s2) != 0; }
 bool operator!=(const TCHAR* s1, const IKString& s2)
	{ return s2.Compare(s1) != 0; }
 bool operator<(const IKString& s1, const IKString& s2)
	{ return s1.Compare(s2) < 0; }
 bool operator<(const IKString& s1, const TCHAR* s2)
	{ return s1.Compare(s2) < 0; }
 bool operator<(const TCHAR* s1, const IKString& s2)
	{ return s2.Compare(s1) > 0; }
 bool operator>(const IKString& s1, const IKString& s2)
	{ return s1.Compare(s2) > 0; }
 bool operator>(const IKString& s1, const TCHAR* s2)
	{ return s1.Compare(s2) > 0; }
 bool operator>(const TCHAR* s1, const IKString& s2)
	{ return s2.Compare(s1) < 0; }
 bool operator<=(const IKString& s1, const IKString& s2)
	{ return s1.Compare(s2) <= 0; }
 bool operator<=(const IKString& s1, const TCHAR* s2)
	{ return s1.Compare(s2) <= 0; }
 bool operator<=(const TCHAR* s1, const IKString& s2)
	{ return s2.Compare(s1) >= 0; }
 bool operator>=(const IKString& s1, const IKString& s2)
	{ return s1.Compare(s2) >= 0; }
 bool operator>=(const IKString& s1, const TCHAR* s2)
	{ return s1.Compare(s2) >= 0; }
 bool operator>=(const TCHAR* s1, const IKString& s2)
	{ return s2.Compare(s1) <= 0; }


//////////////////////////////////////////////////////////////////////

const IKString& IKString::operator+=(TCHAR * lpsz)
{
	int l1 = IKString::strlen(m_data);
	int l2 = IKString::strlen(lpsz);
	
#if IKSTRING_DYNAMIC
	Allocate(l1+l2+1);
#else
	if (l1+l2>=MAX_STRING)
		l2 = MAX_STRING - l1 - 1;
#endif

	if (l2>0)
		IKString::strncat(m_data,lpsz,l2);
		
	m_data[l1+l2] = TEXT('\0');
	  
	return *this;
}

//////////////////////////////////////////////////////////////////////

const IKString& IKString::operator+=(TCHAR ch)
{
	int i = IKString::strlen(m_data);
	
#if IKSTRING_DYNAMIC
	Allocate(i+2);
#endif
	
	m_data[i] = ch;
	m_data[i+1] = 0;
	
	return *this;
}

//////////////////////////////////////////////////////////////////////

const IKString& IKString::operator+=(const IKString& string)
{
	int l1 = IKString::strlen(m_data);
	int l2 = IKString::strlen(string.m_data);
	
#if IKSTRING_DYNAMIC
	Allocate(l1+l2+1);
#else
	if (l1+l2>=MAX_STRING)
		l2 = MAX_STRING - l1 - 1;
#endif

	if (l2>0)
		strncat(m_data,string.m_data,l2);
		
	m_data[l1+l2] = TEXT('\0');
	  
	return *this;
}

//////////////////////////////////////////////////////////////////////

TCHAR IKString::GetAt(int index)
{
	if (index>=0 && index <IKString::strlen(m_data))
		return m_data[index];
	return 0;
}

//////////////////////////////////////////////////////////////////////

void IKString::SetAt ( int index, TCHAR c )
{
	if (index>=0 && index <IKString::strlen(m_data))
		m_data[index] = c;
}

//////////////////////////////////////////////////////////////////////

void IKString::SetAt ( int index, IKString s )
{
	if (index>=0 && index <IKString::strlen(m_data))
	{
		TCHAR c = s.GetAt(0);
		SetAt ( index, c );
	}
}

//////////////////////////////////////////////////////////////////////

int IKString::Find(TCHAR c)
{
	for (int i=0;i<GetLength();i++)
	{
		if (m_data[i] == c)
			return i;
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////

void IKString::ReplaceChar(TCHAR findThis, TCHAR subThis)
{
	for (int i=0;i<GetLength();i++)
	{
		if (m_data[i]==findThis)
			m_data[i] = subThis;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Very simple sub-string extraction

IKString IKString::Mid(int nFirst) const
{
	int l = IKString::strlen(m_data);
	return Mid(nFirst, l - nFirst);
}

//////////////////////////////////////////////////////////////////////

IKString IKString::Mid(int nFirst, int nCount) const
{
	// out-of-bounds requests return sensible things
	int l = IKString::strlen(m_data);
	if (nFirst + nCount > l)
		nCount = l - nFirst;
	if (nFirst > l)
		nCount = 0;

	IKString dest;
	for (int i=0;i<nCount;i++)
		dest += m_data[nFirst+i];

	return dest;
}

//////////////////////////////////////////////////////////////////////

IKString IKString::Right(int nCount) const
{
	int l = IKString::strlen(m_data);

	if (nCount > l)
	{
		nCount = l;
	}

	IKString dest;
	for (int i=0; i < nCount; i++)
	{
		dest += m_data[i + l - nCount];
	}
	return dest;

}

//////////////////////////////////////////////////////////////////////

IKString IKString::Left(int nCount) const
{
	int l = IKString::strlen(m_data);

	if (nCount > l)
	{
		nCount = l;
	}

	IKString dest;
	for (int i=0; i < nCount; i++)
	{
		dest += m_data[i];
	}
	dest += TEXT('\0');

	return dest;
}

//////////////////////////////////////////////////////////////////////

void IKString::Lower()
{
	for (int i=0;i<(int)IKString::strlen(m_data);i++)
	{
		if (m_data[i]>=TEXT('A') && m_data[i]<=TEXT('Z'))
			m_data[i] = m_data[i] - TEXT('A') + TEXT('a');
	}
}

//////////////////////////////////////////////////////////////////////

int IKString::Find ( TCHAR * lpsz )
{
	return IKString::strstr(m_data,lpsz);
}


//////////////////////////////////////////////////////////////////////

int IKString::ReverseFind ( TCHAR c )
{
	for (int i=0;i<(int)IKString::strlen(m_data);i++)
	{
		int j = IKString::strlen(m_data) - i - 1;
		if (m_data[j] == c)
			return j;
	}

	return -1;
}



//////////////////////////////////////////////////////////////////////

void IKString::ConvertTToC(char* pszDest, const TCHAR* pszSrc)
{
	int l=0;
	while (pszSrc[l]!=TEXT('\0'))
		l++;

	int i=0;
	for(i = 0; i < l; i++)
		pszDest[i] = (char) pszSrc[i];
	pszDest[i] = '\0';
}

//////////////////////////////////////////////////////////////////////

void IKString::ConvertCToT(TCHAR* pszDest, const char* pszSrc)
{
	int l=0;
	while (pszSrc[l]!='\0')
		l++;

	int i;
	for(i = 0; i < l; i++)
		pszDest[i] = (TCHAR) pszSrc[i];
	pszDest[i] = TEXT('\0');
}

//////////////////////////////////////////////////////////////////////

TCHAR * IKString::GetBufferAddress()
{
	return m_data;
}


//////////////////////////////////////////////////////////////////////

void IKString::Format(const TCHAR * lpszFormat, ...)
{
#if IKSTRING_DYNAMIC
	Allocate(1024);  //  seems arbitrary.  How do we find out in advance?
#endif

#ifdef PLAT_WINDOWS

	va_list args;
	va_start(args, lpszFormat);

	int nBuf;

#if IKSTRING_DYNAMIC
	nBuf = _vsnprintf(m_data, m_size*sizeof(TCHAR), lpszFormat, args);
#else
	nBuf = _vsnprintf(m_data, sizeof(m_data) / sizeof(TCHAR), lpszFormat, args);
#endif
	m_data[nBuf] = 0;

	va_end(args);

#else
#endif
}

//////////////////////////////////////////////////////////////////////

void IKString::EncodeSpaces()
{
	//  replace all spaces with %20.

	IKString s;

	for (int i=0;i<GetLength();i++)
	{
		if (m_data[i]==' ')
		{
			s+= TEXT('%');
			s+= TEXT('2');
			s+= TEXT('0');
		}
		else
		{
			s += m_data[i];
		}
	}

	*this = s;
	//strcpy(m_data,(TCHAR *)s);
}

//////////////////////////////////////////////////////////////////////

void IKString::DecodeSpaces()
{
	//  replace all %20 with spaces

	IKString s1 = Mid(0);
	IKString s2;

	int i;
	while ((i = s1.Find(TEXT("%20"))) != -1)
	{
		s2 += s1.Left(i);
		s2 += TEXT(' ');
		s1 = s1.Mid(i+3);
	}
	if (s1.GetLength() > 0)
	{
		s2 += s1;
	}

	*this = s2;
	//strcpy(m_data,(TCHAR *)s2);

}

//////////////////////////////////////////////////////////////////////

void IKString::TrimRight()
{
	int i = GetLength();
	while ( (m_data[i-1]==TEXT(' ') || m_data[i-1]==TEXT('\t')) && i>0)
		i--;

	IKString result = Left(i);
	strcpy(m_data,(TCHAR *)result);
}

//////////////////////////////////////////////////////////////////////

void IKString::TrimLeft()
{
	int i = 0;
	while ( (m_data[i]==TEXT(' ') || m_data[i]==TEXT('\t')) && i<GetLength())
		i++;

	IKString result = Mid(i);
	strcpy(m_data,(TCHAR *)result);
}


int IKString::Substitute ( IKString s1, IKString s2 )
{
	//  do  case-insensitive string substitution

	IKString src = *this;
	src.Lower();
	IKString target = s1;
	target.Lower();

	int i = src.Find(target);
	if (i>=0)
	{
		IKString dst;
		dst += (*this).Left(i);
		dst += s2;
		dst += (*this).Mid(i+s1.GetLength());
		*this = dst;
	}

	return i;
}


#if IKSTRING_DYNAMIC

//////////////////////////////////////////////////////////////////////

void IKString::Allocate(int n)
{	
	//  how much is there now?
	int l = 0;
	if (m_data)
		l = strlen(m_data);
		
	//  if that's enough, do nothing.
	if (n-1<l)
		return;
		
	//  allocate a new amount
	TCHAR *pNew = new TCHAR[n+1];
	m_size = n+1;
	
	//  copy the old string
	pNew[0] = TEXT('\0');
	if (m_data)
		strcpy(pNew, m_data);
	
	//  swap the pointers
	TCHAR *pOld = m_data;
	m_data = pNew;
	
	//  delete the old
	if (pOld)
		delete [] pOld;
}

//////////////////////////////////////////////////////////////////////

void IKString::Free()
{
	//  don't do this twice
	if (m_data==NULL)
		return;

	delete [] m_data;
	m_data = NULL;
	m_size = 0;
}

#endif  //  IKSTRING_DYNAMIC


IKString operator+(const IKString& string1, const IKString& string2)
{
	IKString s;
	s = string1;
	s += string2;
	return s;
}

IKString operator+(const IKString& string, TCHAR ch)
{
	IKString s;
	s = string;
	s += ch;
	return s;
}

IKString operator+(TCHAR ch, const IKString& string)
{
	IKString s;
	s = ch;
	s += string;
	return s;
}

IKString operator+(const IKString& string, const TCHAR* psz)
{
	IKString s;
	s = string;
	s += (TCHAR *)psz;
	return s;
}

IKString operator+(const TCHAR* psz, const IKString& string)
{
	IKString s;
	s = psz;
	s += string;
	return s;
}



bool IKString::IsUTF8()
{
	//  figure out if this is UTF8 by converting to UNICODE
	//  and back and see if we get the same thing.

	int len = GetLength();
	WCHAR wide[1024];
	int result = MultiByteToWideChar(CP_UTF8, 0, m_data, len, wide, 1024);
	wide[result] = 0;

	char narrow[1024];
	int result2 = WideCharToMultiByte(CP_UTF8, 0, wide, result, narrow, 1024, 0, 0);
	narrow[result2] = 0;

	return (Compare(narrow) == 0);
}

void IKString::ToUTF8()
{
	if (!IsUTF8())
	{
		int len = GetLength();
		WCHAR wide[1024];
		int result = MultiByteToWideChar(CP_ACP, 0, m_data, len, wide, 1024);
		//int result = MultiByteToWideChar ( IKUtil::GetCodePage(), 0, m_data, len, wide, 1024 );
		wide[result] = 0;

		char narrow[1024];
		int result2 = WideCharToMultiByte(CP_UTF8, 0, wide, result, narrow, 1024, 0, 0);
		narrow[result2] = 0;
		
		*this = IKString(narrow);
	}
}

void IKString::ToACP()
{
	int len = GetLength();
	WCHAR wide[1024];
	int result = MultiByteToWideChar(CP_UTF8, 0, m_data, len, wide, 1024);
	wide[result] = 0;

	char narrow[1024];
	int result2 = WideCharToMultiByte(CP_ACP, 0, wide, result, narrow, 1024, 0, 0);
	//int result2 = WideCharToMultiByte (  IKUtil::GetCodePage(), 0, wide, result, narrow, 1024, 0, 0);
	narrow[result2] = 0;
		
	*this = IKString(narrow);
}
