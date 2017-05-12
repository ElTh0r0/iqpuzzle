/**
 * \file CBoardDialog.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-2017 Thorsten Roth <elthoro@gmx.de>
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
 * Extended file dialog for showing a board preview.
 */

#include <QDebug>
#include <QSettings>
#include <QVBoxLayout>

#include "./CBoardDialog.h"

CBoardDialog::CBoardDialog(QWidget *pParent, const QString &sCaption,
                           const QString &sDirectory, const QString &sFilter)
  : QFileDialog(pParent, sCaption, sDirectory, sFilter) {
  this->setObjectName("BoardFileDialog");
  // Needed for Windows, otherwise native dialog crashes while adapting layout
  this->setOption(QFileDialog::DontUseNativeDialog, true);
  this->setViewMode(QFileDialog::List);
  QVBoxLayout *boxlayout = new QVBoxLayout();

  m_pSolutions = new QLabel(trUtf8("Solutions") + ":", this);
  m_pPreviewCaption = new QLabel(trUtf8("Preview") + ":", this);
  m_pPreview = new QLabel("", this);
  m_pPreview->setAlignment(Qt::AlignCenter);
  m_pPreview->setObjectName("labelPreview");
  m_pPreview->resize(150, 150);

  boxlayout->addWidget(m_pSolutions);
  boxlayout->addWidget(m_pPreviewCaption);
  boxlayout->addWidget(m_pPreview);
  boxlayout->addStretch();
  {
    QGridLayout *layout = reinterpret_cast<QGridLayout*>(this->layout());
    layout->addLayout(boxlayout, 1, 3, 3, 1);
  }

  connect(this, SIGNAL(currentChanged(const QString&)),
          this, SLOT(OnCurrentChanged(const QString&)));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoardDialog::OnCurrentChanged(const QString &sPath) {
  QSettings tmpSet(sPath, QSettings::IniFormat);
  quint32 nSolutions(tmpSet.value("PossibleSolutions", 0).toUInt());
  QString sSolutions(QString::number(nSolutions));
  if ("0" == sSolutions) {
    sSolutions = trUtf8("Unknown");
  }
  m_pSolutions->setText(trUtf8("Solutions") + ": " + sSolutions);

  QString sImage(sPath);
  sImage.replace(".conf", ".png");
  // qDebug() << "Preview image:" << sImage;
  QPixmap pixmap = QPixmap(sImage);
  if (pixmap.isNull()) {
    m_pPreview->setText("\n" + trUtf8("No preview available"));
  } else {
    m_pPreview->resize(200, 200);
    m_pPreview->setPixmap(pixmap.scaled(m_pPreview->width(),
                                        m_pPreview->height(),
                                        Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation));
  }
}
