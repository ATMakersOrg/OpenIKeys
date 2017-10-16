// IKOverlay.cpp: implementation of the IKOverlay class.
//
//////////////////////////////////////////////////////////////////////


#ifdef PLAT_MACINTOSH
  #ifdef BUILD_CW
    #ifdef BUILD_CARBON
      #include <Carbon.h>
    #else
      //  TODO
    #endif
  #else
  #endif
  #include <unistd.h>
  #include <stdio.h>
#endif

#include <string.h>

#include "IKCommon.h"
#include "IKOverlay.h"
#include "IKFile.h"
#include "IKDevice.h"
#include "IKUniversal.h"
#include "IKEngine.h"
#include "IKUtil.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IKOverlay::IKOverlay()
: m_bValid(false), m_bSetupOverlay(false)

{
	Unload();
}

IKOverlay::~IKOverlay()
{
	Unload();
}

#ifdef PLAT_MACINTOSH

bool IKOverlay::MacExtractFromFile ( TCHAR *pFilePath, Handle * p_hOverlay, Handle * p_hSet )
{
    //  make a pascal string for the filename
    TCHAR filename[255];
    strcpy(&(filename[1]),pFilePath);
    filename[0] = IKString::strlen(&(filename[1]));

    //  make a spec
    FSSpec spec;
    OSErr err = FSMakeFSSpec ( (short)0, (long)0, (unsigned TCHAR *)filename, &spec );
    if (err != noErr)
	return false;

    //  open the resource fork
    int resID = FSpOpenResFile ( &spec, fsRdPerm );
    if ( resID != -1 )
    {
		//  use this resource file
		UseResFile ( resID );
			
		bool bHaveOvly = false;
		bool bHaveSet = false;
		
		//  get the overlay resource
		Handle hOverlay = ::GetResource ( 'Ovly', 128 );
		if ( hOverlay )
		{
			::DetachResource ( hOverlay );
			if ( p_hOverlay )
			*p_hOverlay = hOverlay;
			else
			::DisposeHandle ( hOverlay );
			bHaveOvly = true;
		}
		
		//  get the feature resource
		Handle hSet = ::GetResource ( 'Feat', 128 );
		if ( hSet )
		{
			::DetachResource ( hSet );
			if ( p_hSet )
			*p_hSet = hSet;
			else
			::DisposeHandle ( hSet );
			bHaveSet = true;
		}
		
		//  close the file
		CloseResFile ( resID );
		
		//  we're good
		return (bHaveOvly);
    }
    
    return false;
}

bool IKOverlay::LoadFromFileMac(IKString filename)
{    
    Handle hOverlay=0, hSet=0;

	if (MacExtractFromFile ((TCHAR *)filename, &hOverlay, &hSet ))
	{			
		if (hOverlay)
		{
			//  how many bytes?
			BYTE b1 = (*hOverlay)[0];
			BYTE b2 = (*hOverlay)[1];
			WORD nbytes = b1 | 256*b2;

			IKASSERT(nbytes<=MAX_OVL_SIZE);
			if (nbytes>MAX_OVL_SIZE)
			{
				::DisposeHandle ( hOverlay );
				if(hSet)
					::DisposeHandle ( hSet );
				goto ret;
			}
			m_nbytes = nbytes;

			//  checksum?
			b1 = (*hOverlay)[2];
			b2 = (*hOverlay)[3];
			WORD checksum = b1 | 256*b2;
			
			//  get the bytes
			//pData = (BYTE *) malloc(nbytes-4);
			memcpy(ovldata,&((*hOverlay)[4]),nbytes-4);

			//  check the checksum

			WORD cs = 0;
			for (int z=0;z<(nbytes-4);z++)
				cs += ovldata[z];
			if (cs != checksum)
			{
				::DisposeHandle ( hOverlay );
				if(hSet)
					::DisposeHandle ( hSet );
				goto ret;
			}
			m_checksum = cs;
			
			//  get the header
			int i = nbytes;
			memcpy ( header.name,          &((*hOverlay)[i]), sizeof(header.name) ); i = i + sizeof(header.name);
			memcpy ( &(header.number),     &((*hOverlay)[i]), 1 ); i = i + 1;
			memcpy ( &(header.num_levels), &((*hOverlay)[i]), 1 ); i = i + 1;
			memcpy ( &(header.setup_flag), &((*hOverlay)[i]), 1 ); i = i + 1;
			memcpy ( &(header.coord_mode), &((*hOverlay)[i]), 1 ); i = i + 1;
			memcpy ( header.reserved,      &((*hOverlay)[i]), sizeof(header.reserved) ); i = i + sizeof(header.reserved);
			memcpy ( header.base_level,    &((*hOverlay)[i]), sizeof(header.base_level) ); i = i + sizeof(header.base_level);
			memcpy ( header.mouse_level,   &((*hOverlay)[i]), sizeof(header.mouse_level) ); i = i + sizeof(header.mouse_level);
			
			for (int i1=0;i1<NUM_LEVELS;i1++)
			{
				b1 = (*hOverlay)[i];
				b2 = (*hOverlay)[i+1];
				i = i + 2;
				header.offset[i1] = b2 | 256*b1;
			}

			::DisposeHandle ( hOverlay );
		}
					
		if ( hSet )
		{
		
			memcpy ( &rawsettings, *hSet, 24 );
			
			::DisposeHandle ( hSet );
		}
			
		m_bValid = true;
		m_bMacOverlay = true;
	}

ret:	
	//  return whether or not the above procedure worked.
	//  if it did not, then the caling code will proceed to try the
	//  file as a Windows overlay.
	if (m_bValid)
		return true;
	return false;
}
#endif

