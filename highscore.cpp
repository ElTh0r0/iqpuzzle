/**
 * \file highscore.cpp
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
 * Generating, reading, showing highscore of a specific board.
 */

#include "./highscore.h"

#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QSettings>

Highscore::Highscore(QWidget *pParent, QObject *pParentObj)
    : m_pParent(pParent), m_nMAXPOS(3) {
  Q_UNUSED(pParentObj)
#if defined _WIN32
  m_pHighscore = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                               qApp->applicationName().toLower(),
                               QStringLiteral("Highscore"));
#else
  m_pHighscore = new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                               qApp->applicationName().toLower(),
                               QStringLiteral("Highscore"));
#endif
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Highscore::showHighscore(const QString &sBoard) {
  Qt::AlignmentFlag Align;
  QDialog dialog(m_pParent);
  dialog.setWindowTitle(tr("Highscore") + " - " + sBoard);
  dialog.setWindowFlags(dialog.window()->windowFlags() &
                        ~Qt::WindowContextHelpButtonHint);

  auto *layout = new QGridLayout(&dialog);
  layout->setContentsMargins(10, 10, 10, 10);
  layout->setSpacing(10);

  layout->addWidget(new QLabel("<b>" + tr("Position") + "</b>", &dialog), 0, 0,
                    Qt::AlignCenter | Qt::AlignVCenter);
  layout->addWidget(new QLabel("<b>" + tr("Name") + "</b>", &dialog), 0, 1,
                    Qt::AlignLeft | Qt::AlignVCenter);
  layout->addWidget(new QLabel("<b>" + tr("Time") + "</b>", &dialog), 0, 2,
                    Qt::AlignCenter | Qt::AlignVCenter);
  layout->addWidget(new QLabel("<b>" + tr("Moves") + "</b>", &dialog), 0, 3,
                    Qt::AlignCenter | Qt::AlignVCenter);

  for (int nRow = 1; nRow <= m_nMAXPOS; nRow++) {
    layout->addWidget(new QLabel("#" + QString::number(nRow), &dialog), nRow, 0,
                      Qt::AlignCenter | Qt::AlignVCenter);

    QStringList sListTemp =
        this->readHighscore(sBoard, "Position" + QString::number(nRow));

    for (int nCol = 0; nCol < sListTemp.size(); nCol++) {
      if (nCol > 2) {
        break;
      }
      if (0 == nCol) {
        Align = Qt::AlignLeft;
      } else {
        Align = Qt::AlignCenter;
      }
      layout->addWidget(new QLabel(sListTemp[nCol].trimmed(), &dialog), nRow,
                        nCol + 1, Align | Qt::AlignVCenter);
    }
  }

  auto *buttons =
      new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  layout->addWidget(buttons, m_nMAXPOS + 1, 0, 1, 4, Qt::AlignCenter);

  dialog.exec();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Highscore::checkHighscore(const QString &sBoard, const quint32 nMoves,
                               const QTime tTime) {
  QStringList sListTemp;
  quint32 nScoreMoves(0);
  QTime tScoreTime(0, 0, 0);

  for (quint8 i = 1; i <= m_nMAXPOS; i++) {
    sListTemp = readHighscore(sBoard, "Position" + QString::number(i));
    if (3 != sListTemp.size()) {
      qWarning() << "Found invalid highscore:" << sListTemp;
      continue;
    }

    tScoreTime = QTime::fromString(sListTemp[1], QStringLiteral("hh:mm:ss"));
    nScoreMoves = sListTemp[2].toUInt();
    /*
    qDebug() << "Check #" << i << nMoves << "/" << tTime.toString("hh:mm:ss")
             << "---" << nScoreMoves << "/" << tScoreTime.toString("hh:mm:ss");
    */

    if (nMoves < nScoreMoves || 0 == nScoreMoves) {
      this->insertHighscore(sBoard, i, nMoves, tTime);
      break;
    }
    if (nMoves == nScoreMoves) {
      if (tTime < tScoreTime) {
        this->insertHighscore(sBoard, i, nMoves, tTime);
        break;
      }
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Highscore::insertHighscore(const QString &sBoard, const quint8 nPosition,
                                const quint32 nMoves, const QTime tTime) {
  if (nPosition <= m_nMAXPOS) {
    QStringList sListEntries;
    sListEntries.reserve(m_nMAXPOS + 1);
    QByteArray ba;
    bool bOk(false);
    // Try to get user name in Linux
    QString sName = QString::fromLatin1(qgetenv("USER"));
    if (sName.isEmpty()) {
      // Try to get user name in Windows
      sName = QString::fromLatin1(qgetenv("USERNAME"));
    }

    sName = QInputDialog::getText(
        m_pParent, tr("Highscore"),
        tr("Please insert your name for a new highscore:"), QLineEdit::Normal,
        sName, &bOk);
    if (!bOk || sName.isEmpty()) {
      sName = QStringLiteral("Guy Incognito");
    }
    sName.replace('|', ' ');

    for (int i = 1; i <= m_nMAXPOS; i++) {
      sListEntries << m_pHighscore
                          ->value(sBoard + "/Position" + QString::number(i),
                                  "fHw=")
                          .toString();
    }
    ba.append(QString(sName + "|" + tTime.toString(QStringLiteral("hh:mm:ss")) +
                      "|" + QString::number(nMoves))
                  .toUtf8());
    sListEntries.insert(nPosition - 1, QString::fromLatin1(ba.toBase64()));
    for (int i = 0; i < m_nMAXPOS; i++) {
      m_pHighscore->setValue(sBoard + "/Position" + QString::number(i + 1),
                             sListEntries[i]);
    }

    this->showHighscore(sBoard);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Highscore::readHighscore(const QString &sBoard,
                              const QString &sKey) const -> QStringList {
  QStringList sListTemp;
  QByteArray ba(m_pHighscore->value(sBoard + "/" + sKey, "fHw=").toByteArray());
  QString sTemp(QString::fromLatin1(QByteArray::fromBase64(ba)));

  sListTemp = sTemp.split('|');

  for (int j = 0; j < sListTemp.size(); j++) {
    if (sListTemp[j].trimmed().isEmpty()) {
      sListTemp[j] = '-';
    }
  }
  if (3 != sListTemp.size()) {
    qWarning() << "Found invalid highscore:" << sListTemp;
    sListTemp.clear();
    sListTemp << QStringLiteral("Cheater") << QStringLiteral("99:99:99")
              << QStringLiteral("999");
  }

  return sListTemp;
}
