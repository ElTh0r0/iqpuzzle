/**
 * \file iqpuzzle.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-present Thorsten Roth
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
 * along with iQPuzzle.  If not, see <https://www.gnu.org/licenses/>.
 *
 * \section DESCRIPTION
 * Main application generation (GUI, object creation etc.).
 */

#include "./iqpuzzle.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDirIterator>
#include <QFileDialog>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGridLayout>
#include <QLabel>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QStyleHints>
#endif

#include "./board.h"
#include "./boardselection.h"
#include "./highscore.h"
#include "./settings.h"
#include "ui_iqpuzzle.h"

IQPuzzle::IQPuzzle(const QDir &userDataDir, const QDir &sharePath,
                   QWidget *pParent)
    : QMainWindow(pParent),
      m_pUi(new Ui::IQPuzzle),
      m_sCurrLang(QString()),
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

  QString sIconTheme = QStringLiteral("light");
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  if (Qt::ColorScheme::Dark == QGuiApplication::styleHints()->colorScheme()) {
    sIconTheme = QStringLiteral("dark");
  }
#else
  if (this->window()->palette().window().color().lightnessF() < 0.5) {
    sIconTheme = QStringLiteral("dark");
  }
#endif
  QIcon::setThemeName(sIconTheme);

  m_pHighscore = new Highscore();
  m_pSettings = new Settings(m_sSharePath, this);
  connect(m_pSettings, &Settings::changeLang, this, &IQPuzzle::loadLanguage);
  connect(this, &IQPuzzle::updateUiLang, m_pSettings, &Settings::updateUiLang);
  this->loadLanguage(m_pSettings->getLanguage());
  this->setupMenu();

  this->generateFileLists();  // Run before creating BoardSelection
  m_pBoardSelection =
      new BoardSelection(this, m_sSharePath + "/boards", m_sListAllUnsolved,
                         m_pSettings->getLastOpenedDir());

  m_pGraphView = new QGraphicsView(this);
  this->setCentralWidget(m_pGraphView);
  m_pScenePaused = new QGraphicsScene(this);
  QFont font;
  font.setPixelSize(20);
  m_pTextPaused = m_pScenePaused->addText(tr("Game paused"), font);

  m_pTimer = new QTimer(this);
  connect(m_pTimer, &QTimer::timeout, this, [this]() {
    m_Time = m_Time.addSecs(1);
    m_pStatusLabelTime->setText(tr("Time") + ": " +
                                m_Time.toString(QStringLiteral("hh:mm:ss")));
  });
  m_pStatusLabelTime = new QLabel(tr("Time") + ": 00:00:00");
  m_pStatusLabelMoves = new QLabel(tr("Moves") + ": 0");
  m_pUi->statusBar->addWidget(m_pStatusLabelTime);
  m_pUi->statusBar->addPermanentWidget(m_pStatusLabelMoves);

  // Choose board via command line
  QString sStartBoard(QLatin1String(""));
  QString sLoadBoard(QLatin1String(""));
  if (qApp->arguments().size() > 1) {
    for (auto &sBoard : qApp->arguments()) {
      if (sBoard.endsWith(QStringLiteral(".conf"), Qt::CaseInsensitive)) {
        if (QFile::exists(sBoard)) {
          sStartBoard = sBoard;
          break;
        }
        qWarning() << "Specified board not found:" << sBoard;
        QMessageBox::warning(this, tr("File not found"),
                             tr("The chosen file does not exist."));
        break;
      }
      if (sBoard.endsWith(QStringLiteral(".iqsav"), Qt::CaseInsensitive)) {
        if (QFile::exists(sBoard)) {
          sLoadBoard = sBoard;
          break;
        }
        qWarning() << "Specified save game not found:" << sBoard;
        QMessageBox::warning(this, tr("File not found"),
                             tr("The chosen file does not exist."));
        break;
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

IQPuzzle::~IQPuzzle() = default;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::setupMenu() {
  // New game
  m_pUi->action_NewGame->setShortcut(QKeySequence::New);
  connect(m_pUi->action_NewGame, &QAction::triggered, this,
          [this]() { startNewGame(); });

  // Random game
  m_pUi->actionAll->setShortcut(Qt::CTRL | Qt::Key_1);
  connect(m_pUi->actionAll, &QAction::triggered, this,
          [this]() { randomGame(1); });
  m_pUi->actionEasy->setShortcut(Qt::CTRL | Qt::Key_2);
  connect(m_pUi->actionEasy, &QAction::triggered, this,
          [this]() { randomGame(2); });
  m_pUi->actionMedium->setShortcut(Qt::CTRL | Qt::Key_3);
  connect(m_pUi->actionMedium, &QAction::triggered, this,
          [this]() { randomGame(3); });
  m_pUi->actionHard->setShortcut(Qt::CTRL | Qt::Key_4);
  connect(m_pUi->actionHard, &QAction::triggered, this,
          [this]() { randomGame(4); });
  m_pUi->actionAllUnsolved->setShortcut(Qt::CTRL | Qt::Key_5);
  connect(m_pUi->actionAllUnsolved, &QAction::triggered, this,
          [this]() { randomGame(5); });
  m_pUi->actionEasyUnsolved->setShortcut(Qt::CTRL | Qt::Key_6);
  connect(m_pUi->actionEasyUnsolved, &QAction::triggered, this,
          [this]() { randomGame(6); });
  m_pUi->actionMediumUnsolved->setShortcut(Qt::CTRL | Qt::Key_7);
  connect(m_pUi->actionMediumUnsolved, &QAction::triggered, this,
          [this]() { randomGame(7); });
  m_pUi->actionHardUnsolved->setShortcut(Qt::CTRL | Qt::Key_8);
  connect(m_pUi->actionHardUnsolved, &QAction::triggered, this,
          [this]() { randomGame(8); });

  // Restart game
  m_pUi->action_RestartGame->setShortcut(QKeySequence::Refresh);
  connect(m_pUi->action_RestartGame, &QAction::triggered, this,
          &IQPuzzle::restartGame);

  // Load game
  m_pUi->action_LoadGame->setShortcut(QKeySequence::Open);
  connect(m_pUi->action_LoadGame, &QAction::triggered, this,
          [this]() { loadGame(); });

  // Save game
  m_pUi->action_SaveGame->setShortcut(QKeySequence::Save);
  connect(m_pUi->action_SaveGame, &QAction::triggered, this,
          &IQPuzzle::saveGame);

  // Pause
  m_pUi->action_PauseGame->setShortcut(Qt::Key_P);
  connect(m_pUi->action_PauseGame, &QAction::triggered, this,
          &IQPuzzle::pauseGame);

  // Highscore
  m_pUi->action_Highscore->setShortcut(Qt::CTRL | Qt::Key_H);
  connect(m_pUi->action_Highscore, &QAction::triggered, m_pHighscore, [this]() {
    QFileInfo fi(m_sBoardFile);
    m_pHighscore->showHighscore(fi.baseName());
  });
  connect(this, &IQPuzzle::checkHighscore, m_pHighscore,
          &Highscore::checkHighscore);

  // Statistics
  connect(m_pUi->action_Statistics, &QAction::triggered, this,
          &IQPuzzle::showStatistics);

  // Exit game
  m_pUi->action_Quit->setShortcut(QKeySequence::Quit);
  connect(m_pUi->action_Quit, &QAction::triggered, this, &IQPuzzle::close);

  // Zoom in/out
  m_pUi->action_ZoomIn->setShortcut(QKeySequence::ZoomIn);
  m_pUi->action_ZoomOut->setShortcut(QKeySequence::ZoomOut);

  // Settings
  connect(m_pUi->action_Preferences, &QAction::triggered, m_pSettings,
          &Settings::show);

  // Report bug
  connect(m_pUi->action_ReportBug, &QAction::triggered, this, []() {
    QDesktopServices::openUrl(
        QUrl(QStringLiteral("https://github.com/ElTh0r0/iqpuzzle/issues")));
  });

  // About
  connect(m_pUi->action_Info, &QAction::triggered, this,
          &IQPuzzle::showInfoBox);
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

  if (!sSavedGame.isEmpty()) {
    qDebug() << "Load saved game:" << sSavedGame;
    if (!QFile::exists(sSavedGame)) {
      QMessageBox::warning(this, tr("File not found"),
                           tr("The chosen file does not exist."));
      qWarning() << "Saved game not found:" << sSavedGame;
      return;
    }
    m_nMoves = QString(sMoves).toUInt();
    m_pStatusLabelMoves->setText(tr("Moves") + ": " +
                                 QString::number(m_nMoves));
    m_Time = QTime::fromString(sTime, QStringLiteral("hh:mm:ss"));
    m_pStatusLabelTime->setText(tr("Time") + ": " + sTime);
    m_sSavedGame = sSavedGame;
  } else {
    m_nMoves = 0;
    m_pStatusLabelMoves->setText(tr("Moves") + ": 0");
    m_Time = QTime::fromString(QStringLiteral("00:00:00"),
                               QStringLiteral("hh:mm:ss"));
    m_pStatusLabelTime->setText(tr("Time") + ": 00:00:00");
    m_sSavedGame = QLatin1String("");
  }

  this->setGameTitle();
  this->createBoard();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::setGameTitle() {
  QSettings tmpSet(m_sBoardFile, QSettings::IniFormat);
  quint32 nSolutions =
      tmpSet.value(QStringLiteral("PossibleSolutions"), 0).toUInt();
  QString sSolutions(QString::number(nSolutions));
  if ("0" == sSolutions) {
    sSolutions = tr("Unknown");
  }

  this->setWindowTitle(QFileInfo(m_sBoardFile).baseName() + " (" +
                       tr("Solutions") + ": " + sSolutions + ")");
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto IQPuzzle::chooseBoard() -> QString {
  if (m_pBoardSelection->exec()) {
    QString sFile(m_pBoardSelection->getSelectedFile());
    m_pSettings->setLastOpenedDir(m_pBoardSelection->getLastOpenedDir());
    return sFile;
  }

  return QString();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::createBoard() {
  static QString sPreviousBoard(QLatin1String(""));
  quint16 nGridSize(0);

  if (sPreviousBoard == m_sBoardFile) {
    nGridSize = m_pBoard->getGridSize();
  }
  delete m_pBoard;

  m_pBoard = new Board(m_pGraphView, m_sBoardFile, m_pSettings, nGridSize,
                       m_sSavedGame);
  sPreviousBoard = m_sBoardFile;
  connect(m_pBoard, &Board::setWindowSize, this, &IQPuzzle::setMinWindowSize);
  connect(m_pUi->action_ZoomIn, &QAction::triggered, m_pBoard, &Board::zoomIn);
  connect(m_pUi->action_ZoomOut, &QAction::triggered, m_pBoard,
          &Board::zoomOut);
  connect(m_pBoard, &Board::incrementMoves, this, [this]() {
    m_nMoves++;
    m_pStatusLabelMoves->setText(tr("Moves") + ": " +
                                 QString::number(m_nMoves));
  });
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
    if (!m_sListFiles[nChoice - 1]->isEmpty()) {
      int nRand;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
      nRand = QRandomGenerator::global()->bounded(
          m_sListFiles.at(nChoice - 1)->size());
#else
      qsrand(static_cast<uint>(QTime::currentTime().msec()));  // Seed
      nRand = qrand() % m_sListFiles.at(nChoice - 1)->size();
#endif

      if (nRand >= 0 && nRand < m_sListFiles.at(nChoice - 1)->size()) {
        this->startNewGame(m_sSharePath + "/boards/" +
                           m_sListFiles.at(nChoice - 1)->at(nRand));
      }
    } else {
      qWarning() << "Game file list is empty!";
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

  QDirIterator it(
      m_sSharePath + "/boards", QStringList() << QStringLiteral("*.conf"),
      QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);
  while (it.hasNext()) {
    it.next();
    // Filter freestyle boards
    if (!it.filePath().contains(QStringLiteral("freestyle"))) {
      QString sName = it.filePath().remove(m_sSharePath + "/boards/");
      // qDebug() << sName;

      QSettings tmpSet(it.filePath(), QSettings::IniFormat);
      quint32 nSolutions =
          tmpSet.value(QStringLiteral("PossibleSolutions"), 0).toUInt();
      bool bSolved = tmpScore.childGroups().contains(
          it.fileName().remove(QStringLiteral(".conf")));

      m_sListAll << sName;
      if (!bSolved) m_sListAllUnsolved << sName;
      if (nSolutions >= nEasy) {
        m_sListEasy << sName;
        if (!bSolved) m_sListEasyUnsolved << sName;
      } else if ((nHard < nSolutions) && (nSolutions < nEasy)) {
        m_sListMedium << sName;
        if (!bSolved) m_sListMediumUnsolved << sName;
      } else if ((0 < nSolutions) && (nSolutions <= nHard)) {
        m_sListHard << sName;
        if (!bSolved) m_sListHardUnsolved << sName;
      }
    }
  }

  m_sListFiles << &m_sListAll << &m_sListEasy << &m_sListMedium << &m_sListHard
               << &m_sListAllUnsolved << &m_sListEasyUnsolved
               << &m_sListMediumUnsolved << &m_sListHardUnsolved;

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
    sSaveFile = QFileDialog::getOpenFileName(this, tr("Load game"),
                                             m_userDataDir.absolutePath(),
                                             tr("Save games") + "(*.iqsav)");
  }

  if (!sSaveFile.isEmpty()) {
    QSettings tmpSet(sSaveFile, QSettings::IniFormat);
    QString sBoard(tmpSet.value(QStringLiteral("BoardFile"), "").toString());
    QString sBoardRel(
        tmpSet.value(QStringLiteral("BoardFileRelative"), "").toString());
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

    QByteArray ba(tmpSet.value(QStringLiteral("NumOfMoves"), "").toByteArray());
    m_sSavedMoves = QString::fromLatin1(QByteArray::fromBase64(ba));
    ba.clear();
    ba = tmpSet.value(QStringLiteral("ElapsedTime"), "").toByteArray();
    m_sSavedTime = QString::fromLatin1(QByteArray::fromBase64(ba));
    this->startNewGame(sBoard, sSaveFile, m_sSavedTime, m_sSavedMoves);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::saveGame() {
  QString sFile = QFileDialog::getSaveFileName(this, tr("Save game"),
                                               m_userDataDir.absolutePath(),
                                               tr("Save games") + "(*.iqsav)");
  if (!sFile.isEmpty()) {
    if (!sFile.endsWith(QLatin1String(".iqsav"), Qt::CaseInsensitive)) {
      sFile += QLatin1String(".iqsav");
    }
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
      if (m_pSettings->getUseSystemBackground()) {
        m_pScenePaused->setBackgroundBrush(Qt::NoBrush);
        m_pTextPaused->setDefaultTextColor(
            QApplication::palette().color(QPalette::WindowText));
      } else {
        m_pScenePaused->setBackgroundBrush(QBrush(QColor(238, 238, 238)));
        m_pTextPaused->setDefaultTextColor(QColor(0, 0, 0));
      }
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
    this->setMinimumSize(size2);
  }

  if (!this->windowState().testFlag(Qt::WindowMaximized) &&
      !this->windowState().testFlag(Qt::WindowFullScreen)) {
    if (this->size().width() < size2.width() ||
        this->size().height() < size2.height()) {
      this->showNormal();
      this->resize(size2);
    }
    m_pTextPaused->setX(size2.width() / 2.5 / 2 -
                        m_pTextPaused->boundingRect().width() / 2);
    m_pTextPaused->setY(size2.height() / 2.6 / 2 -
                        m_pTextPaused->boundingRect().height() / 2);

    if (bFreestyle) {
      m_pGraphView->centerOn(100, 70);
    }
  }
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

      m_pStatusLabelMoves->setText(tr("Moves") + ": " +
                                   QString::number(m_nMoves));
      emit updateUiLang();
    }
  }
  QMainWindow::changeEvent(pEvent);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::resizeEvent(QResizeEvent *pEvent) {
  if (nullptr != pEvent) {
    if (!this->windowState().testFlag(Qt::WindowMaximized) &&
        !this->windowState().testFlag(Qt::WindowFullScreen)) {
      if (pEvent->size().width() < this->minimumWidth() ||
          pEvent->size().height() < this->minimumHeight()) {
        this->showNormal();
        this->resize(this->minimumSize());
      }
    }
  }
  QMainWindow::resizeEvent(pEvent);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void IQPuzzle::solvedPuzzle() {
  QFileInfo fi(m_sBoardFile);
  m_pTimer->stop();
  m_bSolved = true;
  QMessageBox::information(this, qApp->applicationName(),
                           tr("Puzzle solved!") + "\n\n" + tr("Moves") + ": " +
                               QString::number(m_nMoves) + "\n" + tr("Time") +
                               ": " +
                               m_Time.toString(QStringLiteral("hh:mm:ss")));
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
  // Skip update, if board is from user / not from share folder
  if (sBoard.contains(m_sSharePath + "/boards/", Qt::CaseInsensitive)) {
    m_pBoardSelection->updateSolved(sBoard);
    sBoard = sBoard.remove(m_sSharePath + "/boards/");
    if (m_sListAllUnsolved.indexOf(sBoard) >= 0) {
      m_sListAllUnsolved.removeAt(m_sListAllUnsolved.indexOf(sBoard));
    }
    if (m_sListEasyUnsolved.indexOf(sBoard) >= 0) {
      m_sListEasyUnsolved.removeAt(m_sListEasyUnsolved.indexOf(sBoard));
    }
    if (m_sListMediumUnsolved.indexOf(sBoard) >= 0) {
      m_sListMediumUnsolved.removeAt(m_sListMediumUnsolved.indexOf(sBoard));
    }
    if (m_sListHardUnsolved.indexOf(sBoard) >= 0) {
      m_sListHardUnsolved.removeAt(m_sListHardUnsolved.indexOf(sBoard));
    }
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void IQPuzzle::loadLanguage(const QString &sLang) {
  if (m_sCurrLang != sLang) {
    m_sCurrLang = sLang;
    if (!IQPuzzle::switchTranslator(&m_translatorQt, "qt_" + sLang,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                                    QLibraryInfo::path(
#else
                                    QLibraryInfo::location(
#endif
                                        QLibraryInfo::TranslationsPath))) {
      IQPuzzle::switchTranslator(&m_translatorQt, "qt_" + sLang,
                                 m_sSharePath + "/lang");
    }

    if (!IQPuzzle::switchTranslator(
            &m_translator,
            ":/" + qApp->applicationName().toLower() + "_" + sLang + ".qm")) {
      IQPuzzle::switchTranslator(
          &m_translator, qApp->applicationName().toLower() + "_" + sLang,
          m_sSharePath + "/lang");
    }
  }
  m_pUi->retranslateUi(this);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto IQPuzzle::switchTranslator(QTranslator *translator, const QString &sFile,
                                const QString &sPath) -> bool {
  qApp->removeTranslator(translator);
  if (translator->load(sFile, sPath)) {
    qApp->installTranslator(translator);
  } else {
    qWarning() << "Could not find translation" << sFile << "in" << sPath;
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void IQPuzzle::showStatistics() {
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Statistics"));
  dialog.setWindowFlags(dialog.window()->windowFlags() &
                        ~Qt::WindowContextHelpButtonHint);

  auto *layout = new QGridLayout(&dialog);
  layout->setContentsMargins(10, 10, 10, 10);
  layout->setSpacing(8);

  layout->addWidget(new QLabel("<b>" + tr("Total") + "</b>", &dialog), 0, 1,
                    Qt::AlignCenter);
  layout->addWidget(new QLabel("<b>" + tr("Unsolved") + "</b>", &dialog), 0, 2,
                    Qt::AlignCenter);

  layout->addWidget(new QLabel("<b>" + tr("Easy") + "</b>", &dialog), 1, 0,
                    Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListEasy.size()), &dialog), 1,
                    1, Qt::AlignCenter);
  layout->addWidget(
      new QLabel(QString::number(m_sListEasyUnsolved.size()), &dialog), 1, 2,
      Qt::AlignCenter);

  layout->addWidget(new QLabel("<b>" + tr("Medium") + "</b>", &dialog), 2, 0,
                    Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListMedium.size()), &dialog),
                    2, 1, Qt::AlignCenter);
  layout->addWidget(
      new QLabel(QString::number(m_sListMediumUnsolved.size()), &dialog), 2, 2,
      Qt::AlignCenter);

  layout->addWidget(new QLabel("<b>" + tr("Hard") + "</b>", &dialog), 3, 0,
                    Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListHard.size()), &dialog), 3,
                    1, Qt::AlignCenter);
  layout->addWidget(
      new QLabel(QString::number(m_sListHardUnsolved.size()), &dialog), 3, 2,
      Qt::AlignCenter);

  layout->addWidget(new QLabel("<b>" + tr("Total") + "</b> - " +
                                   tr("including unknown difficulty") + ":",
                               &dialog),
                    4, 0, 1, 3, Qt::AlignCenter);
  layout->addWidget(new QLabel(QString::number(m_sListAll.size()), &dialog), 5,
                    1, Qt::AlignCenter);
  layout->addWidget(
      new QLabel(QString::number(m_sListAllUnsolved.size()), &dialog), 5, 2,
      Qt::AlignCenter);

  auto *buttons =
      new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  layout->addWidget(buttons, 6, 0, 1, 3, Qt::AlignCenter);

  dialog.exec();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void IQPuzzle::showInfoBox() {
  QMessageBox::about(
      this, tr("About"),
      QString::fromLatin1("<center>"
                          "<big><b>%1 %2</b></big><br />"
                          "%3<br />"
                          "<small>%4</small><br /><br />"
                          "%5<br />"
                          "%6<br />"
                          "<small>%7</small>"
                          "</center><br />"
                          "%8")
          .arg(qApp->applicationName(), qApp->applicationVersion(), APP_DESC,
               APP_COPY,
               "URL: <a href=\"https://elth0r0.github.io/iqpuzzle/\">"
               "https://elth0r0.github.io/iqpuzzle/</a>",
               tr("License") +
                   ": <a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">"
                   "GNU General Public License Version 3</a>",
               tr("This application uses "
                  "<a href=\"https://invent.kde.org/frameworks/breeze-icons\">"
                  "Breeze icons from KDE</a>."),
               "<i>" + tr("Translations") +
                   "</i><br />"
                   "&nbsp;&nbsp;- Bulgarian: bogo1966<br />"
                   "&nbsp;&nbsp;- Chinese (zh_CN & zh_TW): liulitchi<br />"
                   "&nbsp;&nbsp;- Dutch: Vistaus, Elbert Pol<br />"
                   "&nbsp;&nbsp;- French: kiarn, mothsART<br />"
                   "&nbsp;&nbsp;- German: ElThoro<br />"
                   "&nbsp;&nbsp;- Greek: liulitchi<br />"
                   "&nbsp;&nbsp;- Italian: davi92, albanobattistella<br />"
                   "&nbsp;&nbsp;- Korean: hyuna1127<br />"
                   "&nbsp;&nbsp;- Norwegian: Allan Nordhøy<br />"
                   "&nbsp;&nbsp;- Portuguese (pt & pt_BR): UchidoF<br />"
                   "&nbsp;&nbsp;- Misc. corrections: J. Lavoie"));
}