bool IKOverlay::LoadFromFile(IKString filename)
{
	//  start over
	Unload();

	//  make it writeable
	//  TODO:  why are we doing this?
	IKFile::MakeWritable(filename,0);


#ifdef PLAT_MACINTOSH
	if (LoadFromFileMac(filename))
		return true;
#endif

	//  if we got here, the mac-specific load failed, so
	//  try it as a windows file.

	//  open the file
	IKFile f;
	if (!f.Open ( filename, IKFile::modeRead ) )
	{
		return false;
	}


	//  how many bytes?
	BYTE b1,b2;
	b1=b2=0;
	f.Read(&b1,1);
	f.Read(&b2,1);
	WORD nbytes = b1 | 256*b2;

	if (nbytes>MAX_OVL_SIZE)
	{
		f.Close();
		return false;
	}

	//  checksum?
	b1=b2=0;
	f.Read(&b1,1);
	f.Read(&b2,1);
	WORD checksum = b1 | 256*b2;

	//  read the bytes
	int result = f.Read(ovldata,nbytes-4);
	if(result!=(nbytes-4))
	{
		f.Close();
		return false;
	}
	m_nbytes = nbytes;

	//  check the checksum

	WORD cs = 0;
	for (int z=0;z<(nbytes-4);z++)
		cs += ovldata[z];

	if (cs != checksum)
	{
		f.Close();
		return false;
	}
	m_checksum = cs;

	//  read the header

	f.Read(header.name,			sizeof(header.name));
	f.Read(&header.number,		1);
	f.Read(&header.num_levels,	1);
	f.Read(&header.setup_flag,	1);
	f.Read(&header.coord_mode,	1);
	f.Read(header.reserved,		sizeof(header.reserved));
	f.Read(header.base_level,	sizeof(header.base_level));
	f.Read(header.mouse_level,	sizeof(header.mouse_level));


	//  calculate the offsets to each level, count the levels
	//  along the way
	int numRealLevels = 0;
	for (int i1=0;i1<NUM_LEVELS;i1++)
	{
		f.Read(&b1,1);
		f.Read(&b2,1);
		header.offset[i1] = b2 | 256*b1;
		if (header.offset[i1]!=0)
			numRealLevels++;
	}


	//  OM-W has a bug where a long name may overrun such that
	//  the number of levels in the header is wrong.  Correct
	//  it here based on the number of levels
	//  just calculated.
	//ASSERT(numRealLevels==header.num_levels);
	if (numRealLevels!=header.num_levels)
		header.num_levels = numRealLevels;

	DoctorData();

	//  read the settings
	f.Read(rawsettings,sizeof(rawsettings));

	//  close the file
	f.Close();

	//  ok.
	m_bValid = true;
	return true;

}

void IKOverlay::Unload()
{
    unsigned int i;

	//  clear overlay data
    for (i=0;i<sizeof(ovldata);i++)
        ovldata[i] = 0;

	//  clear header
    for (i=0;i<sizeof(header);i++)
        ((BYTE *)&header)[i] = 0;

	//  mark invalid
    m_bValid = false;

	m_bSetupOverlay = false;
	m_nameString = TEXT("");

	m_nbytes = 0;

	m_bMacOverlay = false;
}


