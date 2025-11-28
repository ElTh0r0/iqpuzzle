// SPDX-FileCopyrightText: 2014-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QDialog>

class QSettings;

namespace Ui {
class SettingsDialog;
}

class Settings : public QDialog {
  Q_OBJECT

 public:
  explicit Settings(QWidget *pParent, QString sSharePath);
  virtual ~Settings();

  static const quint8 nSHIFT = 0xF0;
  auto getMouseControls() const -> QList<uint>;
  auto getLanguage() -> QString;
  auto getUseSystemBackground() -> bool;

  auto getEasy() const -> uint;
  auto getHard() const -> uint;

  auto getLastOpenedDir() -> QString;
  void setLastOpenedDir(const QString &sLastOpenedDir);

 signals:
  void changeLang(const QString &sLang);
  void useSystemBackgroundColor(const bool bUseSysColor);

 public slots:
  void accept() override;
  void updateUiLang();

 protected:
  void showEvent(QShowEvent *pEvent) override;

 private:
  void readSettings();
  QStringList searchTranslations();

  Ui::SettingsDialog *m_pUi;
  QSettings *m_pSettings;

  QString m_sGuiLanguage;
  const QString m_sSharePath;
  QStringList m_sListMouseButtons;
  QList<uint> m_listMouseButtons;
  QList<uint> m_listMouseControls;
  bool m_bUseSystemBackground{};
  uint m_nEasy{};
  uint m_nHard{};
  QString m_sLastOpenedDir;
};

#endif  // SETTINGS_H_
