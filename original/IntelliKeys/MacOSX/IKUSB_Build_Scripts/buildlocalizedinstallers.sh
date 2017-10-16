#!/bin/bash
# buildinstallpackages.sh
#-----------------------------------------------------------------------------------------------------
# build the common raw package used by all localized install packages
# build the language specific raw packages
# build the language specific install packages
#-----------------------------------------------------------------------------------------------------

DESTINATION_PATH="/ClassroomSuite/IntelliKeys/MacOSX/Staging"
BUILD_ROOT="/ClassroomSuite/IntelliKeys/MacOSX"
SCRIPT_ROOT="/ClassroomSuite/IKUSB_Build_Scripts"
INSTALLER_ROOT="/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package"

# build the common raw package used by all localized install packages
echo "building Common Raw "

mkdir -p "${INSTALLER_ROOT}/CommonRaw/build"
/usr/local/bin/packagesbuild "$INSTALLER_ROOT/CommonRaw/IntelliKeysUSBCommonRaw.pkgproj" 

# build the language specific raw packages
echo "building French Canadian Raw"
/usr/local/bin/packagesbuild "$INSTALLER_ROOT/FrenchCanadianRaw/IntelliKeysUSBFrenchCanadianRaw.pkgproj" 

echo "building Portguese Raw"
/usr/local/bin/packagesbuild "$INSTALLER_ROOT/PortugueseRaw/IntelliKeysUSBPortugueseRaw.pkgproj"

echo "building Russian Raw"
/usr/local/bin/packagesbuild "$INSTALLER_ROOT/RussianRaw/IntelliKeysUSBRussianRaw.pkgproj"

echo "building Spanish Raw"
/usr/local/bin/packagesbuild "$INSTALLER_ROOT/SpanishRaw/IntelliKeysUSBSpanishRaw.pkgproj"

#build the language specific install packages
echo "building French Canadian Package" 
/usr/local/bin/packagesbuild "$INSTALLER_ROOT/IntelliKeys USB FrenchCanadian/IntelliKeys USB French Canadian.pkgproj"

echo "building Portuguese Package"
/usr/local/bin/packagesbuild "$INSTALLER_ROOT/IntelliKeys USB Portuguese/IntelliKeys USB Portuguese.pkgproj"

echo "building Russian Package"
/usr/local/bin/packagesbuild "$INSTALLER_ROOT/IntelliKeys USB Russian/IntelliKeys USB Russian.pkgproj"

echo "building Spanish Package" 
/usr/local/bin/packagesbuild "$INSTALLER_ROOT/IntelliKeys USB Spanish/IntelliKeys USB Spanish.pkgproj"


# Build Uninstallers

echo "building Russian Uninstaller Package"
/usr/local/bin/packagesbuild "/ClassroomSuite/IntelliKeys/MacOSX/IKUSB Uninstaller/Localized/Russian/IKUSB Uninstaller Russian.pkgproj"

 
echo building /ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB/IntelliKeys\ USB\ Build.pkgproj
/usr/local/bin/packagesbuild /ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB/IntelliKeys\ USB\ Build.pkgproj 
 

exit 0;