int IKOverlay::GetDomainFromSwitch(int nswitch, int parlevel)
{
	if (!IsValid())
	{
		if (nswitch>2)
			return 1000 + nswitch;

		return 0;
	}

	int level = parlevel;
	if (IsSetupOverlay())
		level = 1;

	IKASSERT(nswitch>=1);
	IKASSERT(nswitch<=IK_NUM_SWITCHES);

	IKASSERT(level>0);
	IKASSERT(level<=GetNumLevels());

#ifdef _DEBUG
	WORD cs = ComputeChecksum();
	IKASSERT(cs==m_checksum);
#endif

	int i;
	BYTE *pData= &(ovldata[0]);

	if(level>GetNumLevels())
	{
		IKASSERT(false);
		return 0;
	}

	//  pass one, look for match to switch

	i=header.offset[level-1]-4;
	while (true)
	{
		//  bail if end of level
		if (pData[i]==0 && pData[i+1]==0)
		{
			break;
		}
 
		//  get domain and x,y data
		BYTE domain = pData[i];
		BYTE b1     = pData[i+1];
		i = i + 3;

		//  see if the byte matches a switch
		if ( b1==(255-nswitch+1) )
		{
			IKASSERT(i<MAX_OVL_SIZE);
			IKASSERT(domain<=254);  //  per scott's documentation
			return domain;
		}

		//  advance to the next domain
		if (pData[i]!=0)
			while (pData[i]!=0)
				i = i + 1;
		i = i + 1;
	}
	IKASSERT(i<MAX_OVL_SIZE);

	//  no match, bail

	if (nswitch>2)
		return 1000 + nswitch;

	return 0;

}

int IKOverlay::GetDomainFromMembrane(int x, int y, int parlevel)
{
	if (!IsValid())
		return 0;

	int level = parlevel;
	if (IsSetupOverlay())
		level = 1;

	int i;
	BYTE *pData = &(ovldata[0]);

	IKASSERT(x>=0);
	IKASSERT(x<IK_RESOLUTION_X);
	IKASSERT(y>=0);
	IKASSERT(y<IK_RESOLUTION_Y);
	IKASSERT(level>0);
	if (IsValid())
		IKASSERT(level<=GetNumLevels());

#ifdef _DEBUG
	WORD cs = ComputeChecksum();
	IKASSERT(cs==m_checksum);
#endif

	if(level>GetNumLevels())
		return 0;

	//  pass one, get the domain number for x,y

	int theDomain = -1;
	i=header.offset[level-1]-4;
	while (true)
	{
		//  bail if end of level
		if (pData[i]==0 /*&& pData[i+1]==0*/)
		{
			break;
		}
 
		//  get domain and x,y data
		BYTE domain = pData[i];
		BYTE b1     = pData[i+1];
		BYTE b2     = pData[i+2];
		i = i + 3;

		//  ignore switches
		int ns = 255 - b1 + 1;
		if (!(ns>0 && ns<= IK_NUM_SWITCHES))
		{
			int ovx = b1 & 0x1f;
			int ovy = b2 & 0x1f;
			int w = b1 >> 5;
			int h = b2 >> 5;
			if (x>=ovx && x<=ovx+w && y>=ovy && y<=ovy+h)
			{
                //NSLog(@"IKOverlay::GetDomainFromMembrane domain (%d), x(%d), y(%d), ovx(%d), w(%d), ovy(%d), h(%d)", domain, x, y, ovx, w, ovy, h);
 				theDomain = domain;
				break;
			}
		}

		//  advance to the next domain
		if (pData[i]!=0)
			while (pData[i]!=0)
				i = i + 1;
		i = i + 1;
	}
	IKASSERT(i<MAX_OVL_SIZE);

	//  no match
	if (theDomain==-1)
		return 0;

	IKASSERT(theDomain>=0);
	IKASSERT(theDomain<=254);  //  per scott's documentation

	return theDomain;

}

bool IKOverlay::IsValid()
{
	return m_bValid;
}

bool IKOverlay::IsSetupOverlay()
{
	return m_bSetupOverlay;
}

int IKOverlay::GetNumLevels()
{
	if (!IsValid())
		return 0;

	return header.num_levels;
}

WORD IKOverlay::ComputeChecksum()
{
	unsigned int cs = 0;
	for (int i=0;i<MAX_OVL_SIZE;i++)
		cs += ovldata[i];

	return cs;
}

void IKOverlay::MakeSetupOverlay()
{
	m_bSetupOverlay = true;
}

