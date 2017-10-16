// IKMap.cpp: implementation of the IKMap class.

//

//////////////////////////////////////////////////////////////////////



#include "IKCommon.h"

#include "IKMap.h"

#include "IKFile.h"

#include "IKUtil.h"



//////////////////////////////////////////////////////////////////////

// Construction/Destruction

//////////////////////////////////////////////////////////////////////



IKMap::IKMap()

: m_nPairs(0)

#if IKMAP_DYNAMIC

, m_pairs(0), m_size(0)

#endif

{

#if IKMAP_DYNAMIC

	Allocate (100);

#endif

	m_nPairs = 0;

}



IKMap::~IKMap()

{

#if IKMAP_DYNAMIC

	if (m_pairs)

		delete [] m_pairs;

	m_pairs = NULL;

	m_size = 0;

	m_nPairs = 0;

#else

	RemoveAll();

#endif



	m_path = TEXT("");

}



bool IKMap::Add(IKString key, IKString value)

{

	//  reject any keys that have spaces

	if (key.Find(TEXT(" "))!= -1)

		return false;



	int i = Find(key);

	if(i>=0)

	{

		m_pairs[i].value = value;

		return true;

	}



#if IKMAP_DYNAMIC

#else

	if (m_nPairs+1>MAP_MAX_STRINGS)

		return false;

#endif



#if IKMAP_DYNAMIC

	Allocate(m_nPairs+1);

#endif



	m_nPairs++;

	m_pairs[m_nPairs-1].key   = key;

	m_pairs[m_nPairs-1].value = value;



	return true;

}



bool IKMap::Remove(IKString key)

{

	int i = Find(key);

	if (i>=0)

	{

		//  move everything above this down one

		for (int j=i;j<m_nPairs;j++)

		{

			m_pairs[j] = m_pairs[j+1];

		}

		m_nPairs--;

		return true;

	}



	return false;

}



void IKMap::RemoveAll()

{

#if IKMAP_DYNAMIC

	if (m_pairs)

		delete [] m_pairs;

	m_pairs = NULL;

	m_size = 0;

	m_nPairs = 0;

	Allocate (100);

#else

	for (int i=0;i<m_nPairs;i++)

	{

		m_pairs[i].key   = TEXT("");

		m_pairs[i].value = TEXT("");

	}

#endif

	m_nPairs = 0;

}



bool IKMap::Read(IKString filename)

{	

    //NSLog(@"IKMap::Read - File: [%s]", (char*)filename);
    
	//  save the filename

	m_path = filename;



	//  make it writeable

	//  TODO:  I made this zero.  Why?

	IKFile::MakeWritable(filename,0);



	//  open the file

	IKFile f;

	bool bOpened = f.Open ( filename, IKFile::modeRead);

	if (!bOpened)

		return false;



	//  get file length

	int nBytes = f.GetLength();



	//  determine if file is unicode

	int nc = 1;

	if (f.IsUnicode())

	{

		nc = 2;

		//  skip first two bytes

		f.Seek(2,IKFile::begin);

		nBytes -= 2;

	}



	//  read the contents.

	//  add a line feed at the end.



	int nChars = (nBytes/nc)+1;

	TCHAR *chars = new TCHAR[nChars+1];

	int nread = f.Read((void *)chars, nBytes);

	f.Close();



	//  add a line ending

	chars[nChars-1] = ('\n');

	chars[nChars  ] = 0;



	//  march thru the chars



	IKString strLine = "";

	for (int j=0;j<nChars;j++)

	{

		TCHAR c = chars[j];

		switch (c)

		{

		case ('\r'):

		case ('\n'):

			if (!strLine.IsEmpty())

			{

				//  trim away spaces and comments

				int i = strLine.Find((';'));

				if (i>=0)

					strLine = strLine.Left(i);

				strLine.TrimLeft();

				strLine.TrimRight();



				if (!strLine.IsEmpty())

				{

					//  get the key and value

					IKString key, value;

					i=0;

					while (i<strLine.GetLength() && strLine.GetAt(i)!= (' ') && strLine.GetAt(i)!= ('\t'))

						i++;

					key = strLine.Left(i);

					value = strLine.Mid(i+1);

					value.TrimLeft();

					if (!key.IsEmpty())
					{
                        //NSLog(@"IKMap::Read - setting: [%s]=[%s]", (char*)key, (char*)value);
                        
						Add ( key, value );

					}

				}



				strLine = "";

			}

			break;



		default:

			strLine += c;

			break;

		}

	}

	

	if (chars)

		delete [] chars;



	return true;

}



bool IKMap::Write()

