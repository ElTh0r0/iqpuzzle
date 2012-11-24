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
#include <fstream>

#include "./CIQPuzzle.h"

std::ofstream logfile;

void setupLogger(const QString &sDebugFilePath,
                 const QString &sAppName,
                 const QString &sVersion);
void LoggingHandler(QtMsgType type, const char *sMsg);

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(iqpuzzle_resources);

    QApplication app(argc, argv);
    app.setApplicationName("iQPuzzle");
    app.setApplicationVersion(sVERSION);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    /*
    const QString sDebugFile("Debug.log");
    setupLogger(app.applicationDirPath() + "/" + sDebugFile,
                app.applicationName(), app.applicationVersion());
    */

    CIQPuzzle myIQPuzzle(&app);
    myIQPuzzle.show();
    int nRet = app.exec();

    logfile.close();
    return nRet;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void setupLogger(const QString &sDebugFilePath,
                 const QString &sAppName,
                 const QString &sVersion) {
    // Remove old debug file
    if (QFile(sDebugFilePath).exists()) {
        QFile(sDebugFilePath).remove();
    }

    // Create new file
    logfile.open(sDebugFilePath.toStdString().c_str(), std::ios::app);
    qInstallMsgHandler(LoggingHandler);
    qDebug() << sAppName << sVersion;
    qDebug() << "Compiled with Qt" << QT_VERSION_STR;
    qDebug() << "Qt runtime" <<  qVersion();
}

// ----------------------------------------------------------------------------
// Source: http://www.developer.nokia.com/Community/Wiki/File_based_logging_in_Qt_for_debugging

void LoggingHandler(QtMsgType type, const char *sMsg) {
    switch (type) {
        case QtDebugMsg:
            logfile << QTime::currentTime().toString().toAscii().data() <<
                       " Debug: " << sMsg << "\n";
            logfile.flush();
            break;
        case QtCriticalMsg:
            logfile << QTime::currentTime().toString().toAscii().data() <<
                       " Critical: " << sMsg << "\n";
            logfile.flush();
            break;
        case QtWarningMsg:
            logfile << QTime::currentTime().toString().toAscii().data() <<
                       " Warning: " << sMsg << "\n";
            logfile.flush();
            break;
        case QtFatalMsg:
            logfile << QTime::currentTime().toString().toAscii().data() <<
                       " Fatal: " << sMsg << "\n";
            logfile.flush();
            logfile.close();
    }
}
