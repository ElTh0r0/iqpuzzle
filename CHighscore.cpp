/**
 * \file CHighscore.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-2014 Thorsten Roth <elthoro@gmx.de>
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
 */

#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>

#include "CHighscore.h"

CHighscore::CHighscore(QWidget *pParent)
    : m_pParent(pParent),
      m_nMaxPos(3) {
#if defined _WIN32
    m_pHighscore = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                 qApp->applicationName().toLower(),
                                 "Highscore");
#else
    m_pHighscore = new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                                 qApp->applicationName().toLower(),
                                 "Highscore");
#endif
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CHighscore::showHighscore(QString sBoard) {
    Qt::AlignmentFlag Align = Qt::AlignCenter;
    QStringList sListTemp;
    QDialog dialog(m_pParent);
    dialog.setWindowTitle(trUtf8("Highscore") + " - " + sBoard);
    dialog.setWindowFlags(dialog.window()->windowFlags()
                           & ~Qt::WindowContextHelpButtonHint);

    QGridLayout* layout = new QGridLayout(&dialog);
    layout->setMargin(10);
    layout->setSpacing(10);

    layout->addWidget(new QLabel(trUtf8("<b>Position</b>"), &dialog),
                      0, 0, Qt::AlignCenter | Qt::AlignVCenter);
    layout->addWidget(new QLabel(trUtf8("<b>Name</b>"), &dialog),
                      0, 1, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(new QLabel(trUtf8("<b>Time</b>"), &dialog),
                      0, 2, Qt::AlignCenter | Qt::AlignVCenter);
    layout->addWidget(new QLabel(trUtf8("<b>Moves</b>"), &dialog),
                      0, 3, Qt::AlignCenter | Qt::AlignVCenter);

    for (int i = 1; i <= m_nMaxPos; i++) {
        layout->addWidget(new QLabel("#" + QString::number(i), &dialog),
                          i, 0, Qt::AlignCenter | Qt::AlignVCenter);

        sListTemp = readHighscore(sBoard, "Position" + QString::number(i));

        for (int j = 0; j < sListTemp.size(); j++) {
            if (j > 2) {
                break;
            }
            if (0 == j) {
                Align = Qt::AlignLeft;
            } else {
                Align = Qt::AlignCenter;
            }
            layout->addWidget(new QLabel(sListTemp[j].trimmed(), &dialog),
                              i, j + 1, Align | Qt::AlignVCenter);
        }
    }

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close,
                                                     Qt::Horizontal, &dialog);
    connect(buttons, SIGNAL(rejected()),
            &dialog, SLOT(reject()));
    layout->addWidget(buttons, m_nMaxPos + 1, 0, 1, 3);

    dialog.exec();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CHighscore::checkHighscore(QString sBoard, quint32 nMoves, QTime tTime) {
    QStringList sListTemp;
    quint32 nScoreMoves(0);
    QTime tScoreTime(0, 0, 0);

    for (int i = 1; i <= m_nMaxPos; i++) {
        sListTemp = readHighscore(sBoard, "Position" + QString::number(i));
        if (3 != sListTemp.size()) {
            qWarning() << "Found invalid highscore:" << sListTemp;
            continue;
        }

        tScoreTime = tScoreTime.fromString(sListTemp[1], "hh:mm:ss");
        nScoreMoves = sListTemp[2].toUInt();
        /*
        qDebug() << "Check #" << i << nMoves << "/" << tTime.toString("hh:mm:ss")
                 << "---" << nScoreMoves << "/" << tScoreTime.toString("hh:mm:ss");
        */

        if (nMoves < nScoreMoves || 0 == nScoreMoves) {
            this->insertHighscore(sBoard, i, nMoves, tTime);
            break;
        } else if (nMoves == nScoreMoves) {
            if (tTime < tScoreTime) {
                this->insertHighscore(sBoard, i, nMoves, tTime);
                break;
            } else if (tTime >= tScoreTime) {
                this->insertHighscore(sBoard, i+1, nMoves, tTime);
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CHighscore::insertHighscore(QString sBoard, quint8 nPosition,
                                 quint32 nMoves, QTime tTime) {
    if (nPosition <= m_nMaxPos) {
        QStringList sListEntries;
        QByteArray ba;
        bool bOk;
        QString sName = QInputDialog::getText(m_pParent, trUtf8("Highscore"),
                                             trUtf8("Please insert your name for a new highscore:"),
                                             QLineEdit::Normal, "", &bOk);
        if (true != bOk || sName.isEmpty()) {
            sName = "Guy Incognito";
        }
        sName.replace("|", " ");

        for (int i = 1; i <= m_nMaxPos; i++) {
            sListEntries << m_pHighscore->value(sBoard + "/Position" + QString::number(i),
                                                "fHw=").toString();
        }
        ba.append(sName + "|" + tTime.toString("hh:mm:ss") + "|" + QString::number(nMoves));
        sListEntries.insert(nPosition - 1, ba.toBase64());
        for (int i = 0; i < m_nMaxPos; i++) {
            m_pHighscore->setValue(sBoard + "/Position" + QString::number(i + 1),
                                   sListEntries[i]);
        }

        this->showHighscore(sBoard);
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QStringList CHighscore::readHighscore(QString &sBoard, QString sKey) {
    QStringList sListTemp;
    QByteArray ba = m_pHighscore->value(sBoard + "/" + sKey, "fHw=").toByteArray();
    QString sTemp = QByteArray::fromBase64(ba);

    sListTemp = sTemp.split("|");

    for (int j = 0; j < sListTemp.size(); j++) {
        if (sListTemp[j].trimmed().isEmpty()) {
            sListTemp[j] = "-";
        }
    }
    if (3 != sListTemp.size()) {
        qWarning() << "Found invalid highscore:" << sListTemp;
        sListTemp.clear();
        sListTemp << "Cheater" << "99:99:99" << "999";
    }

    return sListTemp;
}
