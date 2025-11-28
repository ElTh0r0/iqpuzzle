// SPDX-FileCopyrightText: 2014-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "./boarddialog.h"

#include <QDebug>
#include <QLabel>
#include <QSettings>
#include <QVBoxLayout>

BoardDialog::BoardDialog(QWidget *pParent, const QString &sCaption,
                         const QString &sDirectory, const QString &sFilter)
    : QFileDialog(pParent, sCaption, sDirectory, sFilter),
      previewsize(250, 250) {
  qDebug() << Q_FUNC_INFO;

  this->setObjectName(QStringLiteral("BoardFileDialog"));
  // Needed for Windows, otherwise native dialog crashes while adapting layout
  this->setOption(QFileDialog::DontUseNativeDialog, true);
  this->setViewMode(QFileDialog::List);
  auto *boxlayout = new QVBoxLayout();

  m_pSolutions = new QLabel(tr("Solutions") + ":", this);
  m_pPreviewCaption = new QLabel(tr("Preview") + ":", this);
  m_pPreview = new QLabel(QString(), this);
  m_pPreview->setAlignment(Qt::AlignCenter);
  m_pPreview->setObjectName(QStringLiteral("labelPreview"));
  m_pPreview->resize(previewsize);
  m_pPreview->setText("\n" + tr("No preview available"));

  boxlayout->addWidget(m_pSolutions);
  boxlayout->addWidget(m_pPreviewCaption);
  boxlayout->addWidget(m_pPreview);
  boxlayout->addStretch();
  {
    auto *layout = reinterpret_cast<QGridLayout *>(this->layout());
    layout->addLayout(boxlayout, 1, 3, 3, 1);
  }

  connect(this, &BoardDialog::currentChanged, this,
          &BoardDialog::OnCurrentChanged);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void BoardDialog::OnCurrentChanged(const QString &sPath) {
  QSettings tmpSet(sPath, QSettings::IniFormat);
  quint32 nSolutions(
      tmpSet.value(QStringLiteral("PossibleSolutions"), 0).toUInt());
  QString sSolutions(QString::number(nSolutions));
  if ("0" == sSolutions) {
    sSolutions = tr("Unknown");
  }
  m_pSolutions->setText(tr("Solutions") + ": " + sSolutions);

  QString sImage(sPath);
  sImage.replace(QStringLiteral(".conf"), QStringLiteral(".png"));
  // qDebug() << "Preview image:" << sImage;
  QPixmap pixmap = QPixmap(sImage);
  if (pixmap.isNull()) {
    m_pPreview->setText("\n" + tr("No preview available"));
  } else {
    m_pPreview->resize(previewsize);
    m_pPreview->setPixmap(
        pixmap.scaled(m_pPreview->width(), m_pPreview->height(),
                      Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }
}
