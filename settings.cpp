/**
 * \file settings.cpp
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
 * Settings dialog.
 */

#include "./settings.h"

#include <QDebug>
#include <QDirIterator>
#include <QMessageBox>
#include <QSettings>

#include "ui_settings.h"

Settings::Settings(QWidget *pParent, QString sSharePath)
    : QDialog(pParent), m_sSharePath(std::move(sSharePath)) {
  qDebug() << Q_FUNC_INFO;

  m_pUi = new Ui::SettingsDialog();
  m_pUi->setupUi(this);
  this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
  this->setModal(true);

#if defined _WIN32
  m_pSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
#else
  m_pSettings = new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
#endif

  m_sListMouseButtons << tr("Left") << tr("Middle") << tr("Right");
  m_listMouseButtons << Qt::LeftButton << Qt::MiddleButton << Qt::RightButton;
  m_pUi->cbMoveBlockMouse->addItems(m_sListMouseButtons);
  m_sListMouseButtons << tr("First X") << tr("Second X") << tr("Vertical wheel")
                      << tr("Horizontal wheel")
                      << tr("Left") + " + " + tr("Shift")
                      << tr("Left") + " + " + tr("Ctrl")
                      << tr("Left") + " + " + tr("Alt")
                      << tr("Left") + " + " + tr("Meta");
  m_listMouseButtons << Qt::XButton1 << Qt::XButton2
                     << (quint8(Qt::Vertical) | nSHIFT)
                     << (quint8(Qt::Horizontal) | nSHIFT)
                     << ((quint32)Qt::LeftButton | Qt::ShiftModifier)
                     << ((quint32)Qt::LeftButton | Qt::ControlModifier)
                     << ((quint32)Qt::LeftButton | Qt::AltModifier)
                     << ((quint32)Qt::LeftButton | Qt::MetaModifier);
  m_pUi->cbRotateBlockMouse->addItems(m_sListMouseButtons);
  m_pUi->cbFlipBlockMouse->addItems(m_sListMouseButtons);

  m_pUi->cbGuiLanguage->addItems(this->searchTranslations());

  connect(m_pUi->buttonBox, &QDialogButtonBox::accepted, this,
          &Settings::accept);
  connect(m_pUi->buttonBox, &QDialogButtonBox::rejected, this,
          &QDialog::reject);
  this->readSettings();
}

