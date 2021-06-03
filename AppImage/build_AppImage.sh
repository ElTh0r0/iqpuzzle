#!/bin/bash

set -o errexit -o nounset

echo "Building..."
lrelease iqpuzzle.pro
qmake CONFIG+=release PREFIX=/usr
make -j$(nproc)
make INSTALL_ROOT=appdir -j$(nproc) install
find appdir

echo "Downloading linuxdeployqt..."
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
unset QTDIR ; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
export VERSION=$(git rev-parse --short HEAD)  # linuxdeployqt uses this for naming the file

echo "Creating AppImage..."
./linuxdeployqt*.AppImage ./appdir/usr/share/applications/*.desktop -appimage -bundle-non-qt-libs -extra-plugins=iconengines,platformthemes/libqgtk3.so
find ./appdir -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq

echo "Uploading..."
ffsend upload iQPuzzle-$VERSION-x86_64.AppImage
##curl --upload-file iQPuzzle-$VERSION-x86_64.AppImage https://transfer.sh/iQPuzzle-$VERSION-x86_64.AppImage
echo ""

echo "Done!"

exit 0
