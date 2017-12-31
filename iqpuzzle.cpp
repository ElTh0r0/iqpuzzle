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

#include "./iqpuzzle.h"
#include "ui_iqpuzzle.h"

IQPuzzle::IQPuzzle(const QDir userDataDir, const QDir &sharePath,
                   QWidget *pParent)
  : QMainWindow(pParent),
    m_pUi(new Ui::IQPuzzle),
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

  m_pHighscore = new Highscore();
  m_pSettings = new Settings(m_sSharePath, this);
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

  // Seed random number generator
  QTime time = QTime::currentTime();
  qsrand((uint)time.msec());
  this->generateFileLists();

  // Choose board via command line
  QString sStartBoard("");
  QString sLoadBoard("");
  if (qApp->arguments().size() > 1) {
    foreach (QString sBoard, qApp->arguments()) {
      if (sBoard.endsWith(".conf", Qt::CaseInsensitive)) {
        if (QFile::exists(sBoard)) {
          sStartBoard = sBoard;
          break;
        } else {
          qWarning() << "Specified board not found:" << sBoard;
          QMessageBox::warning(this, trUtf8("File not found"),
                               trUtf8("The chosen file does not exist."));
          break;
        }
      } else if (sBoard.endsWith(".iqsav", Qt::CaseInsensitive)) {
        if (QFile::exists(sBoard)) {
          sLoadBoard = sBoard;
          break;
        } else {
          qWarning() << "Specified save game not found:" << sBoard;
          QMessageBox::warning(this, trUtf8("File not found"),
                               trUtf8("The chosen file does not exist."));
          break;
        }
      }
    }
  }

  if (!sLoadBoard.isEmpty()) {
      this->loadGame(sLoadBoard);
  } else {
    if (sStartBoard.isEmpty()) {  // Start rectangle_001 as default
      if (QFile::exists(m_sSharePath + "/boards/rectangles")) {
        sStartBoard = m_sSharePath + "/boards/rectangles/rectangle_001.conf";
      } else {
        qWarning() << "Games share path does not exist:" << m_sSharePath;
        QMessageBox::warning(this, qApp->applicationName(),
                             trUtf8("Games share path does not exist!"));
      }
    }
    this->startNewGame(sStartBoard);
  }
}

