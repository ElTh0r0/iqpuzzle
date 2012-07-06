/**
 * \file CBlock.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012 Thorsten Roth <elthoro@gmx.de>
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

#ifndef CBLOCK_H
#define CBLOCK_H

#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QDebug>

class CBlock : public QGraphicsItem
{
public:
    CBlock( QPolygonF shape, unsigned short nScale, QColor color, unsigned short nID,
            QList<CBlock *> *pListBlocks, QPointF posTopLeft = QPoint(0,0) );

    QRectF boundingRect() const;
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0 );

    void setNewZValue( short nZ );
    void rescaleBlock( unsigned short nNewScale );
    unsigned short getIndex();
    enum { Type = UserType + 1 };

protected:
    void mousePressEvent( QGraphicsSceneMouseEvent *p_Event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *p_Event );
    void wheelEvent( QGraphicsSceneWheelEvent *p_Event );

    int type() const;

private:

    QPointF snapToGrid( const QPointF point ) const;

    QPolygonF m_PolyShape;
    unsigned short m_nGridScale;
    const unsigned short m_nAlpha;
    QColor m_bgColor;
    unsigned short m_nCurrentInst;
    QList<CBlock *> *m_pListBlocks;
    short m_nRotation;

    QPointF m_pointTopLeft;

    bool m_bPressed;
};

#endif // CBLOCK_H
