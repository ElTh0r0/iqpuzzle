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

#include "CBlock.h"

CBlock::CBlock( QPolygonF shape,
                unsigned short nScale,
                QColor color,
                unsigned short nID,
                QPointF posTopLeft ) :
    m_PolyShape( shape ),
    m_nGridScale( nScale ),
    m_nAlpha( 100 ),
    m_bgColor( color ),
    m_nCurrentInst (nID ),
    m_pointTopLeft( posTopLeft * nScale )
{
    qDebug() << "Creating BLOCK" << m_nCurrentInst;

    m_bPressed = false;
    this->setFlag( ItemIsMovable );

    // Scale object
    for( int i = 0; i < m_PolyShape.size(); i++ ) {
        m_PolyShape[i] *= m_nGridScale;
    }

    // Move to start position
    this->setPos( m_pointTopLeft );
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

QRectF CBlock::boundingRect() const
{
    return m_PolyShape.boundingRect();
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CBlock::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QBrush brush(m_bgColor);

    if( m_bPressed )
    {
        //this->setZValue();
        m_bgColor.setAlpha( m_nAlpha );
        brush.setColor( m_bgColor );
    }
    else
    {
        m_bgColor.setAlpha( 255 );
        brush.setColor( m_bgColor );
    }

    QPainterPath tmpPath;
    tmpPath.addPolygon( m_PolyShape );
    painter->fillPath( tmpPath, brush );
    painter->drawPolygon( m_PolyShape );
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CBlock::mousePressEvent( QGraphicsSceneMouseEvent *p_Event )
{
    if( p_Event->button() == Qt::LeftButton ) {
        m_bPressed = true;
    }
    else if ( p_Event->button() == Qt::RightButton )
    {
        this->setTransformOriginPoint( this->snapToGrid(this->boundingRect().center()) );
        this->scale(-1,1);
    }

    update();
    QGraphicsItem::mousePressEvent( p_Event );
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CBlock::wheelEvent( QGraphicsSceneWheelEvent *p_Event )
{
    // Vertical mouse wheel
    if( p_Event->orientation() == Qt::Vertical )
    {
        // Set new origin for transformation
        this->setTransformOriginPoint( this->snapToGrid(this->boundingRect().center()) );

        if ( p_Event->delta() < 0 )
        {
            m_nRotation = this->rotation() + 90;
            if ( m_nRotation >= 360 )
                m_nRotation = 0;
        }
        else
        {
            m_nRotation = this->rotation() - 90;
            if ( m_nRotation < 0 )
                m_nRotation = 270;
        }

        this->setRotation( m_nRotation );
    }

    qDebug() << "Rotate BLOCK" << m_nCurrentInst << "  " << m_nRotation << "deg";
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CBlock::mouseReleaseEvent( QGraphicsSceneMouseEvent *p_Event )
{
    m_bPressed = false;

    if( p_Event->button() == Qt::LeftButton ) {
        // Snap block to grid
        this->setPos( this->snapToGrid(this->pos()) );
    }

    update();
    QGraphicsItem::mouseReleaseEvent( p_Event );

    qDebug() << "Top Left BLOCK" << m_nCurrentInst << "  " << this->pos();
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

QPointF CBlock::snapToGrid( const QPointF point ) const
{
    int x = qRound(point.x() / m_nGridScale) * m_nGridScale;
    int y = qRound(point.y() / m_nGridScale) * m_nGridScale;
    return QPointF(x, y);
}
