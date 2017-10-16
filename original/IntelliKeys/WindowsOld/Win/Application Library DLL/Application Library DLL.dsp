# Microsoft Developer Studio Project File - Name="dll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Application Library DLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Application Library DLL.mak" CFG="dll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""Perforce Project""
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /D "PLAT_WINDOWS" /D TARGET_OS_WIN32=1 /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\\" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib imm32.lib /nologo /dll /machine:I386 /out:"Release/AppLib.dll"

!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\common" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLL_EXPORTS" /D "PLAT_WINDOWS" /D TARGET_OS_WIN32=1 /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\\" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib imm32.lib /nologo /dll /debug /machine:I386 /out:"Debug/AppLib.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "dll - Win32 Release"
# Name "dll - Win32 Debug"
# Begin Source File

SOURCE=..\..\common\AppLib.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\AppLib.h
# End Source File
# Begin Source File

SOURCE=".\Application Library DLL.h"
# End Source File
# Begin Source File

SOURCE=.\dll.cpp
# End Source File
# Begin Source File

SOURCE=.\dll.def
# End Source File
# Begin Source File

SOURCE=.\dll.rc
# End Source File
# Begin Source File

SOURCE=..\..\common\ExecImageVersion.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\ExecImageVersion.h
# End Source File
# Begin Source File

SOURCE=..\..\common\IKCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\common\IKFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IKFile.h
# End Source File
# Begin Source File

SOURCE=..\..\common\IKMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IKMap.h
# End Source File
# Begin Source File

SOURCE=..\..\common\IKMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IKMessage.h
# End Source File
# Begin Source File

SOURCE=..\..\common\IKMsg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IKMsg.h
# End Source File
# Begin Source File

SOURCE=..\..\common\IKOverlayList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IKOverlayList.h
# End Source File
# Begin Source File

SOURCE=..\..\common\IKPrefs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IKPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\common\IKString.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IKString.h
# End Source File
# Begin Source File

SOURCE=..\..\common\IKStringArray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IKStringArray.h
# End Source File
# Begin Source File

SOURCE=..\..\common\IKUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\IKUtil.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\common\sharedMemory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\SharedMemory.h
# End Source File
# End Target
# End Project
