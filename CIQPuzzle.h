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
 *
 * \section DESCRIPTION
 * Class definition main application.
 */

#ifndef IQPUZZLE_CIQPUZZLE_H_
#define IQPUZZLE_CIQPUZZLE_H_

#include <QtCore>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QtGui>
#include <QTimer>
#include <QMainWindow>

#include "./CBoard.h"
#include "./CBoardDialog.h"
#include "./CHighscore.h"
#include "./CSettings.h"

namespace Ui {
    class CIQPuzzle;
}

/**
 * \class CIQPuzzle
 * \brief Main application definition (gui, objects, etc.)
 */
class CIQPuzzle : public QMainWindow {
    Q_OBJECT

  public:
    explicit CIQPuzzle(const QDir userDataDir, const QDir &sharePath,
                       QWidget *pParent = 0);
    ~CIQPuzzle();

  protected:
    void closeEvent(QCloseEvent *pEvent);

  public slots:
    void setMinWindowSize(const QSize size);
    void incrementMoves();

  signals:
    void showHighscore(const QString &sBoard);
    void checkHighscore(const QString &sBoard, const quint32 &nMoves,
                        const QTime &tTime);

  private slots:
    void startNewGame(QString sBoardFile = "", const QString sSavedGame = "",
                      const QString sTime = "", const QString sMoves = "");
    void restartGame();
    void loadGame();
    void saveGame();
    void pauseGame(const bool bPaused);
    void updateTimer();
    void solvedPuzzle();
    void showHighscore();
    void showInfoBox();

  private:
    void setupMenu();

    Ui::CIQPuzzle *m_pUi;
    QGraphicsView *m_pGraphView;
    QGraphicsScene *m_pScene;
    QGraphicsScene *m_pScenePaused;
    CBoardDialog *m_pBoardDialog;
    CBoard *m_pBoard;
    QString m_sBoardFile;
    QString m_sSavedGame;
    QDir m_userDataDir;
    QString m_sSharePath;
    QLabel *m_pStatusLabelTime;
    QLabel *m_pStatusLabelMoves;
    quint32 m_nMoves;
    QString m_sSavedTime;
    QString m_sSavedMoves;
    QTime m_Time;
    QTimer *m_pTimer;
    QGraphicsTextItem *m_pTextPaused;
    bool m_bSolved;
    CHighscore *m_pHighscore;
    CSettings *m_pSettings;
};

#endif  // IQPUZZLE_CIQPUZZLE_H_
