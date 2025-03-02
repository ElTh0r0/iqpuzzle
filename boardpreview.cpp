/**
 * \file boardpreview.cpp
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

#include "boardpreview.h"

#include <QFileInfo>
#include <QSettings>

#include "ui_boardpreview.h"

BoardPreview::BoardPreview(const QString &sFilePath, const bool bSolved,
                           const QSize previewsize, QWidget *pParent)
    : QWidget(pParent), m_pUi(new Ui::BoardPreview), m_sFilePath(sFilePath) {
  m_pUi->setupUi(this);
  QFileInfo fi(m_sFilePath);
  QString sFile(fi.baseName());
  sFile[0] = sFile[0].toUpper();
  sFile.replace('_', ' ');
  m_pUi->lbl_BoardName->setText(sFile);

  if (bSolved) {
    m_pUi->lblSolved->setText(tr("Solved") +
                              ": <img src=\":/icons/emblem-checked.png\">");
  } else {
    m_pUi->lblSolved->setText(tr("Solved") +
                              ": <img src=\":/icons/emblem-error.png\">");
  }
  this->setSolutions(m_sFilePath);
  this->setPreview(m_sFilePath, previewsize);

  m_pUi->previewFrame->setStyleSheet(
      ":hover{background:" + this->palette().highlight().color().name() + "}");
}

BoardPreview::~BoardPreview() { delete m_pUi; }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void BoardPreview::setSolutions(QString const &sFilePath) {
  QSettings tmpSet(sFilePath, QSettings::IniFormat);
  m_nSolutions = tmpSet.value(QStringLiteral("PossibleSolutions"), 0).toUInt();
  bool bFreestyle = tmpSet.value(QStringLiteral("Freestyle"), false).toBool();

  QString sSolutions(QString::number(m_nSolutions));
  if (0 == m_nSolutions) {
    sSolutions = tr("Unknown");
  }
  if (bFreestyle) {
    m_pUi->lblSolutions->clear();
    m_pUi->lblSolved->clear();
  } else {
    m_pUi->lblSolutions->setText(tr("Solutions") + ": " + sSolutions);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void BoardPreview::setPreview(QString sFilePath, const QSize previewsize) {
  sFilePath.replace(QStringLiteral(".conf"), QStringLiteral(".png"));
  // qDebug() << "Preview image:" << sImage;
  QPixmap pixmap = QPixmap(sFilePath);
  if (pixmap.isNull()) {
    m_pUi->lblPreviewImage->setText(tr("No preview available"));
  } else {
    m_pUi->lblPreviewImage->resize(previewsize);
    m_pUi->lblPreviewImage->setPixmap(pixmap.scaled(
        m_pUi->lblPreviewImage->width(), m_pUi->lblPreviewImage->height(),
        Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void BoardPreview::mousePressEvent(QMouseEvent *p_Event) {
  if (Qt::NoButton != p_Event->button()) {
    emit selectBoard(m_sFilePath);
  }
  QWidget::mousePressEvent(p_Event);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto BoardPreview::getName() -> const QString {
  QFileInfo fi(m_sFilePath);
  return fi.baseName();
}

void BoardPreview::updateSolved() {
  m_pUi->lblSolved->setText(tr("Solved") +
                            ": <img src=\":/icons/emblem-checked.png\">");
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void BoardPreview::updateUiLang() {
  if (!m_pUi->lblSolved->text().isEmpty()) {
    QString sSolutions(QString::number(m_nSolutions));
    if (0 == m_nSolutions) {
      sSolutions = tr("Unknown");
    }
    m_pUi->lblSolutions->setText(tr("Solutions") + ": " + sSolutions);

    QString sTmp(m_pUi->lblSolved->text());
    sTmp.replace(0, sTmp.indexOf(':'), tr("Solved"));
    m_pUi->lblSolved->setText(sTmp);
  }
  m_pUi->retranslateUi(this);
}
