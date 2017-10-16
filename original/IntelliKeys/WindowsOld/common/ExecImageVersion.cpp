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
#include "IKUtil.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExecImageVersion::CExecImageVersion(LPTSTR lpszImageName)
{
	m_lpszImageName = lpszImageName;
	InitVer();
}

CExecImageVersion::~CExecImageVersion()
{
	if (m_lpBuffer)
	{
		free(m_lpBuffer);
	}
	m_lpBuffer = NULL;
}

IKString CExecImageVersion::GetProductName()
{
	return GetStringTableValue(TEXT("ProductName"));
	
	//if (!m_lpBuffer)
	//{
	//	return IKString("");
	//}

	//IKString strProduct;
	//IKString key;

	//key += IKString(TEXT("\\StringFileInfo\\"));
	//key += m_translation;
	//key += IKString(TEXT("\\"));

	//key += IKString(TEXT("ProductName"));
	//
	////Use the version information block to obtain the product name.
	//BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	//if (!bQueried)
	//{
	//	return IKString(TEXT(""));
	//}

	//strProduct.Format("%s", m_lpData);
	//return strProduct;
}

IKString CExecImageVersion::GetProductVersion()
{
	return GetStringTableValue(TEXT("ProductVersion"));

	//if (!m_lpBuffer)
	//	return IKString("");

	//IKString	strProductVer;
	//
	//IKString key;
	//key += IKString(TEXT("\\StringFileInfo\\"));
	//key += m_translation;
	//key += IKString(TEXT("\\"));

	//key += IKString(TEXT("ProductVersion"));

	////Use the version information block to obtain the product name.
	//BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	//if (!bQueried)
	//	return IKString(TEXT(""));

	//strProductVer.Format("%s", m_lpData);
	//return strProductVer;
}

IKString CExecImageVersion::GetCompanyName()
{
	return GetStringTableValue(TEXT("CompanyName"));

	//if (!m_lpBuffer)
	//	return IKString("");

	//IKString	strCompany;
	//
	//IKString key;
	//key += IKString(TEXT("\\StringFileInfo\\"));
	//key += m_translation;
	//key += IKString(TEXT("\\"));

	//key += IKString(TEXT("CompanyName"));

	////Use the version information block to obtain the product name.
	//BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	//if (!bQueried)
	//	return IKString(TEXT(""));

	//strCompany.Format("%s", m_lpData);
	//return strCompany;
}

IKString CExecImageVersion::GetCopyright()
{
	return GetStringTableValue(TEXT("LegalCopyright"));

	//if (!m_lpBuffer)
	//	return IKString("");
	//IKString	strCopy;

	//IKString key;
	//key += IKString(TEXT("\\StringFileInfo\\"));
	//key += m_translation;
	//key += IKString(TEXT("\\"));

	//key += IKString(TEXT("LegalCopyright"));

	////Use the version information block to obtain the product name.
	//BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	//if (!bQueried)
	//	return IKString(TEXT(""));

	//strCopy.Format("%s", m_lpData);
	//return strCopy;
}

IKString CExecImageVersion::GetComments()
{
	return GetStringTableValue(TEXT("Comments"));

	//if (!m_lpBuffer)
	//	return IKString("");

	//IKString	strComments;
	//
	//IKString key;
	//key += IKString(TEXT("\\StringFileInfo\\"));
	//key += m_translation;
	//key += IKString(TEXT("\\"));

	//key += IKString(TEXT("Comments"));

	////Use the version information block to obtain the product name.
	//BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	//if (!bQueried)
	//	return IKString(TEXT(""));

	//strComments.Format("%s", m_lpData);
	//return strComments;
}

IKString CExecImageVersion::GetFileDescription()
{
	return GetStringTableValue(TEXT("FileDescription"));

	//if (!m_lpBuffer)
	//{
	//	return IKString("");
	//}

	//IKString	strFileDescr;
	//
	//IKString key;
	//key += IKString(TEXT("\\StringFileInfo\\"));
	//key += m_translation;
	//key += IKString(TEXT("\\"));

	//key += IKString(TEXT("FileDescription"));

	////Use the version information block to obtain the product name.
	//BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR*) key, &m_lpData, &m_uiDataSize);
	//if (!bQueried)
	//{
	//	return IKString(TEXT(""));
	//}

	//IKString s = (TCHAR *)m_lpData;
	//return s;
}

IKString CExecImageVersion::GetFileVersion()
{
	return GetStringTableValue(TEXT("FileVersion"));

	//if (!m_lpBuffer)
	//	return IKString("");

	//IKString	strFileVer;

	//IKString key;
	//key += IKString(TEXT("\\StringFileInfo\\"));
	//key += m_translation;
	//key += IKString(TEXT("\\"));

	//key += IKString(TEXT("FileVersion"));

	////Use the version information block to obtain the product name.
	//BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	//if (!bQueried)
	//	return IKString(TEXT(""));

	//strFileVer.Format("%s", m_lpData);
	//return strFileVer;
}

