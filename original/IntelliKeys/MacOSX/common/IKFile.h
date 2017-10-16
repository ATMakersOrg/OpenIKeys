// IKFile.h: interface for the IKFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKFILE_H__C6E278AA_9D2E_47FC_AD5D_23AE7D60A3B8__INCLUDED_)
#define AFX_IKFILE_H__C6E278AA_9D2E_47FC_AD5D_23AE7D60A3B8__INCLUDED_

#include "IKString.h"

class IKFile
{
public:
// Flag values
	enum OpenFlags {
		modeRead =          0x0000,
		modeWrite =         0x0001,
		modeReadWrite =     0x0002,
		shareCompat =       0x0000,
		shareExclusive =    0x0010,
		shareDenyWrite =    0x0020,
		shareDenyRead =     0x0030,
		shareDenyNone =     0x0040,
		modeNoInherit =     0x0080,
		modeCreate =        0x1000,
		modeNoTruncate =    0x2000,
		typeText =          0x4000, // typeText and typeBinary are used in
		typeBinary =   (int)0x8000 // derived classes only
		};
	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
		};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	enum { hFileNull = -1 };

// Constructors
	IKFile();

// Attributes
#ifdef PLAT_WINDOWS
	unsigned int m_hFile;
#endif
#ifdef PLAT_MACINTOSH
        short m_fileRef;
#endif

	virtual unsigned int GetPosition() const;
	virtual IKString GetFileName() const;

// Operations
	virtual bool Open(TCHAR * lpszFileName, unsigned int nOpenFlags);

	static void Rename(TCHAR * lpszOldName,
				TCHAR * lpszNewName);
	static void Remove(TCHAR * lpszFileName);

	static bool NewFolder ( IKString path );

	unsigned int SeekToEnd();
	void SeekToBegin();

	bool IsUnicode() {return m_bUnicode;}

	static bool FileExists ( IKString filename );
	static bool FolderExists ( IKString foldername );

// Overridables

	virtual unsigned int  Seek(unsigned int lOff, unsigned int nFrom);
	virtual void SetLength(unsigned int dwNewLen);
	virtual unsigned int  GetLength() const;

	virtual unsigned int Read(void* lpBuf, unsigned int nCount);
	virtual void Write(const void* lpBuf, unsigned int nCount);
	virtual void Write ( IKString string );
	virtual void WriteLine ( IKString string );

	virtual void Abort();
	virtual void Flush();
	virtual void Close();

// Implementation
public:
	static void MakeWritable ( IKString filename, int nParents = 1 );
	void MarkAsUnicode();
	void MarkAsUTF8();
	static void SetFileLocked ( IKString filename, bool bLock );
	static bool Copy ( IKString src, IKString dst );
	virtual ~IKFile();
	static unsigned int GetModTimeSecs1970(IKString filename);
	
#ifdef PLAT_MACINTOSH
	static IKString HFSToUnix ( IKString filename, bool bIsDirector=false );
	static IKString UnixToHFS ( IKString filename, bool bIsDirector=false );
#endif

	static bool GetOneFile( IKString strTitle, IKString &filename, TCHAR *filter=NULL );

protected:
	bool m_bCloseOnDelete;
	IKString m_strFileName;
	bool m_bUnicode;
};

#endif // !defined(AFX_IKFILE_H__C6E278AA_9D2E_47FC_AD5D_23AE7D60A3B8__INCLUDED_)
