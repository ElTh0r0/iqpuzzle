/**
 * \file boardselection.h
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
 * Class definition of board selection dialog.
 */

#ifndef BOARDSELECTION_H_
#define BOARDSELECTION_H_

#include <QDialog>
#include <QGridLayout>
#include <QScrollArea>

#include "./boarddialog.h"
#include "./boardpreview.h"

namespace Ui {
class BoardSelection;
}

/**
 * \class BoardDialog
 */
class BoardSelection : public QDialog {
  Q_OBJECT

 public:
  explicit BoardSelection(QWidget *pParent, const QString &sBoardsDir,
                          const QStringList &sListAllUnsolved,
                          const QString &sLastOpenedDir);

  virtual ~BoardSelection();

  auto getSelectedFile() -> const QString;
  auto getLastOpenedDir() -> const QString;
  void updateSolved(const QString &sBoard);

 private slots:
  void SelectBoard(const QString &sFileName);
  void SelectOwnBoard();

 private:
  Ui::BoardSelection *m_pUi;
  const quint8 m_nColumns;
  const QSize m_previewsize;
  const QString m_sBoardsDir;
  const QStringList m_sListAllUnsolved;
  QString m_sLastOpenedDir;
  BoardDialog *m_pBoardDialog;
  QString m_sSelectedFile;

  QList<QScrollArea *> m_pListTabScrollArea;
  QList<QWidget *> m_pListContent;
  QList<QGridLayout *> m_pListTabLayouts;
  QList<BoardPreview *> m_pListBoards;
};

#endif  // BOARDSELECTION_H_
