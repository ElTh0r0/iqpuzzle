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

#include <QDebug>

#include "./CBlock.h"

extern bool bDEBUG;

CBlock::CBlock(const quint16 nID, QPolygonF shape, QBrush bgcolor, QPen border,
               quint16 nGrid, QList<CBlock *> *pListBlocks, QPointF posTopLeft)
    : m_nID(nID),
      m_PolyShape(shape),
      m_bgBrush(bgcolor),
      m_borderPen(border),
      m_nGrid(nGrid),
      m_pListBlocks(pListBlocks),
      m_bMousePressed(false) {
    if (bDEBUG) {
        qDebug() << "Creating BLOCK" << m_nID <<
                    "\tPosition:" << posTopLeft * m_nGrid;
    }

    if (!m_PolyShape.isClosed()) {
        qWarning() << "Shape" << m_nID << "is not closed";
    }

    m_pTransform = new QTransform();
    this->setFlag(ItemIsMovable);

    // Scale object
    this->setScale(m_nGrid);
    // Move to start position
    this->moveBlockGrid(posTopLeft);
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

    m_borderPen.setWidth(1/m_nGrid);

    if (m_bMousePressed) {
        painter->setOpacity(0.4);
    } else {
        painter->setOpacity(1);
    }

    QPainterPath tmpPath;
    tmpPath.addPolygon(m_PolyShape);
    painter->fillPath(tmpPath, m_bgBrush);
    painter->setPen(m_borderPen);
    painter->drawPolygon(m_PolyShape);

    // Adding block ID for debugging
    if (bDEBUG) {
        m_ItemNumberText.setFont(QFont("Arial", 1));
        m_ItemNumberText.setText(QString::number(m_nID));
        m_ItemNumberText.setPos(0.2, -1.1);
        m_ItemNumberText.setParentItem(this);
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::mousePressEvent(QGraphicsSceneMouseEvent *p_Event) {
    if (p_Event->button() == Qt::LeftButton) {  // Move
        m_bMousePressed = true;

        // Bring current block to foreground and update Z values
        for (int i = 0; i < m_pListBlocks->size(); i++) {
            (*m_pListBlocks)[i]->setNewZValue(-1);
        }
        this->setZValue(m_pListBlocks->size() + 2);

        m_posBlockSelected = this->pos();  // Save last position
    } else if (p_Event->button() == Qt::RightButton) {  // Flip
        this->prepareGeometryChange();
        // qDebug() << "Before flip:" << m_PolyShape;
        QTransform transform = QTransform::fromScale(-1, 1);
        m_PolyShape = transform.map(m_PolyShape);
        // qDebug() << "After flip:" << m_PolyShape;

        if (bDEBUG) {
            qDebug() << "Flip BLOCK" << m_nID;
        }
    }

    update();
    QGraphicsItem::mousePressEvent(p_Event);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::wheelEvent(QGraphicsSceneWheelEvent *p_Event) {
    qint8 nAngle = 0;
    if (p_Event->orientation() == Qt::Vertical) {  // Vertical mouse wheel
        if (p_Event->delta() < 0) {
            nAngle = 90;
        } else {
            nAngle = -90;
        }
        this->prepareGeometryChange();
        // qDebug() << "Before rotation:" << m_PolyShape;
        m_pTransform->reset();
        m_pTransform->rotate(nAngle);
        m_PolyShape = m_pTransform->map(m_PolyShape);
        // qDebug() << "After rotation:" << m_PolyShape;

        if (bDEBUG) {
            qDebug() << "Rotate BLOCK" << m_nID << " " << nAngle;
        }
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::mouseReleaseEvent(QGraphicsSceneMouseEvent *p_Event) {
    // After moving a block, check for collision
    if (p_Event->button() == Qt::LeftButton) {
        m_bMousePressed = false;
        this->prepareGeometryChange();

        this->setPos(this->snapToGrid(this->pos()));

        QPainterPath thisPath = this->shape();
        thisPath.translate(QPointF(this->pos().x() / m_nGrid,
                                  this->pos().y() / m_nGrid));
        QPainterPath collidingPath;
        QPainterPath intersectedPath;
        foreach (CBlock *block, *m_pListBlocks) {
            if (block->getIndex() != m_nID
                    && this->collidesWithItem(block)) {
                collidingPath = block->shape();
                collidingPath.translate(QPointF(block->pos().x() / m_nGrid,
                                                block->pos().y() / m_nGrid));
                intersectedPath = thisPath.intersected(collidingPath);

                // Path has to be simplified
                // Otherwise paths with area = 0 might be found
                intersectedPath = intersectedPath.simplified();

                if (!intersectedPath.boundingRect().size().isEmpty()) {
                    // Reset position
                    this->setPos(this->snapToGrid(m_posBlockSelected));

                    if (bDEBUG) {
                        qDebug() << "SHAPE 1:" << thisPath
                                 << "SHAPE 2:" << collidingPath;
                        qDebug() << "Col" << m_nID << "with" << block->getIndex()
                                 << "Size" << intersectedPath.boundingRect().size();
                        qDebug() << "Intersection:" << intersectedPath;
                    }
                }
            }
        }
    }

    update();
    QGraphicsItem::mouseReleaseEvent(p_Event);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QPointF CBlock::snapToGrid(const QPointF point) const {
    int x = qRound(point.x() / m_nGrid) * m_nGrid;
    int y = qRound(point.y() / m_nGrid) * m_nGrid;
    return QPointF(x, y);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::moveBlockGrid(const QPointF pos) {
    this->setPos(pos * m_nGrid);
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

    QPointF tmpTopLeft = this->pos() / m_nGrid * nNewScale;
    this->setScale(nNewScale);
    m_nGrid = nNewScale;
    this->setPos(this->snapToGrid(tmpTopLeft));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

int CBlock::type() const {
    // Enable the use of qgraphicsitem_cast with this item.
    return Type;
}

quint16 CBlock::getIndex() const {
    return m_nID;
}

QPointF CBlock::getPosition() const {
    return this->pos();
}
