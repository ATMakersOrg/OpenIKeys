/*
 Written by Steve Bryndin (fishbed@tezcat.com, steveb@gvsi.com).

 This code may be used in compiled form in any way you wish. This
 file may be redistributed unmodified by any means PROVIDING it is 
 not sold for profit without the authors written consent, and 
 providing that this notice and the authors name is included. 
 An email letting me know that you are using it would be 
 nice as well. 

 This software is provided "as is" without express or implied warranty. 
 Use it at you own risk! The author accepts no liability for any damages 
 to your computer or data these products may cause.
*/


// ExecImageVersion.cpp: implementation of the CExecImageVersion class.
//
//////////////////////////////////////////////////////////////////////

#include "IKCommon.h"
#include "ExecImageVersion.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//CExecImageVersion::CExecImageVersion()
//{
//	m_strImage = AfxGetAppName();
//	m_strImage += ".exe";
//	m_lpszImageName = m_strImage.GetBuffer(sizeof(m_strImage));
//
//	InitVer();
//}

CExecImageVersion::CExecImageVersion(LPTSTR lpszImageName)
{
	m_lpszImageName = lpszImageName;
	InitVer();
}

CExecImageVersion::~CExecImageVersion()
{
	if (m_lpBuffer)
		free(m_lpBuffer);
	m_lpBuffer = NULL;
}

IKString CExecImageVersion::GetProductName()
{
	if (!m_lpBuffer)
		return IKString("");

	IKString	strProduct;

	IKString key;
	key += IKString(TEXT("\\StringFileInfo\\"));
	key += m_translation;
	key += IKString(TEXT("\\"));

	key += IKString(TEXT("ProductName"));
	
	//Use the version information block to obtain the product name.
	BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	if (!bQueried)
		return IKString(TEXT(""));

	strProduct.Format("%s", m_lpData);

	return strProduct;
}

IKString CExecImageVersion::GetProductVersion()
{
	if (!m_lpBuffer)
		return IKString("");

	IKString	strProductVer;
	
	IKString key;
	key += IKString(TEXT("\\StringFileInfo\\"));
	key += m_translation;
	key += IKString(TEXT("\\"));

	key += IKString(TEXT("ProductVersion"));

	//Use the version information block to obtain the product name.
	BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	if (!bQueried)
		return IKString(TEXT(""));

	strProductVer.Format("%s", m_lpData);
	return strProductVer;
}

IKString CExecImageVersion::GetCompanyName()
{
	if (!m_lpBuffer)
		return IKString("");

	IKString	strCompany;
	
	IKString key;
	key += IKString(TEXT("\\StringFileInfo\\"));
	key += m_translation;
	key += IKString(TEXT("\\"));

	key += IKString(TEXT("CompanyName"));

	//Use the version information block to obtain the product name.
	BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	if (!bQueried)
		return IKString(TEXT(""));

	strCompany.Format("%s", m_lpData);
	return strCompany;
}

IKString CExecImageVersion::GetCopyright()
{
	if (!m_lpBuffer)
		return IKString("");

	IKString	strCopy;

	IKString key;
	key += IKString(TEXT("\\StringFileInfo\\"));
	key += m_translation;
	key += IKString(TEXT("\\"));

	key += IKString(TEXT("LegalCopyright"));

	//Use the version information block to obtain the product name.
	BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	if (!bQueried)
		return IKString(TEXT(""));

	strCopy.Format("%s", m_lpData);
	return strCopy;
}

IKString CExecImageVersion::GetComments()
{
	if (!m_lpBuffer)
		return IKString("");

	IKString	strComments;
	
	IKString key;
	key += IKString(TEXT("\\StringFileInfo\\"));
	key += m_translation;
	key += IKString(TEXT("\\"));

	key += IKString(TEXT("Comments"));

	//Use the version information block to obtain the product name.
	BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	if (!bQueried)
		return IKString(TEXT(""));

	strComments.Format("%s", m_lpData);
	return strComments;
}

IKString CExecImageVersion::GetFileDescription()
{
	if (!m_lpBuffer)
		return IKString("");

	IKString	strFileDescr;
	
	IKString key;
	key += IKString(TEXT("\\StringFileInfo\\"));
	key += m_translation;
	key += IKString(TEXT("\\"));

	key += IKString(TEXT("FileDescription"));

	//Use the version information block to obtain the product name.
	BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	if (!bQueried)
		return IKString(TEXT(""));

	IKString s = (TCHAR *)m_lpData;
	return s;
}

IKString CExecImageVersion::GetFileVersion()
{
	if (!m_lpBuffer)
		return IKString("");

	IKString	strFileVer;

	IKString key;
	key += IKString(TEXT("\\StringFileInfo\\"));
	key += m_translation;
	key += IKString(TEXT("\\"));

	key += IKString(TEXT("FileVersion"));

	//Use the version information block to obtain the product name.
	BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	if (!bQueried)
		return IKString(TEXT(""));

	strFileVer.Format("%s", m_lpData);
	return strFileVer;
}

