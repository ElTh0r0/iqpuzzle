/**
 * \file main.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-2019 Thorsten Roth <elthoro@gmx.de>
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
 *
 * \section DESCRIPTION
 * Main function, start application, loading translation.
 */

/** \mainpage
 * \section Introduction
 * iQPuzzle is a diverting I.Q. challenging pentomino puzzle.<br />
 * GitHub: https://github.com/ElTh0r0/iqpuzzle
 */

#include <QApplication>
#include <QTextStream>

#include "./iqpuzzle.h"

static QFile logfile;
static QTextStream out(&logfile);

void setupLogger(const QString &sDebugFilePath,
                 const QString &sAppName,
                 const QString &sVersion);

void LoggingHandler(QtMsgType type,
                    const QMessageLogContext &context,
                    const QString &sMsg);

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName(APP_NAME);
  app.setApplicationVersion(APP_VERSION);

  QCommandLineParser cmdparser;
  cmdparser.setApplicationDescription(APP_DESC);
  cmdparser.addHelpOption();
  cmdparser.addVersionOption();
  QCommandLineOption enableDebug("debug", "Enable debug mode");
  cmdparser.addOption(enableDebug);
  cmdparser.addPositionalArgument("file", "Board file to be opened (*.conf) "
                                  "or savegame (*.iqsav)");
  cmdparser.process(app);

  // Default share data path (Windows and debugging)
  QString sSharePath = app.applicationDirPath();
  // Standard installation path (Linux)
  QDir tmpDir(app.applicationDirPath() + "/../share/"
              + app.applicationName().toLower());
  if (!cmdparser.isSet(enableDebug) && tmpDir.exists()) {
    sSharePath = app.applicationDirPath() + "/../share/"
                 + app.applicationName().toLower();
  }
#if defined(Q_OS_OSX)
  sSharePath = app.applicationDirPath() + "/../Resources/";
#endif

  QStringList sListPaths = QStandardPaths::standardLocations(
                             QStandardPaths::DataLocation);
  if (sListPaths.isEmpty()) {
    qCritical() << "Error while getting data standard path.";
    sListPaths << app.applicationDirPath();
  }
  const QDir userDataDir(sListPaths[0].toLower());

  // Create folder including possible parent directories (mkPATH)
  if (!userDataDir.exists()) {
    userDataDir.mkpath(userDataDir.absolutePath());
  }

  const QString sDebugFile(QStringLiteral("Debug.log"));
  setupLogger(userDataDir.absolutePath() + "/" + sDebugFile,
              app.applicationName(), app.applicationVersion());

  IQPuzzle myIQPuzzle(userDataDir, sSharePath);
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
  logfile.setFileName(sDebugFilePath);
  if (!logfile.open(QIODevice::WriteOnly)) {
    qWarning() << "Couldn't create logging file: " << sDebugFilePath;
  } else {
    qInstallMessageHandler(LoggingHandler);
  }

  qDebug() << sAppName << sVersion;
  qDebug() << "Compiled with Qt" << QT_VERSION_STR;
  qDebug() << "Qt runtime" <<  qVersion();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void LoggingHandler(QtMsgType type,
                    const QMessageLogContext &context,
                    const QString &sMsg) {
  QString sMsg2(sMsg);
  QString sContext = sMsg + " (" +
                     QString(context.file) + ":" +
                     QString::number(context.line) + ", " +
                     QString(context.function) + ")";

  QString sTime(QTime::currentTime().toString());

  switch (type) {
    case QtDebugMsg:
      out << sTime << " Debug: " << sMsg2 << "\n";
      out.flush();
      break;
    case QtWarningMsg:
      out << sTime << " Warning: " << sContext << "\n";
      out.flush();
      break;
    case QtCriticalMsg:
      out << sTime << " Critical: " << sContext << "\n";
      out.flush();
      break;
    case QtFatalMsg:
      out << sTime << " Fatal: " << sContext << "\n";
      out.flush();
      logfile.close();
      abort();
    default:
      out << sTime << " OTHER INFO: " << sContext << "\n";
      out.flush();
      break;
  }
}