{

	if (!m_path.IsEmpty())

		return Write(m_path);

	return false;

}





bool IKMap::Write(IKString filename)

{

	//  save the filename

	m_path = filename;

    //NSLog(@"IKMap::Write - File: [%s]", (char*)filename);

	

	//  delete existing file first

	try {

	IKFile::Remove(filename);

	}

	catch (...)

	{

	}



	//  create the file

	IKFile f;

	bool bOpened = f.Open ( filename, IKFile::modeWrite | IKFile::modeCreate);

	if (!bOpened)

		return false;



	//  write a unicode tag in the Windows build

	if (DATAI(TEXT("Save_Text_As_UTF8"),0)==1)
	{
		f.MarkAsUTF8();
	}
	else
		f.MarkAsUnicode();


	//  a blank line would be nice.

	f.WriteLine ( TEXT("") );



	//  write the contents

	IKString data;

	for (int i=0;i<m_nPairs;i++)

	{
        //NSLog(@"IKMap::Write - setting: [%s]=[%s]",  (char*)m_pairs[i].key,  (char*)m_pairs[i].value);

		//f.Write ( m_pairs[i].key );

		//f.Write ( TEXT("\t\t") );

		//f.Write ( m_pairs[i].value );

		//f.WriteLine ( TEXT("") );


		IKString s;

		s = m_pairs[i].key;
		s.ToUTF8();
		data += s;

		data += TEXT("\t\t");

		s = m_pairs[i].value;
		s.ToUTF8();
		data += s;

#ifdef PLAT_WINDOWS

		data += TEXT("\r\n");

#endif

#ifdef PLAT_MACINTOSH

		data += TEXT("\r");

#endif

	}

	f.Write(data);



	//  close the file

	f.Close();



	//  make it writeable

	//  TODO:  I made this zero.  Why?

	IKFile::MakeWritable(filename,0);



	return true;

}



int IKMap::Find(IKString key)

{

	for (int i=0;i<m_nPairs;i++)

	{

		IKString k = m_pairs[i].key;

		if ( key.CompareNoCase(m_pairs[i].key) == 0 )

			return i;

	}



	return -1;

}



IKString IKMap::Lookup(IKString key)

{

	int i = Find(key);

	if(i>=0)

		return m_pairs[i].value;



	return TEXT("");

}



IKString IKMap::GetPath()

{

	return m_path;

}



int	IKMap::Count()

{

	return m_nPairs;

}



bool IKMap::GetNthPair(int n, IKString &key, IKString &value)

{

	if (n>m_nPairs-1)

		return false;



	key   = m_pairs[n].key;

	value = m_pairs[n].value;



	return true;

}



IKMap & IKMap :: operator = ( const IKMap &rhs )

{

#if IKMAP_DYNAMIC

	m_size = rhs.m_size;

	m_pairs = new pair[m_size];

#endif



	m_nPairs = rhs.m_nPairs;

	m_path = rhs.m_path;



	for (int i=0;i<m_nPairs;i++)

	{

		m_pairs[i].key   = rhs.m_pairs[i].key;

		m_pairs[i].value = rhs.m_pairs[i].value;

	}



	return *this;

}



#if IKMAP_DYNAMIC



void IKMap::Allocate (int n)

{

	//  do nothing if already large enough.

	if (n<=m_size)

		return;

		

	//  increase the size

	m_size = n + 50;  //  arbitrary

	

	//  allocate a new larger array

	pair *pNew = new pair[m_size];

	

	//  copy the pairs

	for (int i=0;i<m_nPairs;i++)

	{

		pNew[i] = m_pairs[i];

	}

	

	//  swap pointers

	pair *pOld = m_pairs;

	m_pairs = pNew;

	

	//  delete old array

	if (pOld)

		delete [] pOld;

}



#endif



IKMap::IKMap ( const IKMap& src)

{

#if IKMAP_DYNAMIC

	m_size = src.m_size;

	m_pairs = new pair[m_size];

#endif



	m_nPairs = src.m_nPairs;

	m_path = src.m_path;



	for (int i=0;i<m_nPairs;i++)

	{

		m_pairs[i].key   = src.m_pairs[i].key;

		m_pairs[i].value = src.m_pairs[i].value;

	}

}



void IKMap::ModifyKey ( IKString oldKey, IKString newKey )

{

	//  find entries with the old key and change

	//  the key value to the new key.

	

	for (int i=0;i<m_nPairs;i++)

	{

		if (m_pairs[i].key.CompareNoCase(oldKey)==0)

		{

			m_pairs[i].key = newKey;

		}

	}

}