IKString CExecImageVersion::GetInternalName()
{
	return GetStringTableValue(TEXT("InternalName"));

	//if (!m_lpBuffer)
	//	return IKString("");

	//IKString	strIN;
	//
	//IKString key;
	//key += IKString(TEXT("\\StringFileInfo\\"));
	//key += m_translation;
	//key += IKString(TEXT("\\"));

	//key += IKString(TEXT("InternalName"));

	////Use the version information block to obtain the product name.
	//BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	//if (!bQueried)
	//	return IKString(TEXT(""));

	//strIN.Format("%s", m_lpData);
	//return strIN;
}

IKString CExecImageVersion::GetLegalTrademarks()
{
	return GetStringTableValue(TEXT("LegalTrademarks"));

	//if (!m_lpBuffer)
	//	return IKString("");

	//IKString	strLegTrade;
	//
	//IKString key;
	//key += IKString(TEXT("\\StringFileInfo\\"));
	//key += m_translation;
	//key += IKString(TEXT("\\"));

	//key += IKString(TEXT("LegalTrademarks"));

	////Use the version information block to obtain the product name.
	//BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	//if (!bQueried)
	//	return IKString(TEXT(""));

	//strLegTrade.Format("%s", m_lpData);
	//return strLegTrade;
}

IKString CExecImageVersion::GetPrivateBuild()
{
	return GetStringTableValue(TEXT("PrivateBuild"));

	//if (!m_lpBuffer)
	//	return IKString("");

	//IKString	strPrivBuild;
	//
	//IKString key;
	//key += IKString(TEXT("\\StringFileInfo\\"));
	//key += m_translation;
	//key += IKString(TEXT("\\"));

	//key += IKString(TEXT("PrivateBuild"));

	////Use the version information block to obtain the product name.
	//BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData,&m_uiDataSize);
	//if (!bQueried)
	//	return IKString(TEXT(""));

	//strPrivBuild.Format("%s", m_lpData);
	//return strPrivBuild;
}

IKString CExecImageVersion::GetSpecialBuild()
{
	return GetStringTableValue(TEXT("SpecialBuild"));

	//if (!m_lpBuffer)
	//{
	//	return IKString("");
	//}

	//IKString key;
	//key += IKString(TEXT("\\StringFileInfo\\"));
	//key += m_translation;
	//key += IKString(TEXT("\\"));

	//key += IKString(TEXT("SpecialBuild"));

	////Use the version information block to obtain the product name.
	//BOOL bQueried = ::VerQueryValue(m_lpBuffer, (TCHAR *) key, &m_lpData, &m_uiDataSize);
	//if (!bQueried)
	//{
	//	return IKString(TEXT(""));
	//}

	//IKString strSpecBuild;
	//strSpecBuild.Format("%s", m_lpData);
	//return strSpecBuild;
}

	// Nov 2012 - added method to unify handling of MUI strings in the Version String Table
IKString CExecImageVersion::GetStringTableValue(LPTSTR key) 
{
	if (!m_lpBuffer)
	{
		return IKString("");
	}

	IKString strResult(TEXT(""));
	
	IKString stringTableKey;
	stringTableKey += IKString(TEXT("\\StringFileInfo\\"));
	stringTableKey += m_translation;
	stringTableKey += IKString(TEXT("\\"));
	stringTableKey += IKString(key);

	if (m_TranslationCodePage == 1200)
	{
		// 1200 is WINUNICODE so use the 'Wide Character' version of VerQueryValueW
		WCHAR stringTableKeyWide[1024] = {0};
		int acpToUnicodeBytes = MultiByteToWideChar(CP_ACP, 0, (LPTSTR)stringTableKey, stringTableKey.GetLength(), stringTableKeyWide, 1024);
		stringTableKeyWide[acpToUnicodeBytes] = 0;

		BOOL bQueryResult = ::VerQueryValueW(m_lpBuffer, stringTableKeyWide, &m_lpData, &m_uiDataSize);
		if (bQueryResult)
		{
			char utf8Buffer[1024];
			int unicodeToUtf8Bytes = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)m_lpData, m_uiDataSize, utf8Buffer, 1024, 0, 0);
			utf8Buffer[unicodeToUtf8Bytes] = 0;

			strResult = IKString(utf8Buffer);

			DebugLogToFile("CExecImageVersion::GetStringTableValue - Wide - File=[%s] [%s]=[%s (%04X to UTF8) Size=%d]", m_lpszImageName, (LPTSTR)stringTableKey, (LPTSTR)strResult, m_TranslationCodePage, m_uiDataSize);
		} 
		else 
		{
			DebugLogToFile("CExecImageVersion::GetStringTableValue - Wide - File=[%s] [%s]=[LookupFailed] Error=%d", m_lpszImageName, (LPTSTR)stringTableKey, ::GetLastError());
		}

	}
	else 
	{
		//Use the version information block to obtain the product name.
		BOOL bQueryResult = ::VerQueryValue(m_lpBuffer, (TCHAR*)stringTableKey, &m_lpData, &m_uiDataSize);
		if (bQueryResult)
		{
			//  ANSI to Unicode and back to UTF-8
			WCHAR unicodeBuffer[1024];
			int acpToUnicodeBytes = MultiByteToWideChar(m_TranslationCodePage, 0, (TCHAR*)m_lpData, m_uiDataSize, unicodeBuffer, 1024);
			unicodeBuffer[acpToUnicodeBytes] = 0;

			char utf8Buffer[1024];
			int unicodeToUtf8Bytes = WideCharToMultiByte(CP_UTF8, 0, unicodeBuffer, acpToUnicodeBytes, utf8Buffer, 1024, 0, 0);
			utf8Buffer[unicodeToUtf8Bytes] = 0;

			strResult = IKString(utf8Buffer);

			DebugLogToFile("CExecImageVersion::GetStringTableValue - File=[%s] [%s]=[%s (%04X to UTF8) Size=%d]", m_lpszImageName, (LPTSTR)stringTableKey, (LPTSTR)strResult, m_TranslationCodePage, m_uiDataSize);
		} 
		else 
		{
			DebugLogToFile("CExecImageVersion::GetStringTableValue - File=[%s] [%s]=[LookupFailed] Error=%d", m_lpszImageName, (LPTSTR)stringTableKey, ::GetLastError());
		}
	}
	return strResult;
}


