# Microsoft Developer Studio Project File - Name="ezloader" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ezloader - Win32 For Shop
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ezloader.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ezloader.mak" CFG="ezloader - Win32 For Shop"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ezloader - Win32 For Shop" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ezloader - Win32 For Customers" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""Perforce Project""
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ezloader - Win32 For Shop"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ezloader___Win32_For_Shop"
# PROP BASE Intermediate_Dir "ezloader___Win32_For_Shop"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ForShop"
# PROP Intermediate_Dir "ForShop"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /W3 /WX /Oy /Gy /I "..\..\..\..\98ddk\inc" /I "..\..\..\..\98ddk\inc\win98" /I "i386\\" /I "." /I "..\..\..\..\98ddk\src\usb\inc" /I "..\..\..\..\98ddk\src\wdm\usb\inc" /I "..\..\inc" /I "..\..\firmware" /D WIN32=100 /D "_WINDOWS" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=1 /D "_IDWBUILD" /D "NDEBUG" /D _DLL=1 /D _X86_=1 /D i386=1 /D "DRIVER" /Oxs /Zel -cbstring /QIfdiv- /QIf /GF /c
# ADD CPP /nologo /Gz /W3 /WX /Oy /Gy /I "..\..\..\..\98ddk\inc" /I "..\..\..\..\98ddk\inc\win98" /I "i386\\" /I "." /I "..\..\..\..\98ddk\src\usb\inc" /I "..\..\..\..\98ddk\src\wdm\usb\inc" /I "..\..\inc" /I "..\..\common\firmware" /I "..\..\..\..\ntddk\inc" /D "SHOP" /D WIN32=100 /D "_WINDOWS" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=1 /D "_IDWBUILD" /D "NDEBUG" /D _DLL=1 /D _X86_=1 /D i386=1 /D "DRIVER" /Oxs /Zel -cbstring /QIfdiv- /QIf /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "$(BASEDIR)\inc\Win98" /i "$(BASEDIR)\inc" /i "$(BASEDIR)\src\usb\inc" /i "$(BASEDIR)\src\wdm\usb\inc" /i "..\..\inc" /d "NDEBUG" /d "DRIVER"
# ADD RSC /l 0x409 /i "..\\" /i "$(BASEDIR)\inc\Win98" /i "$(BASEDIR)\inc" /i "$(BASEDIR)\src\usb\inc" /i "$(BASEDIR)\src\wdm\usb\inc" /i "..\..\inc" /d "NDEBUG" /d "DRIVER"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wdm.lib ..\..\..\..\98ddk\lib\i386\free\usbd.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /pdb:none /debug /debugtype:coff /machine:IX86 /nodefaultlib /out:".\LIB\i386\free\IkFirm.sys" /libpath:"..\..\..\..\98ddk\lib\i386\free" /driver /debug:notmapped,MINIMAL /IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native
# ADD LINK32 wdm.lib ..\..\..\..\98ddk\lib\i386\free\usbd.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /pdb:none /debug /debugtype:coff /machine:IX86 /nodefaultlib /out:"ForShop\ikfirms.sys" /libpath:"..\..\..\..\98ddk\lib\i386\free" /driver /debug:notmapped,MINIMAL /IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native

!ELSEIF  "$(CFG)" == "ezloader - Win32 For Customers"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ezloader___Win32_For_Customers"
# PROP BASE Intermediate_Dir "ezloader___Win32_For_Customers"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ForCustomers"
# PROP Intermediate_Dir "ForCustomers"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /W3 /WX /Oy /Gy /I "..\..\..\..\98ddk\inc" /I "..\..\..\..\98ddk\inc\win98" /I "i386\\" /I "." /I "..\..\..\..\98ddk\src\usb\inc" /I "..\..\..\..\98ddk\src\wdm\usb\inc" /I "..\..\inc" /I "..\..\firmware" /D WIN32=100 /D "_WINDOWS" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=1 /D "_IDWBUILD" /D "NDEBUG" /D _DLL=1 /D _X86_=1 /D i386=1 /D "DRIVER" /Oxs /Zel -cbstring /QIfdiv- /QIf /GF /c
# ADD CPP /nologo /Gz /W3 /WX /Oy /Gy /I "..\..\..\..\98ddk\inc" /I "..\..\..\..\98ddk\inc\win98" /I "i386\\" /I "." /I "..\..\..\..\98ddk\src\usb\inc" /I "..\..\..\..\98ddk\src\wdm\usb\inc" /I "..\..\inc" /I "..\..\common\firmware" /I "..\..\..\..\ntddk\inc" /D WIN32=100 /D "_WINDOWS" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=1 /D "_IDWBUILD" /D "NDEBUG" /D _DLL=1 /D _X86_=1 /D i386=1 /D "DRIVER" /Oxs /Zel -cbstring /QIfdiv- /QIf /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "$(BASEDIR)\inc\Win98" /i "$(BASEDIR)\inc" /i "$(BASEDIR)\src\usb\inc" /i "$(BASEDIR)\src\wdm\usb\inc" /i "..\..\inc" /d "NDEBUG" /d "DRIVER"
# ADD RSC /l 0x409 /i "..\\" /i "$(BASEDIR)\inc\Win98" /i "$(BASEDIR)\inc" /i "$(BASEDIR)\src\usb\inc" /i "$(BASEDIR)\src\wdm\usb\inc" /i "..\..\inc" /d "NDEBUG" /d "DRIVER"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wdm.lib ..\..\..\..\98ddk\lib\i386\free\usbd.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /pdb:none /debug /debugtype:coff /machine:IX86 /nodefaultlib /out:".\LIB\i386\free\IkFirm.sys" /libpath:"..\..\..\..\98ddk\lib\i386\free" /driver /debug:notmapped,MINIMAL /IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native
# ADD LINK32 wdm.lib ..\..\..\..\98ddk\lib\i386\free\usbd.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /pdb:none /debug /debugtype:coff /machine:IX86 /nodefaultlib /out:"ForCustomers\ikfirm.sys" /libpath:"..\..\..\..\98ddk\lib\i386\free" /driver /debug:notmapped,MINIMAL /IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native

