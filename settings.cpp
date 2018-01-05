/**
 * \file settings.cpp
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
 * Settings dialog.
 */

#include <QDebug>
#include <QDirIterator>
#include <QMessageBox>

#include "./settings.h"
#include "ui_settings.h"

Settings::Settings(const QString &sSharePath, QWidget *pParent)
  : QDialog(pParent),
    m_sSharePath(sSharePath),
    m_nSHIFT(0xF0) {
  qDebug() << Q_FUNC_INFO;

  m_pUi = new Ui::SettingsDialog();
  m_pUi->setupUi(this);
  this->setWindowFlags(this->windowFlags()
                       & ~Qt::WindowContextHelpButtonHint);
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

  m_sListMouseButtons << trUtf8("Left") << trUtf8("Middle") << trUtf8("Right");
  m_listMouseButtons << Qt::LeftButton << Qt::MidButton << Qt::RightButton;
  m_pUi->cbMoveBlockMouse->addItems(m_sListMouseButtons);
  m_sListMouseButtons << trUtf8("First X") << trUtf8("Second X")
                      << trUtf8("Vertical wheel") << trUtf8("Horizontal wheel");
  m_listMouseButtons << Qt::XButton1 << Qt::XButton2
                     << (quint8(Qt::Vertical)|m_nSHIFT)
                     << (quint8(Qt::Horizontal)|m_nSHIFT);
  m_pUi->cbRotateBlockMouse->addItems(m_sListMouseButtons);
  m_pUi->cbFlipBlockMouse->addItems(m_sListMouseButtons);

  m_pUi->cbGuiLanguage->addItems(this->searchTranslations());

  connect(m_pUi->buttonBox, SIGNAL(accepted()),
          this, SLOT(accept()));
  connect(m_pUi->buttonBox, SIGNAL(rejected()),
          this, SLOT(reject()));

  this->readSettings();
}

