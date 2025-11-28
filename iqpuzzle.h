// SPDX-FileCopyrightText: 2012-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IQPUZZLE_H_
#define IQPUZZLE_H_

#include <QDir>
#include <QMainWindow>
#include <QTime>
#include <QTranslator>

class QLabel;
class QGraphicsScene;
class QGraphicsTextItem;
class QGraphicsView;
class QTimer;

class Board;
class BoardSelection;
class Highscore;
class Settings;

namespace Ui {
class IQPuzzle;
}

class IQPuzzle : public QMainWindow {
  Q_OBJECT

 public:
  explicit IQPuzzle(const QDir &userDataDir, const QDir &sharePath,
                    QWidget *pParent = nullptr);
  ~IQPuzzle();

 protected:
  void changeEvent(QEvent *pEvent) override;
  void resizeEvent(QResizeEvent *pEvent) override;

 public slots:
  void setMinWindowSize(const QSize size = QSize(),
                        const bool bFreestyle = false);

 signals:
  void updateUiLang();
  void checkHighscore(const QString &sBoard, const quint32 nMoves,
                      const QTime tTime);

 private slots:
  void loadLanguage(const QString &sLang);
  void startNewGame(QString sBoardFile = QLatin1String(""),
                    const QString &sSavedGame = QLatin1String(""),
                    const QString &sTime = QLatin1String(""),
                    const QString &sMoves = QLatin1String(""));
  void randomGame(const int nChoice);
  void restartGame();
  void loadGame(QString sSaveFile = QLatin1String(""));
  void saveGame();
  void pauseGame(const bool bPaused);
  void solvedPuzzle();
  void showStatistics();
  void showInfoBox();

 private:
  void createBoard();
  auto chooseBoard() -> QString;
  static auto switchTranslator(QTranslator *translator, const QString &sFile,
                               const QString &sPath = QLatin1String(""))
      -> bool;
  void setupMenu();
  void setGameTitle();
  void generateFileLists();

  Ui::IQPuzzle *m_pUi;
  QTranslator m_translator;    // App translations
  QTranslator m_translatorQt;  // Qt translations
  QString m_sCurrLang;
  QGraphicsView *m_pGraphView;
  QGraphicsScene *m_pScenePaused;
  BoardSelection *m_pBoardSelection;
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
