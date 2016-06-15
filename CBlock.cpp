/**
 * \file CBlock.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-2016 Thorsten Roth <elthoro@gmx.de>
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
 * Block handling (move, rotate, collision check, ...).
 */

#include <QDebug>

#include "./CBlock.h"

CBlock::CBlock(const quint16 nID, QPolygonF shape, QBrush bgcolor, QPen border,
               quint16 nGrid, QList<CBlock *> *pListBlocks, CSettings *pSettings,
               QPointF posTopLeft, const bool bBarrier)
    : m_nID(nID),
      m_PolyShape(shape),
      m_bgBrush(bgcolor),
      m_borderPen(border),
      m_nGrid(nGrid),
      m_pListBlocks(pListBlocks),
      m_pSettings(pSettings),
      m_bActive(false) {
    if (!m_PolyShape.isClosed()) {
        qWarning() << "Shape" << m_nID << "is not closed";
    }

    if (!bBarrier) {
        qDebug() << "Creating BLOCK" << m_nID <<
                    "\tPosition:" << posTopLeft * m_nGrid;
        this->setFlag(ItemIsMovable);
        m_CollTexture.load(":/images/collision_texture.png");
    } else {
        qDebug() << "Creating BARRIER" << m_nID <<
                    "\tPosition:" << posTopLeft * m_nGrid;

        this->setAcceptedMouseButtons(0);
        this->setAcceptTouchEvents(false);
        this->setEnabled(false);
    }

    m_pTransform = new QTransform();

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

    if (m_bActive) {  // Barries are ignored (not enabled)
        painter->setOpacity(0.4);
    } else {
        painter->setOpacity(1);
    }

    QPainterPath tmpPath;
    tmpPath.addPolygon(m_PolyShape);
    painter->fillPath(tmpPath, m_bgBrush);
    painter->setPen(m_borderPen);
    painter->drawPolygon(m_PolyShape);

    /*
    // Adding block ID for debugging
    m_ItemNumberText.setFont(QFont("Arial", 1));
    m_ItemNumberText.setText(QString::number(m_nID));
    m_ItemNumberText.setPos(0.2, -1.1);
    m_ItemNumberText.setParentItem(this);
    */
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::mousePressEvent(QGraphicsSceneMouseEvent *p_Event) {
    this->resetBrushStyle();

    qint8 nIndex(m_pSettings->getMouseControls().indexOf(p_Event->button()));
    if (nIndex >= 0) {
        switch (nIndex) {
        case 0:
            m_posMouseSelected = p_Event->pos();
            m_posMouseSelected = QPointF(m_posMouseSelected.x() * m_nGrid,
                                         m_posMouseSelected.y() * m_nGrid);
            this->moveBlock();
            update();
            break;
        case 1:
            this->rotateBlock();
            update();
            break;
        case 2:
            this->flipBlock();
            update();
            break;
        default:
            qWarning() << "Unexpected mouse press control:" << nIndex;
        }
    }

    QGraphicsItem::mousePressEvent(p_Event);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::mouseMoveEvent(QGraphicsSceneMouseEvent *p_Event) {
    qint8 nIndex(m_pSettings->getMouseControls().indexOf(p_Event->buttons()));
    if (0 == nIndex) {
        this->setPos(p_Event->scenePos() - m_posMouseSelected);
        update();
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::mouseReleaseEvent(QGraphicsSceneMouseEvent *p_Event) {
    qint8 nIndex(m_pSettings->getMouseControls().indexOf(p_Event->button()));
    if (0 == nIndex) {
        this->moveBlock(true);
        update();
    }

    QGraphicsItem::mouseReleaseEvent(p_Event);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::wheelEvent(QGraphicsSceneWheelEvent *p_Event) {
    this->resetBrushStyle();

    qint8 nIndex(m_pSettings->getMouseControls().indexOf(
                     (quint8(p_Event->orientation()) | m_pSettings->getShift())));
    if (nIndex >= 0) {
        switch (nIndex) {
        case 1:
            this->rotateBlock(p_Event->delta());
            update();
            break;
        case 2:
            this->flipBlock();
            update();
            break;
        default:
            qWarning() << "Unexpected mouse wheel control:" << nIndex;
        }
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::moveBlock(const bool bRelease) {
    if (!bRelease) {
        m_bActive = true;

        // Bring current block to foreground and update Z values
        for (int i = 0; i < m_pListBlocks->size(); i++) {
            (*m_pListBlocks)[i]->setNewZValue(-1);
        }
        this->setZValue(m_pListBlocks->size() + 2);

        m_posBlockSelected = this->pos();  // Save last position
    } else {
        m_bActive = false;

        this->prepareGeometryChange();
        this->setPos(this->snapToGrid(this->pos()));

        QPainterPath thisPath = this->shape();
        thisPath.translate(QPointF(this->pos().x() / m_nGrid,
                                   this->pos().y() / m_nGrid));

        emit incrementMoves();
        if (this->checkCollision(thisPath)) {
            // Reset position
            this->setPos(this->snapToGrid(m_posBlockSelected));
            this->checkBlockIntersection();
        } else {
            // Check if puzzle is solved
            emit checkPuzzleSolved();
        }
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::rotateBlock(const int nDelta) {
    qint8 nAngle = 0;
    qint32 nTranslateX = 0;
    qint32 nTranslateY = 0;
    if (nDelta < 0) {
        nAngle = 90;
        nTranslateX = this->boundingRect().height();
        nTranslateY = 0;
    } else {
        nAngle = -90;
        nTranslateX = 0;
        nTranslateY = this->boundingRect().width();
    }
    this->prepareGeometryChange();
    // qDebug() << "Before rot.:" << m_nID << nAngle << "\n" << m_PolyShape;
    m_pTransform->reset();
    m_pTransform->rotate(nAngle);
    m_PolyShape = m_pTransform->map(m_PolyShape);  // Rotate
    m_PolyShape.translate(nTranslateX, nTranslateY);  // Move back
    // qDebug() << "After rot.:" << m_PolyShape;

    this->checkBlockIntersection();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::flipBlock() {
    this->prepareGeometryChange();
    // qDebug() << "Before flip" << m_nID << "-" << m_PolyShape;
    QTransform transform = QTransform::fromScale(-1, 1);
    m_PolyShape = transform.map(m_PolyShape);  // Flip
    m_PolyShape.translate(this->boundingRect().width(), 0);  // Move back
    // qDebug() << "After flip:" << m_PolyShape;

    this->checkBlockIntersection();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::checkBlockIntersection() {
    QPainterPath thisPath = this->shape();
    thisPath.translate(QPointF(this->pos().x() / m_nGrid,
                               this->pos().y() / m_nGrid));

    if (this->checkCollision(thisPath)) {
        m_bgBrush.setTexture(m_CollTexture);
        m_bgBrush.setStyle(Qt::TexturePattern);
        for (int i = 0; i < m_pListBlocks->size(); i++) {
            (*m_pListBlocks)[i]->setNewZValue(-1);
        }
        this->setZValue(m_pListBlocks->size() + 2);
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBlock::resetBrushStyle() const {
    foreach (CBlock *block, *m_pListBlocks) {
        block->setBrushStyle(Qt::SolidPattern);
    }
}

void CBlock::setBrushStyle(Qt::BrushStyle style) {
    m_bgBrush.setStyle(style);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool CBlock::checkCollision(const QPainterPath thisPath) {
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
                /*
                qDebug() << "SHAPE 1:" << thisPath
                         << "SHAPE 2:" << collidingPath;
                qDebug() << "Col" << m_nID << "with" << block->getIndex()
                         << "Size" << intersectedPath.boundingRect().size();
                qDebug() << "Intersection:" << intersectedPath;
                */
                return true;
            }
        }
    }
    return false;
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

void CBlock::setNewZValue(const qint16 nZ) {
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

void CBlock::rescaleBlock(const quint16 nNewScale) {
    this->prepareGeometryChange();

    QPointF tmpTopLeft = this->pos() / m_nGrid * nNewScale;
    this->setScale(nNewScale);
    m_nGrid = nNewScale;
    this->setPos(this->snapToGrid(tmpTopLeft));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

int CBlock::type() const {
    // Enable the use of qgraphicsitem_cast with this item
    return Type;
}

quint16 CBlock::getIndex() const {
    return m_nID;
}

QPointF CBlock::getPosition() const {
    return this->pos();
}

QPolygonF CBlock::getPolygon() const {
    return this->m_PolyShape;
}
