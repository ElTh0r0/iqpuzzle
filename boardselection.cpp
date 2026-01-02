// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "./boardselection.h"

#include <QDebug>
#include <QDir>
#include <QSettings>

#include "ui_boardselection.h"

BoardSelection::BoardSelection(QWidget *pParent, const QString &sBoardsDir,
                               const QStringList &sListAllUnsolved)
    : QDialog(pParent),
      m_nColumns(3),
      m_previewsize(250, 250),
      m_sBoardsDir(sBoardsDir),
      m_sListAllUnsolved(sListAllUnsolved),
      m_pBoardDialog(nullptr) {
  qDebug() << Q_FUNC_INFO;

  m_pUi = new Ui::BoardSelection();
  m_pUi->setupUi(this);
  this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
  this->setModal(true);

  QDir boardsDir(m_sBoardsDir);
  const QStringList sListSubfolders =
      boardsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

  m_pListTabScrollArea.reserve(sListSubfolders.size());
  m_pListContent.reserve(sListSubfolders.size());
  m_pListTabLayouts.reserve(sListSubfolders.size());

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
        dir.entryList(QStringList() << QStringLiteral("*.conf"),
                      QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    int nCol = 0;
    int nRow = 0;
    quint16 nSolved = 0;
    quint16 nSum = 0;
    for (const auto &board : boardfiles) {
      QString sFile(sSubfolder + "/" + board);
      bool bSolved = !m_sListAllUnsolved.contains(sFile);
      m_pListBoards << new BoardPreview(m_sBoardsDir + "/" + sFile, bSolved,
                                        m_previewsize);
      m_pListTabLayouts.last()->addWidget(m_pListBoards.last(), nRow, nCol);

      connect(m_pListBoards.last(), &BoardPreview::selectBoard, this,
              &BoardSelection::selectBoard);
      connect(this, &BoardSelection::updatedUiLang, m_pListBoards.last(),
              &BoardPreview::updateUiLang);

      nCol++;
      if (0 == nCol % m_nColumns) {
        nCol = 0;
        nRow++;
      }

      if (bSolved) {
        nSolved++;
      }
      nSum++;
    }

    sSubfolder[0] = sSubfolder[0].toUpper();
    sSubfolder = sSubfolder.replace('_', ' ');
    if (sSubfolder != QStringLiteral("Freestyle")) {
      sSubfolder +=
          " (" + QString::number(nSolved) + "/" + QString::number(nSum) + ")";
    }
    m_pUi->tabWidget->addTab(m_pListTabScrollArea.last(), sSubfolder);
  }

  connect(m_pUi->openOwnBoard, &QPushButton::clicked, this,
          &BoardSelection::selectOwnBoard);
}

BoardSelection::~BoardSelection() {
  delete m_pUi;
  m_pUi = nullptr;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void BoardSelection::selectBoard(const QString &sFileName) {
  m_sSelectedFile = sFileName;
  this->accept();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void BoardSelection::selectOwnBoard() {
  m_sSelectedFile.clear();
  delete m_pBoardDialog;
  m_pBoardDialog = new BoardDialog(this, tr("Load board"),
                                   Settings::instance()->getLastOpenedDir(),
                                   tr("Board files") + " (*.conf)");

  if (m_pBoardDialog->exec()) {
    QStringList sListFiles;
    sListFiles = m_pBoardDialog->selectedFiles();
    if (!sListFiles.isEmpty()) {
      m_sSelectedFile = sListFiles.first();
      QFileInfo fi(m_sSelectedFile);
      Settings::instance()->setLastOpenedDir(fi.absolutePath());
      this->accept();
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto BoardSelection::getSelectedFile() -> const QString {
  return m_sSelectedFile;
}

void BoardSelection::updateSolved(const QString &sBoard) {
  QString sFileName;
  for (auto board : m_pListBoards) {
    QFileInfo fi(sBoard);
    sFileName = fi.baseName();
    if (sFileName == board->getName()) {
      board->updateSolved();
      break;
    }
  }

  QDir boardsDir(m_sBoardsDir);
  const QStringList sListSubfolders =
      boardsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

  sFileName += QStringLiteral(".conf");
  quint8 nTab = 0;
  for (const auto &sSubfolder : sListSubfolders) {
    QDir dir(m_sBoardsDir + "/" + sSubfolder);
    const QStringList boardfiles =
        dir.entryList(QStringList() << QStringLiteral("*.conf"),
                      QDir::Files | QDir::NoDotAndDotDot, QDir::Name);

    if (boardfiles.contains(sFileName)) {
      QString sTabName = m_pUi->tabWidget->tabText(nTab);
      quint8 i1 = sTabName.lastIndexOf('(') + 1;
      quint8 i2 = sTabName.lastIndexOf('/');
      QString sCurrent = sTabName.mid(i1, i2 - i1);
      // Potentially there was & added automatically
      sCurrent = sCurrent.remove('&');
      uint nSolved = sCurrent.toUInt() + 1;
      sTabName = sTabName.replace(i1, i2 - i1, QString::number(nSolved));
      m_pUi->tabWidget->setTabText(nTab, sTabName);
      break;
    }
    nTab++;
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void BoardSelection::updateUiLang() {
  m_pUi->retranslateUi(this);
  emit updatedUiLang();
}
