/**
 * \file settings.h
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
 * Class definition for settings.
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QDialog>

class QSettings;

namespace Ui {
class SettingsDialog;
}

/**
 * \class Settings
 * \brief Settings dialog.
 */
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

  QWidget *m_pParent;
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
