do shell script "open /ClassroomSuite/IntelliKeys/MacOSX/ControlPanel/IntelliKeys\\ USB.rbp"

delay 30

tell application "System Events"
	keystroke "b" using {command down}
	#delay 3
	#keystroke tab
	#delay 3
	#keystroke return
end tell

delay 15

tell application "Xojo" to quit


do shell script "cp -R " & quoted form of "/ClassroomSuite/IntelliKeys/MacOSX/ControlPanel/IntelliKeys USB.app" & space & quoted form of "/ClassroomSuite/IntelliKeys/MacOSX/Staging"

#do shell script "cp -R " & quoted form of "/ClassroomSuite/IntelliKeys/MacOSX/ControlPanel/Builds - #IntelliKeys USB.rbp/Mac OS X (Intel)/IntelliKeys USB.app" & space & quoted form of "/ClassroomSuite/IntelliKeys/MacOSX/Staging"