IKString IKOverlay::GetName()
{
	if (!IsValid())
	{
		Unload();
		return DATAS(TEXT("Unknown"),TEXT("unknown"));
	}

	if (!m_nameString.IsEmpty())
		return m_nameString;

	IKString s;

	bool bshift   = false;
	bool bcontrol = false;
	bool balt     = false;
	for (int i=0;i<OVL_NAME_LEN;i++)
	{
		BYTE c = header.name[i];
		if(c==0)
			break;

		if (c==UNIVERSAL_SHIFT || c==UNIVERSAL_RIGHT_SHIFT)
		{
			bshift = true;
		}
		else if (c==UNIVERSAL_CONTROL || c==UNIVERSAL_RIGHT_CONTROL )
		{
			bcontrol = true;
		}
		else if (c==UNIVERSAL_ALT || c==UNIVERSAL_ALTGR)
		{
			balt = true;
		}
		else
		{

#ifdef PLAT_WINDOWS
			BYTE keystate[256];
			for (int i=0;i<256;i++)
				keystate[i] = 0;

			if (bshift)
				keystate[VK_SHIFT] = 0x80;
			if (bcontrol)
				keystate[VK_CONTROL] = 0x80;
			if (balt)
				keystate[VK_MENU] = 0x80;

			TCHAR buf[2] = {0,0};
			int ret = ::ToAscii(c, 0, keystate, (WORD *) buf, 0);

			if (buf[0]!=0)
				s += buf[0];

#else
			if (!bshift && c>=('A') && c<=('Z'))
			{
				s += (char) (c -('A') + ('a'));
			}
			else
			{
				s += c;
			}
#endif


			bshift   = false;
			balt     = false;
			bcontrol = false;
		}
	}

	if (s.IsEmpty())
		return DATAS(TEXT("Unknown"),TEXT("unknown"));

	return s;

}

BYTE * IKOverlay::GetUniversalCodesFromDomain(int theDomain, int level)
{
	static BYTE sw3[3] = {UNIVERSAL_MOUSE_BUTTON_CLICK,0,0};
	static BYTE sw4[3] = {UNIVERSAL_CONTROL,UNIVERSAL_RIGHT_ARROW,0};
	static BYTE sw5[3] = {UNIVERSAL_CONTROL,UNIVERSAL_DOWN_ARROW,0};

	if (theDomain==1003)
		return sw3;
	if (theDomain==1004)
		return sw4;
	if (theDomain==1005)
		return sw5;

	if (!IsValid())
		return (BYTE *) NULL;

	BYTE *pData = &(ovldata[0]);

	if (theDomain==0)
		return NULL;

	int i;

	//void *pReturn = 0;
	i=header.offset[level-1]-4;

	while (true)
	{
		//  bail if end of level
		if (pData[i]==0 && pData[i+1]==0)
		{
			break;
		}
 
		//  get domain and read past x,y data
		BYTE domain = pData[i];
		//BYTE b1     = pData[i+1];
		//BYTE b2     = pData[i+2];
		i = i + 3;

		//  does this domain match and does it have data?
		if ( pData[i]!=0 && domain == theDomain )
		{
			IKASSERT(&(pData[i]));
			IKASSERT(i<MAX_OVL_SIZE);
			return &(pData[i]);
		}

		//  advance to the next domain
		if (pData[i]!=0)
			while (pData[i]!=0)
				i = i + 1;
		i = i + 1;
	}
	IKASSERT(i<MAX_OVL_SIZE);

	//  if we get here, no match.
	//IKASSERT(false);
	return NULL;

}

void IKOverlay::SetName(IKString name)
{
	//  Set the name 
	m_nameString = name;
}

void IKOverlay::StoreSettings(BYTE *data)
{
	IKUtil::MemoryCopy(rawsettings,data,24);
}

