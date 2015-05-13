/**
 * \file CBoard.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-2015 Thorsten Roth <elthoro@gmx.de>
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
 * Class definition for a board.
 */

#ifndef IQPUZZLE_CBOARD_H_
#define IQPUZZLE_CBOARD_H_

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsView>
#include <QPolygonF>
#include <QSettings>

#include "./CBlock.h"

/**
 * \class CBoard
 * \brief Complete board generation, including block setup.
 */
class CBoard : public QGraphicsScene {
    Q_OBJECT

  public:
    CBoard(QGraphicsView *pGraphView, const QString &sBoardFile,
           CSettings *pSettings, const QString &sSavedGame = "");

    bool setupBoard();
    void setupBlocks();
    void saveGame(const QString &sSaveFile, const QString &sTime,
                  const QString &sMoves);

  signals:
    void setWindowSize(const QSize size);
    void incrementMoves();
    void solvedPuzzle();

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
    QSettings *m_pBoardConf;
    QSettings *m_pSavedConf;
    QString m_sBoardFile;
    CSettings *m_pSettings;
    bool m_bSavedGame;
    QPolygonF m_BoardPoly;
    QList<CBlock *> m_listBlocks;
    unsigned char m_nNumOfBlocks;
    quint16 m_nGridSize;
    bool m_bNotAllPiecesNeeded;
};

#endif  // IQPUZZLE_CBOARD_H_
