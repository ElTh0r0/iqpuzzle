/**
 * \file board.cpp
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
 * Complete board generation, block setup and check if puzzle is solved.
 */

#include "./board.h"

#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QGraphicsView>
#include <QMessageBox>
#include <QSettings>

#include "./block.h"
#include "./settings.h"

Board::Board(QWidget *pParent, QGraphicsView *pGraphView, QString sBoardFile,
             Settings *pSettings, const quint16 nGridSize,
             const QString &sSavedGame, QObject *pParentObj)
    : m_pParent(pParent),
      m_pGraphView(pGraphView),
      m_sBoardFile(std::move(sBoardFile)),
      m_pSettings(pSettings),
      m_bSavedGame(false),
      m_nGridSize(nGridSize) {
  Q_UNUSED(pParentObj)
  m_pBoardConf = new QSettings(m_sBoardFile, QSettings::IniFormat);
  if (!sSavedGame.isEmpty()) {
    m_bSavedGame = true;
  }
  m_pSavedConf = new QSettings(sSavedGame, QSettings::IniFormat);

  if (!m_pSettings->getUseSystemBackground()) {
    this->setBackgroundBrush(
        QBrush(this->readColor(QStringLiteral("BGColor"))));
  }

  if (0 == m_nGridSize) {
    m_nGridSize = static_cast<quint16>(
        m_pBoardConf->value(QStringLiteral("GridSize"), 0).toUInt());
  }
  if (0 == m_nGridSize || m_nGridSize > Board::MAXGRID) {
    qWarning() << "INVALID GRID SIZE:" << m_nGridSize;
    m_nGridSize = Board::DEFAULTGRID;
    QMessageBox::warning(m_pParent, tr("Warning"),
                         tr("Board grid size not valid.\n"
                            "Reduced grid to default."));
  }

  connect(m_pSettings, &Settings::useSystemBackgroundColor, this,
          &Board::useSystemBackground);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::setupBoard() -> bool {
  qDebug() << Q_FUNC_INFO;
  m_bFreestyle =
      m_pBoardConf->value(QStringLiteral("Freestyle"), false).toBool();

  m_BoardPoly.clear();
  m_BoardPoly =
      this->readPolygon(m_pBoardConf, QStringLiteral("Board/Polygon"), true);
  if (m_BoardPoly.isEmpty()) {
    qWarning() << "BOARD POLYGON IS EMPTY!";
    QMessageBox::warning(m_pParent, tr("Warning"),
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
  QBrush brush(this->readColor(QStringLiteral("Board/Color")));
  QPen pen(this->readColor(QStringLiteral("Board/BorderColor")));
  if (m_pSettings->getUseSystemBackground()) {
    if (m_pBoardConf->contains(QStringLiteral("Barrier1/Color"))) {
      QColor cBG(this->readColor(QStringLiteral("BGColor")));
      if (cBG == this->readColor(QStringLiteral("Barrier1/Color")) &&
          cBG == this->readColor(QStringLiteral("Barrier1/BorderColor")) &&
          cBG == this->readColor(QStringLiteral("Board/BorderColor"))) {
        pen = QApplication::palette().color(QPalette::Base);
      }
    }
  }

  this->addPolygon(m_BoardPoly, pen, brush);
  m_pGraphView->setSceneRect(m_BoardPoly.boundingRect());
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::drawGrid() {
  QLineF lineGrid;
  QPen pen(this->readColor(QStringLiteral("Board/GridColor")));
  if (m_pSettings->getUseSystemBackground()) {
    if (m_pBoardConf->contains(QStringLiteral("Barrier1/Color"))) {
      QColor cBG(this->readColor(QStringLiteral("BGColor")));
      if (cBG == this->readColor(QStringLiteral("Barrier1/Color")) &&
          cBG == this->readColor(QStringLiteral("Barrier1/BorderColor")) &&
          cBG == this->readColor(QStringLiteral("Board/GridColor"))) {
        pen = QApplication::palette().color(QPalette::Base);
      }
    }
  }

  // Horizontal
  for (int i = 1; i < m_BoardPoly.boundingRect().height() / m_nGridSize; i++) {
    lineGrid.setLine(1, i * m_nGridSize, m_BoardPoly.boundingRect().width() - 1,
                     i * m_nGridSize);
    this->addLine(lineGrid, pen);
  }
  // Vertical
  for (int i = 1; i < m_BoardPoly.boundingRect().width() / m_nGridSize; i++) {
    lineGrid.setLine(i * m_nGridSize, 1, i * m_nGridSize,
                     m_BoardPoly.boundingRect().height() - 1);
    this->addLine(lineGrid, pen);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::setupBlocks() -> bool {
  qDebug() << Q_FUNC_INFO;
  m_nNumOfBlocks = 0;
  m_listBlocks.clear();

  if (this->createBlocks() && this->createBarriers()) {
    // Add blocks to board
    for (auto &block : m_listBlocks) {
      this->addItem(block);
    }

    m_bNotAllPiecesNeeded =
        m_pBoardConf->value(QStringLiteral("NotAllPiecesNeeded"), false)
            .toBool();
    if (m_bNotAllPiecesNeeded) {
      QMessageBox::information(m_pParent, tr("Hint"),
                               tr("Not all pieces are needed for a solution."));
    }

    m_pGraphView->setEnabled(true);
  }
  return m_bFreestyle;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::createBlocks() -> bool {
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
      QMessageBox::warning(m_pParent, tr("Warning"),
                           tr("Polygon not valid:") + "\n" + sPrefix);
      return false;
    }

    // Create new block
    m_listBlocks.append(new Block(
        i, polygon, this->readColor(sPrefix + "/Color"),
        this->readColor(sPrefix + "/BorderColor"), m_nGridSize, &m_listBlocks,
        m_pSettings,
        Board::readStartPosition(tmpSet, sPrefix + "/StartPos", m_pParent)));
    if (!m_bFreestyle) {
      connect(m_listBlocks.last(), &Block::checkPuzzleSolved, this,
              &Board::checkPuzzleSolved);
      connect(m_listBlocks.last(), &Block::incrementMoves, this,
              &Board::incrementMoves);
    }
  }

  if (0 == m_nNumOfBlocks) {
    qWarning() << "NO VALID BLOCKS FOUND.";
    QMessageBox::warning(m_pParent, tr("Warning"),
                         tr("Could not find valid blocks."));
    return false;
  }

  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::createBarriers() -> bool {
  const unsigned char nMaxNumOfBlocks(250);
  QColor cBG(this->readColor(QStringLiteral("BGColor")));

  for (quint16 i = 1; i <= nMaxNumOfBlocks; i++) {
    QString sPrefix = "Barrier" + QString::number(i);
    if (!m_pBoardConf->contains(sPrefix + "/Polygon")) {
      break;
    }

    QPolygonF polygon = this->readPolygon(m_pBoardConf, sPrefix + "/Polygon");
    if (polygon.isEmpty()) {
      this->clear();  // Clear all objects
      qWarning() << "POLYGON IS EMPTY FOR BARRIER" << i;
      QMessageBox::warning(m_pParent, tr("Warning"),
                           tr("Polygon not valid:") + "\n" + sPrefix);
      return false;
    }

    bool bIsBoardBG = (cBG == this->readColor(sPrefix + "/Color") &&
                       cBG == this->readColor(sPrefix + "/BorderColor"));

    // Create new barrier
    m_listBlocks.append(
        new Block(m_nNumOfBlocks + i, polygon,
                  this->readColor(sPrefix + "/Color", bIsBoardBG),
                  this->readColor(sPrefix + "/BorderColor", bIsBoardBG),
                  m_nGridSize, &m_listBlocks, m_pSettings,
                  Board::readStartPosition(m_pBoardConf, sPrefix + "/StartPos",
                                           m_pParent),
                  true));
  }

  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::readColor(const QString &sKey, const bool bColorIsBoardBG) const
    -> QColor {
  if (bColorIsBoardBG && m_pSettings->getUseSystemBackground()) {
    return QApplication::palette().color(QPalette::Base);
  }

  QString sValue = m_pBoardConf->value(sKey, "").toString();
  QColor color(255, 0, 255);

  if (sValue.isEmpty()) {
    sValue = QStringLiteral("#00FFFF");
    qWarning() << "Set fallback color for key" << sKey;
    QMessageBox::warning(
        m_pParent, tr("Warning"),
        tr("No color defined - using fallback:") + "\n" + sKey);
  }

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
  color.setNamedColor(sValue);
#else
  color = QColor::fromString(sValue);
#endif
  if (!color.isValid()) {
    color = QColor(255, 0, 255);
    qWarning() << "Found invalid color for key" << sKey;
    QMessageBox::warning(
        m_pParent, tr("Warning"),
        tr("Invalid color defined - using fallback:") + "\n" + sKey);
  }
  return color;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::readPolygon(const QSettings *tmpSet, const QString &sKey,
                        const bool bScale) -> QPolygonF {
  QStringList sList;
  QStringList sListPoint;
  QString sValue(tmpSet->value(sKey, "").toString());
  quint16 nScale(1);
  if (bScale) {
    nScale = m_nGridSize;
  }

  Board::checkOrthogonality(QPointF(-99999, -99999));
  sList << sValue.split('|');
  QPolygonF polygon;
  polygon.reserve(sList.size());
  for (auto &s : sList) {
    sListPoint.clear();
    sListPoint << s.split(',');
    if (2 == sListPoint.size()) {
      polygon << QPointF(sListPoint[0].trimmed().toShort() * nScale,
                         sListPoint[1].trimmed().toShort() * nScale);

      if (!Board::checkOrthogonality(polygon.last())) {
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

auto Board::checkOrthogonality(QPointF point) -> bool {
  static QVector<QPointF> listPoints;
  static quint16 nCnt;

  if (QPointF(-99999, -99999) == point) {  // Reset
    listPoints.clear();
    nCnt = 0;
    return true;
  }
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

  qWarning() << "Wrong point #" << nCnt;
  return false;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::readStartPosition(const QSettings *tmpSet, const QString &sKey,
                              QWidget *pParent) -> QPointF {
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
    QMessageBox::warning(
        pParent, tr("Warning"),
        tr("Invalid start position - using fallback:") + "\n" + sKey);
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

  for (auto &block : m_listBlocks) {
    QPointF pos =
        QPointF(block->pos().x() / m_nGridSize, block->pos().y() / m_nGridSize);

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
          // Block touches board outline, but completely outside
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
  if (m_nGridSize <= (Board::MAXGRID - Board::ZOOMGRID)) {
    m_nGridSize += Board::ZOOMGRID;
  } else {
    m_nGridSize = Board::MAXGRID;
  }
  this->doZoom();
}

void Board::zoomOut() {
  if (m_nGridSize >= (2 * Board::ZOOMGRID)) {
    m_nGridSize -= Board::ZOOMGRID;
  } else {
    m_nGridSize = Board::ZOOMGRID;
  }
  this->doZoom();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::doZoom() {
  qDebug() << "Zoom - new grid: " << m_nGridSize;

  // Get all QGraphicItems in scene
  QList<QGraphicsItem *> objList = this->items();

  // Remove objects from scene which are no blocks
  for (auto &gi : objList) {
    if (gi->type() != Block::Type) {
      this->removeItem(gi);
    }
  }

  // Draw resized board again
  this->setupBoard();

  // Rescale blocks
  for (auto &block : m_listBlocks) {
    block->rescaleBlock(m_nGridSize);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::useSystemBackground(const bool bUseSysColor) -> void {
  if (!bUseSysColor) {
    this->setBackgroundBrush(
        QBrush(this->readColor(QStringLiteral("BGColor"))));
  } else {
    this->setBackgroundBrush(Qt::NoBrush);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::getGridSize() const -> quint16 { return m_nGridSize; }

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
  ba.append(sTime.toLatin1());
  saveConf.setValue(QStringLiteral("ElapsedTime"), ba.toBase64());
  ba.clear();
  ba.append(sMoves.toLatin1());
  saveConf.setValue(QStringLiteral("NumOfMoves"), ba.toBase64());

  for (int i = 0; i < m_nNumOfBlocks; i++) {
    QString sPrefix = "Block" + QString::number(i + 1);
    QPolygonF poly = m_listBlocks.at(i)->getPolygon();
    QString sPoly(QLatin1String(""));
    for (auto &point : poly) {
      sPoly +=
          QString::number(point.x()) + "," + QString::number(point.y()) + " | ";
    }
    sPoly.remove(sPoly.length() - 3, sPoly.length());

    saveConf.setValue(sPrefix + "/Polygon", sPoly);
    QPointF pos = m_listBlocks.at(i)->getPosition();
    saveConf.setValue(sPrefix + "/StartPos",
                      QString::number(pos.x() / m_nGridSize) + "," +
                          QString::number(pos.y() / m_nGridSize));

    if (sSaveFile.endsWith(QStringLiteral("S0LV3D.debug"))) {
      sDebug += "[" + sPrefix + "]\n";
      sDebug += "Polygon=\"" + sPoly + "\"\n";
      sDebug += "StartPos=\"" + QString::number(pos.x() / m_nGridSize) + "," +
                QString::number(pos.y() / m_nGridSize) + "\"\n";
    }
  }

  if (sSaveFile.endsWith(QStringLiteral("S0LV3D.debug"))) {
    ba.clear();
    ba.append(sDebug.toUtf8());
    qDebug() << "SOLVED" << m_sBoardFile;
    qDebug() << ba.toBase64();
  }
}
