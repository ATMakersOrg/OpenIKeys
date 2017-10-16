:: from WindowsOld\win
@ECHO OFF

SET PWD=%CD%
SET LANG=%1

IF [%LANG%] == [] GOTO USAGE

ECHO copying ..\Control Panel\Private\Text\EnglishMainStringTable.txt to ..\Control Panel\Private\Text\%LANG%MainStringTable.txt
IF NOT EXIST "..\Control Panel\Private\Text\English\%LANG%MainStringTable.txt" copy "..\Control Panel\Private\Text\English\EnglishMainStringTable.txt" "..\Control Panel\Private\Text\English\%LANG%MainStringTable.txt"

ECHO copying "..\root\English\private\Application Overlays\Win\*" "..\root\%LANG%\private\Application Overlays\Win\*"
IF NOT EXIST "..\root\%LANG%\private\Application Overlays\Win\*" xcopy /s /Y /f "..\root\English\private\Application Overlays\Win\*" "..\root\%LANG%\private\Application Overlays\Win\*"

ECHO copying "..\root\English\private\Standard Overlays\Win\*" "..\root\%LANG%\private\Standard Overlays\Win\*"
IF NOT EXIST "..\root\%LANG%\private\Standard Overlays\Win\*" xcopy /s /Y /f "..\root\English\private\Standard Overlays\Win\*" "..\root\%LANG%\private\Standard Overlays\Win\*"

ECHO copying "..\root\English\private\Switch Settings\Win\*" "..\root\%LANG%\private\Switch Settings\Win\*"
IF NOT EXIST "..\root\%LANG%\private\Switch Settings\Win\*" xcopy /s /Y /f "..\root\English\private\Switch Settings\Win\*" "..\root\%LANG%\private\Switch Settings\Win\*"

ECHO copying "..\root\English\private\channels\*" "..\root\%LANG%\private\channels\*"
IF NOT EXIST "..\root\%LANG%\private\Channels\*" xcopy /s /Y /f "..\root\English\private\Channels\*" "..\root\%LANG%\private\Channels\*"

ECHO copying "..\root\English\private\*" "..\root\%LANG%\private\*"
IF NOT EXIST "..\root\%LANG%\private\ikusbhelp.html" xcopy /Y /f "..\root\English\private\*" "..\root\%LANG%\private\*"

ECHO copying ..\root\English\data.txt to ..\root\%LANG%\data.txt
IF NOT EXIST ..\root\%LANG%\data.txt xcopy /s /Y /f ..\root\English\data.txt ..\root\%LANG%\*

ECHO copying ..\root\English\datawin.txt to ..\root\%LANG%\datawin.txt
IF NOT EXIST ..\root\%LANG%\datawin.txt xcopy /s /Y /f ..\root\English\datawin.txt ..\root\%LANG%\*

ECHO copying .\Documentation\English\* to .\Documentation\%LANG%\*
IF NOT EXIST .\Documentation\%LANG% xcopy /s /Y /f .\Documentation\English\* .\Documentation\%LANG%\*

ECHO copying .\Overlays\English\* to .\Overlays\%LANG%\*
IF NOT EXIST .\Overlays\%LANG% xcopy /s /Y /f .\Overlays\English\* .\Overlays\%LANG%\*

ECHO copying .\Overlays for Editing\English\* to new .\Overlays for Editing\%LANG%\*
IF NOT EXIST ".\Overlays for Editing\%LANG%" xcopy /s /Y /f ".\Overlays for Editing\English\*" ".\Overlays for Editing\%LANG%\*"

ECHO copying .\install 2\setup.wse to .\install 2\%LANG%Setup.wse
IF NOT EXIST ".\install 2\%LANG%Setup.wse" copy ".\install 2\setup.wse" ".\install 2\%LANG%Setup.wse"

ECHO copying .\install 2\setup-common.wse to .\install 2\%LANG%Setup-common.wse
IF NOT EXIST ".\install 2\%LANG%Setup-common.wse" copy ".\install 2\setup-common.wse" ".\install 2\%LANG%Setup-common.wse"

ECHO copying .\install 2\setup-strings.wse to .\install 2\%LANG%Setup-strings.wse
IF NOT EXIST ".\install 2\%LANG%Setup_strings.wse" copy ".\install 2\setup_strings.wse" ".\install 2\%LANG%Setup_strings.wse"

ECHO copying .\install 2\uninstall2.wse to .\install 2\%LANG%Uninstall2.wse
IF NOT EXIST ".\install 2\%LANG%Uninstall2.wse" copy ".\install 2\uninstall2.wse" ".\install 2\%LANG%Uninstall2.wse"

ECHO copying .\install 2\uninstall.wse to .\install 2\%LANG%Uninstall.wse
IF NOT EXIST ".\install 2\%LANG%Uninstall.wse" copy ".\install 2\uninstall.wse" ".\install 2\%LANG%Uninstall.wse"

GOTO DONE


:USAGE

ECHO Usage: CreateIKUSBLanguage %LANG%

:DONE