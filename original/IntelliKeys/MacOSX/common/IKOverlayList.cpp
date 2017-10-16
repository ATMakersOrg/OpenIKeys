// IKOverlayList.cpp: implementation of the IKOverlayList class.
//
//////////////////////////////////////////////////////////////////////

#include "IKCommon.h"
#include "IKUtil.h"
#include "IKOverlayList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IKOverlayList::IKOverlayList()
{
	m_count = 0;
	m_selected = 0;
}
	
IKOverlayList::IKOverlayList(IKString &encoded)
{
	m_count = 0;
	m_selected = 0;
	
	//  get the encoded string
	IKString strEncoded = encoded;

	//  decode it.
	int part = 0;
	int start = 0;
	int index = 0;
	
	for (int i=0;i<strEncoded.GetLength();i++)
	{
		if (strEncoded.GetAt(i)==CHAR('|'))
		{
			IKString s = strEncoded.Mid(start,i-start);
			part++;
			if (part==1)
			{
				//  it's the index
				m_selected = IKUtil::StringToInt(s);
			}
			else
			{
				m_entries[part-2] = s;
				m_count++;
			}
			start = i+1;
		}
	}

	if (m_selected<=0)
		m_selected = 0;
	if (m_selected>m_count)
		m_selected = 0;
}

IKOverlayList::~IKOverlayList()
{

}

bool IKOverlayList::GetEntry(int numEntry, IKString &value, bool &bSelected)
{
	value = IKString(TEXT(""));
	bSelected = false;

	if (numEntry<=0)
		return false;
	if (numEntry>m_count)
		return false;

	value = m_entries[numEntry-1];
	if (numEntry==m_selected)
		bSelected = true;

	return true;
}

IKString IKOverlayList::GetSelectedOverlay ()
{
	if (m_selected<=0)
		return IKString(TEXT(""));

	return m_entries[m_selected-1];

}

int IKOverlayList::GetNumSelected ()
{
	return m_selected;
}

void IKOverlayList::SetNumSelected (int n)
{
	if (n>GetCount())
		return;

	m_selected = n;
}

IKString IKOverlayList::GetNumberedEntry (int n)
{
	if (n>GetCount())
		return IKString(TEXT(""));

	return m_entries[n-1];
}


IKString IKOverlayList::GetEncodedList()
{
	//if (GetCount()<=0)
		//return IKString(TEXT(""));

	IKString result;

	result += IKUtil::IntToString(m_selected);
	result += TEXT("|");

	for (int i=0;i<GetCount();i++)
	{
		result += m_entries[i];
		result += TEXT("|");
	}

	return result;
}

void IKOverlayList::AddEntry ( IKString entry, bool bSelected /*=false*/  )
{
	int i = Find(entry);
	if (i==-1)
	{
		m_count++;
		m_entries[m_count-1] = entry;
		if (bSelected)
			SetNumSelected(m_count);
		
	}
}

int IKOverlayList::Find(IKString &entry)
{
	int n = GetCount();
	if (n<=0)
		return -1;

	for (int i=0;i<n;i++)
	{
		if (m_entries[i].CompareNoCase(entry)==0)
			return i;
	}

	return -1;
}

void IKOverlayList::RemoveEntry ( IKString entry )
{
	//  find it first
	int i = Find(entry);
	if (i==-1)
		return;
		
	//  move 'em down
	for (int n=i+1;i<m_count;i++)
		m_entries[n-1] = m_entries[n];
	m_count--;
	
	//  reset selected
	if (m_selected==i+1)
		m_selected = 0;
	if (m_selected>i+1)
		m_selected--;

}
