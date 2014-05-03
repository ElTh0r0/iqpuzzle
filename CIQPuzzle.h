/**
 * \file CIQPuzzle.h
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

#ifndef IQPUZZLE_CIQPUZZLE_H_
#define IQPUZZLE_CIQPUZZLE_H_

#include <QtCore>
#include <QGraphicsView>
#include <QtGui>
#include <QMainWindow>

#include "./CBlock.h"

namespace Ui {
    class CIQPuzzle;
}

class CIQPuzzle : public QMainWindow {
    Q_OBJECT

  public:
    explicit CIQPuzzle(QWidget *pParent = 0);
    ~CIQPuzzle();

  protected:
    void closeEvent(QCloseEvent *pEvent);

  private slots:
    void startNewGame();
    void zoomIn();
    void zoomOut();

    void showControlsBox();
    void showInfoBox();

  private:
    void setupMenu();
    void setupBoard();
    void setupBlocks();
    void doZoom();

    Ui::CIQPuzzle *m_pUi;
    QSettings *m_pConfig;
    QGraphicsView *m_pGraphView;
    QGraphicsScene *m_pScene;

    quint16 m_nGridSize;
    QSize m_BoardSize;

    QList<CBlock *> m_listBlocks;
};

#endif  // IQPUZZLE_CIQPUZZLE_H_
