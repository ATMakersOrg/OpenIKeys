//  IKString.h: interface for the IKString class.
//  It's basically CString with only the essentials.
//  Optional dynamic memory allocation.
//
//////////////////////////////////////////////////////////////////////

#if !defined(STRING_H_INCLUDED)
#define STRING_H_INCLUDED

#define MAX_STRING 256
#define IKSTRING_DYNAMIC 1

class IKString  
{
public:
	void ToACP();
	void ToUTF8();
	bool IsUTF8();
	int Substitute ( IKString s1, IKString s2 );
	void DecodeSpaces();
	void EncodeSpaces();
	TCHAR * GetBufferAddress();
	IKString();
	IKString(const TCHAR *s);
	IKString(BYTE *data, int len );
	IKString ( const IKString& src);  //  copy constructor
	//IKString ( IKString& src);

	virtual ~IKString();

	static int strlen(const TCHAR *psz);
	static int strncpy(TCHAR *dst,const TCHAR *src,const int nchar);
	static int strcat(TCHAR *dst,const TCHAR *src);
	static int strncat(TCHAR *dst,const TCHAR *src,const int nchar);
	static int strcmp(const TCHAR *s1,const TCHAR *s2);
	static int stricmp(const TCHAR *s1,const TCHAR *s2);
	static int strstr(const TCHAR *s1,const TCHAR *s2);
	static int strchr(const TCHAR *s1,const TCHAR c);
	static int strcpy(TCHAR *dst,const TCHAR *src);

	static void ConvertTToC(char* pszDest, const TCHAR* pszSrc);
	static void ConvertCToT(TCHAR* pszDest, const char* pszSrc);

	// overloaded assignment
	const IKString& operator=(const IKString& stringSrc);
	const IKString& operator=(TCHAR ch);
	const IKString& operator=(const TCHAR* psz);

	int GetLength();
	int GetByteCount();
	bool IsEmpty();
	void Empty();
	void Lower();
	void ReplaceChar ( TCHAR findThis, TCHAR subThis );
	int ReverseFind ( TCHAR c );
	int Find ( const TCHAR * lpsz );
	int Find ( TCHAR c );
	TCHAR GetAt ( int index );
	void SetAt ( int index, TCHAR c );
	void SetAt ( int index, IKString s );

	operator TCHAR *() const;       // as a C string
	operator const TCHAR *() const;       // as a C string
	//operator unsigned TCHAR *() const;       // as a Pascal string

	const IKString& operator+=(const TCHAR * lpsz);
	const IKString& operator+=(TCHAR ch);
	const IKString& operator+=(const IKString& string);

	friend IKString operator+(const IKString& string1, const IKString& string2);
	friend IKString operator+(const IKString& string, TCHAR ch);
	friend IKString operator+(TCHAR ch, const IKString& string);
	friend IKString operator+(const IKString& string, const TCHAR* psz);
	friend IKString operator+(const TCHAR* psz, const IKString& string);

	int Compare(const TCHAR* psz) const;         // straight character
	int CompareNoCase(const TCHAR* psz) const;   // ignore case

	//  string extraction
	IKString Mid(int nFirst) const;
	IKString Mid(int nFirst, int nCount) const;
	IKString Right(int nCount) const;
	IKString Left(int nCount) const;

	void TrimRight();
	void TrimLeft();

	void Format(const TCHAR * lpszFormat, ...);

private:

#if IKSTRING_DYNAMIC
	void Allocate(int n);
	void Free();
	TCHAR *m_data;
	int m_size;
#else
	TCHAR m_data[MAX_STRING];
#endif

};

// Compare helpers
bool operator==(const IKString& s1, const IKString& s2);
bool operator==(const IKString& s1, const TCHAR* s2);
bool operator==(const char* s1, const IKString& s2);
bool operator!=(const IKString& s1, const IKString& s2);
bool operator!=(const IKString& s1, const TCHAR* s2);
bool operator!=(const char* s1, const IKString& s2);
bool operator<(const IKString& s1, const IKString& s2);
bool operator<(const IKString& s1, const TCHAR* s2);
bool operator<(const char* s1, const IKString& s2);
bool operator>(const IKString& s1, const IKString& s2);
bool operator>(const IKString& s1, const TCHAR* s2);
bool operator>(const char* s1, const IKString& s2);
bool operator<=(const IKString& s1, const IKString& s2);
bool operator<=(const IKString& s1, const TCHAR* s2);
bool operator<=(const char* s1, const IKString& s2);
bool operator>=(const IKString& s1, const IKString& s2);
bool operator>=(const IKString& s1, const TCHAR* s2);
bool operator>=(const char* s1, const IKString& s2);

#endif // !defined(STRING_H_INCLUDED)
