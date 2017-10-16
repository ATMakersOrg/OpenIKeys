#!/bin/bash
# uninstaller preinstall.sh

/usr/bin/killall USBMenu
/usr/bin/killall ikusb

if [ -e /Applications/IntelliTools/IntelliKeys\ USB/Private/RemoveLoginItem ]; then
  /Applications/IntelliTools/IntelliKeys\ USB/Private/RemoveLoginItem
fi

if [ -e /Applications/IntelliTools/IntelliKeys\ USB ]; then
  rm -rf /Applications/IntelliTools/IntelliKeys\ USB
fi

if [ -e /Library/Application\ Support/IntelliTools ]; then
  rm -rf /Library/Application\ Support/IntelliTools/ControlPanel.bundle
  rm -rf /Library/Application\ Support/IntelliTools/ICSPath.txt
  rm -rf /Library/Application\ Support/IntelliTools/IKUSBRoot.txt
  rm -rf /Library/Application\ Support/IntelliTools/placeholder.txt
  rm -rf /Library/Application\ Support/IntelliTools/Universal\ Sending.dylib
fi

if [ -e /Library/LaunchDaemons/com.intellitools.ikusb1.plist ]; then
  rm -rf /Library/LaunchDaemons/com.intellitools.ikusb1.plist
fi

if [ -e /System/Library/Extensions/IKUSBMatch.kext ]; then
  rm -rf /System/Library/Extensions/IKUSBMatch.kext
fi

if [ -e /System/Library/Extensions/IKUSBMatch2.kext ]; then
  rm -rf /System/Library/Extensions/IKUSBMatch2.kext
fi

exit 0;
