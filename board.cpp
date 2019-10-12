/**
 * \file board.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-2019 Thorsten Roth <elthoro@gmx.de>
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

#include "./board.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMessageBox>

Board::Board(QGraphicsView *pGraphView, const QString &sBoardFile,
             Settings *pSettings, const quint16 nGridSize,
             const QString &sSavedGame)
  : m_pGraphView(pGraphView),
    m_sBoardFile(sBoardFile),
    m_pSettings(pSettings),
    m_bSavedGame(false),
    m_nGridSize(nGridSize) {
  this->setBackgroundBrush(QBrush(QColor(238, 238, 238)));

  m_pBoardConf = new QSettings(m_sBoardFile, QSettings::IniFormat);
  if (!sSavedGame.isEmpty()) {
    m_bSavedGame = true;
  }
  m_pSavedConf = new QSettings(sSavedGame, QSettings::IniFormat);

  this->setBackgroundBrush(QBrush(this->readColor(QStringLiteral("BGColor"))));
  if (0 == m_nGridSize) {
    m_nGridSize = static_cast<quint16>(
                    m_pBoardConf->value(
                      QStringLiteral("GridSize"), 0).toUInt());
  }
  if (0 == m_nGridSize || m_nGridSize > 255) {
    qWarning() << "INVALID GRID SIZE:" << m_nGridSize;
    m_nGridSize = 25;
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("Board grid size not valid.\n"
                            "Reduced grid to default."));
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool Board::setupBoard() {
  qDebug() << Q_FUNC_INFO;
  m_bFreestyle = m_pBoardConf->value(QStringLiteral("Freestyle"),
                                     false).toBool();

  m_BoardPoly.clear();
  m_BoardPoly = this->readPolygon(m_pBoardConf,
                                  QStringLiteral("Board/Polygon"), true);
  if (m_BoardPoly.isEmpty()) {
    qWarning() << "BOARD POLYGON IS EMPTY!";
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("Board polygon not valid."));
    return false;
  }

  if (!m_bFreestyle) {
    this->drawBoard();
    this->drawGrid();
    m_pGraphView->setDragMode(QGraphicsView::NoDrag);
    m_pGraphView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pGraphView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  } else {
    m_pGraphView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_pGraphView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pGraphView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pGraphView->setSceneRect(QRect(-1000, -1000, 3000, 3000));
  }

  // Set main window size
  const QSize WinSize(
        static_cast<int>(m_BoardPoly.boundingRect().width() * 2.5),
        static_cast<int>(m_BoardPoly.boundingRect().height() * 2.6));
  emit setWindowSize(WinSize, m_bFreestyle);
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::drawBoard() {
  QPen pen(this->readColor(QStringLiteral("Board/BorderColor")));
  QBrush brush(this->readColor(QStringLiteral("Board/Color")));
  this->addPolygon(m_BoardPoly, pen, brush);
  m_pGraphView->setSceneRect(m_BoardPoly.boundingRect());
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::drawGrid() {
  QLineF lineGrid;
  QPen pen(this->readColor(QStringLiteral("Board/GridColor")));

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
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool Board::setupBlocks() {
  qDebug() << Q_FUNC_INFO;
  m_nNumOfBlocks = 0;
  m_listBlocks.clear();

  if (this->createBlocks() &&
      this->createBarriers()) {
    // Add blocks to board
    foreach (Block *pB, m_listBlocks) {
      this->addItem(pB);
    }

    m_bNotAllPiecesNeeded = m_pBoardConf->value(
                              QStringLiteral("NotAllPiecesNeeded"),
                              false).toBool();
    if (m_bNotAllPiecesNeeded) {
      QMessageBox::information(nullptr, tr("Hint"),
                               tr("Not all pieces are needed for a solution!"));
    }

    m_pGraphView->setEnabled(true);
  }
  return m_bFreestyle;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool Board::createBlocks() {
  const unsigned char nMaxNumOfBlocks(250);
  QSettings *tmpSet = m_pBoardConf;
  if (m_bSavedGame) {
    tmpSet = m_pSavedConf;
  }

  for (quint16 i = 1; i <= nMaxNumOfBlocks; i++) {
    QString sPrefix = "Block" + QString::number(i);
    if (!tmpSet->contains(sPrefix + "/Polygon")) {
      break;
    }
    m_nNumOfBlocks++;

    QPolygonF polygon = this->readPolygon(tmpSet, sPrefix + "/Polygon");
    if (polygon.isEmpty()) {
      this->clear();  // Clear all objects
      qWarning() << "POLYGON IS EMPTY FOR BLOCK" << i;
      QMessageBox::warning(nullptr, tr("Warning"),
                           tr("Polygon not valid:") + "\n" + sPrefix);
      return false;
    }

    // Create new block
    m_listBlocks.append(new Block(
                          i, polygon, this->readColor(sPrefix + "/Color"),
                          this->readColor(sPrefix + "/BorderColor"),
                          m_nGridSize, &m_listBlocks, m_pSettings,
                          this->readStartPosition(
                            tmpSet, sPrefix + "/StartPos")));
    if (!m_bFreestyle) {
      connect(m_listBlocks.last(), &Block::checkPuzzleSolved,
              this, &Board::checkPuzzleSolved);
      connect(m_listBlocks.last(), &Block::incrementMoves,
              this, &Board::incrementMoves);
    }
  }

  if (0 == m_nNumOfBlocks) {
    qWarning() << "NO VALID BLOCKS FOUND.";
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("Could not find valid blocks."));
    return false;
  }

  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool Board::createBarriers() {
  const unsigned char nMaxNumOfBlocks(250);

  for (quint16 i = 1; i <= nMaxNumOfBlocks; i++) {
    QString sPrefix = "Barrier" + QString::number(i);
    if (!m_pBoardConf->contains(sPrefix + "/Polygon")) {
      break;
    }

    QPolygonF polygon = this->readPolygon(m_pBoardConf, sPrefix + "/Polygon");
    if (polygon.isEmpty()) {
      this->clear();  // Clear all objects
      qWarning() << "POLYGON IS EMPTY FOR BARRIER" << i;
      QMessageBox::warning(nullptr, tr("Warning"),
                           tr("Polygon not valid:") + "\n" + sPrefix);
      return false;
    }

    // Create new barrier
    m_listBlocks.append(new Block(
                          m_nNumOfBlocks + i, polygon,
                          this->readColor(sPrefix + "/Color"),
                          this->readColor(sPrefix + "/BorderColor"),
                          m_nGridSize, &m_listBlocks, m_pSettings,
                          this->readStartPosition(m_pBoardConf,
                                                  sPrefix + "/StartPos"),
                          true));
  }

  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QColor Board::readColor(const QString &sKey) const {
  QString sValue = m_pBoardConf->value(sKey, "").toString();
  QColor color(255, 0, 255);

  if (sValue.isEmpty()) {
    sValue = QStringLiteral("#00FFFF");
    qWarning() << "Set fallback color for key" << sKey;
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("No color defined - using fallback:") +
                         "\n" + sKey);
  }
  color.setNamedColor(sValue);
  if (!color.isValid()) {
    color = QColor(255, 0, 255);
    qWarning() << "Found invalid color for key" << sKey;
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("Invalid color defined - using fallback:") +
                         "\n" + sKey);
  }
  return color;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QPolygonF Board::readPolygon(const QSettings *tmpSet, const QString &sKey,
                             const bool bScale) {
  QStringList sList;
  QStringList sListPoint;
  QPolygonF polygon;
  QString sValue(tmpSet->value(sKey, "").toString());
  quint16 nScale(1);
  if (bScale) {
    nScale = m_nGridSize;
  }

  this->checkOrthogonality(QPointF(-99999, -99999));
  sList << sValue.split('|');
  foreach (QString s, sList) {
    sListPoint.clear();
    sListPoint << s.split(',');
    if (2 == sListPoint.size()) {
      polygon << QPointF(sListPoint[0].trimmed().toShort() * nScale,
          sListPoint[1].trimmed().toShort() * nScale);

      if (!this->checkOrthogonality(polygon.last())) {
        qWarning() << "Polygon not orthogonal" << sKey;
        polygon.clear();
        break;
      }
    } else {
      qWarning() << "Found invalid polygon point for" << sKey;
      polygon.clear();
      break;
    }
  }

  if (!polygon.isClosed()) {
    qWarning() << "Polygon not closed:" << sKey;
    polygon.clear();
  }
  return polygon;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool Board::checkOrthogonality(QPointF point) const {
  static QList<QPointF> listPoints;
  static quint16 nCnt;

  if (QPointF(-99999, -99999) == point) {  // Reset
    listPoints.clear();
    nCnt = 0;
    return true;
  } else {
    nCnt++;
    listPoints.push_back(point);
    if (listPoints.size() > 2) {
      if ((listPoints[0].x() == listPoints[1].x() &&
          listPoints[1].y() == listPoints[2].y()) ||
          (listPoints[0].y() == listPoints[1].y() &&
           listPoints[1].x() == listPoints[2].x())) {
        listPoints.removeFirst();
        return true;
      }
    } else {  // If size <= 2
      return true;
    }
  }

  qWarning() << "Wrong point #" << nCnt;
  return false;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QPointF Board::readStartPosition(const QSettings *tmpSet,
                                 const QString &sKey) const {
  QStringList sList;
  QPointF point(1, -1);
  QString sValue = tmpSet->value(sKey, "").toString();
  bool bOk1(true);
  bool bOk2(true);

  if (sValue.count(',') != 1) {
    sValue = QStringLiteral("-1,-1");
    bOk1 = false;
  }

  sList << sValue.split(',');
  if (2 == sList.size() && bOk1) {
    point.setX(sList[0].trimmed().toInt(&bOk1, 10));
    point.setY(sList[1].trimmed().toInt(&bOk2, 10));
  } else {
    bOk1 = false;
  }

  if (!bOk1 || !bOk2) {
    qWarning() << "Found invalid start point for key" << sKey;
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("Invalid start position - using fallback:") +
                         "\n" + sKey);
  }
  return point;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::checkPuzzleSolved() {
  QPainterPath boardPath;
  QTransform transform;
  transform /= m_nGridSize;
  QPolygonF resizedPoly;
  resizedPoly = transform.map(m_BoardPoly);
  boardPath.addPolygon(resizedPoly);

  QPainterPath unitedBlocks;
  QPainterPath tempPath;
  QPainterPath tempPath2;

  foreach (Block *block, m_listBlocks) {
    QPointF pos = QPointF(block->pos().x() / m_nGridSize,
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
    tempPath2 = tempPath;
    unitedBlocks += tempPath;

    // Check, if block intersects board
    if (tempPath.intersects(boardPath) && m_bNotAllPiecesNeeded) {
      tempPath = tempPath.subtracted(boardPath);
      // Intersection
      if (!tempPath.isEmpty() &&
      // Block touches board outline, but completly outside
          tempPath != tempPath2) {
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

void Board::zoomIn() {
  if (m_nGridSize <= 250) {
    m_nGridSize += 5;
  } else {
    m_nGridSize = 255;
  }
  this->doZoom();
}

void Board::zoomOut() {
  if (m_nGridSize > 9) {
    m_nGridSize -= 5;
  } else {
    m_nGridSize = 5;
  }
  this->doZoom();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::doZoom() {
  qDebug() << Q_FUNC_INFO << "Grid: " << m_nGridSize;

  // Get all QGraphicItems in scene
  QList<QGraphicsItem *> objList = this->items();

  // Remove objects from scene which are no blocks
  foreach (QGraphicsItem *gi, objList) {
    if (gi->type() != Block::Type) {
      this->removeItem(gi);
    }
  }

  // Draw resized board again
  this->setupBoard();

  // Rescale blocks
  foreach (Block *pB, m_listBlocks) {
    pB->rescaleBlock(m_nGridSize);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

quint16 Board::getGridSize() const {
  return m_nGridSize;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::saveGame(const QString &sSaveFile, const QString &sTime,
                     const QString &sMoves) {
  QSettings saveConf(sSaveFile, QSettings::IniFormat);
  QByteArray ba;
  QString sDebug;

  // Save relative board folder, needed e.g. for AppImage
  QDir dir(qApp->applicationDirPath());
  QString sRelativeDir;
  sRelativeDir = dir.relativeFilePath(m_sBoardFile);

  saveConf.clear();
  saveConf.setValue(QStringLiteral("BoardFile"), m_sBoardFile);
  saveConf.setValue(QStringLiteral("BoardFileRelative"), sRelativeDir);
  ba.append(sTime);
  saveConf.setValue(QStringLiteral("ElapsedTime"), ba.toBase64());
  ba.clear();
  ba.append(sMoves);
  saveConf.setValue(QStringLiteral("NumOfMoves"), ba.toBase64());

  for (int i = 0; i < m_nNumOfBlocks; i++) {
    QString sPrefix = "Block" + QString::number(i + 1);
    QPolygonF poly = m_listBlocks.at(i)->getPolygon();
    QString sPoly("");
    foreach (QPointF point, poly) {
      sPoly += QString::number(point.x()) + "," +
               QString::number(point.y()) + " | ";
    }
    sPoly.remove(sPoly.length() - 3, sPoly.length());

    saveConf.setValue(sPrefix + "/Polygon", sPoly);
    QPointF pos = m_listBlocks.at(i)->getPosition();
    saveConf.setValue(sPrefix + "/StartPos",
                      QString::number(pos.x() / m_nGridSize) + "," +
                      QString::number(pos.y() / m_nGridSize));

    if (sSaveFile.endsWith(QStringLiteral("S0LV3D.debug"))) {
      sDebug = "[" + sPrefix + "]\n";
      sDebug += "Polygon=\"" + sPoly + "\"\n";
      sDebug += "StartPos=\"" + QString::number(pos.x() / m_nGridSize) +
                "," + QString::number(pos.y() / m_nGridSize) + "\"\n";
    }
  }

  if (sSaveFile.endsWith(QStringLiteral("S0LV3D.debug"))) {
    ba.clear();
    ba.append(sDebug);
    qDebug() << "SOLVED" << m_sBoardFile;
    qDebug() << ba.toBase64();
  }
}