IQPuzzle::~IQPuzzle() {
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::setupMenu() {
  // New game
  m_pUi->action_NewGame->setShortcut(QKeySequence::New);
  connect(m_pUi->action_NewGame, SIGNAL(triggered()),
          this, SLOT(startNewGame()));

  // Random game
  m_pSigMapRandom = new QSignalMapper(this);
  m_pUi->actionAll->setShortcut(Qt::CTRL + Qt::Key_1);
  m_pSigMapRandom->setMapping(m_pUi->actionAll, 1);
  connect(m_pUi->actionAll, SIGNAL(triggered()),
          m_pSigMapRandom, SLOT(map()));
  m_pUi->actionEasy->setShortcut(Qt::CTRL + Qt::Key_2);
  m_pSigMapRandom->setMapping(m_pUi->actionEasy, 2);
  connect(m_pUi->actionEasy, SIGNAL(triggered()),
          m_pSigMapRandom, SLOT(map()));
  m_pUi->actionMedium->setShortcut(Qt::CTRL + Qt::Key_3);
  m_pSigMapRandom->setMapping(m_pUi->actionMedium, 3);
  connect(m_pUi->actionMedium, SIGNAL(triggered()),
          m_pSigMapRandom, SLOT(map()));
  m_pUi->actionHard->setShortcut(Qt::CTRL + Qt::Key_4);
  m_pSigMapRandom->setMapping(m_pUi->actionHard, 4);
  connect(m_pUi->actionHard, SIGNAL(triggered()),
          m_pSigMapRandom, SLOT(map()));
  m_pUi->actionAllUnsolved->setShortcut(Qt::CTRL + Qt::Key_5);
  m_pSigMapRandom->setMapping(m_pUi->actionAllUnsolved, 5);
  connect(m_pUi->actionAllUnsolved, SIGNAL(triggered()),
          m_pSigMapRandom, SLOT(map()));
  m_pUi->actionEasyUnsolved->setShortcut(Qt::CTRL + Qt::Key_6);
  m_pSigMapRandom->setMapping(m_pUi->actionEasyUnsolved, 6);
  connect(m_pUi->actionEasyUnsolved, SIGNAL(triggered()),
          m_pSigMapRandom, SLOT(map()));
  m_pUi->actionMediumUnsolved->setShortcut(Qt::CTRL + Qt::Key_7);
  m_pSigMapRandom->setMapping(m_pUi->actionMediumUnsolved, 7);
  connect(m_pUi->actionMediumUnsolved, SIGNAL(triggered()),
          m_pSigMapRandom, SLOT(map()));
  m_pUi->actionHardUnsolved->setShortcut(Qt::CTRL + Qt::Key_8);
  m_pSigMapRandom->setMapping(m_pUi->actionHardUnsolved, 8);
  connect(m_pUi->actionHardUnsolved, SIGNAL(triggered()),
          m_pSigMapRandom, SLOT(map()));
  connect(m_pSigMapRandom, SIGNAL(mapped(int)),
          this, SLOT(randomGame(int)));

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

void IQPuzzle::startNewGame(QString sBoardFile, const QString sSavedGame,
                            const QString sTime, const QString sMoves) {
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

void IQPuzzle::setGameTitle() {
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

QString IQPuzzle::chooseBoard() {
  if (NULL != m_pBoardDialog) {
    delete m_pBoardDialog;
  }
  m_pBoardDialog = new BoardDialog(this, trUtf8("Load board"),
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

void IQPuzzle::createBoard() {
  if (NULL != m_pBoard) {
    delete m_pBoard;
  }
  m_pBoard = new Board(m_pGraphView, m_sBoardFile, m_pSettings, m_sSavedGame);
  connect(m_pBoard, SIGNAL(setWindowSize(const QSize, const bool)),
          this, SLOT(setMinWindowSize(const QSize, const bool)));
  connect(m_pUi->action_ZoomIn, SIGNAL(triggered()),
          m_pBoard, SLOT(zoomIn()));
  connect(m_pUi->action_ZoomOut, SIGNAL(triggered()),
          m_pBoard, SLOT(zoomOut()));
  connect(m_pBoard, SIGNAL(incrementMoves()),
          this, SLOT(incrementMoves()));
  connect(m_pBoard, SIGNAL(solvedPuzzle()),
          this, SLOT(solvedPuzzle()));

  if (m_pBoard->setupBoard()) {
    bool bFreestyle = m_pBoard->setupBlocks();
    if (bFreestyle) {
      m_pTimer->stop();
      m_pUi->action_PauseGame->setEnabled(false);
      m_pUi->action_Highscore->setEnabled(false);
    } else {
      m_pTimer->start(1000);
      m_pUi->action_PauseGame->setEnabled(true);
      m_pUi->action_Highscore->setEnabled(true);
    }

    m_pUi->action_PauseGame->setChecked(false);
    m_pUi->action_SaveGame->setEnabled(true);
    m_pUi->action_RestartGame->setEnabled(true);
    m_bSolved = false;
    m_pGraphView->setScene(m_pBoard);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::randomGame(const int nChoice) {
  qDebug() << "Random game:" << nChoice;

  if (nChoice > 0 && nChoice <= m_sListFiles.size()) {
    if (!m_sListFiles[nChoice-1]->isEmpty()) {
      int nRand = qrand() % m_sListFiles[nChoice-1]->size();
      if (nRand >= 0 && nRand < m_sListFiles[nChoice-1]->size()) {
        this->startNewGame(m_sSharePath + "/boards/" +
                           m_sListFiles[nChoice-1]->at(nRand));
      }
    } else {
      qWarning() << "Game file list is emtpy!";
      QMessageBox::warning(this, qApp->applicationName(),
                           trUtf8("No boards available!"));
    }
  } else {
    qWarning() << "Invalid random choice:" << nChoice;
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::generateFileLists() {
  QDir boardsDir(m_sSharePath + "/boards");
  QFileInfoList fiListFiles = boardsDir.entryInfoList(
                                QDir::NoDotAndDotDot |
                                QDir::Files |
                                QDir::Dirs);
#if defined _WIN32
  QSettings tmpScore(QSettings::IniFormat, QSettings::UserScope,
                     qApp->applicationName().toLower(), "Highscore");
#else
  QSettings tmpScore(QSettings::NativeFormat, QSettings::UserScope,
                     qApp->applicationName().toLower(), "Highscore");
#endif
  QString sName("");
  bool bSolved(false);
  quint32 nSolutions(0);
  quint16 nEasy(m_pSettings->getEasy());
  quint16 nHard(m_pSettings->getHard());

  foreach (QFileInfo fi, fiListFiles) {
    if (fi.isDir() &&
        !fi.fileName().startsWith("freestyle")) {  // Check only one subfolder
      QDir boardsDir2(boardsDir.absolutePath() + "/" + fi.fileName());
      QFileInfoList fiListFiles2 = boardsDir2.entryInfoList(
                                     QDir::NoDotAndDotDot | QDir::Files);
      foreach (QFileInfo fi2, fiListFiles2) {
        if ("conf" == fi2.suffix()) {
          sName = fi.fileName() + "/" + fi2.fileName();
          QSettings tmpSet(m_sSharePath + "/boards/" + sName, QSettings::IniFormat);
          nSolutions = tmpSet.value("PossibleSolutions", 0).toUInt();
          bSolved = tmpScore.childGroups().contains(fi2.baseName());

          // qDebug() << fi.fileName() + "/" + fi2.baseName();
          m_sListAll << sName;
          if (!bSolved) m_sListAllUnsolved << sName;
          if (nSolutions >= nEasy) {
            m_sListEasy << sName;
            if (!bSolved) m_sListEasyUnsolved << sName;
          } else if ((nHard < nSolutions) &&  (nSolutions < nEasy)) {
            m_sListMedium << sName;
            if (!bSolved) m_sListMediumUnsolved << sName;
          } else if ((0 < nSolutions) && (nSolutions <= nHard)) {
            m_sListHard << sName;
            if (!bSolved) m_sListHardUnsolved << sName;
          }
        }
      }
    } else if ("conf" == fi.suffix() &&
               !fi.fileName().startsWith("freestyle")) {
      sName = fi.fileName();
      QSettings tmpSet(m_sSharePath + "/boards/" + sName, QSettings::IniFormat);
      nSolutions = tmpSet.value("PossibleSolutions", 0).toUInt();
      bSolved = tmpScore.childGroups().contains(fi.baseName());

      // qDebug() << fi.baseName();
      m_sListAll << sName;
      if (!bSolved) m_sListAllUnsolved << sName;
      if (nSolutions >= nEasy) {
        m_sListEasy << sName;
        if (!bSolved) m_sListEasyUnsolved << sName;
      } else if ((nHard < nSolutions) &&  (nSolutions < nEasy)) {
        m_sListMedium << sName;
        if (!bSolved) m_sListMediumUnsolved << sName;
      } else if ((0 < nSolutions) && (nSolutions <= nHard)) {
        m_sListHard << sName;
        if (!bSolved) m_sListHardUnsolved << sName;
      }
    }
  }

  m_sListFiles << &m_sListAll << &m_sListEasy << &m_sListMedium <<
                  &m_sListHard << &m_sListAllUnsolved << &m_sListEasyUnsolved <<
                  &m_sListMediumUnsolved << &m_sListHardUnsolved;

  qDebug() << "Threshold easy:" << nEasy << " Threshold hard:" << nHard;
  qDebug() << "All:" << m_sListAll.size() <<
              "- unsolved:" << m_sListAllUnsolved.size();
  qDebug() << "Easy:" << m_sListEasy.size() <<
              "- unsolved:" << m_sListEasyUnsolved.size();
  qDebug() << "Medium:" << m_sListMedium.size() <<
              "- unsolved:" << m_sListMediumUnsolved.size();
  qDebug() << "Hard:" << m_sListHard.size() <<
              "- unsolved:" << m_sListHardUnsolved.size();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::restartGame() {
  this->startNewGame(m_sBoardFile, m_sSavedGame, m_sSavedTime, m_sSavedMoves);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::loadGame(QString sSaveFile) {
  if (sSaveFile.isEmpty()) {
    sSaveFile = QFileDialog::getOpenFileName(this, trUtf8("Load game"),
                                             m_userDataDir.absolutePath(),
                                             trUtf8("Save games") + "(*.iqsav)");
  }

  if (!sSaveFile.isEmpty()) {
    QSettings tmpSet(sSaveFile, QSettings::IniFormat);
    QString sBoard(tmpSet.value("BoardFile", "").toString());
    if (!sBoard.isEmpty()) {
      QByteArray ba(tmpSet.value("NumOfMoves", "").toByteArray());
      m_sSavedMoves = QByteArray::fromBase64(ba);
      ba.clear();
      ba = tmpSet.value("ElapsedTime", "").toByteArray();
      m_sSavedTime = QByteArray::fromBase64(ba);
      this->startNewGame(sBoard, sSaveFile, m_sSavedTime, m_sSavedMoves);
    } else {
      QMessageBox::warning(this, qApp->applicationName(),
                           trUtf8("Invalid saved puzzle."));
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::saveGame() {
  QString sFile = QFileDialog::getSaveFileName(this, trUtf8("Save game"),
                                               m_userDataDir.absolutePath(),
                                               trUtf8("Save games") + "(*.iqsav)");
  if (!sFile.isEmpty()) {
    m_sSavedMoves = QString::number(m_nMoves);
    m_sSavedTime = m_Time.toString("hh:mm:ss");
    m_pBoard->saveGame(sFile, m_sSavedTime, m_sSavedMoves);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::pauseGame(const bool bPaused) {
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

void IQPuzzle::setMinWindowSize(const QSize size, const bool bFreestyle) {
  static QSize size2(100, 100);
  if (!size.isEmpty()) {
    size2 = size;
  }

  if (this->size().width() < size2.width() ||
      this->size().height() < size2.height()) {
    this->showNormal();
    this->resize(size2);
  }
  m_pTextPaused->setX(
        size2.width()/2.5/2 - m_pTextPaused->boundingRect().width()/2);
  m_pTextPaused->setY(
        size2.height()/2.6/2 - m_pTextPaused->boundingRect().height()/2);

  if (bFreestyle) {
    m_pGraphView->centerOn(100,70);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::incrementMoves() {
  m_nMoves++;
  m_pStatusLabelMoves->setText(
        trUtf8("Moves") + ": " + QString::number(m_nMoves));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::updateTimer() {
  m_Time = m_Time.addSecs(1);
  m_pStatusLabelTime->setText(
        trUtf8("Time") + ": " + m_Time.toString("hh:mm:ss"));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::solvedPuzzle() {
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

  // Update "unsolved lists" for random games
  QString sBoard(m_sBoardFile);
  sBoard = sBoard.remove(m_sSharePath + "/boards/");
  m_sListAllUnsolved.removeAt(m_sListAllUnsolved.indexOf(sBoard));
  m_sListEasyUnsolved.removeAt(m_sListEasyUnsolved.indexOf(sBoard));
  m_sListMediumUnsolved.removeAt(m_sListMediumUnsolved.indexOf(sBoard));
  m_sListHardUnsolved.removeAt(m_sListHardUnsolved.indexOf(sBoard));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void IQPuzzle::loadLanguage(const QString &sLang) {
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

bool IQPuzzle::switchTranslator(QTranslator &translator,
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

void IQPuzzle::showHighscore() {
  QFileInfo fi(m_sBoardFile);
  emit showHighscore(fi.baseName());
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::reportBug() const {
  QDesktopServices::openUrl(QUrl("https://github.com/ElTh0r0/iqpuzzle/issues"));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void IQPuzzle::showInfoBox() {
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
                          "&nbsp;&nbsp;- French: kiarn, mothsART<br />"
                          "&nbsp;&nbsp;- German: ElThoro"));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::changeEvent(QEvent *pEvent) {
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
void IQPuzzle::closeEvent(QCloseEvent *pEvent) {
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
