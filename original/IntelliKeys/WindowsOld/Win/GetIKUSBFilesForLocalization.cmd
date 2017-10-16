:: from WindowsOld\win
@ECHO OFF

SET PWD=%CD%
SET LANG=%1

IF [%LANG%] == [] GOTO USAGE

:: extract files that need to be localized into a directory but keep the sub-directory structure
SET LOCALIZATION_DIR=.\ToBeLocalizedFiles\%LANG%
MD %LOCALIZATION_DIR%

ECHO copying /Y /f "..\Control Panel\Private\Text\English\%LANG%MainStringTable.txt" "%LOCALIZATION_DIR%\Control Panel\Private\Text\English\*"
xcopy /Y /f "..\Control Panel\Private\Text\English\%LANG%MainStringTable.txt" "%LOCALIZATION_DIR%\Control Panel\Private\Text\English\*"

ECHO copying /Y /f ..\root\%LANG%\data.txt %LOCALIZATION_DIR%\root\%LANG%\*
xcopy /Y /f ..\root\%LANG%\data.txt %LOCALIZATION_DIR%\root\%LANG%\*

ECHO copying /Y /f ..\root\%LANG%\datawin.txt %LOCALIZATION_DIR%\root\%LANG%\*
xcopy /Y /f ..\root\%LANG%\datawin.txt %LOCALIZATION_DIR%\root\%LANG%\*

ECHO copying /Y /f ..\root\%LANG%\Private\ikusbhelp.html %LOCALIZATION_DIR%\root\%LANG%\Private\*
xcopy /Y /f ..\root\%LANG%\Private\ikusbhelp.html %LOCALIZATION_DIR%\root\%LANG%\Private\*

ECHO copying /Y /f ..\root\%LANG%\Private\PSO-help.htm %LOCALIZATION_DIR%\root\%LANG%\Private\*
xcopy /Y /f ..\root\%LANG%\Private\PSO-help.htm %LOCALIZATION_DIR%\root\%LANG%\Private\*

ECHO copying /Y /f ".\install 2\%LANG%Setup_strings.wse" "%LOCALIZATION_DIR%\win\install 2\*"
xcopy /Y /f ".\install 2\%LANG%Setup_strings.wse" "%LOCALIZATION_DIR%\win\install 2\*"

ECHO copying /Y /f .\Documentation\%LANG%\* %LOCALIZATION_DIR%\win\Documentation\%LANG%\*
xcopy /Y /f .\Documentation\%LANG%\* %LOCALIZATION_DIR%\win\Documentation\%LANG%\*

ECHO copying /s /Y /f ".\Overlays\%LANG%\Alternative Overlays\*" "%LOCALIZATION_DIR%\win\Overlays\%LANG%\Alternative Overlays\*"
xcopy /s /Y /f ".\Overlays\%LANG%\Alternative Overlays\*.rtf" "%LOCALIZATION_DIR%\win\Overlays\%LANG%\Alternative Overlays\*"

ECHO copying /s /Y /f ".\Overlays\%LANG%\Standard Overlays\*" "%LOCALIZATION_DIR%\win\Overlays\%LANG%\Standard Overlays\*"
xcopy /s /Y /f ".\Overlays\%LANG%\Standard Overlays\*.rtf" "%LOCALIZATION_DIR%\win\Overlays\%LANG%\Standard Overlays\*"

GOTO DONE

:USAGE

ECHO Usage: GetIKUSBFilesForLocalization %LANG%

:DONE