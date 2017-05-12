/**
 * \file CIQPuzzle.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-2017 Thorsten Roth <elthoro@gmx.de>
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
 * Main application generation (gui, object creation etc.).
 */

#include <QApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

#include "./CIQPuzzle.h"
#include "ui_CIQPuzzle.h"

CIQPuzzle::CIQPuzzle(const QDir userDataDir, const QDir &sharePath,
                     QWidget *pParent)
  : QMainWindow(pParent),
    m_pUi(new Ui::CIQPuzzle),
    m_sCurrLang(""),
    m_pBoardDialog(NULL),
    m_pBoard(NULL),
    m_sSavedGame(""),
    m_userDataDir(userDataDir),
    m_sSharePath(sharePath.absolutePath()),
    m_nMoves(0),
    m_sSavedTime(""),
    m_sSavedMoves(""),
    m_Time(0, 0, 0),
    m_bSolved(false) {
  qDebug() << Q_FUNC_INFO;

  m_pUi->setupUi(this);
  this->setWindowTitle(qApp->applicationName());
  {
    QIcon fallback(":/images/hicolor/256x256/apps/iqpuzzle.png");
    fallback.addFile(":/images/hicolor/128x128/apps/iqpuzzle.png");
    fallback.addFile(":/images/hicolor/96x96/apps/iqpuzzle.png");
    fallback.addFile(":/images/hicolor/64x64/apps/iqpuzzle.png");
    fallback.addFile(":/images/hicolor/48x48/apps/iqpuzzle.png");
    fallback.addFile(":/images/hicolor/32x32/apps/iqpuzzle.png");
    fallback.addFile(":/images/hicolor/24x24/apps/iqpuzzle.png");
    fallback.addFile(":/images/hicolor/16x16/apps/iqpuzzle.png");
    this->setWindowIcon(QIcon::fromTheme("iqpuzzle", fallback));
  }

  m_pHighscore = new CHighscore();
  m_pSettings = new CSettings(m_sSharePath, this);
  connect (m_pSettings, SIGNAL(changeLang(QString)),
           this, SLOT(loadLanguage(QString)));
  connect (this, SIGNAL(updateUiLang()),
           m_pSettings, SLOT(updateUiLang()));
  this->loadLanguage(m_pSettings->getLanguage());
  this->setupMenu();

  m_pGraphView = new QGraphicsView(this);
  this->setCentralWidget(m_pGraphView);
  m_pScenePaused = new QGraphicsScene(this);
  m_pScenePaused->setBackgroundBrush(QBrush(QColor("#EEEEEE")));
  QFont font;
  font.setPixelSize(20);
  m_pTextPaused = m_pScenePaused->addText(trUtf8("Game paused"), font);

  m_pTimer = new QTimer(this);
  connect(m_pTimer, SIGNAL(timeout()), this, SLOT(updateTimer()));
  m_pStatusLabelTime = new QLabel(trUtf8("Time") + ": 00:00:00");
  m_pStatusLabelMoves = new QLabel(trUtf8("Moves") + ": 0");
  m_pUi->statusBar->addWidget(m_pStatusLabelTime);
  m_pUi->statusBar->addPermanentWidget(m_pStatusLabelMoves);

  // Choose board via command line
  QString sStartBoard("");
  if (qApp->arguments().size() > 1) {
    foreach (QString sBoard, qApp->arguments()) {
      if (sBoard.endsWith(".conf", Qt::CaseInsensitive)) {
        if (QFile::exists(sBoard)) {
          sStartBoard = sBoard;
          break;
        } else {
          qWarning() << "Specified board not found:" << sBoard;
          break;
        }
      }
    }
  }

  // Start rectangle_001 as default
  if (sStartBoard.isEmpty()) {
    if (QFile::exists(m_sSharePath + "/boards/rectangles")) {
      sStartBoard = m_sSharePath + "/boards/rectangles/rectangle_001.conf";
    } else {
      qWarning() << "Games share path does not exist:" << m_sSharePath;
    }
  }

  // Seed random number generator
  QTime time = QTime::currentTime();
  qsrand((uint)time.msec());

  this->startNewGame(sStartBoard);
}

