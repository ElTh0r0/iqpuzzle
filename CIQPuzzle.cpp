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

CIQPuzzle::CIQPuzzle(QWidget *pParent)
    : QMainWindow(pParent),
      m_pUi(new Ui::CIQPuzzle),
      m_pBoardDialog(NULL),
      m_pBoard(NULL) {
    qDebug() << Q_FUNC_INFO;

    m_pUi->setupUi(this);
    this->setWindowTitle(qApp->applicationName());
    this->setupMenu();

    m_pGraphView = new QGraphicsView(this);
    this->setCentralWidget(m_pGraphView);
    m_pScene = new QGraphicsScene(this);
    m_pScene->setBackgroundBrush(QBrush(QColor("#EEEEEE")));
    m_pGraphView->setScene(m_pScene);

    // Choose board via command line
    if (qApp->arguments().size() > 1) {
        foreach (QString s, qApp->arguments()) {
            if (s.endsWith(".conf", Qt::CaseInsensitive)) {
                this->startNewGame(s);
                break;
            }
        }
    }
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

void CIQPuzzle::startNewGame(QString sBoardFile) {
    qDebug() << Q_FUNC_INFO;

    if (sBoardFile.isEmpty()) {
        // No installation: Use app path
        QString sPath = qApp->applicationDirPath() + "/boards";
        // Path from normal installation
        if (QFile::exists("/usr/share/" + qApp->applicationName().toLower()
                          + "/boards") && !bDEBUG) {
            sPath = "/usr/share/" + qApp->applicationName().toLower()
                    + "/boards";
        }

        if (NULL != m_pBoardDialog) {
            delete m_pBoardDialog;
        }
        m_pBoardDialog = new CBoardDialog(this, trUtf8("Load board"), sPath,
                                          trUtf8("Board files (*.conf)"));
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
        m_pUi->action_RestartGame->setEnabled(true);
        this->setWindowTitle(qApp->applicationName() + " - " +
                             QFileInfo(m_sBoardFile).baseName());
        m_pScene->clear();  // Clear old objects

        if (NULL != m_pBoard) {
            delete m_pBoard;
        }
        m_pBoard = new CBoard(m_pGraphView, m_pScene, sBoardFile);
        connect(m_pBoard, SIGNAL(setWindowSize(const QSize)),
                this, SLOT(setMinWindowSize(const QSize)));
        connect(m_pUi->action_ZoomIn, SIGNAL(triggered()),
                m_pBoard, SLOT(zoomIn()));
        connect(m_pUi->action_ZoomOut, SIGNAL(triggered()),
                m_pBoard, SLOT(zoomOut()));

        if (m_pBoard->setupBoard()) {
            m_pBoard->setupBlocks();
        }
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::restartGame() {
    this->startNewGame(m_sBoardFile);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CIQPuzzle::setMinWindowSize(const QSize size) {
    this->setMinimumSize(size);
    this->resize(size);
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

    layout->addWidget(new QLabel(trUtf8("<b>Move block:</b>"), &dialog),
                      0, 0, Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(new QLabel(trUtf8("Drag & drop with left mouse button"), &dialog),
                      0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(new QLabel(trUtf8("<b>Rotate block:</b>"), &dialog),
                      1, 0, Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(new QLabel(trUtf8("Vertical mousewheel"), &dialog),
                      1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(new QLabel(trUtf8("<b>Flip block:</b>"), &dialog),
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
                               "%5"
                               "</center>")
                       .arg(qApp->applicationName())
                       .arg(qApp->applicationVersion())
                       .arg(APP_DESC)
                       .arg(APP_COPY)
                       .arg(trUtf8("Licence: <a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">GNU General Public License Version 3</a>")));
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