IKString CExecImageVersion::GetInternalName()
{
	if (!m_lpBuffer)
		return IKString("");

	IKString	strIN;
	
	IKString key;
	key += IKString(TEXT("\\StringFileInfo\\"));
	key += m_translation;
	key += IKString(TEXT("\\"));

	key += IKString(TEXT("InternalName"));

	//Use the version information block to obtain the product name.
	BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	if (!bQueried)
		return IKString(TEXT(""));

	strIN.Format("%s", m_lpData);
	return strIN;
}

IKString CExecImageVersion::GetLegalTrademarks()
{
	if (!m_lpBuffer)
		return IKString("");

	IKString	strLegTrade;
	
	IKString key;
	key += IKString(TEXT("\\StringFileInfo\\"));
	key += m_translation;
	key += IKString(TEXT("\\"));

	key += IKString(TEXT("LegalTrademarks"));

	//Use the version information block to obtain the product name.
	BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	if (!bQueried)
		return IKString(TEXT(""));

	strLegTrade.Format("%s", m_lpData);
	return strLegTrade;
}

IKString CExecImageVersion::GetPrivateBuild()
{
	if (!m_lpBuffer)
		return IKString("");

	IKString	strPrivBuild;
	
	IKString key;
	key += IKString(TEXT("\\StringFileInfo\\"));
	key += m_translation;
	key += IKString(TEXT("\\"));

	key += IKString(TEXT("PrivateBuild"));

	//Use the version information block to obtain the product name.
	BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	if (!bQueried)
		return IKString(TEXT(""));

	strPrivBuild.Format("%s", m_lpData);
	return strPrivBuild;
}

IKString CExecImageVersion::GetSpecialBuild()
{
	if (!m_lpBuffer)
		return IKString("");

	IKString	strSpecBuild;
	
	IKString key;
	key += IKString(TEXT("\\StringFileInfo\\"));
	key += m_translation;
	key += IKString(TEXT("\\"));

	key += IKString(TEXT("SpecialBuild"));

	//Use the version information block to obtain the product name.
	BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	if (!bQueried)
		return IKString(TEXT(""));

	strSpecBuild.Format("%s", m_lpData);
	return strSpecBuild;
}



static BOOL GetVersionInfo(LPTSTR szFilePath, DWORD& dwLanguageAndCodepage) 
{ 
    // Inital parameters 
    DWORD dwHandle;                         // Set to Zero (Unused) 
    DWORD dwInfoSize;                       // Size of the Info Structure. 
    UINT uiLanguageSize;            // Size of language buffer 
    DWORD const* pdwLanguages; 
	BYTE * pbData= NULL;

    // Retrieve the File information 
    if (dwInfoSize = ::GetFileVersionInfoSize(szFilePath, &dwHandle)) 
    { 
        // Allocate the memory 
        pbData = new BYTE[dwInfoSize];
        if (!pbData) 
			return FALSE; 

        // Get the Version data 
        if (::GetFileVersionInfo(szFilePath, dwHandle, dwInfoSize, pbData)) 
        { 
            // Get the translation information. (Language) 
            if (::VerQueryValue(pbData, TEXT("\\VarFileInfo\\Translation"), 
						(void**)&pdwLanguages, &uiLanguageSize)) 
            { 
                if (uiLanguageSize) 
                { 
                    dwLanguageAndCodepage = *pdwLanguages;
					if (pbData)
						delete [] pbData;
                    return TRUE; 
                } 
            } 
        } 

		if (pbData)
			delete [] pbData;
        return FALSE; 
    } 

    return FALSE; 
} 


void CExecImageVersion::InitVer()
{
	m_dwHandle = 0;
	m_uiDataSize = 80;

	m_lpData = malloc(m_uiDataSize);

	// Get the version information block size,
	// then use it to allocate a storage buffer.
	m_dwSize = ::GetFileVersionInfoSize(m_lpszImageName, &m_dwHandle);
	m_lpBuffer = NULL;
	if (m_dwSize)
		m_lpBuffer = malloc(m_dwSize);
	else
		return;

	// Get the versioninformation block
	::GetFileVersionInfo(m_lpszImageName, 0, m_dwSize, m_lpBuffer);

	//  get the translation
	DWORD langandcodepage;
	GetVersionInfo ( m_lpszImageName, langandcodepage );
	TCHAR buffer[9];
	MySprintf ( buffer, TEXT("%4.4x%4.4x"), LOWORD(langandcodepage), HIWORD(langandcodepage) );
	m_translation = IKString(buffer);
}
