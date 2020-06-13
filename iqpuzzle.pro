#  This file is part of iQPuzzle.
#  Copyright (C) 2012-2020 Thorsten Roth
#
#  iQPuzzle is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  iQPuzzle is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with iQPuzzle.  If not, see <http://www.gnu.org/licenses/>.

TEMPLATE      = app

unix: !macx {
       TARGET = iqpuzzle
} else {
       TARGET = iQPuzzle
}

VERSION       = 1.2.1
QMAKE_TARGET_PRODUCT     = "iQPuzzle"
QMAKE_TARGET_DESCRIPTION = "A diverting I.Q. challenging pentomino puzzle"
QMAKE_TARGET_COPYRIGHT   = "(C) 2012-2020 Thorsten Roth"

DEFINES      += APP_NAME=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\" \
                APP_VERSION=\"\\\"$$VERSION\\\"\" \
                APP_DESC=\"\\\"$$QMAKE_TARGET_DESCRIPTION\\\"\" \
                APP_COPY=\"\\\"$$QMAKE_TARGET_COPYRIGHT\\\"\"

MOC_DIR       = ./.moc
OBJECTS_DIR   = ./.objs
UI_DIR        = ./.ui
RCC_DIR       = ./.rcc

QT           += core gui widgets
CONFIG       += c++11

CONFIG(debug, debug|release) {
  CONFIG     += warn_on
  DEFINES    += QT_DEPRECATED_WARNINGS
  DEFINES    += QT_DISABLE_DEPRECATED_BEFORE=0x051500
}

SOURCES      += main.cpp\
                iqpuzzle.cpp \
                board.cpp \
                block.cpp \
                boarddialog.cpp \
                highscore.cpp \
                settings.cpp

HEADERS      += iqpuzzle.h \
                board.h \
                block.h \
                boarddialog.h \
                highscore.h \
                settings.h

FORMS        += iqpuzzle.ui \
                settings.ui

RESOURCES     = res/iqpuzzle_resources.qrc \
                res/translations.qrc
win32:RC_FILE = res/iqpuzzle_win.rc
os2:RC_FILE   = res/iqpuzzle_os2.rc

TRANSLATIONS += lang/iqpuzzle_bg.ts \
                lang/iqpuzzle_de.ts \
                lang/iqpuzzle_nl.ts \
                lang/iqpuzzle_fr.ts \
                lang/iqpuzzle_it.ts \
                lang/iqpuzzle_ko.ts

macx {
  ICON               = res/images/icon.icns
  QMAKE_INFO_PLIST   = res/Info.plist

  BOARDS_DATA.path   = Contents/Resources
  BOARDS_DATA.files += data/boards
  QMAKE_BUNDLE_DATA += BOARDS_DATA
}

unix: !macx {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    isEmpty(BINDIR) {
        BINDIR = bin
    }

    target.path    = $$PREFIX/$$BINDIR/

    data.path      = $$PREFIX/share/iqpuzzle
    data.files    += data/boards

    desktop.path   = $$PREFIX/share/applications
    desktop.files += data/iqpuzzle.desktop

    pixmap.path    = $$PREFIX/share/pixmaps
    pixmap.files  += res/images/iqpuzzle_64x64.png \
                     res/images/iqpuzzle.xpm

    icons.path     = $$PREFIX/share/icons
    icons.files   += res/images/hicolor

    man.path       = $$PREFIX/share
    man.files     += man

    meta.path      = $$PREFIX/share/metainfo
    meta.files    += res/iqpuzzle.appdata.xml

    INSTALLS      += target \
                     data \
                     desktop \
                     pixmap \
                     icons \
                     man \
                     meta
}
