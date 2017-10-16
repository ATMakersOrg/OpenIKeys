// IKOverlayList.h: interface for the IKOverlayList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKOVERLAYLIST_H__2D3AEC30_BE00_4D52_BE47_BCA1A8DC1C0C__INCLUDED_)
#define AFX_IKOVERLAYLIST_H__2D3AEC30_BE00_4D52_BE47_BCA1A8DC1C0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IKString.h"

class IKOverlayList  
{
public:
	IKOverlayList();
	IKOverlayList(IKString &encoded);
	virtual ~IKOverlayList();
	
	IKString GetEncodedList ();
	bool GetEntry ( int numEntry, IKString &value, bool &bSelected );
	IKString GetSelectedOverlay();
	int GetCount() {return m_count;}
	int GetNumSelected ();
	IKString GetNumberedEntry (int n);
	void SetNumSelected(int n);
	void AddEntry ( IKString entry, bool bSelected=false );
	void RemoveEntry ( IKString entry );
	int Find ( IKString &entry );

private:
	int m_count;
	int m_selected;
	IKString m_entries[50];
};

#endif // !defined(AFX_IKOVERLAYLIST_H__2D3AEC30_BE00_4D52_BE47_BCA1A8DC1C0C__INCLUDED_)
