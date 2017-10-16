#!/bin/bash
# preinstall.sh

/usr/bin/killall USBMenu
/usr/bin/killall ikusb

mkdir /tmp/ikusb/

if [ -d /Applications/IntelliTools/IntelliKeys\ USB/Documents ]; then
  cp -pr /Applications/IntelliTools/IntelliKeys\ USB/Documents /tmp/ikusb/Documents
fi

if [ -e /Applications/IntelliTools/IntelliKeys\ USB/Private/applications.txt ]; then
  cp -pr /Applications/IntelliTools/IntelliKeys\ USB/Private/applications.txt /tmp/ikusb/applications.txt
fi

if [ -e /Applications/IntelliTools/IntelliKeys\ USB/Private/interest.txt ]; then
  cp -pr /Applications/IntelliTools/IntelliKeys\ USB/Private/interest.txt /tmp/ikusb/interest.txt 
fi

if [ -e /Applications/IntelliTools/IntelliKeys\ USB/Private/disallowed.txt ]; then
  cp -pr /Applications/IntelliTools/IntelliKeys\ USB/Private/disallowed.txt /tmp/ikusb/disallowed.txt 
fi

if [ -e /Applications/IntelliTools/IntelliKeys\ USB/Private/RemoveLoginItem ]; then
   /Applications/IntelliTools/Intellikeys\ USB/Private/RemoveLoginItem
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
  rm -f /Library/LaunchDaemons/com.intellitools.ikusb1.plist
fi

if [ -e /System/Library/Extensions/IKUSBMatch.kext ]; then
  rm -rf /System/Library/Extensions/IKUSBMatch.kext
fi

if [ -e /System/Library/Extensions/IKUSBMatch2.kext ]; then
  rm -rf /System/Library/Extensions/IKUSBMatch2.kext
fi

exit 0;
