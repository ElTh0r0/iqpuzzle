/**
 * \file main.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-2014 Thorsten Roth <elthoro@gmx.de>
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

#include <QApplication>

#include <iostream>
#include <fstream>

#include "./CIQPuzzle.h"

std::ofstream logfile;

void setupLogger(const QString &sDebugFilePath,
                 const QString &sAppName,
                 const QString &sVersion);

#if QT_VERSION >= 0x050000
    void LoggingHandler(QtMsgType type,
                        const QMessageLogContext &context,
                        const QString &sMsg);
#else
    void LoggingHandler(QtMsgType type, const char *sMsg);
#endif

// Don't change this value! Use "--debug" command line option instead.
bool bDEBUG = false;

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(iqpuzzle_resources);

    QApplication app(argc, argv);
    app.setApplicationName(APP_NAME);
    app.setApplicationVersion(APP_VERSION);

    if (app.arguments().size() >= 2) {
        if (app.arguments().contains("-v")
                || app.arguments().contains("--version")) {
            std::cout << app.arguments()[0].toStdString() << "\t v"
                      << app.applicationVersion().toStdString() << std::endl;
            exit(0);
        } else if (app.arguments().contains("--debug")) {
            qWarning() << "Debugging enabled!";
            bDEBUG = true;
        }
    }

    // TODO: Add translation

    /*
    const QString sDebugFile("Debug.log");
    setupLogger(app.applicationDirPath() + "/" + sDebugFile,
                app.applicationName(), app.applicationVersion());
    */

    CIQPuzzle myIQPuzzle;
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
#if QT_VERSION >= 0x050000
    qInstallMessageHandler(LoggingHandler);
    // qInstallMessageHandler(0);
#else
    qInstallMsgHandler(LoggingHandler);
#endif
    qDebug() << sAppName << sVersion;
    qDebug() << "Compiled with Qt" << QT_VERSION_STR;
    qDebug() << "Qt runtime" <<  qVersion();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#if QT_VERSION >= 0x050000
void LoggingHandler(QtMsgType type,
                    const QMessageLogContext &context,
                    const QString &sMsg) {
    QString sMsg2(sMsg);
    QString sContext = sMsg + " (" +
            QString(context.file) + ":" +
            QString::number(context.line) + ", " +
            QString(context.function) + ")";
#else
void LoggingHandler(QtMsgType type, const char *sMsg) {
    QString sMsg2(sMsg);
    QString sContext(sMsg);
#endif
    char* sTime(QTime::currentTime().toString().toLatin1().data());

    switch (type) {
    case QtDebugMsg:
        logfile << sTime << " Debug: " << sMsg2.toStdString().c_str() << "\n";
        logfile.flush();
        break;
    case QtWarningMsg:
        logfile << sTime << " Warning: " << sContext.toStdString().c_str() << "\n";
        logfile.flush();
        break;
    case QtCriticalMsg:
        logfile << sTime << " Critical: " << sContext.toStdString().c_str() << "\n";
        logfile.flush();
        break;
    case QtFatalMsg:
        logfile << sTime << " Fatal: " << sContext.toStdString().c_str() << "\n";
        logfile.flush();
        logfile.close();
        abort();
    }
}
