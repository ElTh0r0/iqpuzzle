/**
 * \file CIQPuzzle.cpp
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
 * Main application generation (gui, object creation etc.).
 */

#include "./iqpuzzle.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

#include "ui_iqpuzzle.h"

IQPuzzle::IQPuzzle(const QDir &userDataDir, const QDir &sharePath,
                   QWidget *pParent)
  : QMainWindow(pParent),
    m_pUi(new Ui::IQPuzzle),
    m_sCurrLang(QString()),
    m_pBoardDialog(nullptr),
    m_pBoard(nullptr),
    m_sSavedGame(QString()),
    m_userDataDir(userDataDir),
    m_sSharePath(sharePath.absolutePath()),
    m_nMoves(0),
    m_sSavedTime(QString()),
    m_sSavedMoves(QString()),
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
  connect(m_pSettings, &Settings::changeLang, this, &IQPuzzle::loadLanguage);
  connect(this, &IQPuzzle::updateUiLang, m_pSettings, &Settings::updateUiLang);
  this->loadLanguage(m_pSettings->getLanguage());
  this->setupMenu();

  m_pGraphView = new QGraphicsView(this);
  this->setCentralWidget(m_pGraphView);
  m_pScenePaused = new QGraphicsScene(this);
  m_pScenePaused->setBackgroundBrush(QBrush(QColor(238, 238, 238)));
  QFont font;
  font.setPixelSize(20);
  m_pTextPaused = m_pScenePaused->addText(tr("Game paused"), font);

  m_pTimer = new QTimer(this);
  connect(m_pTimer, &QTimer::timeout, this, &IQPuzzle::updateTimer);
  m_pStatusLabelTime = new QLabel(tr("Time") + ": 00:00:00");
  m_pStatusLabelMoves = new QLabel(tr("Moves") + ": 0");
  m_pUi->statusBar->addWidget(m_pStatusLabelTime);
  m_pUi->statusBar->addPermanentWidget(m_pStatusLabelMoves);

  // Seed random number generator
  QTime time = QTime::currentTime();
  qsrand(static_cast<uint>(time.msec()));
  this->generateFileLists();

  // Choose board via command line
  QString sStartBoard("");
  QString sLoadBoard("");
  if (qApp->arguments().size() > 1) {
    foreach (QString sBoard, qApp->arguments()) {
      if (sBoard.endsWith(QStringLiteral(".conf"),
                          Qt::CaseInsensitive)) {
        if (QFile::exists(sBoard)) {
          sStartBoard = sBoard;
          break;
        } else {
          qWarning() << "Specified board not found:" << sBoard;
          QMessageBox::warning(this, tr("File not found"),
                               tr("The chosen file does not exist."));
          break;
        }
      } else if (sBoard.endsWith(QStringLiteral(".iqsav"),
                                 Qt::CaseInsensitive)) {
        if (QFile::exists(sBoard)) {
          sLoadBoard = sBoard;
          break;
        } else {
          qWarning() << "Specified save game not found:" << sBoard;
          QMessageBox::warning(this, tr("File not found"),
                               tr("The chosen file does not exist."));
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
                             tr("Games share path does not exist!"));
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
  connect(m_pUi->action_NewGame, &QAction::triggered,
          this, [this]() { startNewGame(); });

  // Random game
  m_pUi->actionAll->setShortcut(Qt::CTRL + Qt::Key_1);
  connect(m_pUi->actionAll, &QAction::triggered,
          this, [this]() { randomGame(1); });
  m_pUi->actionEasy->setShortcut(Qt::CTRL + Qt::Key_2);
  connect(m_pUi->actionEasy, &QAction::triggered,
          this, [this]() { randomGame(2); });
  m_pUi->actionMedium->setShortcut(Qt::CTRL + Qt::Key_3);
  connect(m_pUi->actionMedium, &QAction::triggered,
          this, [this]() { randomGame(3); });
  m_pUi->actionHard->setShortcut(Qt::CTRL + Qt::Key_4);
  connect(m_pUi->actionHard, &QAction::triggered,
          this, [this]() { randomGame(4); });
  m_pUi->actionAllUnsolved->setShortcut(Qt::CTRL + Qt::Key_5);
  connect(m_pUi->actionAllUnsolved, &QAction::triggered,
          this, [this]() { randomGame(5); });
  m_pUi->actionEasyUnsolved->setShortcut(Qt::CTRL + Qt::Key_6);
  connect(m_pUi->actionEasyUnsolved, &QAction::triggered,
          this, [this]() { randomGame(6); });
  m_pUi->actionMediumUnsolved->setShortcut(Qt::CTRL + Qt::Key_7);
  connect(m_pUi->actionMediumUnsolved, &QAction::triggered,
          this, [this]() { randomGame(7); });
  m_pUi->actionHardUnsolved->setShortcut(Qt::CTRL + Qt::Key_8);
  connect(m_pUi->actionHardUnsolved, &QAction::triggered,
          this, [this]() { randomGame(8); });

  // Restart game
  m_pUi->action_RestartGame->setShortcut(QKeySequence::Refresh);
  connect(m_pUi->action_RestartGame, &QAction::triggered,
          this, &IQPuzzle::restartGame);

  // Load game
  m_pUi->action_LoadGame->setShortcut(QKeySequence::Open);
  connect(m_pUi->action_LoadGame, &QAction::triggered,
          this, [this]() { loadGame(); });

  // Save game
  m_pUi->action_SaveGame->setShortcut(QKeySequence::Save);
  connect(m_pUi->action_SaveGame, &QAction::triggered,
          this, &IQPuzzle::saveGame);

  // Pause
  m_pUi->action_PauseGame->setShortcut(Qt::Key_P);
  connect(m_pUi->action_PauseGame, &QAction::triggered,
          this, &IQPuzzle::pauseGame);

  // Highscore
  m_pUi->action_Highscore->setShortcut(Qt::CTRL + Qt::Key_H);
  connect(m_pUi->action_Highscore, &QAction::triggered, m_pHighscore, [this]() {
    QFileInfo fi(m_sBoardFile);
    m_pHighscore->showHighscore(fi.baseName());
  });
  connect(this, &IQPuzzle::checkHighscore,
          m_pHighscore, &Highscore::checkHighscore);

  // Statistics
  connect(m_pUi->action_Statistics, &QAction::triggered,
          this, &IQPuzzle::showStatistics);

  // Exit game
  m_pUi->action_Quit->setShortcut(QKeySequence::Quit);
  connect(m_pUi->action_Quit, &QAction::triggered, this, &IQPuzzle::close);

  // Zoom in/out
  m_pUi->action_ZoomIn->setShortcut(QKeySequence::ZoomIn);
  m_pUi->action_ZoomOut->setShortcut(QKeySequence::ZoomOut);

  // Settings
  connect(m_pUi->action_Preferences, &QAction::triggered,
          m_pSettings, &Settings::show);

  // Report bug
  connect(m_pUi->action_ReportBug, &QAction::triggered,
          this, &IQPuzzle::reportBug);

  // About
  connect(m_pUi->action_Info, &QAction::triggered,
          this, &IQPuzzle::showInfoBox);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::startNewGame(QString sBoardFile, const QString &sSavedGame,
                            const QString &sTime, const QString &sMoves) {
  if (sBoardFile.isEmpty()) {
    sBoardFile = this->chooseBoard();
    if (sBoardFile.isEmpty()) {
      return;
    }
  }

  qDebug() << "Board:" << sBoardFile;
  if (!QFile::exists(sBoardFile)) {
    QMessageBox::warning(this, tr("File not found"),
                         tr("The chosen file does not exist."));
    qWarning() << "Board file not found:" << sBoardFile;
    return;
  }
  m_sBoardFile = sBoardFile;
  m_sSavedGame = "";

  if (!sSavedGame.isEmpty()) {
    qDebug() << "Saved game:" << sSavedGame;
    if (!QFile::exists(sSavedGame)) {
      QMessageBox::warning(this, tr("File not found"),
                           tr("The chosen file does not exist."));
      qWarning() << "Saved game not found:" << sSavedGame;
      return;
    }
    m_nMoves = QString(sMoves).toUInt();
    m_pStatusLabelMoves->setText(tr("Moves") + ": " +
                                 QString::number(m_nMoves));
    m_Time = m_Time.fromString(sTime, QStringLiteral("hh:mm:ss"));
    m_pStatusLabelTime->setText(tr("Time") + ": " + sTime);
    m_sSavedGame = sSavedGame;
  } else {
    m_nMoves = 0;
    m_pStatusLabelMoves->setText(tr("Moves") + ": 0");
    m_Time = m_Time.fromString(QStringLiteral("00:00:00"),
                               QStringLiteral("hh:mm:ss"));
    m_pStatusLabelTime->setText(tr("Time") + ": 00:00:00");
  }

  this->setGameTitle();
  this->createBoard();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::setGameTitle() {
  QSettings tmpSet(m_sBoardFile, QSettings::IniFormat);
  quint32 nSolutions = tmpSet.value(
                         QStringLiteral("PossibleSolutions"), 0).toUInt();
  QString sSolutions(QString::number(nSolutions));
  if ("0" == sSolutions) {
    sSolutions = tr("Unknown");
  }

  this->setWindowTitle(qApp->applicationName() + " - " +
                       QFileInfo(m_sBoardFile).baseName() + " ("
                       + tr("Solutions") + ": " + sSolutions + ")");
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QString IQPuzzle::chooseBoard() {
  if (nullptr != m_pBoardDialog) {
    delete m_pBoardDialog;
  }
  m_pBoardDialog = new BoardDialog(this, tr("Load board"),
                                   m_sSharePath + "/boards",
                                   tr("Board files") + " (*.conf)");

  if (m_pBoardDialog->exec()) {
    QStringList sListFiles;
    sListFiles = m_pBoardDialog->selectedFiles();
    if (sListFiles.size() >= 1) {
      return sListFiles.first();
    }
  }

  return QString();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::createBoard() {
  static QString sPreviousBoard("");
  quint16 nGridSize(0);

  if (nullptr != m_pBoard) {
    if (sPreviousBoard == m_sBoardFile) {
      nGridSize = m_pBoard->getGridSize();
    }
    delete m_pBoard;
  }
  m_pBoard = new Board(m_pGraphView, m_sBoardFile, m_pSettings,
                       nGridSize, m_sSavedGame);
  sPreviousBoard = m_sBoardFile;
  connect(m_pBoard, &Board::setWindowSize, this, &IQPuzzle::setMinWindowSize);
  connect(m_pUi->action_ZoomIn, &QAction::triggered,
          m_pBoard, &Board::zoomIn);
  connect(m_pUi->action_ZoomOut, &QAction::triggered,
          m_pBoard, &Board::zoomOut);
  connect(m_pBoard, &Board::incrementMoves, this, &IQPuzzle::incrementMoves);
  connect(m_pBoard, &Board::solvedPuzzle, this, &IQPuzzle::solvedPuzzle);

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
      int nRand = qrand() % m_sListFiles.at(nChoice-1)->size();
      if (nRand >= 0 && nRand < m_sListFiles.at(nChoice-1)->size()) {
        this->startNewGame(m_sSharePath + "/boards/" +
                           m_sListFiles.at(nChoice-1)->at(nRand));
      }
    } else {
      qWarning() << "Game file list is emtpy!";
      QMessageBox::warning(this, qApp->applicationName(),
                           tr("No boards available!"));
    }
  } else {
    qWarning() << "Invalid random choice:" << nChoice;
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::generateFileLists() {
#if defined _WIN32
  QSettings tmpScore(QSettings::IniFormat, QSettings::UserScope,
                     qApp->applicationName().toLower(),
                     QStringLiteral("Highscore"));
#else
  QSettings tmpScore(QSettings::NativeFormat, QSettings::UserScope,
                     qApp->applicationName().toLower(),
                     QStringLiteral("Highscore"));
#endif
  const uint nEasy(m_pSettings->getEasy());
  const uint nHard(m_pSettings->getHard());

  QDirIterator it(m_sSharePath + "/boards",
                  QStringList() << QStringLiteral("*.conf"),
                  QDir::NoDotAndDotDot | QDir::Files,
                  QDirIterator::Subdirectories);
  while (it.hasNext()) {
    it.next();
    // Filter freestyle boards
    if (!it.filePath().contains(QStringLiteral("freestyle"))) {
      QString sName = it.filePath().remove(m_sSharePath + "/boards/");
      // qDebug() << sName;

      QSettings tmpSet(it.filePath(), QSettings::IniFormat);
      quint32 nSolutions = tmpSet.value(
                             QStringLiteral("PossibleSolutions"), 0).toUInt();
      bool bSolved = tmpScore.childGroups().contains(
                       it.fileName().remove(QStringLiteral(".conf")));

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

  /*
  qDebug() << "Threshold easy:" << nEasy << " Threshold hard:" << nHard;
  qDebug() << "All:" << m_sListAll.size() <<
              "- unsolved:" << m_sListAllUnsolved.size();
  qDebug() << "Easy:" << m_sListEasy.size() <<
              "- unsolved:" << m_sListEasyUnsolved.size();
  qDebug() << "Medium:" << m_sListMedium.size() <<
              "- unsolved:" << m_sListMediumUnsolved.size();
  qDebug() << "Hard:" << m_sListHard.size() <<
              "- unsolved:" << m_sListHardUnsolved.size();
  */
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
    sSaveFile = QFileDialog::getOpenFileName(
                  this, tr("Load game"),
                  m_userDataDir.absolutePath(),
                  tr("Save games") + "(*.iqsav)");
  }

  if (!sSaveFile.isEmpty()) {
    QSettings tmpSet(sSaveFile, QSettings::IniFormat);
    QString sBoard(tmpSet.value(QStringLiteral("BoardFile"), "").toString());
    QString sBoardRel(tmpSet.value(
                        QStringLiteral("BoardFileRelative"), "").toString());
    if (sBoard.isEmpty() || !QFile::exists(sBoard)) {
      if (!sBoardRel.isEmpty() &&
          QFile::exists(qApp->applicationDirPath() + "/" + sBoardRel)) {
        sBoard = qApp->applicationDirPath() + "/" + sBoardRel;
      } else {
        QMessageBox::warning(this, qApp->applicationName(),
                             tr("Invalid saved puzzle."));
        return;
      }
    }

    QByteArray ba(tmpSet.value(
                    QStringLiteral("NumOfMoves"), "").toByteArray());
    m_sSavedMoves = QByteArray::fromBase64(ba);
    ba.clear();
    ba = tmpSet.value(QStringLiteral("ElapsedTime"), "").toByteArray();
    m_sSavedTime = QByteArray::fromBase64(ba);
    this->startNewGame(sBoard, sSaveFile, m_sSavedTime, m_sSavedMoves);
  } else {
    QMessageBox::warning(this, qApp->applicationName(),
                         tr("Invalid saved puzzle."));
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::saveGame() {
  QString sFile = QFileDialog::getSaveFileName(
                    this, tr("Save game"),
                    m_userDataDir.absolutePath(),
                    tr("Save games") + "(*.iqsav)");
  if (!sFile.isEmpty()) {
    m_sSavedMoves = QString::number(m_nMoves);
    m_sSavedTime = m_Time.toString(QStringLiteral("hh:mm:ss"));
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

  if (!this->windowState().testFlag(Qt::WindowMaximized) &&
      !this->windowState().testFlag(Qt::WindowFullScreen)) {
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
      m_pGraphView->centerOn(100, 70);
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::incrementMoves() {
  m_nMoves++;
  m_pStatusLabelMoves->setText(tr("Moves") + ": " + QString::number(m_nMoves));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::updateTimer() {
  m_Time = m_Time.addSecs(1);
  m_pStatusLabelTime->setText(tr("Time") + ": " + m_Time.toString(
                                QStringLiteral("hh:mm:ss")));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::solvedPuzzle() {
  QFileInfo fi(m_sBoardFile);
  m_pTimer->stop();
  m_bSolved = true;
  QMessageBox::information(this, qApp->applicationName(),
                           tr("Puzzle solved!") + "\n\n" +
                           tr("Moves") + ": " + QString::number(m_nMoves)
                           + "\n" + tr("Time") + ": "
                           + m_Time.toString(QStringLiteral("hh:mm:ss")));
  m_pUi->action_PauseGame->setEnabled(false);
  m_pUi->action_PauseGame->setChecked(false);
  m_pUi->action_SaveGame->setEnabled(false);

  // Save won game state for debugging
  m_pBoard->saveGame(m_userDataDir.absolutePath() + "/S0LV3D.debug",
                     QStringLiteral("55:55:55"), QStringLiteral("10000"));
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
    if (!this->switchTranslator(&m_translatorQt, "qt_" + sLang,
                                QLibraryInfo::location(
                                  QLibraryInfo::TranslationsPath))) {
      this->switchTranslator(&m_translatorQt, "qt_" + sLang,
                             m_sSharePath + "/lang");
    }

    if (!this->switchTranslator(
          &m_translator,
          ":/" + qApp->applicationName().toLower() + "_" + sLang + ".qm")) {
      this->switchTranslator(
            &m_translator, qApp->applicationName().toLower() + "_" + sLang,
            m_sSharePath + "/lang");
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool IQPuzzle::switchTranslator(QTranslator *translator,
                                const QString &sFile, const QString &sPath) {
  qApp->removeTranslator(translator);
  if (translator->load(sFile, sPath)) {
    qApp->installTranslator(translator);
  } else {
    if (!sFile.endsWith(QStringLiteral("_en")) &&
        !sFile.endsWith(QStringLiteral("_en.qm"))) {
      // EN is build in translation -> no file
      qWarning() << "Could not find translation" << sFile << "in" << sPath;
    }
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void IQPuzzle::showStatistics() {
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Statistics"));
  dialog.setWindowFlags(dialog.window()->windowFlags()
                        & ~Qt::WindowContextHelpButtonHint);

  QGridLayout* layout = new QGridLayout(&dialog);
  layout->setContentsMargins(10, 10, 10, 10);
  layout->setSpacing(8);

  layout->addWidget(new QLabel("<b>" + tr("Total") + "</b>", &dialog),
                    0, 1, Qt::AlignCenter);
  layout->addWidget(new QLabel("<b>" + tr("Unsolved") + "</b>", &dialog),
                    0, 2, Qt::AlignCenter);

  layout->addWidget(new QLabel("<b>" + tr("Easy") + "</b>", &dialog),
                    1, 0, Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListEasy.size()), &dialog),
                    1, 1, Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListEasyUnsolved.size()),
                               &dialog), 1, 2, Qt::AlignCenter);

  layout->addWidget(new QLabel("<b>" + tr("Medium") + "</b>", &dialog),
                    2, 0, Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListMedium.size()), &dialog),
                    2, 1, Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListMediumUnsolved.size()),
                               &dialog), 2, 2, Qt::AlignCenter);

  layout->addWidget(new QLabel("<b>" + tr("Hard") + "</b>", &dialog),
                    3, 0, Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListHard.size()), &dialog),
                    3, 1, Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListHardUnsolved.size()),
                               &dialog), 3, 2, Qt::AlignCenter);

  layout->addWidget(new QLabel("<b>" + tr("Total") + "</b> - " +
                               tr("including unknown difficulty") + ":",
                               &dialog), 4, 0, 1, 3, Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListAll.size()), &dialog),
                    5, 1, Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListAllUnsolved.size()),
                               &dialog), 5, 2, Qt::AlignCenter);

  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close,
                                                   Qt::Horizontal, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  layout->addWidget(buttons, 6, 0, 1, 3, Qt::AlignCenter);

  dialog.exec();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::reportBug() const {
  QDesktopServices::openUrl(
        QUrl(QStringLiteral("https://github.com/ElTh0r0/iqpuzzle/issues")));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void IQPuzzle::showInfoBox() {
  QMessageBox::about(
        this, tr("About"),
        QString("<center>"
                "<big><b>%1 %2</b></big><br />"
                "%3<br />"
                "<small>%4</small><br /><br />"
                "%5<br />"
                "%6<br />"
                "<small>%7</small>"
                "</center><br />"
                "%8")
        .arg(qApp->applicationName(),
             qApp->applicationVersion(),
             APP_DESC,
             APP_COPY,
             "URL: <a href=\"https://elth0r0.github.io/iqpuzzle/\">"
             "https://elth0r0.github.io/iqpuzzle/</a>",
             tr("License") +
             ": <a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">"
             "GNU General Public License Version 3</a>",
             tr("This application uses icons from "
                "<a href=\"http://tango.freedesktop.org\">"
                "Tango project</a>."),
             "<i>" + tr("Translations") +
             "</i><br />"
             "&nbsp;&nbsp;- Bulgarian: bogo1966<br />"
             "&nbsp;&nbsp;- Dutch: Vistaus, Elbert Pol<br />"
             "&nbsp;&nbsp;- French: kiarn, mothsART<br />"
             "&nbsp;&nbsp;- German: ElThoro<br />"
             "&nbsp;&nbsp;- Italian: davi92"));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::changeEvent(QEvent *pEvent) {
  if (nullptr != pEvent) {
    if (QEvent::LanguageChange == pEvent->type()) {
      m_pUi->retranslateUi(this);
      this->setGameTitle();

      m_pScenePaused->removeItem(m_pTextPaused);
      QFont font;
      font.setPixelSize(20);
      m_pTextPaused = m_pScenePaused->addText(tr("Game paused"), font);
      this->setMinWindowSize();

      m_pStatusLabelMoves->setText(
            tr("Moves") + ": " + QString::number(m_nMoves));
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
  int nRet = QMessageBox::question(this, tr("Quit") + " - " +
                                   qApp->applicationName(),
                                   tr("Do you really want to quit?"),
                                   QMessageBox::Yes | QMessageBox::No);

  if (QMessageBox::Yes == nRet) {
    pEvent->accept();
  } else {
    pEvent->ignore();
  }
  */
}