CIQPuzzle::~CIQPuzzle() {
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::setupMenu() {
  qDebug() << Q_FUNC_INFO;

  // New game
  m_pUi->action_NewGame->setShortcut(QKeySequence::New);
  connect(m_pUi->action_NewGame, SIGNAL(triggered()),
          this, SLOT(startNewGame()));

  // Random game
  m_pUi->action_RandomGame->setShortcut(Qt::CTRL + Qt::Key_R);
  connect(m_pUi->action_RandomGame, SIGNAL(triggered()),
          this, SLOT(randomGame()));

  // Restart game
  m_pUi->action_RestartGame->setShortcut(QKeySequence::Refresh);
  connect(m_pUi->action_RestartGame, SIGNAL(triggered()),
          this, SLOT(restartGame()));

  // Load game
  m_pUi->action_LoadGame->setShortcut(QKeySequence::Open);
  connect(m_pUi->action_LoadGame, SIGNAL(triggered()),
          this, SLOT(loadGame()));
  // Save game
  m_pUi->action_SaveGame->setShortcut(QKeySequence::Save);
  connect(m_pUi->action_SaveGame, SIGNAL(triggered()),
          this, SLOT(saveGame()));

  // Pause
  m_pUi->action_PauseGame->setShortcut(Qt::Key_P);
  connect(m_pUi->action_PauseGame, SIGNAL(triggered(bool)),
          this, SLOT(pauseGame(bool)));

  // Highscore
  m_pUi->action_Highscore->setShortcut(Qt::CTRL + Qt::Key_H);
  connect(m_pUi->action_Highscore, SIGNAL(triggered()),
          this, SLOT(showHighscore()));
  connect(this, SIGNAL(showHighscore(QString)),
          m_pHighscore, SLOT(showHighscore(QString)));
  connect(this, SIGNAL(checkHighscore(QString, quint32, QTime)),
          m_pHighscore, SLOT(checkHighscore(QString, quint32, QTime)));

  // Exit game
  m_pUi->action_Quit->setShortcut(QKeySequence::Quit);
  connect(m_pUi->action_Quit, SIGNAL(triggered()),
          this, SLOT(close()));

  // Zoom in/out
  m_pUi->action_ZoomIn->setShortcut(QKeySequence::ZoomIn);
  m_pUi->action_ZoomOut->setShortcut(QKeySequence::ZoomOut);

  // Settings
  connect(m_pUi->action_Preferences, SIGNAL(triggered()),
          m_pSettings, SLOT(show()));

  // Report bug
  connect(m_pUi->action_ReportBug, SIGNAL(triggered()),
          this, SLOT(reportBug()));

  // About
  connect(m_pUi->action_Info, SIGNAL(triggered()),
          this, SLOT(showInfoBox()));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::startNewGame(QString sBoardFile, const QString sSavedGame,
                             const QString sTime, const QString sMoves) {
  qDebug() << Q_FUNC_INFO;

  if (sBoardFile.isEmpty()) {
    sBoardFile = this->chooseBoard();
    if (sBoardFile.isEmpty()) {
      return;
    }
  }

  qDebug() << "Board:" << sBoardFile;
  if (!QFile::exists(sBoardFile)) {
    QMessageBox::warning(this, trUtf8("File not found"),
                         trUtf8("The chosen file does not exist."));
    qWarning() << "Board file not found:" << sBoardFile;
    return;
  }
  m_sBoardFile = sBoardFile;
  m_sSavedGame = "";

  if (!sSavedGame.isEmpty()) {
    qDebug() << "Saved game:" << sSavedGame;
    if (!QFile::exists(sSavedGame)) {
      QMessageBox::warning(this, trUtf8("File not found"),
                           trUtf8("The chosen file does not exist."));
      qWarning() << "Saved game not found:" << sSavedGame;
      return;
    }
    m_nMoves = QString(sMoves).toUInt();
    m_pStatusLabelMoves->setText(trUtf8("Moves") + ": "
                                 + QString::number(m_nMoves));
    m_Time = m_Time.fromString(sTime, "hh:mm:ss");
    m_pStatusLabelTime->setText(trUtf8("Time") + ": " + sTime);
    m_sSavedGame = sSavedGame;
  } else {
    m_nMoves = 0;
    m_pStatusLabelMoves->setText(trUtf8("Moves") + ": 0");
    m_Time = m_Time.fromString("00:00:00", "hh:mm:ss");
    m_pStatusLabelTime->setText(trUtf8("Time") + ": 00:00:00");
  }

  this->setGameTitle();
  this->createBoard();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::setGameTitle() {
  QSettings tmpSet(m_sBoardFile, QSettings::IniFormat);
  quint32 nSolutions = tmpSet.value("PossibleSolutions", 0).toUInt();
  QString sSolutions(QString::number(nSolutions));
  if ("0" == sSolutions) {
    sSolutions = trUtf8("Unknown");
  }

  this->setWindowTitle(qApp->applicationName() + " - " +
                       QFileInfo(m_sBoardFile).baseName() + " ("
                       + trUtf8("Solutions") + ": " + sSolutions + ")");
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QString CIQPuzzle::chooseBoard() {
  if (NULL != m_pBoardDialog) {
    delete m_pBoardDialog;
  }
  m_pBoardDialog = new CBoardDialog(this, trUtf8("Load board"),
                                    m_sSharePath + "/boards",
                                    trUtf8("Board files") + " (*.conf)");

  if (m_pBoardDialog->exec()) {
    QStringList sListFiles;
    sListFiles = m_pBoardDialog->selectedFiles();
    if (sListFiles.size() >= 1) {
      return sListFiles.first();
    }
  }

  return "";
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::createBoard() {
  if (NULL != m_pBoard) {
    delete m_pBoard;
  }
  m_pBoard = new CBoard(m_pGraphView, m_sBoardFile,
                        m_pSettings, m_sSavedGame);
  connect(m_pBoard, SIGNAL(setWindowSize(const QSize)),
          this, SLOT(setMinWindowSize(const QSize)));
  connect(m_pUi->action_ZoomIn, SIGNAL(triggered()),
          m_pBoard, SLOT(zoomIn()));
  connect(m_pUi->action_ZoomOut, SIGNAL(triggered()),
          m_pBoard, SLOT(zoomOut()));
  connect(m_pBoard, SIGNAL(incrementMoves()),
          this, SLOT(incrementMoves()));
  connect(m_pBoard, SIGNAL(solvedPuzzle()),
          this, SLOT(solvedPuzzle()));

  if (m_pBoard->setupBoard()) {
    m_pBoard->setupBlocks();
    m_pTimer->start(1000);
    m_pUi->action_PauseGame->setEnabled(true);
    m_pUi->action_PauseGame->setChecked(false);
    m_pUi->action_SaveGame->setEnabled(true);
    m_pUi->action_RestartGame->setEnabled(true);
    m_bSolved = false;
    m_pGraphView->setScene(m_pBoard);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::randomGame() {
  qDebug() << Q_FUNC_INFO;
  if (!QFile::exists(m_sSharePath + "/boards")) {
    qWarning() << "Games share path does not exist" << m_sSharePath;
    QMessageBox::warning(this, "Folder not found",
                         "Games share path does not exist!");
    return;
  } else {
    QStringList slistBoards;
    QDir boardsDir(m_sSharePath + "/boards");
    QFileInfoList fiListFiles = boardsDir.entryInfoList(
                                  QDir::NoDotAndDotDot |
                                  QDir::Files |
                                  QDir::Dirs);
    foreach (QFileInfo fi, fiListFiles) {
      if (fi.isDir()) {  // Check only one sub folder level
        QDir boardsDir2(boardsDir.absolutePath() + "/" + fi.fileName());
        QFileInfoList fiListFiles2 = boardsDir2.entryInfoList(
                                       QDir::NoDotAndDotDot | QDir::Files);
        foreach (QFileInfo fi2, fiListFiles2) {
          if ("conf" == fi2.suffix()) {
            // qDebug() << fi.fileName() + "/" + fi2.baseName();
            slistBoards << fi.fileName() + "/" + fi2.fileName();
          }
        }
      } else if ("conf" == fi.suffix()) {
        // qDebug() << fi.baseName();
        slistBoards << fi.fileName();
      }
    }
    int nRand = qrand() % slistBoards.size();
    if (nRand >= 0 && nRand < slistBoards.size()) {
      this->startNewGame(m_sSharePath + "/boards/" + slistBoards[nRand]);
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::restartGame() {
  this->startNewGame(m_sBoardFile, m_sSavedGame, m_sSavedTime, m_sSavedMoves);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::loadGame() {
  QString sSaved = QFileDialog::getOpenFileName(this, trUtf8("Load game"),
                                                m_userDataDir.absolutePath());
  if (!sSaved.isEmpty()) {
    QSettings tmpSet(sSaved, QSettings::IniFormat);
    QString sBoard(tmpSet.value("BoardFile", "").toString());
    if (!sBoard.isEmpty()) {
      QByteArray ba(tmpSet.value("NumOfMoves", "").toByteArray());
      m_sSavedMoves = QByteArray::fromBase64(ba);
      ba.clear();
      ba = tmpSet.value("ElapsedTime", "").toByteArray();
      m_sSavedTime = QByteArray::fromBase64(ba);
      this->startNewGame(sBoard, sSaved, m_sSavedTime, m_sSavedMoves);
    } else {
      QMessageBox::warning(this, qApp->applicationName(),
                           trUtf8("Invalid saved puzzle."));
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::saveGame() {
  QString sFile = QFileDialog::getSaveFileName(this, trUtf8("Save game"),
                                               m_userDataDir.absolutePath());
  if (!sFile.isEmpty()) {
    m_sSavedMoves = QString::number(m_nMoves);
    m_sSavedTime = m_Time.toString("hh:mm:ss");
    m_pBoard->saveGame(sFile, m_sSavedTime, m_sSavedMoves);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::pauseGame(const bool bPaused) {
  if (!m_bSolved) {
    if (bPaused) {
      m_pTimer->stop();
      m_pGraphView->setEnabled(false);
      m_pGraphView->setScene(m_pScenePaused);
    } else {
      m_pTimer->start();
      m_pGraphView->setEnabled(true);
      m_pGraphView->setScene(m_pBoard);
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::setMinWindowSize(const QSize size) {
  static QSize size2(100, 100);
  if (!size.isEmpty()) {
    size2 = size;
  }

  this->resize(size2);
  m_pTextPaused->setX(
        size2.width()/2.5/2 - m_pTextPaused->boundingRect().width()/2);
  m_pTextPaused->setY(
        size2.height()/2.6/2 - m_pTextPaused->boundingRect().height()/2);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::incrementMoves() {
  m_nMoves++;
  m_pStatusLabelMoves->setText(
        trUtf8("Moves") + ": " + QString::number(m_nMoves));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::updateTimer() {
  m_Time = m_Time.addSecs(1);
  m_pStatusLabelTime->setText(
        trUtf8("Time") + ": " + m_Time.toString("hh:mm:ss"));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::solvedPuzzle() {
  QFileInfo fi(m_sBoardFile);
  m_pTimer->stop();
  m_bSolved = true;
  QMessageBox::information(this, qApp->applicationName(),
                           trUtf8("Puzzle solved!") + "\n\n" +
                           trUtf8("Moves") + ": " + QString::number(m_nMoves)
                           + "\n" + trUtf8("Time") + ": "
                           + m_Time.toString("hh:mm:ss"));
  m_pUi->action_PauseGame->setEnabled(false);
  m_pUi->action_PauseGame->setChecked(false);
  m_pUi->action_SaveGame->setEnabled(false);

  // Save won game state for debugging
  m_pBoard->saveGame(m_userDataDir.absolutePath() + "/S0LV3D.debug",
                     "55:55:55", "10000");
  QFile::remove(m_userDataDir.absolutePath() + "/S0LV3D.debug");

  emit checkHighscore(fi.baseName(), m_nMoves, m_Time);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CIQPuzzle::loadLanguage(const QString &sLang) {
  if (m_sCurrLang != sLang) {
    m_sCurrLang = sLang;
    if (!this->switchTranslator(m_translatorQt, "qt_" + sLang,
                                QLibraryInfo::location(
                                  QLibraryInfo::TranslationsPath))) {
      this->switchTranslator(m_translatorQt, "qt_" + sLang,
                             m_sSharePath + "/lang");
    }

    this->switchTranslator(m_translator,
                           qApp->applicationName().toLower() + "_" + sLang,
                           m_sSharePath + "/lang");
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool CIQPuzzle::switchTranslator(QTranslator &translator,
                                 const QString &sFile, const QString &sPath) {
  qApp->removeTranslator(&translator);
  if (translator.load(sFile, sPath)) {
    qApp->installTranslator(&translator);
  } else {
    if (!sFile.endsWith("_en")) {  // EN is build in translation -> no file
      qWarning() << "Could not find translation" << sFile << "in" << sPath;
    }
    return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::showHighscore() {
  QFileInfo fi(m_sBoardFile);
  emit showHighscore(fi.baseName());
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::reportBug() const {
  QDesktopServices::openUrl(QUrl("https://github.com/ElTh0r0/iqpuzzle/issues"));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CIQPuzzle::showInfoBox() {
  QMessageBox::about(this, trUtf8("About"),
                     QString("<center>"
                             "<big><b>%1 %2</b></big><br />"
                             "%3<br />"
                             "<small>%4</small><br /><br />"
                             "%5<br />"
                             "%6<br />"
                             "<small>%7</small>"
                             "</center><br />"
                             "%8")
                     .arg(qApp->applicationName())
                     .arg(qApp->applicationVersion())
                     .arg(APP_DESC)
                     .arg(APP_COPY)
                     .arg("URL: <a href=\"https://github.com/ElTh0r0/iqpuzzle\">"
                          "https://github.com/ElTh0r0/iqpuzzle</a>")
                     .arg(trUtf8("License") +
                          ": "
                          "<a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">"
                          "GNU General Public License Version 3</a>")
                     .arg(trUtf8("This application uses icons from "
                                 "<a href=\"http://tango.freedesktop.org\">"
                                 "Tango project</a>."))
                     .arg("<i>" + trUtf8("Translations") +
                          "</i><br />"
                          "&nbsp;&nbsp;- Bulgarian: bogo1966<br />"
                          "&nbsp;&nbsp;- Dutch: Elbert Pol<br />"
                          "&nbsp;&nbsp;- German: ElThoro"));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::changeEvent(QEvent *pEvent) {
  if (0 != pEvent) {
    if (QEvent::LanguageChange == pEvent->type()) {
      m_pUi->retranslateUi(this);
      this->setGameTitle();

      m_pScenePaused->removeItem(m_pTextPaused);
      QFont font;
      font.setPixelSize(20);
      m_pTextPaused = m_pScenePaused->addText(trUtf8("Game paused"), font);
      this->setMinWindowSize();

      m_pStatusLabelMoves->setText(
            trUtf8("Moves") + ": " + QString::number(m_nMoves));
      emit updateUiLang();
    }
  }
  QMainWindow::changeEvent(pEvent);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// Close event (File -> Close or X)
void CIQPuzzle::closeEvent(QCloseEvent *pEvent) {
  pEvent->accept();
  /*
  int nRet = QMessageBox::question(this, trUtf8("Quit") + " - " +
                                   qApp->applicationName(),
                                   trUtf8("Do you really want to quit?"),
                                   QMessageBox::Yes | QMessageBox::No);

  if (QMessageBox::Yes == nRet) {
    pEvent->accept();
  } else {
    pEvent->ignore();
  }
  */
}
