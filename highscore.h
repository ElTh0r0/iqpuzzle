// SPDX-FileCopyrightText: 2014-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HIGHSCORE_H_
#define HIGHSCORE_H_

#include <QObject>
#include <QTime>

class QSettings;

class Highscore : public QObject {
  Q_OBJECT

 public:
  explicit Highscore(QWidget *pParent, QObject *pParentObj = nullptr);

 public slots:
  void showHighscore(QString sBoard);
  void checkHighscore(const QString &sBoard, const quint32 nMoves,
                      const QTime tTime);

 private:
  auto readHighscore(const QString &sBoard, const QString &sKey) const
      -> QStringList;
  void insertHighscore(const QString &sBoard, const quint8 nPosition,
                       const quint32 nMoves, const QTime tTime);

  QWidget *m_pParent;
  QSettings *m_pHighscore;
  const quint8 m_nMAXPOS;
};

#endif  // HIGHSCORE_H_
