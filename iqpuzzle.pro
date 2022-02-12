#  This file is part of iQPuzzle.
#  Copyright (C) 2012-2022 Thorsten Roth
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

TEMPLATE       = app

unix: !macx {
       TARGET  = iqpuzzle
} else {
       TARGET  = iQPuzzle
}

win32:VERSION  = 1.2.6.0
else:VERSION   = 1.2.6

QMAKE_TARGET_PRODUCT     = "iQPuzzle"
QMAKE_TARGET_DESCRIPTION = "A diverting I.Q. challenging pentomino puzzle"
QMAKE_TARGET_COPYRIGHT   = "(C) 2012-2022 Thorsten Roth"

DEFINES       += APP_NAME=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\" \
                 APP_VERSION=\"\\\"$$VERSION\\\"\" \
                 APP_DESC=\"\\\"$$QMAKE_TARGET_DESCRIPTION\\\"\" \
                 APP_COPY=\"\\\"$$QMAKE_TARGET_COPYRIGHT\\\"\"

MOC_DIR        = ./.moc
OBJECTS_DIR    = ./.objs
UI_DIR         = ./.ui
RCC_DIR        = ./.rcc

QT            += core gui widgets
CONFIG        += c++11

CONFIG(debug, debug|release) {
  CONFIG      += warn_on
  DEFINES     += QT_DEPRECATED_WARNINGS
  DEFINES     += QT_DISABLE_DEPRECATED_BEFORE=0x060000
}

SOURCES       += main.cpp\
                 iqpuzzle.cpp \
                 board.cpp \
                 block.cpp \
                 boarddialog.cpp \
                 highscore.cpp \
                 settings.cpp

HEADERS       += iqpuzzle.h \
                 board.h \
                 block.h \
                 boarddialog.h \
                 highscore.h \
                 settings.h

FORMS         += iqpuzzle.ui \
                 settings.ui

RESOURCES      = data/data.qrc \
                 lang/translations.qrc

TRANSLATIONS  += lang/iqpuzzle_bg.ts \
                 lang/iqpuzzle_de.ts \
                 lang/iqpuzzle_en.ts \
                 lang/iqpuzzle_fr.ts \
                 lang/iqpuzzle_it.ts \
                 lang/iqpuzzle_ko.ts \
                 lang/iqpuzzle_nb_NO.ts \
                 lang/iqpuzzle_nl.ts \
                 lang/iqpuzzle_zh_CN.ts

win32:RC_ICONS = icons/iqpuzzle.ico
os2:RC_ICONS   = icons/iqpuzzle_os2.ico

macx {
  ICON               = icons/icon.icns
  QMAKE_INFO_PLIST   = data/mac/Info.plist

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
    desktop.files += data/unix/com.github.elth0r0.iqpuzzle.desktop

    icons.path     = $$PREFIX/share/icons
    icons.files   += icons/hicolor

    man.path       = $$PREFIX/share
    man.files     += man

    meta.path      = $$PREFIX/share/metainfo
    meta.files    += data/unix/com.github.elth0r0.iqpuzzle.metainfo.xml

    INSTALLS      += target \
                     data \
                     desktop \
                     icons \
                     man \
                     meta
}
