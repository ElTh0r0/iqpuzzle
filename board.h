/**
 * \file board.h
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
 * Class definition for a board.
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <QGraphicsScene>
#include <QPolygonF>

class QSettings;

class Block;
class Settings;

/**
 * \class Board
 * \brief Complete board generation, including block setup.
 */
class Board : public QGraphicsScene {
  Q_OBJECT

 public:
  Board(QWidget *pParent, QGraphicsView *pGraphView, QString sBoardFile,
        Settings *pSettings, const quint16 nGridSize = 0,
        const QString &sSavedGame = QLatin1String(""),
        QObject *pParentObj = nullptr);

  auto setupBoard() -> bool;
  auto setupBlocks() -> bool;
  void saveGame(const QString &sSaveFile, const QString &sTime,
                const QString &sMoves);
  auto getGridSize() const -> quint16;

 signals:
  void setWindowSize(const QSize size, const bool bFreestyle);
  void incrementMoves();
  void solvedPuzzle();

 public slots:
  void zoomIn();
  void zoomOut();
  void checkPuzzleSolved();

 private slots:
  void useSystemBackground(const bool bUseSysColor);

 private:
  void drawBoard();
  void drawGrid();
  auto createBlocks() -> bool;
  auto createBarriers() -> bool;
  auto readColor(const QString &sKey,
                 const bool bColorIsBoardBG = false) const -> QColor;
  QPolygonF readPolygon(const QSettings *tmpSet, const QString &sKey,
                        const bool bScale = false);
  static auto checkOrthogonality(QPointF point) -> bool;
  static auto readStartPosition(const QSettings *tmpSet, const QString &sKey,
                                QWidget *pParent) -> QPointF;
  void doZoom();

  QWidget *m_pParent;
  QGraphicsView *m_pGraphView;
  QSettings *m_pBoardConf;
  QSettings *m_pSavedConf;
  QString m_sBoardFile;
  Settings *m_pSettings;
  bool m_bSavedGame;
  QPolygonF m_BoardPoly;
  QList<Block *> m_listBlocks;
  quint8 m_nNumOfBlocks{};
  quint16 m_nGridSize;
  bool m_bNotAllPiecesNeeded{};
  bool m_bFreestyle{};

  static const quint8 ZOOMGRID = 5;
  static const quint8 DEFAULTGRID = 25;
  static const quint8 MAXGRID = 255;
};

#endif  // BOARD_H_
