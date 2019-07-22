!include "MUI2.nsh"
!include "GetWindowsVersion.nsh"
!include "x64.nsh"

!define MUI_CUSTOMFUNCTION_GUIINIT Startup

;--------------------------------
;General

  ;Name and file
  Name "IntelliKeys USB Windows 10"
  OutFile "Setup_IntelliKeys_Windows_3.5.3.exe"

  ;Default installation folder
  InstallDir "$LOCALAPPDATA\IntelliKeysUSBWindows10"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\IntellikeysUSBWindows10" ""
  
  ShowInstDetails show

  ;Request administrator privileges
  RequestExecutionLevel admin
  
;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages
  !define MUI_PAGE_CUSTOMFUNCTION_SHOW ShowWelcomePage
  !insertmacro MUI_PAGE_WELCOME
;  !insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
;  !insertmacro MUI_PAGE_COMPONENTS
;  !insertmacro MUI_PAGE_DIRECTORY
   !define MUI_FINISHPAGE_NOAUTOCLOSE
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Windows 10 Driver" Windows10Driver

      DetailPrint "Installing Windows 10 driver"

      SetOutPath $INSTDIR
${If} ${RunningX64}
      File "ikusb_w10_x64\ikusb.cat"
      File "ikusb_w10_x64\ikusb.inf"
      File "ikusb_w10_x64\ikusb.sys"
${Else}
      File "ikusb_w10_x86\ikusb.cat"
      File "ikusb_w10_x86\ikusb.inf"
      File "ikusb_w10_x86\ikusb.sys"
${Endif}

${DisableX64FSRedirection}
nsExec::ExecToStack 'pnputil /add-driver ikusb.inf /install'
${EnableX64FSRedirection}
Pop $0 # return value/error/timeout
Pop $1 # printed text, up to ${NSIS_MAX_STRLEN}
DetailPrint 'printed: $1'
DetailPrint ""
DetailPrint "       Return value: $0"
DetailPrint ""


  ;Store installation folder
;  WriteRegStr HKCU "Software\Modern UI Test" "" $INSTDIR

  ;Create uninstaller
;  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
;  LangString DESC_SecDummy ${LANG_ENGLISH} "A test section."

  ;Assign language strings to sections
;  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
;    !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
;  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

;  Delete "$INSTDIR\Uninstall.exe"

;  RMDir "$INSTDIR"

;  DeleteRegKey /ifempty HKCU "Software\Modern UI Test"

SectionEnd

Function Startup

;
; Run the legacy 3.5.2 Windows installer before showing the NSIS UI pages
;

  SetOutPath $TEMP
  File "IntelliKeys_English_W_3_5_2.exe"
  ExecWait '"$TEMP\IntelliKeys_English_W_3_5_2.exe"'
  Delete '"$TEMP\IntelliKeys_English_W_3_5_2.exe"'

;
; Is this Windows 10?
;
${GetWindowsVersion} $R0
StrCmp $R0 "2000" 0 +2
      Abort
StrCmp $R0 "XP" 0 +2
      Abort
StrCmp $R0 "2003" 0 +2
      Abort
StrCmp $R0 "Vista" 0 +2
      Abort
StrCmp $R0 "7" 0 +2
      Abort
StrCmp $R0 "8" 0 +2
      Abort
StrCmp $R0 "8.1" 0 +2
      Abort

;
; This is Windows 10; proceed with rest of install
;

FunctionEnd


Function ShowWelcomePage

         BringToFront

FunctionEnd