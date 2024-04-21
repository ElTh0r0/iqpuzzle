/**
 * \file boardselection.cpp
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
 * Board selection dialog.
 */

#include "./boardselection.h"

#include <QDebug>
#include <QDir>
#include <QSettings>

#include "ui_boardselection.h"

BoardSelection::BoardSelection(QWidget *pParent, const QString &sBoardsDir,
                               const QStringList &sListAllUnsolved,
                               const QString &sLastOpenedDir)
    : QDialog(pParent),
      m_nColumns(3),
      m_previewsize(250, 250),
      m_sBoardsDir(sBoardsDir),
      m_sListAllUnsolved(sListAllUnsolved),
      m_sLastOpenedDir(sLastOpenedDir),
      m_pBoardDialog(nullptr) {
  qDebug() << Q_FUNC_INFO;

  m_pUi = new Ui::BoardSelection();
  m_pUi->setupUi(this);
  this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
  this->setModal(true);

  QDir boardsDir(m_sBoardsDir);
  const QStringList sListSubfolders =
      boardsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

  m_pListTabScrollArea.reserve(sListSubfolders.length());
  m_pListContent.reserve(sListSubfolders.length());
  m_pListTabLayouts.reserve(sListSubfolders.length());

  for (auto sSubfolder : sListSubfolders) {
    m_pListTabScrollArea << new QScrollArea(this);
    m_pListTabScrollArea.last()->setWidgetResizable(true);
    m_pListTabScrollArea.last()->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);
    m_pListContent << new QWidget(this);
    m_pListTabScrollArea.last()->setWidget(m_pListContent.last());
    m_pListTabLayouts << new QGridLayout(m_pListContent.last());

    QDir dir(m_sBoardsDir + "/" + sSubfolder);
    const QStringList boardfiles =
        dir.entryList(QStringList() << QLatin1String("*.conf"),
                      QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    int nCol = 0;
    int nRow = 0;
    for (const auto &board : boardfiles) {
      QString sFile(sSubfolder + "/" + board);
      m_pListBoards << new BoardPreview(m_sBoardsDir + "/" + sFile,
                                        !m_sListAllUnsolved.contains(sFile),
                                        m_previewsize);
      m_pListTabLayouts.last()->addWidget(m_pListBoards.last(), nRow, nCol);

      connect(m_pListBoards.last(), &BoardPreview::selectBoard, this,
              &BoardSelection::SelectBoard);

      nCol++;
      if (0 == nCol % m_nColumns) {
        nCol = 0;
        nRow++;
      }
    }

    sSubfolder[0] = sSubfolder[0].toUpper();
    sSubfolder = sSubfolder.replace('_', ' ');
    m_pUi->tabWidget->addTab(m_pListTabScrollArea.last(), sSubfolder);
  }

  connect(m_pUi->openOwnBoard, &QPushButton::clicked, this,
          &BoardSelection::SelectOwnBoard);
}

BoardSelection::~BoardSelection() {
  delete m_pUi;
  m_pUi = nullptr;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void BoardSelection::SelectBoard(const QString &sFileName) {
  m_sSelectedFile = sFileName;
  this->accept();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void BoardSelection::SelectOwnBoard() {
  m_sSelectedFile.clear();
  delete m_pBoardDialog;
  m_pBoardDialog = new BoardDialog(this, tr("Load board"), m_sLastOpenedDir,
                                   tr("Board files") + " (*.conf)");

  if (m_pBoardDialog->exec()) {
    QStringList sListFiles;
    sListFiles = m_pBoardDialog->selectedFiles();
    if (!sListFiles.isEmpty()) {
      m_sSelectedFile = sListFiles.first();
      QFileInfo fi(m_sSelectedFile);
      m_sLastOpenedDir = fi.absolutePath();
      this->accept();
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto BoardSelection::getSelectedFile() -> const QString {
  return m_sSelectedFile;
}

auto BoardSelection::getLastOpenedDir() -> const QString {
  return m_sLastOpenedDir;
}

void BoardSelection::updateSolved(const QString &sBoard) {
  for (auto board : m_pListBoards) {
    QFileInfo fi(sBoard);
    if (fi.baseName() == board->getName()) {
      board->updateSolved();
      break;
    }
  }
}
