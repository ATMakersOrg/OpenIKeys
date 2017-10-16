
#ifndef __IKFILEDIALOG_H__
#define __IKFILEDIALOG_H__

#include "IKString.h"
#include "IKStringArray.h"


class IKFileDialog
{
public:
	enum OpenSave { Open, Save };

public:
	IKFileDialog( OpenSave o, IKStringArray &typeArray, bool bAllowMultiOpen = FALSE );
	IKFileDialog( OpenSave o );
	
	void	SetDialogTitle ( LPCSTR lpTitle );
	void	SetInitialDirectory ( IKString strDir );
	void	SetInitialFile ( IKString strFile );

	bool	DoModal ( IKString & );

	void SetName ( IKString name );
	
private:
	void	Initialize ( void );
	
private:
	IKString m_strInitialFile;
	IKString m_strInitialDir;

#ifdef PLAT_WINDOWS
IKString m_strFilter;
IKString m_strDefaultExtension;
//CFileDialog m_fileDlg;
#endif

	OpenSave m_openOrSave;
	bool m_bAllowMultiOpen;
	const IKStringArray &m_typeArray;
	IKString m_title;
};

#endif //__IKFILEDIALOG_H__