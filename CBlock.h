/**
 * \file CBlock.cpp
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

#ifndef IQPUZZLE_CBLOCK_H_
#define IQPUZZLE_CBLOCK_H_

#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QPainter>

class CBlock : public QGraphicsItem {
  public:
    CBlock(QPolygonF shape, quint16 nScale, QColor color,
           quint16 nID, QList<CBlock *> *pListBlocks,
           QPointF posTopLeft = QPoint(0, 0));

    QRectF boundingRect() const;
    QPainterPath shape() const;
    QPointF getPosition() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

    void setNewZValue(qint16 nZ);
    void rescaleBlock(quint16 nNewScale);
    quint16 getIndex() const;
    enum { Type = UserType + 1 };

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *p_Event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *p_Event);
    void wheelEvent(QGraphicsSceneWheelEvent *p_Event);

    int type() const;

  private:
    QPointF snapToGrid(const QPointF point) const;

    QPolygonF m_PolyShape;
    QTransform *m_pTransform;
    QGraphicsSimpleTextItem m_ItemNumberText;
    quint16 m_nGridScale;
    const quint16 m_nAlpha;
    QColor m_bgColor;
    quint16 m_nCurrentInst;
    QList<CBlock *> *m_pListBlocks;

    QPointF m_pointTopLeft;
    QPointF m_posBlockSelected;

    bool m_bPressed;
};

#endif  // IQPUZZLE_CBLOCK_H_
