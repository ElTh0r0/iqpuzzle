// SPDX-FileCopyrightText: 2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include <QDialog>

#include "./settings.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SettingsDialog(QWidget *pParent);
  virtual ~SettingsDialog();

 public slots:
  void accept() override;
  void updateUiLang();

 protected:
  void showEvent(QShowEvent *pEvent) override;

 private:
  void readSettings();
  QStringList searchTranslations();

  Ui::SettingsDialog *m_pUi;
  Settings *m_pSettings;

  QStringList m_sListMouseButtons;
  QList<uint> m_listMouseButtons;
};

#endif  // SETTINGSDIALOG_H_
