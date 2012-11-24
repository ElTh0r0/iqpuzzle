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

#include "./CBlock.h"

extern bool bDEBUG;

CBlock::CBlock(QPolygonF shape, quint16 nScale, QColor color,
               quint16 nID, QList<CBlock *> *pListBlocks,
               QPointF posTopLeft)
    : m_PolyShape(shape),
      m_nGridScale(nScale),
      m_nAlpha(100),
      m_bgColor(color),
      m_nCurrentInst(nID),
      m_pListBlocks(pListBlocks),
      m_pointTopLeft(posTopLeft * nScale) {
    qDebug() << "Creating BLOCK" << m_nCurrentInst <<
                "\tPosition:" << m_pointTopLeft <<
                "\tClosed shape:" << m_PolyShape.isClosed();

    m_bPressed = false;
    this->setFlag(ItemIsMovable);

    // Scale object
    this->setScale(m_nGridScale);

    // Move to start position
    this->setPos(m_pointTopLeft);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QRectF CBlock::boundingRect() const {
    return m_PolyShape.boundingRect();
}

QPainterPath CBlock::shape() const {
    QPainterPath path;
    path.addPolygon(m_PolyShape);
    return path;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QBrush brush(m_bgColor);

    if (m_bPressed) {
        m_bgColor.setAlpha(m_nAlpha);
        brush.setColor(m_bgColor);
    } else {
        m_bgColor.setAlpha(255);
        brush.setColor(m_bgColor);
    }

    QPainterPath tmpPath;
    tmpPath.addPolygon(m_PolyShape);
    painter->fillPath(tmpPath, brush);
    painter->drawPolygon(m_PolyShape);

    // Adding block ID for debugging
    if (bDEBUG) {
        m_ItemNumberText.setFont(QFont("Arial", 1));
        m_ItemNumberText.setText(QString::number(m_nCurrentInst));
        m_ItemNumberText.setPos(0.2, -1.1);
        m_ItemNumberText.setParentItem(this);
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::mousePressEvent(QGraphicsSceneMouseEvent *p_Event) {
    if (p_Event->button() == Qt::LeftButton) {
        m_bPressed = true;

        // Bring current block to foreground and update Z values
        for (int i = 0; i < m_pListBlocks->size(); i++) {
            (*m_pListBlocks)[i]->setNewZValue(-1);
        }
        this->setZValue(m_pListBlocks->size() + 2);

        m_posBlockSelected = this->pos();  // Save last position
    } else if (p_Event->button() == Qt::RightButton) {
        this->prepareGeometryChange();
        this->setTransformOriginPoint(this->snapToGrid(this->boundingRect().center()));
        this->scale(-1, 1);
    }

    update();
    QGraphicsItem::mousePressEvent(p_Event);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::wheelEvent(QGraphicsSceneWheelEvent *p_Event) {
    // Vertical mouse wheel
    if (p_Event->orientation() == Qt::Vertical) {
        // Set new origin for transformation
        this->setTransformOriginPoint(this->snapToGrid(this->boundingRect().center()));

        if (p_Event->delta() < 0) {
            m_nRotation = this->rotation() + 90;
            if (m_nRotation >= 360) {
                m_nRotation = 0;
            }
        } else {
            m_nRotation = this->rotation() - 90;
            if (m_nRotation < 0) {
                m_nRotation = 270;
            }
        }

        this->prepareGeometryChange();
        this->setRotation(m_nRotation);
    }

    qDebug() << "Rotate BLOCK" << m_nCurrentInst << "  " <<
                m_nRotation << "deg";
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::mouseReleaseEvent(QGraphicsSceneMouseEvent *p_Event) {
    if (p_Event->button() == Qt::LeftButton) {
        m_bPressed = false;
        this->prepareGeometryChange();

        this->setPos(this->snapToGrid(this->pos()));

        unsigned short nCnt = 0;

        /*
        QList<QGraphicsItem *> collidingGraphItems = this->collidingItems();

        // Filter colliding objects for CBlocks
        foreach (QGraphicsItem* gi, collidingGraphItems) {
            if (gi->type() == this->Type) {
                QPainterPath test = this->shape().intersected(shape());

                if (!test.isEmpty()) {
                    nCnt++;
                    qDebug() << test;
                }
            }
        }
        */



        QPolygonF thisPoly = m_PolyShape;
        thisPoly.translate(this->pos());
        QPolygonF collidingPoly;
        QPolygonF tmpPoly;
        foreach (CBlock *block, *m_pListBlocks) {
            if (block->getIndex() != m_nCurrentInst) {
                if (this->collidesWithItem(block))
                {
                    collidingPoly = block->getPolygon();
                    collidingPoly.translate(block->getPosition());

                    qDebug() << thisPoly << "\n" << collidingPoly;
                    tmpPoly = thisPoly.intersected(collidingPoly);
                    qDebug() << "Col:" << block->getIndex() << "  Interpoly:" <<
                                tmpPoly <<
                                "  Pos:" << tmpPoly.boundingRect().x() << "," <<
                                tmpPoly.boundingRect().y();
                    nCnt++;
                }
            }
        }

        qDebug() << "COLLISIONS:" << nCnt;
    }


    update();
    QGraphicsItem::mouseReleaseEvent(p_Event);

    //qDebug() << "Top Left BLOCK" << m_nCurrentInst << "  " << this->pos();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QPointF CBlock::snapToGrid(const QPointF point) const {
    int x = qRound(point.x() / m_nGridScale) * m_nGridScale;
    int y = qRound(point.y() / m_nGridScale) * m_nGridScale;
    return QPointF(x, y);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::setNewZValue(qint16 nZ) {
    if (nZ < 0) {
        if (this->zValue() > 1) {
            this->setZValue(this->zValue() - 1);
        } else {
            this->setZValue(1);
        }
    } else {
        this->setZValue(nZ);
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::rescaleBlock(quint16 nNewScale) {
    this->prepareGeometryChange();

    QPointF tmpTopLeft = this->pos() / m_nGridScale * nNewScale;
    this->setScale(nNewScale);
    m_nGridScale = nNewScale;
    this->setPos(this->snapToGrid(tmpTopLeft));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

int CBlock::type() const {
    // Enable the use of qgraphicsitem_cast with this item.
    return Type;
}

quint16 CBlock::getIndex() const {
    return m_nCurrentInst;
}

QPolygonF CBlock::getPolygon() const {
    return m_PolyShape;
}

QPointF CBlock::getPosition() const {
    return this->pos();
}

