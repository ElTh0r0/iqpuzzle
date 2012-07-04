
QT       += core gui

TARGET    = iQPuzzel
TEMPLATE  = app

RESOURCES = iqpuzzle_resources.qrc

SOURCES  += main.cpp\
            CIQPuzzle.cpp \
            CBlock.cpp

HEADERS  += CIQPuzzle.h \
            CBlock.h

FORMS    += CIQPuzzle.ui
