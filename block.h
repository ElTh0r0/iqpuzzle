/**
 * \file block.h
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
 * Class definition for a block.
 */

#ifndef BLOCK_H_
#define BLOCK_H_

#include <QBrush>
#include <QGraphicsObject>
#include <QPen>

class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;
class QPainter;

class Settings;

/**
 * \class Block
 * \brief Block handling (move, rotate, collision check).
 */
class Block : public QGraphicsObject {
  Q_OBJECT

 public:
  Block(const quint16 nID, QPolygonF shape, const QBrush &bgcolor, QPen border,
        quint16 nGrid, QList<Block *> *pListBlocks, Settings *pSettings,
        QPointF posTopLeft = QPoint(0, 0), const bool bBarrier = false,
        QObject *pParentObj = nullptr);

  auto boundingRect() const -> QRectF override;
  auto shape() const -> QPainterPath override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget = nullptr) override;
  auto getPosition() const -> QPointF;
  void setBrushStyle(Qt::BrushStyle style);

  auto getPolygon() const -> QPolygonF;
  auto isBarrier() const -> bool;
  auto isActive() const -> bool;
  void bringToForeground();
  void rescaleBlock(const quint16 nNewScale);
  auto getIndex() const -> quint16;
  enum { Type = UserType + 1 };

 signals:
  void incrementMoves();
  void checkPuzzleSolved();

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *p_Event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *p_Event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *p_Event) override;
  void wheelEvent(QGraphicsSceneWheelEvent *p_Event) override;
  auto type() const -> int override;

 private:
  void moveBlockGrid(const QPointF pos);
  auto checkCollision(const QPainterPath &thisPath) -> bool;
  void checkBlockIntersection();
  auto snapToGrid(const QPointF point) const -> QPointF;
  void resetBrushStyle() const;
  auto isAnyBlockActive() -> bool;

  void moveBlock(const bool bRelease = false);
  void rotateBlock(const int nDelta = -1);
  void flipBlock();

  const quint16 m_nID;
  const quint16 m_nZBlock;
  const quint8 m_nZBarrier;
  const bool m_bBarrier;
  QPolygonF m_PolyShape;
  QBrush m_bgBrush;
  QPen m_borderPen;
  quint16 m_nGrid;
  QList<Block *> *m_pListBlocks;
  Settings *m_pSettings;
  bool m_bActive;
  QPixmap m_CollTexture;

  QTransform *m_pTransform;
  QPointF m_posBlockSelected;
  QPointF m_posMouseSelected;
  QPolygonF m_PolyShapeSelected;
  QGraphicsSimpleTextItem m_ItemNumberText;
};

#endif  // BLOCK_H_