Settings::~Settings() {
  if (m_pUi) {
    delete m_pUi;
    m_pUi = NULL;
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::accept() {
  qDebug() << Q_FUNC_INFO;

  QList<quint8> tmp_listMouseControls;
  tmp_listMouseControls << m_listMouseButtons[
                           m_pUi->cbMoveBlockMouse->currentIndex()];
  tmp_listMouseControls << m_listMouseButtons[
                           m_pUi->cbRotateBlockMouse->currentIndex()];
  tmp_listMouseControls << m_listMouseButtons[
                           m_pUi->cbFlipBlockMouse->currentIndex()];

  if (tmp_listMouseControls[0] == tmp_listMouseControls[1] ||
      tmp_listMouseControls[0] == tmp_listMouseControls[2] ||
      tmp_listMouseControls[1] == tmp_listMouseControls[2]) {
    QMessageBox::warning(0, this->windowTitle(),
                             trUtf8("Please change your settings. Same mouse "
                                    "button is used for several actions."));
    return;
  } else {
    m_listMouseControls[0] = tmp_listMouseControls[0];
    m_listMouseControls[1] = tmp_listMouseControls[1];
    m_listMouseControls[2] = tmp_listMouseControls[2];
  }

  QString sOldGuiLang = m_sGuiLanguage;
  m_sGuiLanguage = m_pUi->cbGuiLanguage->currentText();
  m_pSettings->setValue("GuiLanguage", m_sGuiLanguage);
  if (sOldGuiLang != m_sGuiLanguage) {
    emit changeLang(this->getLanguage());
  }

  m_pSettings->beginGroup("MouseControls");
  m_pSettings->setValue("MoveBlock", m_listMouseControls[0]);
  m_pSettings->setValue("RotateBlock", m_listMouseControls[1]);
  m_pSettings->setValue("FlipBlock", m_listMouseControls[2]);
  m_pSettings->remove("Enabled");
  m_pSettings->endGroup();

  QDialog::accept();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::reject() {
  this->readSettings();
  QDialog::reject();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::readSettings() {
  m_sGuiLanguage = m_pSettings->value("GuiLanguage", "auto").toString();
  if (-1 != m_pUi->cbGuiLanguage->findText(m_sGuiLanguage)) {
    m_pUi->cbGuiLanguage->setCurrentIndex(
          m_pUi->cbGuiLanguage->findText(m_sGuiLanguage));
  } else {
    m_pUi->cbGuiLanguage->setCurrentIndex(
          m_pUi->cbGuiLanguage->findText("auto"));
  }
  m_sGuiLanguage = m_pUi->cbGuiLanguage->currentText();

  m_nEasy = m_pSettings->value("ThresholdEasy", 200).toUInt();
  if (0 == m_nEasy) m_nEasy = 200;
  m_nHard = m_pSettings->value("ThresholdHard", 10).toUInt();
  if (0 == m_nHard) m_nHard = 10;

  m_listMouseControls.clear();
  m_listMouseControls << 0 << 0 << 0;
  m_pSettings->beginGroup("MouseControls");
  m_listMouseControls[0] = m_pSettings->value("MoveBlock",
                                              Qt::LeftButton).toUInt();
  m_listMouseControls[1] = m_pSettings->value("RotateBlock",
                                              (quint8(Qt::Vertical) |
                                               m_nSHIFT)).toUInt();
  m_listMouseControls[2] = m_pSettings->value("FlipBlock",
                                              Qt::RightButton).toUInt();
  m_pUi->cbMoveBlockMouse->setCurrentIndex(
        m_listMouseButtons.indexOf(m_listMouseControls[0]));
  m_pUi->cbRotateBlockMouse->setCurrentIndex(
        m_listMouseButtons.indexOf(m_listMouseControls[1]));
  m_pUi->cbFlipBlockMouse->setCurrentIndex(
        m_listMouseButtons.indexOf(m_listMouseControls[2]));
  m_pSettings->endGroup();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

quint8 Settings::getShift() const {
  return m_nSHIFT;
}

QList<quint8> Settings::getMouseControls() const {
  return m_listMouseControls;
}

quint16 Settings::getEasy() const {
  return m_nEasy;
}

quint16 Settings::getHard() const {
  return m_nHard;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::updateUiLang() {
  m_pUi->retranslateUi(this);

  m_sListMouseButtons.clear();
  m_sListMouseButtons << trUtf8("Left") << trUtf8("Middle") << trUtf8("Right");
  m_pUi->cbMoveBlockMouse->clear();
  m_pUi->cbMoveBlockMouse->addItems(m_sListMouseButtons);

  m_sListMouseButtons << trUtf8("First X") << trUtf8("Second X")
                      << trUtf8("Vertical wheel") << trUtf8("Horizontal wheel");
  m_pUi->cbRotateBlockMouse->clear();
  m_pUi->cbRotateBlockMouse->addItems(m_sListMouseButtons);
  m_pUi->cbFlipBlockMouse->clear();
  m_pUi->cbFlipBlockMouse->addItems(m_sListMouseButtons);

  m_pUi->cbMoveBlockMouse->setCurrentIndex(
        m_listMouseButtons.indexOf(m_listMouseControls[0]));
  m_pUi->cbRotateBlockMouse->setCurrentIndex(
        m_listMouseButtons.indexOf(m_listMouseControls[1]));
  m_pUi->cbFlipBlockMouse->setCurrentIndex(
        m_listMouseButtons.indexOf(m_listMouseControls[2]));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QStringList Settings::searchTranslations() {
  QStringList sList;
  QString sTmp;

  // Translations build in resources
  QDirIterator it(":", QStringList() << "*.qm",
                  QDir::NoDotAndDotDot | QDir::Files);
  qDebug() << qAppName().toLower() << qApp->applicationName().toLower();
  while (it.hasNext()) {
    it.next();
    sTmp = it.fileName();
    qDebug() << sTmp;

    if (sTmp.startsWith(qAppName().toLower() + "_") &&
        sTmp.endsWith(".qm")) {
      sList << sTmp.remove(qAppName().toLower() + "_").remove(".qm");
    }
  }

  // Check for additional translation files in share folder
  QDirIterator it2(m_sSharePath + "/lang", QStringList() << "*.qm",
                   QDir::NoDotAndDotDot | QDir::Files);
  while (it2.hasNext()) {
    it2.next();
    sTmp = it2.fileName();
    // qDebug() << sTmp;

    if (sTmp.startsWith(qAppName().toLower() + "_")) {
      sTmp = sTmp.remove(qAppName().toLower() + "_") .remove(".qm");
      if (!sList.contains(sTmp)) {
        sList << sTmp;
      }
    }
  }

  sList << "en";
  sList.sort();
  sList.push_front("auto");
  return sList;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QString Settings::getLanguage() {
  if ("auto" == m_sGuiLanguage) {
#ifdef Q_OS_UNIX
    QByteArray lang = qgetenv("LANG");
    if (!lang.isEmpty()) {
      return QLocale(lang).name();
    }
#endif
    return QLocale::system().name();
  } else if (!QFile(":/" + qApp->applicationName().toLower() +
                    "_" + m_sGuiLanguage + ".qm").exists() &&
             !QFile(m_sSharePath + "/lang/" +
                    qApp->applicationName().toLower() +
                    "_" + m_sGuiLanguage + ".qm").exists()) {
    m_sGuiLanguage = "en";
    m_pSettings->setValue("GuiLanguage", m_sGuiLanguage);
    return m_sGuiLanguage;
  }
  return m_sGuiLanguage;
}
