/**
 * \file CHighscore.h
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

#ifndef IQPUZZLE_CHIGHSCORE_H_
#define IQPUZZLE_CHIGHSCORE_H_

#include <QObject>
#include <QSettings>
#include <QTime>

class CHighscore : public QObject {
    Q_OBJECT

  public:
    explicit CHighscore(QWidget *pParent = 0);

  public slots:
    void showHighscore(QString sBoard);
    void checkHighscore(QString sBoard, quint32 nMoves, QTime tTime);

  private:
    QStringList readHighscore(QString &sBoard, QString sKey);
    void insertHighscore(QString sBoard, quint8 nPosition,
                         quint32 nMoves, QTime tTime);

    QWidget *m_pParent;
    QSettings *m_pHighscore;
    const quint8 m_nMaxPos;
};

#endif //  IQPUZZLE_CHIGHSCORE_H_
