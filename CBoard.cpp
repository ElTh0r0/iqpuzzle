/**
 * \file CBoard.cpp
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
 * Complete board generation, block setup and check if puzzle is solved.
 */

#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QMessageBox>

#include "./CBoard.h"

CBoard::CBoard(QGraphicsView *pGraphView, const QString &sBoardFile,
               CSettings *pSettings, const QString &sSavedGame)
  : m_pGraphView(pGraphView),
    m_sBoardFile(sBoardFile),
    m_pSettings(pSettings),
    m_bSavedGame(false) {
  this->setBackgroundBrush(QBrush(QColor("#EEEEEE")));

  m_pBoardConf = new QSettings(m_sBoardFile, QSettings::IniFormat);
  if (!sSavedGame.isEmpty()) {
    m_bSavedGame = true;
  }
  m_pSavedConf = new QSettings(sSavedGame, QSettings::IniFormat);

  this->setBackgroundBrush(QBrush(this->readColor("BGColor")));
  m_nGridSize = m_pBoardConf->value("GridSize", 25).toUInt();
  if (0 == m_nGridSize || m_nGridSize > 255) {
    qWarning() << "INVALID GRID SIZE:" << m_nGridSize;
    m_nGridSize = 25;
    QMessageBox::warning(0, trUtf8("Warning"),
                         trUtf8("Board grid size not valid.\n"
                                "Reduced grid to default."));
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool CBoard::setupBoard() {
  qDebug() << Q_FUNC_INFO;

  m_BoardPoly.clear();
  m_BoardPoly = this->readPolygon(m_pBoardConf, "Board/Polygon", true);
  if (m_BoardPoly.isEmpty()) {
    qWarning() << "BOARD POLYGON IS EMPTY!";
    QMessageBox::warning(0, trUtf8("Warning"),
                         trUtf8("Board polygon not valid."));
    return false;
  }

  // Draw board
  QPen pen(this->readColor("Board/BorderColor"));
  QBrush brush(this->readColor("Board/Color"));
  this->addPolygon(m_BoardPoly, pen, brush);
  m_pGraphView->setSceneRect(m_BoardPoly.boundingRect());

  // Draw grid
  QLineF lineGrid;
  pen.setColor(this->readColor("Board/GridColor"));
  // Horizontal
  for (int i = 1; i < m_BoardPoly.boundingRect().height()/m_nGridSize; i++) {
    lineGrid.setLine(1, i*m_nGridSize,
                     m_BoardPoly.boundingRect().width()-1, i*m_nGridSize);
    this->addLine(lineGrid, pen);
  }
  // Vertical
  for (int i = 1; i < m_BoardPoly.boundingRect().width()/m_nGridSize; i++) {
    lineGrid.setLine(i*m_nGridSize, 1, i*m_nGridSize,
                     m_BoardPoly.boundingRect().height()-1);
    this->addLine(lineGrid, pen);
  }

  // Set main window size
  const QSize WinSize(m_BoardPoly.boundingRect().width() * 2.5,
                      m_BoardPoly.boundingRect().height() * 2.6);
  emit setWindowSize(WinSize);
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::setupBlocks() {
  qDebug() << Q_FUNC_INFO;

  const unsigned char nMaxNumOfBlocks = 250;
  m_nNumOfBlocks = 0;
  QPolygonF polygon;
  QString sPrefix("");
  m_listBlocks.clear();
  QSettings *tmpSet = m_pBoardConf;
  if (m_bSavedGame) {
    tmpSet = m_pSavedConf;
  }

  // Get blocks
  for (unsigned int i = 1; i <= nMaxNumOfBlocks; i++) {
    sPrefix = "Block" + QString::number(i);
    if (!tmpSet->contains(sPrefix + "/Polygon")) {
      break;
    }
    m_nNumOfBlocks++;

    polygon = this->readPolygon(tmpSet, sPrefix + "/Polygon");
    if (polygon.isEmpty()) {
      this->clear();  // Clear all objects
      qWarning() << "POLYGON IS EMPTY FOR BLOCK" << i;
      QMessageBox::warning(0, trUtf8("Warning"),
                           trUtf8("Polygon not valid:") + "\n" + sPrefix);
      return;
    }

    // Create new block
    m_listBlocks.append(new CBlock(
                          i, polygon, this->readColor(sPrefix + "/Color"),
                          this->readColor(sPrefix + "/BorderColor"),
                          m_nGridSize, &m_listBlocks, m_pSettings,
                          this->readStartPosition(
                            tmpSet, sPrefix + "/StartPos")));
    connect(m_listBlocks.last(), SIGNAL(checkPuzzleSolved()),
            this, SLOT(checkPuzzleSolved()));
    connect(m_listBlocks.last(), SIGNAL(incrementMoves()),
            this, SIGNAL(incrementMoves()));
  }
  if (0 == m_nNumOfBlocks) {
    qWarning() << "NO VALID BLOCKS FOUND.";
    QMessageBox::warning(0, trUtf8("Warning"),
                         trUtf8("Could not find valid blocks."));
    return;
  }

  // Get barriers
  for (unsigned int i = 1; i <= nMaxNumOfBlocks; i++) {
    sPrefix = "Barrier" + QString::number(i);
    if (!m_pBoardConf->contains(sPrefix + "/Polygon")) {
      break;
    }

    polygon = this->readPolygon(m_pBoardConf, sPrefix + "/Polygon");
    if (polygon.isEmpty()) {
      this->clear();  // Clear all objects
      qWarning() << "POLYGON IS EMPTY FOR BARRIER" << i;
      QMessageBox::warning(0, trUtf8("Warning"),
                           trUtf8("Polygon not valid:") + "\n" + sPrefix);
      return;
    }

    // Create new barrier
    m_listBlocks.append(new CBlock(
                          m_nNumOfBlocks + i, polygon,
                          this->readColor(sPrefix + "/Color"),
                          this->readColor(sPrefix + "/BorderColor"),
                          m_nGridSize, &m_listBlocks, m_pSettings,
                          this->readStartPosition(m_pBoardConf,
                                                  sPrefix + "/StartPos"),
                          true));
  }

  // Add blocks to board
  foreach (CBlock *pB, m_listBlocks) {
    this->addItem(pB);
  }

  m_bNotAllPiecesNeeded = m_pBoardConf->value("NotAllPiecesNeeded",
                                              false).toBool();
  if (m_bNotAllPiecesNeeded) {
    QMessageBox::information(
          0, trUtf8("Hint"),
          trUtf8("Not all pieces are needed for a solution!"));
  }

  m_pGraphView->setEnabled(true);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QColor CBoard::readColor(const QString sKey) {
  QString sValue = m_pBoardConf->value(sKey, "").toString();
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

QPolygonF CBoard::readPolygon(const QSettings *tmpSet, const QString sKey,
                              bool bScale) {
  QStringList sList;
  QStringList sListPoint;
  QPolygonF polygon;
  QString sValue = tmpSet->value(sKey, "").toString();

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
      qWarning() << "Found invalid polygon point for" << sKey;
    }
  }

  if (!polygon.isClosed()) {
    qWarning() << "POLYGON NOT CLOSED:" << sKey;
    polygon.clear();
  }
  return polygon;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QPointF CBoard::readStartPosition(const QSettings *tmpSet, const QString sKey) {
  QStringList sList;
  QPointF point(4, -4);
  QString sValue = tmpSet->value(sKey, "").toString();

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

void CBoard::checkPuzzleSolved() {
  QPainterPath boardPath;
  QTransform transform;
  transform /= m_nGridSize;
  QPolygonF resizedPoly;
  resizedPoly = transform.map(m_BoardPoly);
  boardPath.addPolygon(resizedPoly);

  QPainterPath unitedBlocks;
  QPainterPath tempPath;
  QPointF pos;

  foreach (CBlock *block, m_listBlocks) {
    pos = QPointF(block->pos().x() / m_nGridSize,
                  block->pos().y() / m_nGridSize);

    // Check, if block is outside the board
    if (!m_bNotAllPiecesNeeded) {
      if (pos.x() < 0 || pos.y() < 0 ||
          pos.x() >= resizedPoly.boundingRect().width() ||
          pos.y() >= resizedPoly.boundingRect().height()) {
          return;
      }
    }

    tempPath = block->shape();
    tempPath.translate(pos);
    unitedBlocks += tempPath;

    // Check, if block intersects board
    if (tempPath.intersects(boardPath) && m_bNotAllPiecesNeeded) {
      tempPath = tempPath.subtracted(boardPath);
      if (!tempPath.isEmpty()) {
        // TODO: not all pieces needed and one piece touches board outline.
        return;
      }
    }
  }
  unitedBlocks = unitedBlocks.simplified();

  // qDebug() << "United blocks:" << unitedBlocks;
  // qDebug() << "Board:" << boardPath;

  tempPath = boardPath.subtracted(unitedBlocks);
  if (tempPath.isEmpty()) {
    m_pGraphView->setEnabled(false);
    emit solvedPuzzle();
  }
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
  QList<QGraphicsItem *> objList = this->items();

  // Remove objects from scene which are no blocks
  foreach (QGraphicsItem *gi, objList) {
    if (gi->type() != CBlock::Type) {
      this->removeItem(gi);
    }
  }

  // Rescale blocks
  foreach (CBlock *pB, m_listBlocks) {
    pB->rescaleBlock(m_nGridSize);
  }

  // Draw resized board again
  this->setupBoard();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::saveGame(const QString &sSaveFile, const QString &sTime,
                      const QString &sMoves) {
  QSettings saveConf(sSaveFile, QSettings::IniFormat);
  QByteArray ba;
  QString sPrefix("");
  QPolygonF poly;
  QString sPoly;
  QPointF pos;
  QString sDebug("");

  saveConf.clear();
  saveConf.setValue("BoardFile", m_sBoardFile);
  ba.append(sTime);
  saveConf.setValue("ElapsedTime", ba.toBase64());
  ba.clear();
  ba.append(sMoves);
  saveConf.setValue("NumOfMoves", ba.toBase64());

  for (int i = 0; i < m_nNumOfBlocks; i++) {
    sPrefix = "Block" + QString::number(i + 1);
    poly = m_listBlocks[i]->getPolygon();
    sPoly.clear();
    foreach (QPointF point, poly) {
      sPoly += QString::number(point.x()) + "," +
               QString::number(point.y()) + " | ";
    }
    sPoly.remove(sPoly.length() - 3, sPoly.length());

    saveConf.setValue(sPrefix + "/Polygon", sPoly);
    pos = m_listBlocks[i]->getPosition();
    saveConf.setValue(sPrefix + "/StartPos",
                      QString::number(pos.x() / m_nGridSize) + "," +
                      QString::number(pos.y() / m_nGridSize));

    if (sSaveFile.endsWith("S0LV3D.debug")) {
      sDebug += "[" + sPrefix + "]\n";
      sDebug += "Polygon=\"" + sPoly + "\"\n";
      sDebug += "StartPos=\"" + QString::number(pos.x() / m_nGridSize) +
                "," + QString::number(pos.y() / m_nGridSize) + "\"\n";
    }
  }

  if (sSaveFile.endsWith("S0LV3D.debug")) {
    ba.clear();
    ba.append(sDebug);
    qDebug().noquote() << "SOLVED\n" + m_sBoardFile + "\n" + ba.toBase64();
  }
}
