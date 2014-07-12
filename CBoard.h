/**
 * \file CBoard.h
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

#ifndef IQPUZZLE_CBOARD_H_
#define IQPUZZLE_CBOARD_H_

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPolygonF>
#include <QSettings>

#include "./CBlock.h"

class CBoard : public QObject {
    Q_OBJECT

  public:
    CBoard(QGraphicsView *pGraphView, QGraphicsScene *pScene,
           const QString &sBoardFile, const QString &sSavedGame = "");

    bool setupBoard();
    void setupBlocks();
    void saveGame(const QString &sSaveFile);

  signals:
    void setWindowSize(const QSize size);

  public slots:
    void zoomIn();
    void zoomOut();
    void checkPuzzleSolved();

  private:
    QColor readColor(const QString sKey);
    QPolygonF readPolygon(const QSettings *tmpSet, const QString sKey,
                          bool bScale = false);
    QPointF readStartPosition(const QSettings *tmpSet, const QString sKey);
    void doZoom();

    QGraphicsView *m_pGraphView;
    QGraphicsScene *m_pScene;
    QSettings *m_pBoardConf;
    QSettings *m_pSavedConf;
    QString m_sBoardFile;
    bool m_bSavedGame;
    QPolygonF m_BoardPoly;
    QList<CBlock *> m_listBlocks;
    unsigned char m_nNumOfBlocks;
    quint16 m_nGridSize;
};

#endif  // IQPUZZLE_CBOARD_H_
