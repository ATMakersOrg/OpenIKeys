#!/bin/bash
# postinstall.sh

if [ -e /Applications/IntelliTools/IntelliKeys\ USB/Private/Root\ Locator ]; then
   /Applications/IntelliTools/IntelliKeys\ USB/Private/Root\ Locator create
fi

if [ -e /Applications/IntelliTools/IntelliKeys\ USB/Private/NoKSA ]; then
   /Applications/IntelliTools/Intellikeys\ USB/Private/NoKSA
fi

if [ -e /Applications/IntelliTools/IntelliKeys\ USB/Private/ikusb ]; then
  /bin/launchctl start com.intellitools.ikusb1
fi

if [ -e /Applications/IntelliTools/IntelliKeys\ USB/Private/USBMenu.app ]; then
   open /Applications/IntelliTools/IntelliKeys\ USB/Private/USBMenu.app &
fi

#if [ -e /Applications/IntelliTools/IntelliKeys\ USB/Private/AddLoginItem ]; then
#   /Applications/IntelliTools/Intellikeys\ USB/Private/AddLoginItem
#fi

if [ -d /tmp/ikusb/Documents ]; then
  rm -rf /Applications/IntelliTools/IntelliKeys\ USB/Documents
  cp -pr /tmp/ikusb/Documents /Applications/IntelliTools/IntelliKeys\ USB/Documents
fi

if [ -e /tmp/ikusb/applications.txt ]; then
  rm -rf /Applications/IntelliTools/IntelliKeys\ USB/Private/applications.txt
  cp -pr /tmp/ikusb/applications.txt /Applications/IntelliTools/IntelliKeys\ USB/Private/applications.txt
fi

if [ -e /tmp/ikusb/interest.txt ]; then
  rm -rf /Applications/IntelliTools/IntelliKeys\ USB/Private/interest.txt
  cp -pr /tmp/ikusb/interest.txt /Applications/IntelliTools/IntelliKeys\ USB/Private/interest.txt
fi

if [ -e /tmp/ikusb/disallowed.txt ]; then
  rm -rf /Applications/IntelliTools/IntelliKeys\ USB/Private/disallowed.txt
  cp -pr /tmp/ikusb/disallowed.txt /Applications/IntelliTools/IntelliKeys\ USB/Private/disallowed.txt
fi

if [ -d /tmp/ikusb ]; then
  rm -rf /tmp/ikusb
fi

exit 0;