static BOOL GetVersionInfo(LPTSTR szFilePath, DWORD& dwLanguageAndCodepage) 
{ 
	// Initial parameters 
	DWORD dwHandle;                 // Set to Zero (Unused) 
	DWORD dwInfoSize;               // Size of the Info Structure. 
	UINT uiLanguageSize;            // Size of language buffer 
	DWORD const* pdwLanguages; 
	BYTE * pbData= NULL;

	// Retrieve the File information 
	if (dwInfoSize = ::GetFileVersionInfoSize(szFilePath, &dwHandle)) 
	{ 
		// Allocate the memory 
		pbData = new BYTE[dwInfoSize];
		if (!pbData) 
		{
			return FALSE; 
		}

		// Get the Version data 
		if (::GetFileVersionInfo(szFilePath, dwHandle, dwInfoSize, pbData)) 
		{ 
			// Get the translation information. (Language) 
			if (::VerQueryValue(pbData, TEXT("\\VarFileInfo\\Translation"), (void**)&pdwLanguages, &uiLanguageSize)) 
			{ 
				if (uiLanguageSize) 
				{ 
					dwLanguageAndCodepage = *pdwLanguages;
					if (pbData)
					{
						delete [] pbData;
					}
					return TRUE; 
				} 
			} 
		} 

		if (pbData)
		{
			delete [] pbData;
		}
		return FALSE; 
	} 

	return FALSE; 
} 


void CExecImageVersion::InitVer()
{
	m_dwHandle = 0;
	m_uiDataSize = 512;
	m_lpData = malloc(m_uiDataSize);
	m_TranslationLocaleAndCodePage = 0;
	m_TranslationLocale = 0;
	m_TranslationCodePage = 0;

	// Get the version information block size,
	// then use it to allocate a storage buffer.
	m_dwSize = ::GetFileVersionInfoSize(m_lpszImageName, &m_dwHandle);
	m_lpBuffer = NULL;
	if (m_dwSize)
	{
		m_lpBuffer = malloc(m_dwSize);
	}
	else
	{
		return;
	}

	// Get the version information block
	::GetFileVersionInfo(m_lpszImageName, 0, m_dwSize, m_lpBuffer);

	//  get the translation
	GetVersionInfo(m_lpszImageName, m_TranslationLocaleAndCodePage);
	m_TranslationLocale = LOWORD(m_TranslationLocaleAndCodePage);
	m_TranslationCodePage = HIWORD(m_TranslationLocaleAndCodePage);
	TCHAR buffer[9] = {0};
	// Format: BLOCK: 040904B0 - Locale / CP
	MySprintf(buffer, TEXT("%4.4x%4.4x"), m_TranslationLocale, m_TranslationCodePage);
	m_translation = IKString(buffer);

	//DebugLogToFile("CExecImageVersion::InitVer - File=[%s] Translation=[%s] Locale=[%04X / %d] CodePage=[%04X %d]", m_lpszImageName, (LPTSTR)m_translation, m_TranslationLocale, m_TranslationLocale, m_TranslationCodePage, m_TranslationCodePage);

}
