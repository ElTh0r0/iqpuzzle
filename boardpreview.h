/**
 * \file boardpreview.h
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
 * Board preview widget.
 */

#ifndef BOARDPREVIEW_H
#define BOARDPREVIEW_H

#include <QMouseEvent>
#include <QWidget>

namespace Ui {
class BoardPreview;
}

class BoardPreview : public QWidget {
  Q_OBJECT

 public:
  explicit BoardPreview(const QString &sFilePath, const bool bSolved,
                        const QSize previewsize, QWidget *pParent = nullptr);
  ~BoardPreview();

  auto getName() -> const QString;
  void updateSolved();

 signals:
  void selectBoard(const QString &sFileName);

 protected:
  void mousePressEvent(QMouseEvent *p_Event) override;

 private:
  void setSolutions(QString const &sFilePath);
  void setPreview(QString sFilePath, const QSize previewsize);

  Ui::BoardPreview *m_pUi;
  QString m_sFilePath;
};

#endif  // BOARDPREVIEW_H
