/**
 * \file board.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-2018 Thorsten Roth <elthoro@gmx.de>
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

#ifndef BOARD_H_
#define BOARD_H_

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsView>
#include <QPolygonF>
#include <QSettings>

#include "./block.h"

/**
 * \class Board
 * \brief Complete board generation, including block setup.
 */
class Board : public QGraphicsScene {
  Q_OBJECT

 public:
    Board(QGraphicsView *pGraphView, const QString &sBoardFile,
          Settings *pSettings, const quint16 nGridSize = 0,
          const QString &sSavedGame = "");

    bool setupBoard();
    bool setupBlocks();
    void saveGame(const QString &sSaveFile, const QString &sTime,
                  const QString &sMoves);
    quint16 getGridSize() const;

 signals:
    void setWindowSize(const QSize size, const bool bFreestyle);
    void incrementMoves();
    void solvedPuzzle();

 public slots:
    void zoomIn();
    void zoomOut();
    void checkPuzzleSolved();

 private:
    void drawBoard();
    void drawGrid();
    bool createBlocks();
    bool createBarriers();
    QColor readColor(const QString &sKey) const;
    QPolygonF readPolygon(const QSettings *tmpSet, const QString &sKey,
                          const bool bScale = false);
    bool checkOrthogonality(QPointF point) const;
    QPointF readStartPosition(const QSettings *tmpSet,
                              const QString &sKey) const;
    void doZoom();

    QGraphicsView *m_pGraphView;
    QSettings *m_pBoardConf;
    QSettings *m_pSavedConf;
    QString m_sBoardFile;
    Settings *m_pSettings;
    bool m_bSavedGame;
    QPolygonF m_BoardPoly;
    QList<Block *> m_listBlocks;
    unsigned char m_nNumOfBlocks;
    quint16 m_nGridSize;
    bool m_bNotAllPiecesNeeded;
    bool m_bFreestyle;
};

#endif  // BOARD_H_
