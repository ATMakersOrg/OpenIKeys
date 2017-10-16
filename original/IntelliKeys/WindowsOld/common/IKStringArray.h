//  IKStringArray.h: interface for the IKStringArray class.
//  It's basically CStringArray with only the essentials,
//
//////////////////////////////////////////////////////////////////////

#if !defined(STRINGARRAY_H_INCLUDED)
#define STRINGARRAY_H_INCLUDED

class IKString;


class IKStringArray
{

public:
	void Remove ( const IKString &string );
	void AddNoDup ( IKString &string );

	IKStringArray();
	virtual ~IKStringArray();

	IKStringArray ( const IKStringArray& src);  //  copy constructor
	const IKStringArray& operator=(const IKStringArray& src);  //  assigment
	IKString operator [] ( int i );

	void Add ( IKString &string );
	void Add ( TCHAR *pString );
	void RemoveAll ();
	IKString GetAt (int i) const;
	int GetSize () const;
	void Sort ();
	int Find(const IKString &string);

private:

	void Allocate (int n );
	void Free ();

	IKString * m_array;
	int m_allocated;
	int m_used;
};

#endif // !defined(STRINGARRAY_H_INCLUDED)
