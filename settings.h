// SPDX-FileCopyrightText: 2014-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QSettings>

class Settings : public QObject {
  Q_OBJECT

 public:
  static Settings *instance();
  auto getSharePath() const -> QString;
  void setSharePath(const QString &sPath);

  auto getMouseControls() -> QList<uint>;
  void setMouseControls(const QList<uint> &mouseControls);
  auto getGuiLanguage() -> QString;
  void setGuiLanguage(const QString &sLanguage);
  auto getUseSystemBackground() const -> bool;
  void setUseSystemBackground(const bool bUseSystemBackground);
  auto getLastOpenedDir() -> QString;
  void setLastOpenedDir(const QString &sLastOpenedDir);
  auto getThresholdEasy() const -> uint;
  auto getThresholdHard() const -> uint;

  static const quint8 SHIFT = 0xF0;

 signals:
  void updateUseSystemBackgroundColor(const bool bUseSystemBackground);
  void changeGuiLanguage(const QString &sLanguage);

 private:
  explicit Settings(QObject *pParent = nullptr);
  QSettings m_settings;
  QString m_sSharePath;

  static const uint THRESHOLD_EASY = 200;
  static const uint THRESHOLD_HARD = 10;
};

#endif  // SETTINGS_H_
