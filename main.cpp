/**
 * \file main.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012 Thorsten Roth <elthoro@gmx.de>
 *
 * This file is part of iQPuzzle.
 *
 * iQPuzzle is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * iQPuzzle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with iQPuzzle.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGui/QApplication>
#include "./CIQPuzzle.h"

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(iqpuzzle_resources);

    QApplication app(argc, argv);
    app.setApplicationName("iQPuzzle");
    app.setApplicationVersion( sVERSION );

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    qDebug() << app.applicationName() << app.applicationVersion();
    qDebug() << "Compiled with Qt" << QT_VERSION_STR;
    qDebug() << "Qt runtime" <<  qVersion();

    CIQPuzzle myIQPuzzle(&app);
    myIQPuzzle.show();
    
    return app.exec();
}
