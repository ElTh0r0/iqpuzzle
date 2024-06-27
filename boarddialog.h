/**
 * \file boarddialog.h
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
 * Class definition for board loading dialog.
 */

#ifndef BOARDDIALOG_H_
#define BOARDDIALOG_H_

#include <QFileDialog>

class QLabel;

/**
 * \class BoardDialog
 * \brief Extended file dialog for showing a board preview.
 */
class BoardDialog : public QFileDialog {
  Q_OBJECT

 public:
  explicit BoardDialog(QWidget *pParent, const QString &sCaption = QString(),
                       const QString &sDirectory = QString(),
                       const QString &sFilter = QString());

 protected slots:
  void OnCurrentChanged(const QString &sPath);

 private:
  QLabel *m_pSolutions;
  QLabel *m_pPreviewCaption;
  QLabel *m_pPreview;
  const QSize previewsize;
};

#endif  // BOARDDIALOG_H_
