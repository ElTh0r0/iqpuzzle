// SPDX-FileCopyrightText: 2012-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOARD_H_
#define BOARD_H_

#include <QGraphicsScene>
#include <QPolygonF>

class QSettings;

class Block;

class Board : public QGraphicsScene {
  Q_OBJECT

 public:
  Board(QWidget *pParent, QGraphicsView *pGraphView, QString sBoardFile,
        const quint16 nGridSize = 0,
        const QString &sSavedGame = QLatin1String(""),
        QObject *pParentObj = nullptr);

  auto setupBoard() -> bool;
  auto setupBlocks() -> bool;
  void saveGame(const QString &sSaveFile, const QString &sTime,
                const QString &sMoves);
  auto getGridSize() const -> quint16;

 signals:
  void setWindowSize(const QSize size, const bool bFreestyle);
  void incrementMoves();
  void solvedPuzzle();

 public slots:
  void zoomIn();
  void zoomOut();
  void checkPuzzleSolved();

 private slots:
  void updateUseSystemBackground(const bool bUseSystemBackground);

 private:
  void drawBoard();
  void drawGrid();
  auto drawCalendar(bool bMonth = false) -> bool;
  auto createBlocks() -> bool;
  auto createBarriers() -> bool;
  auto readColor(const QString &sKey, const bool bColorIsBoardBG = false) const
      -> QColor;
  QPolygonF readPolygon(const QSettings *tmpSet, const QString &sKey,
                        const bool bScale = false);
  static auto checkOrthogonality(QPointF point) -> bool;
  static auto readStartPosition(const QSettings *tmpSet, const QString &sKey,
                                QWidget *pParent) -> QPointF;
  void doZoom();

  QWidget *m_pParent;
  QGraphicsView *m_pGraphView;
  QSettings *m_pBoardConf;
  QSettings *m_pSavedConf;
  QString m_sBoardFile;
  bool m_bSavedGame;
  QPolygonF m_BoardPoly;
  QList<Block *> m_listBlocks;
  quint8 m_nNumOfBlocks{};
  quint16 m_nGridSize;
  bool m_bUseSystemBackground;
  bool m_bNotAllPiecesNeeded{};
  bool m_bFreestyle{};

  static const quint8 ZOOMGRID = 5;
  static const quint8 DEFAULTGRID = 25;
  static const quint8 MAXGRID = 255;
};

#endif  // BOARD_H_
