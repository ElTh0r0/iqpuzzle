// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOARDSELECTION_H_
#define BOARDSELECTION_H_

#include <QDialog>
#include <QGridLayout>
#include <QScrollArea>

#include "./boarddialog.h"
#include "./boardpreview.h"
#include "./settings.h"

namespace Ui {
class BoardSelection;
}

class BoardSelection : public QDialog {
  Q_OBJECT

 public:
  explicit BoardSelection(QWidget *pParent, const QString &sBoardsDir,
                          const QStringList &sListAllUnsolved);

  virtual ~BoardSelection();

  auto getSelectedFile() -> const QString;
  void updateSolved(const QString &sBoard);

 public slots:
  void updateUiLang();

 private slots:
  void selectBoard(const QString &sFileName);
  void selectOwnBoard();

 signals:
  void updatedUiLang();

 private:
  Ui::BoardSelection *m_pUi;
  const quint8 m_nColumns;
  const QSize m_previewsize;
  const QString m_sBoardsDir;
  const QStringList m_sListAllUnsolved;
  BoardDialog *m_pBoardDialog;
  QString m_sSelectedFile;

  QList<QScrollArea *> m_pListTabScrollArea;
  QList<QWidget *> m_pListContent;
  QList<QGridLayout *> m_pListTabLayouts;
  QList<BoardPreview *> m_pListBoards;
};

#endif  // BOARDSELECTION_H_
