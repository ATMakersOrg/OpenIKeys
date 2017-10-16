#!/bin/bash
#
#-----------------------------------------------------------------------------------------------------
# IKUSB Intel Build Script
#
# Checkout the sources from the CVS Server and build the components and copy the build
# artifacts to a local staging area.
#
# Command line usage:
#
# Clean = Clean the source code paths before checkout to force a complete checkout,
#         default is to NOT do a Clean.
# NoCheckout - do not perform the cvs checkout operation, 
#         default is to Checkout
# Tagged--tag the build
# Stage = the desired staging area path, default is C:\Builds\Local
#-----------------------------------------------------------------------------------------------------

#--------------------------------------------------------------------------------------
# Set command line defaults
#--------------------------------------------------------------------------------------
CLEAN_BUILD=0
NO_CHECKOUT=0

function BuildIKUSB()
{
	DESTINATION_PATH="/ClassroomSuite/IntelliKeys/MacOSX/Staging/"
	DESTINATION_PARAM="CONFIGURATION_BUILD_DIR=$DESTINATION_PATH"
	BUILD_ROOT="/ClassroomSuite/IntelliKeys/MacOSX"
	SCRIPT_ROOT="/ClassroomSuite/IntelliKeys/MacOSX/IKUSB_Build_Scripts"
	INSTALLER_ROOT="/ClassroomSuite/IntelliKeys/MacOSX/Installer/Package/IntelliKeys USB/IntelliKeys USB Build.pkgproj"
	UNINSTALLER_ROOT="/ClassroomSuite/IntelliKeys/MacOSX/IKUSB Uninstaller/IKUSB Uninstaller.pkgproj"
	#mblock RBIDE_PATH="/Applications/Real Studio 2011 Release 4.3/Extras/IDE Scripting/RBIDEScript/rbidescript"
	DOCO_ROOT="/ClassroomSuite/Documentation/IntelliKeys"


	#Remove all CVS Folders
	cd $BUILD_ROOT
	find . -name 'CVS' | xargs rm -r

	PATHS=( 'Add Login Item' USBMenu UniversalLibrary ControlPanel 'Root Locator' NoKSA ikusb shutdown SendingBundle 'Remove Login Item' HelperWatcher )
	PROJECTS=( AddLoginItem USBMenu Library ControlPanel 'Root Locator' NoKSA ikusb shutdown 'Overlay Sending' RemoveLoginItem HelperWatcher )


	for (( i = 0 ; i < ${#PROJECTS[@]} ; i++ )) do
		CURRENT_PROJECT="${PROJECTS[$i]}"
		CURRENT_PATH="${PATHS[$i]}"
	
		cd "$BUILD_ROOT/$CURRENT_PATH"
	
		if [ "$CURRENT_PATH" == "SendingBundle" ] ; then
			rm Bundle.rsrc
			binhex -o Bundle.rsrc Bundle.rsrc.binhex.rsrc 
		fi
	
		xcodebuild -project "$CURRENT_PROJECT.xcodeproj" -configuration "Release" clean $DESTINATION_PARAM
		xcodebuild -project "$CURRENT_PROJECT.xcodeproj" -configuration "Release" build $DESTINATION_PARAM
		errorLevel=$? >> "$BUILD_LOGS_PATH/xcodebuild_stdout" 2>> "$BUILD_LOGS_PATH/xcodebuild_stderr"
		echo xcodebuild ErrorLevel Result $errorLevel >> $BUILD_LOGFILE_NAME 2>&1
		if [ "$errorLevel" -ne 0 ] ; then
 			echo build failed at $CURRENT_STEP on $CURRENT_PROJECT >> $BUILD_LOGFILE_NAME 2>&1 
 			echo build failed at $CURRENT_STEP >> $BUILD_LOGFILE_NAME 2>&1 
 			echo build failed at $CURRENT_STEP, see log file: $BUILD_LOGFILE_NAME
 			exit 1
		fi
	done

	cd $BUILD_ROOT
	osascript "$SCRIPT_ROOT/BuildRB.applescript"

}

function incrementBuildNumber()
{
	
	BUILD_NUMBER=$1

	BUILD_NUMBER=$[$BUILD_NUMBER+1]

	cd "$SCRIPT_ROOT"
	(	echo $BUILD_NUMBER
	) > build_Number.tmp
	
	mv -f Build_Number.tmp Build_Number.txt
	#cvs commit -m "Incrementing build number to $BUILD_NUMBER" Build_Number.txt
}

function createFlagFile()
{
	BUILD_NUMBER=$1
	
	TIME=`date +%r`
	DATE=`date +"%A %m/%d/%Y"`
	
	(	echo '<?xml version="1.0" encoding="utf-8"?>'
	    echo '<BuildInfoRecord xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema">'
		echo "<BuildNumber>$BUILD_NUMBER</BuildNumber>"
		echo "<ProductVersion>$VERSION</ProductVersion>"
		echo "<CompletionStatus>Success</CompletionStatus>"
		echo "<PostDate>$DATE</PostDate>"
		echo "<PostTime>$TIME</PostTime>"
	    echo '</BuildInfoRecord>'
	) > "$FLAG_LOCATION"/IKUSB_Flag.xml
}


#--------------------------------------------------------------------------------------
# Parse the command line --need to process all the arguments, more work here!
#--------------------------------------------------------------------------------------
if [ "$1" == "clean" ] 
	then CLEAN_BUILD=1
fi

if [ "$1" == "Clean" ] 
	then CLEAN_BUILD=1
fi

if [ "$1" == "CLEAN" ] 
	then CLEAN_BUILD=1
fi

if [ "$1" == "NoCheckout" ] 
	then NO_CHECKOUT=1
fi

if [ "$1" == "nocheckout" ] 
	then NO_CHECKOUT=1
fi

if [ "$1" == "Nocheckout" ] 
	then NO_CHECKOUT=1
fi

if [ "$1" == "NOCHECHOUT" ] 
	then NO_CHECKOUT=1
fi

#-----------------------------------------------------------------------------------------------------
#the build requires we are in /ClassroomSuite
#-----------------------------------------------------------------------------------------------------
echo Starting Directory is
pwd

cd "/ClassroomSuite"

currentD=`pwd`
echo currentD is $currentD

#-----------------------------------------------------------------------------------------------------
#set some local variables for easier modifications
#-----------------------------------------------------------------------------------------------------
BUILD_PATH="$currentD/IntelliTools"
LOCAL_STAGING_PATH="/Builds/Local"
STAGING_PATH="$LOCAL_STAGING_PATH/IntelliKeys/MacOSX"
BUILD_LOGS_PATH="$STAGING_PATH/BuildLogs"
BUILD_RND_NBR=$RANDOM
BUILD_LOGFILE_NAME="$BUILD_LOGS_PATH/buildResults_$BUILD_RND_NBR.log"
BUILD_ERROR_LOGFILE_NAME="$BUILD_LOGS_PATH/buildResults_$BUILD_RND_NBR.Error.log"
XCODE_PATH="/Developer/"
FLAG_LOCATION="/Builds/Local/Flag"
VERSION="3.5.2"
LANGUAGES=( Portuguese Russian Spanish 'French Canadian'  )
ECHO errorlog name is $BUILD_ERROR_LOGFILE_NAME


#-----------------------------------------------------------------------------------------------------
#make the log directory before writing to the log
#----------------------------------------------------------------------------------------------------- 
mkdir -p $BUILD_LOGS_PATH > /dev/null 2>&1

if [ "$CLEAN_BUILD" == 1 ] ; then 
	NO_CHECKOUT=0
fi

#-----------------------------------------------------------------------------------------------------
#place the local variables at the top of the log
#-----------------------------------------------------------------------------------------------------
ECHO IKUSB_BUILD.txt > $BUILD_LOGFILE_NAME 2>&1
ECHO . >> $BUILD_LOGFILE_NAME 2>&1
ECHO $currentD >> $BUILD_LOGFILE_NAME 2>&1
ECHO Current Date / Time: `date`  >> $BUILD_LOGFILE_NAME 2>&1
ECHO NO_CHECKOUT=$NO_CHECKOUT >> $BUILD_LOGFILE_NAME 2>&1
ECHO CLEAN_BUILD=$CLEAN_BUILD >> $BUILD_LOGFILE_NAME 2>&1
ECHO BUILD_PATH=$BUILD_PATH >>  $BUILD_LOGFILE_NAME 2>&1
ECHO STAGING_PATH=$STAGING_PATH >>  $BUILD_LOGFILE_NAME 2>&1
ECHO XCODE_PATH=$XCODE_PATH >> $BUILD_LOGFILE_NAME 2>&1


#-----------------------------------------------------------------------------------------------------
#login CVS server
#-----------------------------------------------------------------------------------------------------
#mblock ECHO log in CVS server
#mblock $CVS_PATH $CVS_DASH_D_CMD login >> $BUILD_LOGFILE_NAME 2>&1


#-----------------------------------------------------------------------------------------------------
# If CLEAN_BUILD is set must do a checkout
#If not doing a CLEAN_BUILD, can skip the checkout if requested
#-----------------------------------------------------------------------------------------------------
#mblock ECHO CLEAN_BUILD = [$CLEAN_BUILD]
#mblock ECHO NO_CHECKOUT = [$NO_CHECKOUT]
#mblock if [ "$NO_CHECKOUT" == 0 ] ; then
	#-----------------------------------------------------------------------------------------------------
	# checkout and log the source files.
	#  Note: checkout will 'update' if the sources already exist.
	#-----------------------------------------------------------------------------------------------------
	
	#------------------------------------------------------------------------------------------------
	#mblock ECHO Checkout IKUSB sources...
	#mblock CURRENT_STEP="Checkout IKUSB"
	#------------------------------------------------------------------------------------------------	

	#mblock ECHO . >> $BUILD_LOGFILE_NAME 2>&1
	#mblock if [ "$CLEAN_BUILD" == 1 ] ; then
		#mblock ECHO Clean Build, deleting "$currentD/IKUSB_Lion" >> $BUILD_LOGFILE_NAME 2>&1
		#mblock rm -r "$currentD/IKUSB_Lion" >> $BUILD_LOGFILE_NAME 2>&1
		#mblock rm -r "$currentD/IKUSB_Documentation" >> $BUILD_LOGFILE_NAME 2>&1
	#mblock fi
	#$CVS_PATH $CVS_DASH_D_CMD checkout "Suite_4_4" >> $BUILD_LOGFILE_NAME 2>&1
	#mblock $CVS_PATH $CVS_DASH_D_CMD export -DNOW -d "IKUSB_Lion" "IntelliTools/IntelliKeys/MacOSX" >> $BUILD_LOGFILE_NAME 2>&1
	#mblock errorLevel=$?
	
	#Check out Documentation
	#mblock $CVS_PATH $CVS_DASH_D_CMD export -DNOW -d "IKUSB_Documentation" "IntelliTools/Documentation/IntelliKeys" >> $BUILD_LOGFILE_NAME 2>&1
	#mblock errorLevel=$?
#mblock fi



#--------------------------------------------------------------------------------
# Select Xcode version and Build the projects
#--------------------------------------------------------------------------------

#------------------------------------------------------------------------------------------------
	ECHO Building the components...
	CURRENT_STEP="Build Components"
#------------------------------------------------------------------------------------------------	
# sudo xcode-select -switch $XCODE_PATH
BuildIKUSB


#------------------------------------------------------------------------------------------------
	ECHO preparing KEXT ownership
	CURRENT_STEP="KEXT Permissions"
#------------------------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
# For Installers, all kext files must be root:wheel before being packaged
#--------------------------------------------------------------------------------
cd "$BUILD_ROOT/kext"
sudo chown -R root:wheel *.kext/

#------------------------------------------------------------------------------------------------
	ECHO removing Symbolic links
	CURRENT_STEP="Removing Symbolic Links"
#------------------------------------------------------------------------------------------------
cd "$DESTINATION_PATH"
mkdir -p temp
cp -L AddLoginItem temp/
cp -RL ControlPanel.bundle temp/
cp -L HelperWatcher temp/
cp -L ikusb temp/
cp -L NoKSA temp/
cp -RL "Overlay Sending.bundle" temp/
cp -L RemoveLoginItem temp/
cp -L "Root Locator" temp/
cp -L "Shutdown" temp/
cp -L "Universal Sending.dylib" temp/
cp -RL "USBMenu.app" temp/

#Move over the Documentation Assets
#Move Updated IntelliTools/Documentation assets into place
cp -f "$DOCO_ROOT/readme.txt" "$BUILD_ROOT/ControlPanel/Documentation/Read Me.txt"
cp -f "$DOCO_ROOT/IntelliKeys USB Users Guide.pdf" "$BUILD_ROOT/ControlPanel/Documentation/"
cp -f "$DOCO_ROOT/PSO Users Guide.pdf" "$BUILD_ROOT/ControlPanel/Documentation/"
cp -f "$DOCO_ROOT/ikusbhelp.html" "$BUILD_ROOT/ControlPanel/private/"

#------------------------------------------------------------------------------------------------
	ECHO Creating Localized Installers with Packages
	CURRENT_STEP="Creating Localized Packages"
#------------------------------------------------------------------------------------------------
#Comment out for English Only
sh "$SCRIPT_ROOT/buildlocalizedinstallers.sh"

#------------------------------------------------------------------------------------------------
	ECHO Creating English Installer with Packages
	CURRENT_STEP="Creating Installer.pkg"
#------------------------------------------------------------------------------------------------

cd $BUILD_ROOT/Staging
echo "INSTALLER_ROOT = $INSTALLER_ROOT"
/usr/local/bin/packagesbuild "$INSTALLER_ROOT" >> $BUILD_LOGFILE_NAME 2>&1


#------------------------------------------------------------------------------------------------
	ECHO Creating Uninstaller with Packages
	CURRENT_STEP="Creating Uninstaller.pkg"
#------------------------------------------------------------------------------------------------
cd $BUILD_ROOT/Staging
/usr/local/bin/packagesbuild "$UNINSTALLER_ROOT" >> $BUILD_LOGFILE_NAME 2>&1


#------------------------------------------------------------------------------------------------
	ECHO Clean Up
	CURRENT_STEP="Cleaning up Staging"
#------------------------------------------------------------------------------------------------
cd "$BUILD_ROOT/kext"
sudo chown -R mblock:staff *.kext/

if [ "($ls -A $STAGING_PATH/Installer)" ]
then
    cd "$STAGING_PATH/Installer"
	rm *.dmg
else 
	echo Nothing to clear at Builds folder
fi

#------------------------------------------------------------------------------------------------
	ECHO Getting build number
	CURRENT_STEP="Getting Build Number"
#------------------------------------------------------------------------------------------------
cd "$SCRIPT_ROOT"
lines=($(cat "Build_Number.txt")) # read lines of file into an array
	BUILD_NUMBER=${lines[0]}
	

#------------------------------------------------------------------------------------------------
	ECHO Creating Disk Image Folder Structure
	CURRENT_STEP="Folder Creation"
#------------------------------------------------------------------------------------------------

mkdir -p "$STAGING_PATH/Installer/DiskImage"
mkdir -p "$STAGING_PATH/Installer/DiskImage/Documentation"
mkdir -p "$STAGING_PATH/Installer/DiskImage/Installers"
mkdir -p "$STAGING_PATH/Installer/DiskImage/Installers/v$VERSION (10.x, Intel Only)"

#Move over older Installer Versions
# cp -R "/ClassroomSuite/IKUSB_Legacy/" "$STAGING_PATH/Installer/DiskImage/Installers/"


#------------------------------------------------------------------------------------------------
	ECHO Packaging
	CURRENT_STEP="Packaging English"
#------------------------------------------------------------------------------------------------

#Move 3.5 Uninstaller
cd "$BUILD_ROOT/Staging/temp"
find ./ -maxdepth 1 -name '*Uninstaller.pkg' -exec mv -f {} "$STAGING_PATH/Installer/DiskImage/Installers/v$VERSION (10.x, Intel Only)" \;
 
#Copy to Distribution
# cd $BUILD_ROOT/Staging/temp

#Move 3.5 Installer pkg
find ./ -maxdepth 1 -name '*USB.pkg' -exec mv -f {} "$STAGING_PATH/Installer/DiskImage/Installers/v$VERSION (10.x, Intel Only)" \;

#Copy readme and Documentation
cp -R "$BUILD_ROOT/ControlPanel/Documentation/" "$STAGING_PATH/Installer/DiskImage/Documentation"

hdiutil create "$STAGING_PATH/Installer/IntelliKeys_USB_$VERSION.dmg" -volname "IntelliKeys USB $VERSION" -srcfolder "$STAGING_PATH/Installer/DiskImage"

#------------------------------------------------------------------------------------------------
	ECHO Packaging
	CURRENT_STEP="Packaging Localized"
#------------------------------------------------------------------------------------------------
#for each Localized version stored in $LANGUAGES
 for (( i = 0 ; i < ${#LANGUAGES[@]} ; i++ )) do
	LOCALIZATION="${LANGUAGES[$i]}"
        echo "Creating $STAGING_PATH/Installer/DiskImage/$LOCALIZATION"
	mkdir -p "$STAGING_PATH/Installer/DiskImage/$LOCALIZATION"
	cd "$BUILD_ROOT/Staging/temp"
	mv "IntelliKeys USB $LOCALIZATION.pkg" "$STAGING_PATH/Installer/DiskImage/$LOCALIZATION"
	
	#Check for Localized Uninstaller and copy if there
	if [ -d "$BUILD_ROOT/Staging/temp/IntelliKeys USB $LOCALIZATION Uninstaller.mpkg" ]
	then
	  echo found $LOCALIZATION Uninstaller
		cp -R "$BUILD_ROOT/Staging/temp/IntelliKeys USB $LOCALIZATION Uninstaller.mpkg" "$STAGING_PATH/Installer/DiskImage/$LOCALIZATION"
	fi
	
	#Copy Localized Readme to root of disk image, deal with spaces in path name
	LOCALIZATION_NO_SPACE=`echo $LOCALIZATION | sed 's/ //g'`
	echo "Read Me.Txt is in $BUILD_ROOT/Localization/$LOCALIZATION_NO_SPACE/Documentation"
	cd "$BUILD_ROOT/Localization/$LOCALIZATION_NO_SPACE/Documentation"
	cp "Read Me.txt" "$STAGING_PATH/Installer/DiskImage/$LOCALIZATION"
	
	#Copy Documentation folder to localized Image
	cp -R "$BUILD_ROOT/Localization/$LOCALIZATION_NO_SPACE/Documentation/" "$STAGING_PATH/Installer/DiskImage/$LOCALIZATION/Documentation"
	
  echo "$STAGING_PATH/Installer/IntelliKeys_USB_${LOCALIZATION_NO_SPACE}_${VERSION}.dmg"
	hdiutil create "$STAGING_PATH/Installer/IntelliKeys_USB_${LOCALIZATION_NO_SPACE}_${VERSION}.dmg" -volname "IntelliKeys USB $LOCALIZATION $VERSION" -srcfolder "$STAGING_PATH/Installer/DiskImage/$LOCALIZATION"
done

#Clean up for distribution
# rm -R "$STAGING_PATH/Installer/DiskImage/"

#------------------------------------------------------------------------------------------------
	ECHO Increment Build Number
	CURRENT_STEP="Increment Build Number and distribute"
#------------------------------------------------------------------------------------------------
#increment build number and check in.
if [ -f "$STAGING_PATH/Installer/IntelliKeys_USB_$VERSION.dmg" ]
then
	incrementBuildNumber $BUILD_NUMBER
	createFlagFile $BUILD_NUMBER
	echo found .dmg
else
	echo Build delivery folder is empty
fi

#Copy Build log to current.log
mkdir -p "$BUILD_LOGS_PATH/Current"
echo "$BUILD_LOGFILE_NAME"
cp -f $BUILD_LOGFILE_NAME "$BUILD_LOGS_PATH/Current/IKUSB_Build.log"

exit 0
