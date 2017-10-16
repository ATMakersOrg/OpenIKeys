::----------------------------------------------------------------
:: IKUSB_Compile_Close_RB.cmd
::
:: launches a Real Basic application call RBIdeScript.exe giving
:: it Real Basic IDE Script that will build the Control Panel
:: application and close the Real basic app.
::
:: Assumptions:
:: 1. The script assumes it is launched from the primary
:: IKUSB build script so the necessary environment
:: is alreay set.
::
:: 2. The script assumes that Real Basic was launched with 
:: IKUSB Control Panel application opened.
::----------------------------------------------------------------
:: The contents of the Real Basic IDE Script:
:: PropertyValue("App.StageCode") = "3"
:: BuildLinux = False
:: BuildMacMachOX86 = False
:: BuildWin32 = True
:: BuildRegion = "United States"
:: BuildLanguage = "English"
:: DoCommand "BuildApp"
:: QuitIDE False
::----------------------------------------------------------------

::----------------------------------------------------------------
:: This script must be started before the Real Basic app
:: is started so the ping -n 40 -w 1000 is meant to delay
:: the execution of the rbidescript.exe until hopefully
:: Real Basic is up and running.
::----------------------------------------------------------------
ping -n 40 -w 1000 localhost

CD "%IK_INTELLIKEYS_CVS_DIR%\WindowsOld\Control Panel"

ECHO Current Working Directory: %CD% >> %IK_BUILD_LOG% 2>&1

ECHO "RBIDEScript\RBIDEScriptExecutor.exe" "IKUSB_Build_Control_Panel_Final.rbs"
ECHO "RBIDEScript\RBIDEScriptExecutor.exe" "IKUSB_Build_Control_Panel_Final.rbs" >> %IK_BUILD_LOG% 2>&1
"RBIDEScript\RBIDEScriptExecutor.exe" "IKUSB_Build_Control_Panel_Final.rbs" >> %IK_BUILD_LOG% 2>&1

:: No need keep the cmd shell open
:: EXIT