!ENDIF 

# Begin Target

# Name "ezloader - Win32 For Shop"
# Name "ezloader - Win32 For Customers"
# Begin Group "Source Files"

# PROP Default_Filter ".c;.cpp"
# Begin Source File

SOURCE=.\ezloader.c
DEP_CPP_EZLOA=\
	"..\..\..\98ddk\inc\win98\basetsd.h"\
	"..\..\..\98ddk\inc\win98\bugcodes.h"\
	"..\..\..\98ddk\inc\win98\ntdef.h"\
	"..\..\..\98ddk\inc\win98\ntiologc.h"\
	"..\..\..\98ddk\inc\win98\ntstatus.h"\
	"..\..\..\98ddk\inc\win98\usb100.h"\
	"..\..\..\98ddk\inc\win98\usbdi.h"\
	"..\..\..\98ddk\inc\win98\usbdlib.h"\
	"..\..\..\98ddk\inc\win98\usbioctl.h"\
	"..\..\..\98ddk\inc\win98\wdm.h"\
	"..\..\..\ntddk\inc\alpharef.h"\
	".\ezloader.h"\
	
# End Source File
# Begin Source File

SOURCE=.\firmware.c
DEP_CPP_FIRMW=\
	"..\..\..\98ddk\inc\win98\basetsd.h"\
	"..\..\..\98ddk\inc\win98\bugcodes.h"\
	"..\..\..\98ddk\inc\win98\ntdef.h"\
	"..\..\..\98ddk\inc\win98\ntiologc.h"\
	"..\..\..\98ddk\inc\win98\ntstatus.h"\
	"..\..\..\98ddk\inc\win98\usb100.h"\
	"..\..\..\98ddk\inc\win98\usbdi.h"\
	"..\..\..\98ddk\inc\win98\usbdlib.h"\
	"..\..\..\98ddk\inc\win98\usbioctl.h"\
	"..\..\..\98ddk\inc\win98\wdm.h"\
	"..\..\..\ntddk\inc\alpharef.h"\
	"..\..\common\firmware\EzLoader_Firmware.c"\
	".\ezloader.h"\
	
NODEP_CPP_FIRMW=\
	".\EzLoader_Firmware_Scribbler.c"\
	
# End Source File
# Begin Source File

SOURCE=.\loader.c
DEP_CPP_LOADE=\
	"..\..\..\98ddk\inc\win98\basetsd.h"\
	"..\..\..\98ddk\inc\win98\bugcodes.h"\
	"..\..\..\98ddk\inc\win98\ntdef.h"\
	"..\..\..\98ddk\inc\win98\ntiologc.h"\
	"..\..\..\98ddk\inc\win98\ntstatus.h"\
	"..\..\..\98ddk\inc\win98\usb100.h"\
	"..\..\..\98ddk\inc\win98\usbdi.h"\
	"..\..\..\98ddk\inc\win98\usbdlib.h"\
	"..\..\..\98ddk\inc\win98\usbioctl.h"\
	"..\..\..\98ddk\inc\win98\wdm.h"\
	"..\..\..\ntddk\inc\alpharef.h"\
	".\ezloader.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\ezloader.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ".rc;.mc"
# Begin Source File

SOURCE=.\ezloader.rc
# ADD BASE RSC /l 0x409 /i "..\..\..\..\98ddk\inc\Win98" /i "..\..\..\..\98ddk\inc" /i "..\..\..\..\98ddk\src\usb\inc" /i "..\..\..\..\98ddk\src\wdm\usb\inc"
# SUBTRACT BASE RSC /i "$(BASEDIR)\inc\Win98" /i "$(BASEDIR)\inc" /i "$(BASEDIR)\src\usb\inc" /i "$(BASEDIR)\src\wdm\usb\inc"
# ADD RSC /l 0x409 /i "..\..\..\..\98ddk\inc\Win98" /i "..\..\..\..\98ddk\inc" /i "..\..\..\..\98ddk\src\usb\inc" /i "..\..\..\..\98ddk\src\wdm\usb\inc"
# SUBTRACT RSC /i "$(BASEDIR)\inc\Win98" /i "$(BASEDIR)\inc" /i "$(BASEDIR)\src\usb\inc" /i "$(BASEDIR)\src\wdm\usb\inc"
# End Source File
# End Group
# End Target
# End Project
