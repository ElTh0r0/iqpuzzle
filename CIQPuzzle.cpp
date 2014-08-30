/**
 * \file CIQPuzzle.cpp
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

extern bool bDEBUG;

CIQPuzzle::CIQPuzzle(const QDir userDataDir, QWidget *pParent)
    : QMainWindow(pParent),
      m_pUi(new Ui::CIQPuzzle),
      m_pBoardDialog(NULL),
      m_pBoard(NULL),
      m_sSavedGame(""),
      m_userDataDir(userDataDir),
      m_nMoves(0),
      m_sSavedTime(""),
      m_sSavedMoves(""),
      m_Time(0, 0, 0),
      m_bSolved(false) {
    qDebug() << Q_FUNC_INFO;

    m_pUi->setupUi(this);
    this->setWindowTitle(qApp->applicationName());
    m_pHighscore = new CHighscore();
    this->setupMenu();

    m_pGraphView = new QGraphicsView(this);
    this->setCentralWidget(m_pGraphView);
    m_pScene = new QGraphicsScene(this);
    m_pScene->setBackgroundBrush(QBrush(QColor("#EEEEEE")));
    m_pGraphView->setScene(m_pScene);
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
    if (qApp->arguments().size() > 1) {
        foreach (QString s, qApp->arguments()) {
            if (s.endsWith(".conf", Qt::CaseInsensitive)) {
                this->startNewGame(s);
                break;
            }
        }
    }

    // Start board_001 as default
    QString sPath = qApp->applicationDirPath() + "/boards";
    // Path from normal installation
    if (QFile::exists("/usr/share/games/" + qApp->applicationName().toLower()
                      + "/boards") && !bDEBUG) {
        sPath = "/usr/share/games/" + qApp->applicationName().toLower()
                + "/boards";
    }
    this->startNewGame(sPath + "/board_001.conf");
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

    // Controls info
    m_pUi->action_Controls->setShortcut(QKeySequence::HelpContents);
    connect(m_pUi->action_Controls, SIGNAL(triggered()),
            this, SLOT(showControlsBox()));

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
        // No installation: Use app path
        QString sPath = qApp->applicationDirPath() + "/boards";
        // Path from normal installation
        if (QFile::exists("/usr/share/games/"
                          + qApp->applicationName().toLower()
                          + "/boards") && !bDEBUG) {
            sPath = "/usr/share/games/" + qApp->applicationName().toLower()
                    + "/boards";
        }

        if (NULL != m_pBoardDialog) {
            delete m_pBoardDialog;
        }
        m_pBoardDialog = new CBoardDialog(this, trUtf8("Load board"), sPath,
                                          trUtf8("Board files") + "(*.conf)");
        if (m_pBoardDialog->exec()) {
            QStringList sListFiles;
            sListFiles = m_pBoardDialog->selectedFiles();
            if (sListFiles.size() >= 1) {
                sBoardFile = sListFiles.first();
            }
        }
    }

    if (!sBoardFile.isEmpty()) {
        qDebug() << "Board:" << sBoardFile;
        if (!QFile::exists(sBoardFile)) {
            QMessageBox::warning(this, trUtf8("File not found"),
                                 trUtf8("The chosen file does not exist."));
            qWarning() << "Board file not found:" << sBoardFile;
            return;
        }
        m_sBoardFile = sBoardFile;
        m_nMoves = 0;
        m_pStatusLabelMoves->setText(trUtf8("Moves") + ": 0");
        m_Time = m_Time.fromString("00:00:00", "hh:mm:ss");
        m_pStatusLabelTime->setText(trUtf8("Time") + ": 00:00:00");

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
        }

        m_pUi->action_RestartGame->setEnabled(true);
        m_pUi->action_SaveGame->setEnabled(true);

        QSettings tmpSet(m_sBoardFile, QSettings::IniFormat);
        quint32 nSolutions = tmpSet.value("PossibleSolutions", 0).toUInt();
        QString sSolutions(QString::number(nSolutions));
        if ("0" == sSolutions) {
            sSolutions = trUtf8("Unknown");
        }

        this->setWindowTitle(qApp->applicationName() + " - " +
                             QFileInfo(m_sBoardFile).baseName() + " ("
                             + trUtf8("Solutions") + ": " + sSolutions + ")");
        m_pScene->clear();  // Clear old objects

        if (NULL != m_pBoard) {
            delete m_pBoard;
        }
        m_pBoard = new CBoard(m_pGraphView, m_pScene, sBoardFile, sSavedGame);
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
            m_pUi->action_PauseGame->setChecked(false);
            m_bSolved = false;
            m_pGraphView->setScene(m_pScene);
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
    QString sSaved = QFileDialog::getOpenFileName(this, tr("Load game"),
                                                  m_userDataDir.absolutePath());
    if (!sSaved.isEmpty()) {
        QSettings tmpSet(sSaved, QSettings::IniFormat);
        QString sBoard = tmpSet.value("BoardFile", "").toString();
        if (!sBoard.isEmpty()) {
            QByteArray ba = tmpSet.value("NumOfMoves", "").toByteArray();
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
    QString sFile = QFileDialog::getSaveFileName(this, tr("Save game"),
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
            m_pGraphView->setScene(m_pScene);
        }
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::setMinWindowSize(const QSize size) {
    this->setMinimumSize(size);
    this->resize(size);
    m_pTextPaused->setX(
                size.width()/2.5/2 - m_pTextPaused->boundingRect().width()/2);
    m_pTextPaused->setY(
                size.height()/2.6/2 - m_pTextPaused->boundingRect().height()/2);
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

    emit checkHighscore(fi.baseName(), m_nMoves, m_Time);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::showHighscore() {
    QFileInfo fi(m_sBoardFile);
    emit showHighscore(fi.baseName());
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::showControlsBox() {
    QDialog dialog(this);
    dialog.setWindowTitle(trUtf8("Controls"));
    dialog.setWindowFlags(this->windowFlags()
                          & ~Qt::WindowContextHelpButtonHint);

    QGridLayout* layout = new QGridLayout(&dialog);
    layout->setMargin(12);
    layout->setSpacing(0);
    layout->setColumnMinimumWidth(1, 2);
    layout->setRowMinimumHeight(3, 12);

    layout->addWidget(new QLabel(
                          "<b>" + trUtf8("Move block") + ":</b> ", &dialog),
                      0, 0, Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(
                new QLabel(
                    trUtf8("Drag & drop with left mouse button"), &dialog),
                      0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(new QLabel(
                          "<b>" + trUtf8("Rotate block") + ":</b> ", &dialog),
                      1, 0, Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(new QLabel(trUtf8("Vertical mousewheel"), &dialog),
                      1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(new QLabel(
                          "<b>" + trUtf8("Flip block") + ":</b> ", &dialog),
                      2, 0, Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(new QLabel(trUtf8("Right mouse button"), &dialog),
                      2, 2, Qt::AlignLeft | Qt::AlignVCenter);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close,
                                                     Qt::Horizontal, &dialog);
    connect(buttons, SIGNAL(rejected()),
            &dialog, SLOT(reject()));
    layout->addWidget(buttons, 4, 0, 1, 3);

    dialog.exec();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::showInfoBox() {
    QMessageBox::about(this, trUtf8("About"),
                       QString("<center>"
                               "<big><b>%1 %2</b></big><br/>"
                               "%3<br/>"
                               "<small>%4</small><br/><br/>"
                               "%5<br/>"
                               "%6"
                               "</center>")
                       .arg(qApp->applicationName())
                       .arg(qApp->applicationVersion())
                       .arg(APP_DESC)
                       .arg(APP_COPY)
                       .arg("URL: <a href=\"https://launchpad.net/iqpuzzle\">"
                            "https://launchpad.net/iqpuzzle</a>")
                       .arg(trUtf8("License") + ": "
                            "<a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">"
                            "GNU General Public License Version 3</a>"));
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