void IKOverlay::StoreData(BYTE *data, int datalen)
{
	//  "file" marker
	int marker = 0;

	//  how many bytes?
	BYTE b1,b2;
	b1=b2=0;
	b1 = data[marker]; marker++;
	b2 = data[marker]; marker++;
	WORD nbytes = b1 | 256*b2;

	if (nbytes>MAX_OVL_SIZE)
	{
		IKASSERT(false);
		return;
	}

	//  checksum?
	b1=b2=0;
	b1 = data[marker]; marker++;
	b2 = data[marker]; marker++;
	WORD checksum = b1 | 256*b2;
	m_nbytes = nbytes;

	//  "read" the bytes
	IKUtil::MemoryCopy(ovldata,&(data[marker]),nbytes-4);
	marker = marker + nbytes-4;

	//  check the checksum
	WORD cs = 0;
	for (int z=0;z<(nbytes-4);z++)
		cs += ovldata[z];
	if (cs != checksum)
	{
		IKASSERT(false);
		return;
	}
	m_checksum = cs;

	//  "read" the header
	IKUtil::MemoryCopy(header.name,&(data[marker]),sizeof(header.name));
	marker = marker + sizeof(header.name);

	header.number     = data[marker]; marker++;
	header.num_levels = data[marker]; marker++;
	header.setup_flag = data[marker]; marker++;
	header.coord_mode = data[marker]; marker++;

	IKUtil::MemoryCopy(header.reserved,&(data[marker]),sizeof(header.reserved));
	marker = marker + sizeof(header.reserved);

	IKUtil::MemoryCopy(header.base_level,&(data[marker]),sizeof(header.base_level));
	marker = marker + sizeof(header.base_level);

	IKUtil::MemoryCopy(header.mouse_level,&(data[marker]),sizeof(header.mouse_level));
	marker = marker + sizeof(header.mouse_level);


	//  calculate the offsets to each level, count the levels
	//  along the way
	int numRealLevels = 0;
	for (int i1=0;i1<NUM_LEVELS;i1++)
	{
		b1=b2=0;
		b1 = data[marker]; marker++;
		b2 = data[marker]; marker++;
#ifdef PLAT_WINDOWS
		header.offset[i1] = b2 | 256*b1;
#else
		header.offset[i1] = b1 | 256*b2;
#endif
		if (header.offset[i1]!=0)
			numRealLevels++;
	}

	//  OM-W has a bug where a long name may overrun such that
	//  the number of levels in the header is wrong.  Correct
	//  it here based on the number of levels
	//  just calculated.
	//ASSERT(numRealLevels==header.num_levels);
	if (numRealLevels!=header.num_levels)
		header.num_levels = numRealLevels;


	//  read the settings
	//f.Read(rawsettings,sizeof(rawsettings));

	DoctorData();

}

bool IKOverlay::SaveToFile(IKString filename)
{
	//  create the file
	IKFile f;
	bool bOpened = f.Open ( filename, IKFile::modeWrite | IKFile::modeCreate);
	if (!bOpened)
		return false;

	//  write the contents

	//  number of bytes
	BYTE b1,b2;
	b2 = m_nbytes/256;
	b1 = m_nbytes - 256*b2;
	f.Write(&b1,1);
	f.Write(&b2,1);        

	//  checksum
	b2 = (m_checksum & 0x0000ff00)/256;
	b1 = m_checksum & 0x000000ff;
	f.Write(&b1,1);
	f.Write(&b2,1);

	//  bytes
	f.Write(ovldata,m_nbytes-4);

	//  header
	f.Write(header.name,			sizeof(header.name));
	f.Write(&header.number,		1);
	f.Write(&header.num_levels,	1);
	f.Write(&header.setup_flag,	1);
	f.Write(&header.coord_mode,	1);
	f.Write(header.reserved,		sizeof(header.reserved));
	f.Write(header.base_level,	sizeof(header.base_level));
	f.Write(header.mouse_level,	sizeof(header.mouse_level));

	//  levels
	for (int i=0;i<NUM_LEVELS;i++)
	{
		b2 = header.offset[i]/256;
		b1 = header.offset[i] - b2*256;
		f.Write(&b1,1);
		f.Write(&b2,1);
	}

	//  settings
	f.Write(rawsettings,sizeof(rawsettings));


	//  close the file
	f.Close();

	//  make it writeable
	IKFile::MakeWritable(filename,2);


	return true;
}

void IKOverlay::DoctorData()
{

#ifdef PLAT_WINDOWS
#ifdef REPLACE_ALTGR

	//  replace UNIVERSAL_ALTGR
	//  with UNIVERSAL_CONTROL, UNIVERSAL_MENU

	int nbytes = m_nbytes-4;
	int i = 0;

	while (i<nbytes)
	{
		if (ovldata[i]==UNIVERSAL_UNICODE)
		{
			i = i + 2;
		}

		else if (ovldata[i]==UNIVERSAL_ALTGR)
		{
			IKUtil::MemoryCopy(&(ovldata[i]),&(ovldata[i+1]),nbytes-i);
			ovldata[i]   = UNIVERSAL_CONTROL;
			ovldata[i+1] = UNIVERSAL_MENU;
			nbytes++;
			i++;
		}

		i++;
	}

#endif
#endif

}

void IKOverlay::SwapOffsetBytes()
{
    for (int i=0;i<NUM_LEVELS;i++)
    {
            BYTE b2 = header.offset[i]/256;
            BYTE b1 = header.offset[i] - b2*256;
            
            header.offset[i] = b2 + 256*b1;
    }
}

