/**
 * \file iqpuzzle.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-2018 Thorsten Roth <elthoro@gmx.de>
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

#ifndef IQPUZZLE_H_
#define IQPUZZLE_H_

#include <QtCore>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QtGui>
#include <QTimer>
#include <QMainWindow>

#include "./board.h"
#include "./boarddialog.h"
#include "./highscore.h"
#include "./settings.h"

namespace Ui {
class IQPuzzle;
}

/**
 * \class IQPuzzle
 * \brief Main application definition (gui, objects, etc.)
 */
class IQPuzzle : public QMainWindow {
  Q_OBJECT

  public:
    explicit IQPuzzle(const QDir &userDataDir, const QDir &sharePath,
                      QWidget *pParent = 0);
    ~IQPuzzle();

  protected:
    void changeEvent(QEvent *pEvent);
    void closeEvent(QCloseEvent *pEvent);

  public slots:
    void setMinWindowSize(const QSize size = QSize(),
                          const bool bFreestyle = false);
    void incrementMoves();

  signals:
    void updateUiLang();
    void showHighscore(const QString &sBoard);
    void checkHighscore(const QString &sBoard, const quint32 &nMoves,
                        const QTime &tTime);

  private slots:
    void loadLanguage(const QString &sLang);
    void startNewGame(QString sBoardFile = "", const QString sSavedGame = "",
                      const QString sTime = "", const QString sMoves = "");
    QString chooseBoard();
    void createBoard();
    void randomGame(const int nChoice);
    void restartGame();
    void loadGame(QString sSaveFile = "");
    void saveGame();
    void pauseGame(const bool bPaused);
    void updateTimer();
    void solvedPuzzle();
    void showHighscore();
    void showStatistics();
    void reportBug() const;
    void showInfoBox();

  private:
    bool switchTranslator(QTranslator *translator, const QString &sFile,
                          const QString &sPath = "");
    void setupMenu();
    void setGameTitle();
    void generateFileLists();

    Ui::IQPuzzle *m_pUi;
    QTranslator m_translator;  // App translations
    QTranslator m_translatorQt;  // Qt translations
    QString m_sCurrLang;
    QGraphicsView *m_pGraphView;
    QGraphicsScene *m_pScenePaused;
    BoardDialog *m_pBoardDialog;
    Board *m_pBoard;
    QString m_sBoardFile;
    QString m_sSavedGame;
    const QDir m_userDataDir;
    const QString m_sSharePath;
    QLabel *m_pStatusLabelTime;
    QLabel *m_pStatusLabelMoves;
    quint32 m_nMoves;
    QString m_sSavedTime;
    QString m_sSavedMoves;
    QTime m_Time;
    QTimer *m_pTimer;
    QGraphicsTextItem *m_pTextPaused;
    bool m_bSolved;
    Highscore *m_pHighscore;
    Settings *m_pSettings;

    QSignalMapper *m_pSigMapRandom;
    QList<QStringList *> m_sListFiles;
    QStringList m_sListAll;
    QStringList m_sListAllUnsolved;
    QStringList m_sListEasy;
    QStringList m_sListEasyUnsolved;
    QStringList m_sListMedium;
    QStringList m_sListMediumUnsolved;
    QStringList m_sListHard;
    QStringList m_sListHardUnsolved;
};

#endif  // IQPUZZLE_H_
