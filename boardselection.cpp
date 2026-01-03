// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "./boardselection.h"

#include <QDebug>
#include <QDir>
#include <QSettings>

#include "./settings.h"
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

  QString sCategory;
  for (const auto &sSubfolder : sListSubfolders) {
    m_pListTabScrollArea << new QScrollArea(this);
    m_pListTabScrollArea.last()->setWidgetResizable(true);
    m_pListTabScrollArea.last()->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);
    m_pListContent << new QWidget(this);
    m_pListTabScrollArea.last()->setWidget(m_pListContent.last());
    m_pListTabLayouts << new QGridLayout(m_pListContent.last());

    sCategory = sSubfolder;
    sCategory[0] = sCategory[0].toUpper();
    sCategory = sCategory.replace('_', ' ');

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
      m_pListBoards[board] = new BoardPreview(
          m_sBoardsDir + "/" + sFile, sCategory, bSolved, m_previewsize);
      m_pListTabLayouts.last()->addWidget(m_pListBoards[board], nRow, nCol);

      connect(m_pListBoards[board], &BoardPreview::selectBoard, this,
              &BoardSelection::selectBoard);
      connect(this, &BoardSelection::updatedUiLang, m_pListBoards[board],
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

    if (sCategory != QStringLiteral("Freestyle")) {
      sCategory +=
          " (" + QString::number(nSolved) + "/" + QString::number(nSum) + ")";
    }
    m_pUi->tabWidget->addTab(m_pListTabScrollArea.last(), sCategory);
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
  QFileInfo fi(sBoard);
  QString sFileName(fi.fileName());
  if (m_pListBoards.contains(sFileName)) {
    if (!m_pListBoards[sFileName]->isSolved()) {
      m_pListBoards[sFileName]->updateSolved();
      QString sCategory = m_pListBoards[sFileName]->getCategory() + " (";
      for (int i = 0; i < m_pUi->tabWidget->count(); ++i) {
        if (m_pUi->tabWidget->tabText(i).startsWith(sCategory)) {
          QString sTabName = m_pUi->tabWidget->tabText(i);
          sTabName.remove(sCategory);
          sTabName.remove(')');
          QStringList sList = sTabName.split('/');
          if (sList.size() >= 2) {
            sList[0] = QString::number(sList[0].trimmed().toUInt() + 1);
            sList[1] = sList[1].trimmed();
            m_pUi->tabWidget->setTabText(
                i, sCategory + sList[0] + "/" + sList[1] + ")");
          }
          break;
        }
      }
    }
  } else {
    qWarning() << "BoardSelection::updateSolved was called with unknown board:"
               << sFileName;
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void BoardSelection::updateUiLang() {
  m_pUi->retranslateUi(this);
  emit updatedUiLang();
}
