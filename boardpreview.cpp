// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

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
