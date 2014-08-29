#  This file is part of iQPuzzle.
#  Copyright (C) 2012-2014 Thorsten Roth
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
TARGET        = iqpuzzle

VERSION       = 0.8.0
QMAKE_TARGET_PRODUCT     = "iQPuzzle"
QMAKE_TARGET_DESCRIPTION = "A diverting I.Q. challenging pentomino puzzle"
QMAKE_TARGET_COPYRIGHT   = "(C) 2012-2014 Thorsten Roth"

DEFINES      += APP_NAME=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\" \
                APP_VERSION=\"\\\"$$VERSION\\\"\" \
                APP_DESC=\"\\\"$$QMAKE_TARGET_DESCRIPTION\\\"\" \
                APP_COPY=\"\\\"$$QMAKE_TARGET_COPYRIGHT\\\"\"

MOC_DIR       = ./.moc
OBJECTS_DIR   = ./.objs
UI_DIR        = ./.ui
RCC_DIR       = ./.rcc

QT           += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES      += main.cpp\
                CIQPuzzle.cpp \
                CBoard.cpp \
                CBlock.cpp \
                CBoardDialog.cpp \
                CHighscore.cpp

HEADERS      += CIQPuzzle.h \
                CBoard.h \
                CBlock.h \
                CBoardDialog.h \
                CHighscore.h

FORMS        += CIQPuzzle.ui

RESOURCES     = res/iqpuzzle_resources.qrc

TRANSLATIONS += lang/iqpuzzle_de.ts

win32 {
    RC_FILE   = res/iqpuzzle.rc
}

unix {
    data.path      = /usr/share/games/iqpuzzle
    data.files    += data/boards
    lang.path      = /usr/share/games/iqpuzzle/lang
    lang.files    += lang/*.qm
    desktop.path   = /usr/share/applications
    desktop.files += data/iqpuzzle.desktop
    pixmap.path    = /usr/share/pixmaps
    pixmap.files  += res/images/iqpuzzle_64x64.png \
                     res/images/iqpuzzle.xpm
    icons.path     = /usr/share/icons
    icons.files   += res/images/hicolor
    man.path       = /usr/share
    man.files     += man
    target.path    = /usr/games
    INSTALLS      += data \
                     lang \
                     desktop \
                     pixmap \
                     icons \
                     man \
                     target
}
