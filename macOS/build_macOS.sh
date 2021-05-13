#!/bin/bash

set -o errexit -o nounset

APP='iQPuzzle'

#FFSEND_VERSION='v0.2.71'
# Get latest ffsend version number
FFSEND_VERSION=$(curl --silent "https://github.com/timvisee/ffsend/releases/latest" | sed 's#.*tag/\(.*\)\".*#\1#')

# Hold on to current directory
project_dir=$(pwd)

# Output macOS version
sw_vers

# Update platform (enable if needed)
#echo "Updating platform..."
#brew update

# Install p7zip for packaging and Qt
echo "Installing p7zip and Qt..."
HOMEBREW_NO_AUTO_UPDATE=1 brew install p7zip qt

# Add Qt binaries to path
PATH=/usr/local/opt/qt/bin/:${PATH}

# Build app
echo "Building..."
lrelease iqpuzzle.pro
cd ${project_dir}
mkdir build
cd build
qmake ../iqpuzzle.pro CONFIG+=release
make

# Build and run tests here

# Package
echo "Packaging..."

# Remove build directories that should not be deployed
rm -rf .moc
rm -rf .obj
rm -rf .qrc

echo "Creating dmg archive..."
macdeployqt "${APP}.app" -dmg
mv "${APP}.dmg" "${APP}_${REV_NAME}.dmg"

# Copy other project files
curl -fsSL "https://raw.githubusercontent.com/ElTh0r0/iqpuzzle/packaging/Windows/ReadMe.txt" > "ReadMe.txt"
cp "${project_dir}/COPYING" "${project_dir}/build/COPYING"

echo "Packaging zip archive..."
7z a ${APP}_${REV_NAME}_macOS.zip "${APP}_${REV_NAME}.dmg" "ReadMe.txt" "COPYING"

echo "Downloading ffsend..."
curl -L https://github.com/timvisee/ffsend/releases/download/${FFSEND_VERSION}/ffsend-${FFSEND_VERSION}-macos > ffsend
chmod +x ./ffsend
echo "Uploading..."
./ffsend upload ${APP}_${REV_NAME}_macOS.zip
##curl --upload-file ${APP}_${REV_NAME}_macOS.zip https://transfer.sh/${APP}_${REV_NAME}_macOS.zip
echo ""

echo "Done!"

exit 0
