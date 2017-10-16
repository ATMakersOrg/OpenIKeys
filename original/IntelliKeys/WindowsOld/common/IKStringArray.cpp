
#include "IKCommon.h"
#include "IKString.h"
#include "IKStringArray.h"
#include "IKUtil.h"


IKStringArray::IKStringArray()
{
	m_array = 0;
	Free ();
}

IKStringArray::~IKStringArray()
{
	Free ();
}


IKStringArray::IKStringArray ( const IKStringArray& src)
{
	m_array = 0;
	Free ();

	for (int i=0;i<src.GetSize();i++)
		Add(src.GetAt(i));
}

const IKStringArray& IKStringArray::operator=(const IKStringArray& src)
{
	Free ();

	for (int i=0;i<src.GetSize();i++)
		Add(src.GetAt(i));

	return *this;
}

IKString IKStringArray::operator [] ( int i ) 
{
	return GetAt(i); 
}


void IKStringArray::Add ( IKString &string )
{
	Allocate(m_used + 1);
	m_array[m_used] = string;
	m_used++;
}

void IKStringArray::Add ( TCHAR *pString )
{
	IKString s = pString;
	Add(s);
}

void IKStringArray::RemoveAll ()
{
	Free ();
}

IKString IKStringArray::GetAt (int i) const
{
	if (i>=m_used)
		return "";

	return m_array[i];
}

int IKStringArray::GetSize () const
{
	return m_used;
}


void IKStringArray::Allocate (int n )
{
	//  already enough?
	if (n<m_allocated)
		return;

	//  first allocation?
	if (!m_array)
	{
		m_allocated = n + 50;
		m_array = new IKString[m_allocated];
		return;
	}

	//  realocation

	//  make a new larger array
	m_allocated = n + 50;
	IKString *pNew = new IKString[m_allocated];

	//  copy elements
	for (int i=0;i<m_used;i++)
		pNew[i] = m_array[i];

	//  swap pointers
	IKString *pOld = m_array;
	m_array = pNew;

	//  delete old array
	delete [] pOld;
}

void IKStringArray::Free ()
{
	if (m_array)
		delete [] m_array;
	m_array = 0;
	m_used = 0;
	m_allocated = 0;
}

void IKStringArray::Sort ()
{
	if (m_used<=0)
		return;

	if (!m_array)
		return;

	for (int i=0;i<m_used;i++)
	{
		for (int j=i+1;j<m_used;j++)
		{
			if (m_array[i]>m_array[j])
			{
				IKString s = m_array[i];
				m_array[i] = m_array[j];
				m_array[j] = s;
			}
		}
	}
}

int IKStringArray::Find(const IKString &string)
{
	for (int i=0;i<m_used;i++)
	{
		if (m_array[i].CompareNoCase(string)==0)
			return i;
	}

	return -1;
}



void IKStringArray::AddNoDup(IKString &string)
{
	int i = Find(string);
	if (i==-1)
		Add(string);
}

void IKStringArray::Remove(const IKString &string)
{
	int i = Find(string);
	if (i != -1)
	{
		int n = GetSize();
		for (int j=i;j<n;j++)
		{
			m_array[i] = m_array[i+1];
		}
		m_used--;
	}
}
