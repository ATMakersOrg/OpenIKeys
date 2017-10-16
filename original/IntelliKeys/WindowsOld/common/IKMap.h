// IKMap.h: interface for the IKMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKMAP_H__87C62D2C_22DB_4636_A522_DF3FA5DF90F6__INCLUDED_)
#define AFX_IKMAP_H__87C62D2C_22DB_4636_A522_DF3FA5DF90F6__INCLUDED_


#include "IKString.h"

//#define MAP_MAX_STRINGS 300
#define IKMAP_DYNAMIC 1

class IKMap  
{
public:
	IKString GetPath();

	IKMap();
	virtual ~IKMap();
	IKString Lookup ( IKString key );
	bool Write ( IKString filename );
	bool Write ();
	bool Read ( IKString filename );
	void RemoveAll ();
	bool Remove ( IKString key );
	bool Add ( IKString key, IKString value );
	int	Count();
	bool GetNthPair(int n, IKString &key, IKString &value);
	IKMap & operator = ( const IKMap &rhs );
	IKMap ( const IKMap& src);  //  copy ctor
	void ModifyKey ( IKString oldKey, IKString newKey );

private:
	int Find ( IKString key );

	class pair
	{
	public:
		pair (){key=TEXT(""), value=TEXT("");}
		~pair (){key=TEXT(""), value=TEXT("");}

		pair ( const pair& src)
		{
			key = src.key;
			value = src.value;
		}

		pair & operator = ( const pair &rhs )
		{
			key = rhs.key;
			value = rhs.value;

			return *this;
		}


		IKString key;
		IKString value;
	};

#if IKMAP_DYNAMIC
	pair *m_pairs;
	void Allocate (int n);
	//void Free();
	int m_size;
#else
	pair m_pairs[MAP_MAX_STRINGS];
#endif
	int m_nPairs;

	IKString m_path;
};

#endif // !defined(AFX_IKMAP_H__87C62D2C_22DB_4636_A522_DF3FA5DF90F6__INCLUDED_)
