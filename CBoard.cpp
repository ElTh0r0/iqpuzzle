/**
 * \file CBoard.cpp
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

#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>
#include <QTime>

#include "./CBoard.h"

CBoard::CBoard(QGraphicsView *pGraphView, QGraphicsScene *pScene,
               const QString &sBoardFile)
    : m_pGraphView(pGraphView),
      m_pScene(pScene) {
    m_pConfig = new QSettings(sBoardFile, QSettings::IniFormat);

    m_pScene->setBackgroundBrush(QBrush(this->getColor("BGColor")));
    m_nGridSize = m_pConfig->value("GridSize", 25).toUInt();
    if (0 == m_nGridSize || m_nGridSize > 255) {
        qWarning() << "INVALID GRID SIZE:" << m_nGridSize;
        m_nGridSize = 25;
        QMessageBox::warning(0, trUtf8("Warning"),
                             trUtf8("Board grid size not valid.\n"
                                    "Reduced grid to default."));
    }

    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool CBoard::setupBoard() {
    qDebug() << Q_FUNC_INFO;

    m_BoardPoly.clear();
    m_BoardPoly = this->getPolygon("Board/Polygon", true);
    if (m_BoardPoly.isEmpty()) {
        qWarning() << "BOARD POLYGON IS EMPTY!";
        QMessageBox::warning(0, trUtf8("Warning"),
                             trUtf8("Board polygon not valid."));
        return false;
    }

    // Draw board
    QPen pen(this->getColor("Board/BorderColor"));
    QBrush brush(this->getColor("Board/Color"));
    m_pScene->addPolygon(m_BoardPoly, pen, brush);
    m_pGraphView->setSceneRect(m_BoardPoly.boundingRect());

    // Draw grid
    QLineF lineGrid;
    pen.setColor(this->getColor("Board/GridColor"));
    // Horizontal
    for (int i = 1; i < m_BoardPoly.boundingRect().height() / m_nGridSize; i++) {
        lineGrid.setLine(1, i*m_nGridSize,
                         m_BoardPoly.boundingRect().width()-1, i*m_nGridSize);
        m_pScene->addLine(lineGrid, pen);
    }
    // Vertical
    for (int i = 1; i < m_BoardPoly.boundingRect().width() / m_nGridSize; i++) {
        lineGrid.setLine(i*m_nGridSize, 1, i*m_nGridSize,
                         m_BoardPoly.boundingRect().height()-1);
        m_pScene->addLine(lineGrid, pen);
    }

    // Set main window (fixed) size
    const QSize WinSize(m_BoardPoly.boundingRect().width() * 2.5,
                        m_BoardPoly.boundingRect().height() * 2.5);
    emit setWindowSize(WinSize);
    return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::setupBlocks() {
    qDebug() << Q_FUNC_INFO;

    const unsigned char nMaxNumOfBlocks = 250;
    unsigned char nCount = 0;
    QPolygonF polygon;
    m_listBlocks.clear();

    // Get blocks
    for (unsigned int i = 1; i <= nMaxNumOfBlocks; i++) {
        if (!m_pConfig->contains("Block" + QString::number(i) + "/Polygon")) {
            break;
        }

        nCount++;
        m_pConfig->beginGroup("Block" + QString::number(i));

        polygon = this->getPolygon("Polygon");
        if (polygon.isEmpty()) {
            m_pScene->clear();  // Clear all objects
            qWarning() << "POLYGON IS EMPTY FOR BLOCK" << i;
            QMessageBox::warning(0, trUtf8("Warning"),
                                 trUtf8("Polygon not valid:") + i);
            return;
        }

        // Create new block
        m_listBlocks.append(new CBlock(i, polygon, this->getColor("Color"),
                                       this->getColor("BorderColor"),
                                       m_nGridSize, &m_listBlocks,
                                       this->getStartPosition("StartPos")));
        m_pConfig->endGroup();
    }
    if (0 == nCount) {
        qWarning() << "NO VALID BLOCKS FOUND.";
        QMessageBox::warning(0, trUtf8("Warning"),
                             trUtf8("Could not find valid blocks."));
        return;
    }

    // Random start block
    nCount = qrand() % (nCount-1 + 1);
    qDebug() << "Start BLOCK:" << nCount;
    if (nCount < m_listBlocks.size()) {
        m_listBlocks[nCount]->moveBlockGrid(QPointF(0, 0));
        // TODO: Set random rotation
    } else {
        qWarning() << "Generated invalid start block:" << nCount;
    }

    // Add blocks to board
    foreach (CBlock *pB, m_listBlocks) {
        m_pScene->addItem(pB);
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QColor CBoard::getColor(const QString sKey) {
    QString sValue = m_pConfig->value(sKey, "").toString();
    QColor color("#FF00FF");

    if (sValue.isEmpty()) {
        sValue = "#00FFFF";
        qWarning() << "Set fallback color for key" << sKey;
    }
    color.setNamedColor(sValue);
    if (!color.isValid()) {
        color.setNamedColor("#FF00FF");
        qWarning() << "Found invalid color for key" << sKey;
    }
    return color;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QPolygonF CBoard::getPolygon(const QString sKey, bool bScale) {
    QStringList sList;
    QStringList sListPoint;
    QPolygonF polygon;
    QString sValue = m_pConfig->value(sKey, "").toString();

    quint16 nScale = 1;
    if (bScale) {
        nScale = m_nGridSize;
    }

    sList << sValue.split("|");
    foreach (QString s, sList) {
        sListPoint.clear();
        sListPoint << s.split(",");
        if (2 == sListPoint.size()) {
            polygon << QPointF(sListPoint[0].trimmed().toShort() * nScale,
                               sListPoint[1].trimmed().toShort() * nScale);
        } else {
            qWarning() << "Found invalid polygon point for key" << sKey;
        }
    }

    if (!polygon.isClosed()) {
        qWarning() << "BOARD POLYGON NOT CLOSED!";
        polygon.clear();
    }
    return polygon;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QPointF CBoard::getStartPosition(const QString sKey) {
    QStringList sList;
    QPointF point(4, -4);
    QString sValue = m_pConfig->value(sKey, "").toString();

    if (sValue.count(',') != 1) {
        sValue = "-4,-4";
        qWarning() << "Set fallback start position for key" << sKey;
    }
    sList << sValue.split(",");
    if (2 == sList.size()) {
        point.setX(sList[0].trimmed().toInt());
        point.setY(sList[1].trimmed().toInt());
    } else {
        qWarning() << "Found invalid start point for key" << sKey;
    }
    return point;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::zoomIn() {
    if (m_nGridSize <= 250) {
        m_nGridSize += 5;
    } else {
        m_nGridSize = 255;
    }
    this->doZoom();
}

void CBoard::zoomOut() {
    if (m_nGridSize > 9) {
        m_nGridSize -= 5;
    } else {
        m_nGridSize = 5;
    }
    this->doZoom();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::doZoom() {
    qDebug() << Q_FUNC_INFO << "Grid: " << m_nGridSize;

    // Get all QGraphicItems in scene
    QList<QGraphicsItem *> objList = m_pScene->items();

    // Remove objects from scene which are no blocks
    foreach (QGraphicsItem* gi, objList) {
        if (gi->type() != CBlock::Type) {
            m_pScene->removeItem(gi);
        }
    }

    // Rescale blocks
    foreach (CBlock *pB, m_listBlocks) {
        pB->rescaleBlock(m_nGridSize);
    }

    // Draw resized board again
    this->setupBoard();
}
