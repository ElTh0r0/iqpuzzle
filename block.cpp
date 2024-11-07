/**
 * \file block.cpp
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
 * Block handling (move, rotate, collision check, ...).
 */

#include "./block.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QPainter>

#include "./settings.h"

Block::Block(const quint16 nID, QPolygonF shape, const QBrush &bgcolor,
             QPen border, quint16 nGrid, QList<Block *> *pListBlocks,
             Settings *pSettings, QPointF posTopLeft, const bool bBarrier,
             QObject *pParentObj)
    : m_nID(nID),
      m_nZBlock(1000),
      m_nZBarrier(1),
      m_bBarrier(bBarrier),
      m_PolyShape(std::move(shape)),
      m_bgBrush(bgcolor),
      m_borderPen(std::move(border)),
      m_nGrid(nGrid),
      m_pListBlocks(pListBlocks),
      m_pSettings(pSettings),
      m_bActive(false) {
  Q_UNUSED(pParentObj)
  if (!m_PolyShape.isClosed()) {
    qWarning() << "Shape" << m_nID << "is not closed";
  }

  if (!m_bBarrier) {
    // qDebug() << "Creating BLOCK" << m_nID <<
    //             "\tPosition:" << posTopLeft * m_nGrid;
    this->setFlag(ItemIsMovable);
    m_CollTexture.load(QStringLiteral(":/collision_texture.png"));
    this->setZValue(m_nZBlock);
  } else {
    // qDebug() << "Creating BARRIER" << m_nID <<
    //             "\tPosition:" << posTopLeft * m_nGrid;
    this->setAcceptedMouseButtons(Qt::NoButton);
    this->setAcceptTouchEvents(false);
    this->setEnabled(false);
    this->setZValue(m_nZBarrier);
  }

  m_pTransform = new QTransform();

  // Scale object
  this->setScale(m_nGrid);
  // Move to start position
  this->moveBlockGrid(posTopLeft);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Block::boundingRect() const -> QRectF {
  return m_PolyShape.boundingRect();
}

auto Block::shape() const -> QPainterPath {
  QPainterPath path;
  path.addPolygon(m_PolyShape);
  return path;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Block::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *widget) {
  Q_UNUSED(option)
  Q_UNUSED(widget)
  static const qreal OPACITY = 0.4;

  m_borderPen.setWidth(1 / m_nGrid);

  if (m_bActive) {  // Barriers are ignored (not enabled)
    painter->setOpacity(OPACITY);
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

void Block::mousePressEvent(QGraphicsSceneMouseEvent *p_Event) {
  quint32 button = p_Event->button();
  this->resetBrushStyle();
  if (button == Qt::LeftButton) button |= p_Event->modifiers();

  int nIndex(m_pSettings->getMouseControls().indexOf(button));
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

void Block::mouseMoveEvent(QGraphicsSceneMouseEvent *p_Event) {
  if (0 == m_pSettings->getMouseControls().indexOf(p_Event->buttons())) {
    this->setPos(p_Event->scenePos() - m_posMouseSelected);
    update();
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Block::mouseReleaseEvent(QGraphicsSceneMouseEvent *p_Event) {
  if (0 == m_pSettings->getMouseControls().indexOf(p_Event->button())) {
    this->moveBlock(true);
    update();
  }

  QGraphicsItem::mouseReleaseEvent(p_Event);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Block::wheelEvent(QGraphicsSceneWheelEvent *p_Event) {
  this->resetBrushStyle();

  int nIndex(m_pSettings->getMouseControls().indexOf(
      (quint32(p_Event->orientation()) | Settings::nSHIFT)));
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

void Block::moveBlock(const bool bRelease) {
  if (!bRelease) {
    m_bActive = true;
    this->bringToForeground();
    m_posBlockSelected = this->pos();   // Save last position
    m_PolyShapeSelected = m_PolyShape;  // Save shape
  } else {
    m_bActive = false;

    this->prepareGeometryChange();
    this->setPos(this->snapToGrid(this->pos()));

    QPainterPath thisPath = this->shape();
    thisPath.translate(
        QPointF(this->pos().x() / m_nGrid, this->pos().y() / m_nGrid));

    emit incrementMoves();
    if (this->checkCollision(thisPath)) {
      m_PolyShape = m_PolyShapeSelected;  // Reset shape (flip / rotation)
      this->setPos(this->snapToGrid(m_posBlockSelected));  // Reset position
      this->checkBlockIntersection();
    } else {
      // Check if puzzle is solved
      emit checkPuzzleSolved();
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Block::rotateBlock(const int nDelta) {
  static const quint8 RIGHTANGLE = 90;
  if (m_bActive || !this->isAnyBlockActive(m_pListBlocks)) {
    qint8 nAngle(0);
    qreal nTranslateX(0);
    qreal nTranslateY(0);

    if (nDelta < 0) {
      nAngle = RIGHTANGLE;
      nTranslateX = this->boundingRect().height();
      nTranslateY = 0;
    } else {
      nAngle = -RIGHTANGLE;
      nTranslateX = 0;
      nTranslateY = this->boundingRect().width();
    }
    this->prepareGeometryChange();
    // qDebug() << "Before rot.:" << m_nID << nAngle << "\n" << m_PolyShape;
    m_pTransform->reset();
    m_pTransform->rotate(nAngle);
    m_PolyShape = m_pTransform->map(m_PolyShape);     // Rotate
    m_PolyShape.translate(nTranslateX, nTranslateY);  // Move back
    // qDebug() << "After rot.:" << m_PolyShape;
  }

  if (!this->isAnyBlockActive(m_pListBlocks)) {
    this->checkBlockIntersection();
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Block::flipBlock() {
  if (m_bActive || !this->isAnyBlockActive(m_pListBlocks)) {
    this->prepareGeometryChange();
    // qDebug() << "Before flip" << m_nID << "-" << m_PolyShape;
    QTransform transform = QTransform::fromScale(-1, 1);
    m_PolyShape = transform.map(m_PolyShape);                // Flip
    m_PolyShape.translate(this->boundingRect().width(), 0);  // Move back
    // qDebug() << "After flip:" << m_PolyShape;
  }

  if (!this->isAnyBlockActive(m_pListBlocks)) {
    this->checkBlockIntersection();
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Block::checkBlockIntersection() {
  QPainterPath thisPath = this->shape();
  thisPath.translate(
      QPointF(this->pos().x() / m_nGrid, this->pos().y() / m_nGrid));

  if (this->checkCollision(thisPath)) {
    m_bgBrush.setTexture(m_CollTexture);
    m_bgBrush.setStyle(Qt::TexturePattern);
    this->bringToForeground();
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Block::resetBrushStyle() const {
  for (auto &pBlock : *m_pListBlocks) {
    pBlock->setBrushStyle(Qt::SolidPattern);
  }
}

void Block::setBrushStyle(Qt::BrushStyle style) { m_bgBrush.setStyle(style); }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Block::checkCollision(const QPainterPath &thisPath) -> bool {
  QPainterPath collidingPath;
  QPainterPath intersectedPath;
  for (auto &pBlock : *m_pListBlocks) {
    if (pBlock->getIndex() != m_nID && this->collidesWithItem(pBlock)) {
      collidingPath = pBlock->shape();
      collidingPath.translate(
          QPointF(pBlock->pos().x() / m_nGrid, pBlock->pos().y() / m_nGrid));
      intersectedPath = thisPath.intersected(collidingPath);

      // Path has to be simplified
      // Otherwise paths with area = 0 might be found
      intersectedPath = intersectedPath.simplified();

      if (!intersectedPath.boundingRect().size().isEmpty()) {
        /*
        qDebug() << "SHAPE 1:" << thisPath << "SHAPE 2:" << collidingPath;
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

auto Block::snapToGrid(const QPointF point) const -> QPointF {
  return QPointF(qRound(point.x() / m_nGrid) * m_nGrid,   // x
                 qRound(point.y() / m_nGrid) * m_nGrid);  // y
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Block::moveBlockGrid(const QPointF pos) { this->setPos(pos * m_nGrid); }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Block::bringToForeground() {
  if (!m_bBarrier) {
    for (auto &pBlock : *m_pListBlocks) {
      if (pBlock->isBarrier()) {
        pBlock->setZValue(m_nZBarrier);
      } else {
        if (pBlock->getIndex() == m_nID) {
          this->setZValue(m_nZBlock + 1);
        } else {
          pBlock->setZValue(m_nZBlock);
        }
      }
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Block::rescaleBlock(const quint16 nNewScale) {
  this->prepareGeometryChange();

  QPointF tmpTopLeft = this->pos() / m_nGrid * nNewScale;
  this->setScale(nNewScale);
  m_nGrid = nNewScale;
  this->setPos(this->snapToGrid(tmpTopLeft));

  // Z value ++, otherwise board might be in foreground until block is moved
  this->setZValue(this->zValue() + 1);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Block::isAnyBlockActive(const QList<Block *> *listBlocks) -> bool {
  for (const auto pBlock : *listBlocks) {
    if (pBlock->isActive()) {
      return true;
    }
  }
  return false;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Block::type() const -> int {
  // Enable the use of qgraphicsitem_cast with this item
  return Type;
}

auto Block::getIndex() const -> quint16 { return m_nID; }

auto Block::isBarrier() const -> bool { return m_bBarrier; }

auto Block::isActive() const -> bool { return m_bActive; }

auto Block::getPosition() const -> QPointF { return this->pos(); }

auto Block::getPolygon() const -> QPolygonF { return this->m_PolyShape; }
