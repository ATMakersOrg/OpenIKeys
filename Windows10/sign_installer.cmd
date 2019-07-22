@set TOOLS=C:\Tools\SignTools_1_8_0\bin
@set CERTIFICATE_NAME="Echo Digital Audio Corporation"
"%TOOLS%\signtool" sign /v /n %CERTIFICATE_NAME% /t http://timestamp.globalsign.com/scripts/timstamp.dll /d "IntelliKeys USB" /du "https://github.com/ATMakersOrg/OpenIKeys" %1
