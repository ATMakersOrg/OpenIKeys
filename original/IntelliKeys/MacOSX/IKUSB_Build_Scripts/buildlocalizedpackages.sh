#!/bin/bash
# buildinstallpackages.sh

# build the common raw package used by all localized install packages
echo building /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/CommonRaw/IntelliKeysUSBCommonRaw.pkgproj
/usr/local/bin/packagesbuild /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/CommonRaw/IntelliKeysUSBCommonRaw.pkgproj 

# build the language specific raw packages
echo building /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/FrenchCanadianRaw/IntelliKeysUSBFrenchCanadianRaw.pkgproj 
/usr/local/bin/packagesbuild /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/FrenchCanadianRaw/IntelliKeysUSBFrenchCanadianRaw.pkgproj 

echo building /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/PortugueseRaw/IntelliKeysUSBPortugueseRaw.pkgproj
/usr/local/bin/packagesbuild /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/PortugueseRaw/IntelliKeysUSBPortugueseRaw.pkgproj 

echo building /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/RussianRaw/IntelliKeysUSBRussianRaw.pkgproj
# /usr/local/bin/packagesbuild /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/RussianRaw/IntelliKeysUSBRussianRaw.pkgproj 

echo building /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/SpanishRaw/IntelliKeysUSBSpanishRaw.pkgproj
/usr/local/bin/packagesbuild /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/SpanishRaw/IntelliKeysUSBSpanishRaw.pkgproj 


#build the language specific install packages
echo building /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB\ FrenchCanadian/IntelliKeys\ USB\ French\ Canadian.pkgproj 
/usr/local/bin/packagesbuild /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB\ FrenchCanadian/IntelliKeys\ USB\ French\ Canadian.pkgproj 

echo building /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB\ Portuguese/IntelliKeys\ USB\ Portuguese.pkgproj 
/usr/local/bin/packagesbuild /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB\ Portuguese/IntelliKeys\ USB\ Portuguese.pkgproj 

echo building /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB\ Russian/IntelliKeys\ USB\ Russian.pkgproj 
/usr/local/bin/packagesbuild /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB\ Russian/IntelliKeys\ USB\ Russian.pkgproj 

echo building /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB\ Spanish/IntelliKeys\ USB\ Spanish.pkgproj
/usr/local/bin/packagesbuild /Users/mblock/Documents/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB\ Spanish/IntelliKeys\ USB\ Spanish.pkgproj 
 
echo building /ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB/IntelliKeys\ USB\ Build.pkgproj
/usr/local/bin/packagesbuild /ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys\ USB/IntelliKeys\ USB\ Build.pkgproj 
 

exit 0;