Settings::~Settings() {
  delete m_pUi;
  m_pUi = nullptr;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::showEvent(QShowEvent *pEvent) {
  this->readSettings();
  QDialog::showEvent(pEvent);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::accept() {
  qDebug() << Q_FUNC_INFO;

  QList<uint> tmp_listMouseControls;
  tmp_listMouseControls
      << m_listMouseButtons[m_pUi->cbMoveBlockMouse->currentIndex()];
  tmp_listMouseControls
      << m_listMouseButtons[m_pUi->cbRotateBlockMouse->currentIndex()];
  tmp_listMouseControls
      << m_listMouseButtons[m_pUi->cbFlipBlockMouse->currentIndex()];

  if (tmp_listMouseControls[0] == tmp_listMouseControls[1] ||
      tmp_listMouseControls[0] == tmp_listMouseControls[2] ||
      tmp_listMouseControls[1] == tmp_listMouseControls[2]) {
    QMessageBox::warning(this, this->windowTitle(),
                         tr("Please change your settings. Same mouse "
                            "button is used for several actions."));
    return;
  }
  m_listMouseControls[0] = tmp_listMouseControls[0];
  m_listMouseControls[1] = tmp_listMouseControls[1];
  m_listMouseControls[2] = tmp_listMouseControls[2];

  QString sOldGuiLang = m_sGuiLanguage;
  m_sGuiLanguage = m_pUi->cbGuiLanguage->currentText();
  m_pSettings->setValue(QStringLiteral("GuiLanguage"), m_sGuiLanguage);
  if (sOldGuiLang != m_sGuiLanguage) {
    emit changeLang(this->getLanguage());
  }

  bool bOldUseSysColor = m_bUseSystemBackground;
  m_bUseSystemBackground = m_pUi->checkSystemBackground->isChecked();
  m_pSettings->setValue(QStringLiteral("UseSystemBackground"),
                        m_bUseSystemBackground);
  if (bOldUseSysColor != m_bUseSystemBackground) {
    emit useSystemBackgroundColor(m_bUseSystemBackground);
  }

  m_pSettings->beginGroup(QStringLiteral("MouseControls"));
  m_pSettings->setValue(QStringLiteral("MoveBlock"), m_listMouseControls[0]);
  m_pSettings->setValue(QStringLiteral("RotateBlock"), m_listMouseControls[1]);
  m_pSettings->setValue(QStringLiteral("FlipBlock"), m_listMouseControls[2]);
  m_pSettings->remove(QStringLiteral("Enabled"));
  m_pSettings->endGroup();

  QDialog::accept();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::readSettings() {
  m_sGuiLanguage =
      m_pSettings->value(QStringLiteral("GuiLanguage"), QStringLiteral("auto"))
          .toString();
  if (-1 != m_pUi->cbGuiLanguage->findText(m_sGuiLanguage)) {
    m_pUi->cbGuiLanguage->setCurrentIndex(
        m_pUi->cbGuiLanguage->findText(m_sGuiLanguage));
  } else {
    m_pUi->cbGuiLanguage->setCurrentIndex(
        m_pUi->cbGuiLanguage->findText(QStringLiteral("auto")));
  }
  m_sGuiLanguage = m_pUi->cbGuiLanguage->currentText();

  m_bUseSystemBackground =
      m_pSettings->value(QStringLiteral("UseSystemBackground"), false).toBool();
  m_pUi->checkSystemBackground->setChecked(m_bUseSystemBackground);

  m_nEasy = m_pSettings->value(QStringLiteral("ThresholdEasy"), 200).toUInt();
  if (0 == m_nEasy) m_nEasy = 200;
  m_nHard = m_pSettings->value(QStringLiteral("ThresholdHard"), 10).toUInt();
  if (0 == m_nHard) m_nHard = 10;

  m_listMouseControls.clear();
  m_listMouseControls << 0 << 0 << 0;
  m_pSettings->beginGroup(QStringLiteral("MouseControls"));
  m_listMouseControls[0] =
      m_pSettings->value(QStringLiteral("MoveBlock"), Qt::LeftButton).toUInt();
  m_listMouseControls[1] = m_pSettings
                               ->value(QStringLiteral("RotateBlock"),
                                       (quint8(Qt::Vertical) | nSHIFT))
                               .toUInt();
  m_listMouseControls[2] =
      m_pSettings->value(QStringLiteral("FlipBlock"), Qt::RightButton).toUInt();
  m_pUi->cbMoveBlockMouse->setCurrentIndex(
      m_listMouseButtons.indexOf(m_listMouseControls.at(0)));
  m_pUi->cbRotateBlockMouse->setCurrentIndex(
      m_listMouseButtons.indexOf(m_listMouseControls.at(1)));
  m_pUi->cbFlipBlockMouse->setCurrentIndex(
      m_listMouseButtons.indexOf(m_listMouseControls.at(2)));
  m_pSettings->endGroup();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getMouseControls() const -> QList<uint> {
  return m_listMouseControls;
}

auto Settings::getEasy() const -> uint { return m_nEasy; }

auto Settings::getHard() const -> uint { return m_nHard; }

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::updateUiLang() {
  m_pUi->retranslateUi(this);

  m_sListMouseButtons.clear();
  m_sListMouseButtons << tr("Left") << tr("Middle") << tr("Right");
  m_pUi->cbMoveBlockMouse->clear();
  m_pUi->cbMoveBlockMouse->addItems(m_sListMouseButtons);

  m_sListMouseButtons << tr("First X") << tr("Second X") << tr("Vertical wheel")
                      << tr("Horizontal wheel")
                      << tr("Left") + " + " + tr("Shift")
                      << tr("Left") + " + " + tr("Ctrl")
                      << tr("Left") + " + " + tr("Alt")
                      << tr("Left") + " + " + tr("Meta");
  m_pUi->cbRotateBlockMouse->clear();
  m_pUi->cbRotateBlockMouse->addItems(m_sListMouseButtons);
  m_pUi->cbFlipBlockMouse->clear();
  m_pUi->cbFlipBlockMouse->addItems(m_sListMouseButtons);

  m_pUi->cbMoveBlockMouse->setCurrentIndex(
      m_listMouseButtons.indexOf(m_listMouseControls.at(0)));
  m_pUi->cbRotateBlockMouse->setCurrentIndex(
      m_listMouseButtons.indexOf(m_listMouseControls.at(1)));
  m_pUi->cbFlipBlockMouse->setCurrentIndex(
      m_listMouseButtons.indexOf(m_listMouseControls.at(2)));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::searchTranslations() -> QStringList {
  QStringList sList;
  QString sTmp;

  // Translations build in resources
  QDirIterator it(QStringLiteral(":"), QStringList() << QStringLiteral("*.qm"),
                  QDir::NoDotAndDotDot | QDir::Files);
  while (it.hasNext()) {
    it.next();
    sTmp = it.fileName();
    // qDebug() << sTmp;

    if (sTmp.startsWith(qApp->applicationName().toLower() + "_") &&
        sTmp.endsWith(QStringLiteral(".qm"))) {
      sList << sTmp.remove(qApp->applicationName().toLower() + "_")
                   .remove(QStringLiteral(".qm"));
    }
  }

  // Check for additional translation files in share folder
  QDirIterator it2(m_sSharePath + "/lang",
                   QStringList() << QStringLiteral("*.qm"),
                   QDir::NoDotAndDotDot | QDir::Files);
  while (it2.hasNext()) {
    it2.next();
    sTmp = it2.fileName();
    // qDebug() << sTmp;

    if (sTmp.startsWith(qApp->applicationName().toLower() + "_")) {
      sTmp = sTmp.remove(qApp->applicationName().toLower() + "_")
                 .remove(QStringLiteral(".qm"));
      if (!sList.contains(sTmp)) {
        sList << sTmp;
      }
    }
  }

  sList.sort();
  sList.push_front(QStringLiteral("auto"));
  return sList;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getLanguage() -> QString {
  if ("auto" == m_sGuiLanguage) {
#ifdef Q_OS_UNIX
    QByteArray lang = qgetenv("LANG");
    if (!lang.isEmpty()) {
      return QLocale(QString::fromLatin1(lang)).name();
    }
#endif
    return QLocale::system().name();
  }
  if (!QFile(":/" + qApp->applicationName().toLower() + "_" + m_sGuiLanguage +
             ".qm")
           .exists() &&
      !QFile(m_sSharePath + "/lang/" + qApp->applicationName().toLower() + "_" +
             m_sGuiLanguage + ".qm")
           .exists()) {
    m_sGuiLanguage = QStringLiteral("en");
    m_pSettings->setValue(QStringLiteral("GuiLanguage"), m_sGuiLanguage);
    return m_sGuiLanguage;
  }
  return m_sGuiLanguage;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getLastOpenedDir() -> QString {
  m_sLastOpenedDir =
      m_pSettings
          ->value(QStringLiteral("LastOpenedDir"), QString(QDir::homePath()))
          .toString();
  return m_sLastOpenedDir;
}

void Settings::setLastOpenedDir(const QString &sLastOpenedDir) {
  m_sLastOpenedDir = sLastOpenedDir;
  m_pSettings->setValue(QStringLiteral("LastOpenedDir"), m_sLastOpenedDir);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getUseSystemBackground() -> bool {
  return m_bUseSystemBackground;
}
