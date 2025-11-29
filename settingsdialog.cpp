// SPDX-FileCopyrightText: 2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "./settingsdialog.h"

#include <QDebug>
#include <QDirIterator>
#include <QMessageBox>

#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *pParent)
    : QDialog(pParent),
      m_pUi(new Ui::SettingsDialog()),
      m_pSettings(Settings::instance()) {
  m_pUi->setupUi(this);
  this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
  this->setModal(true);

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
                     << (quint8(Qt::Vertical) | Settings::SHIFT)
                     << (quint8(Qt::Horizontal) | Settings::SHIFT)
                     << ((quint32)Qt::LeftButton | Qt::ShiftModifier)
                     << ((quint32)Qt::LeftButton | Qt::ControlModifier)
                     << ((quint32)Qt::LeftButton | Qt::AltModifier)
                     << ((quint32)Qt::LeftButton | Qt::MetaModifier);
  m_pUi->cbRotateBlockMouse->addItems(m_sListMouseButtons);
  m_pUi->cbFlipBlockMouse->addItems(m_sListMouseButtons);

  m_pUi->cbGuiLanguage->addItems(this->searchTranslations());

  connect(m_pUi->buttonBox, &QDialogButtonBox::accepted, this,
          &SettingsDialog::accept);
  connect(m_pUi->buttonBox, &QDialogButtonBox::rejected, this,
          &QDialog::reject);
  this->readSettings();
}

SettingsDialog::~SettingsDialog() {
  delete m_pUi;
  m_pUi = nullptr;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::showEvent(QShowEvent *pEvent) {
  this->readSettings();
  QDialog::showEvent(pEvent);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::accept() {
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
  m_pSettings->setMouseControls(tmp_listMouseControls);

  m_pSettings->setGuiLanguage(m_pUi->cbGuiLanguage->currentText());

  m_pSettings->setUseSystemBackground(
      m_pUi->checkSystemBackground->isChecked());

  QDialog::accept();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::readSettings() {
  m_pUi->cbMoveBlockMouse->setCurrentIndex(
      m_listMouseButtons.indexOf(m_pSettings->getMouseControls().at(0)));
  m_pUi->cbRotateBlockMouse->setCurrentIndex(
      m_listMouseButtons.indexOf(m_pSettings->getMouseControls().at(1)));
  m_pUi->cbFlipBlockMouse->setCurrentIndex(
      m_listMouseButtons.indexOf(m_pSettings->getMouseControls().at(2)));

  QString sGuiLanguage = m_pSettings->getGuiLanguage();
  if (-1 != m_pUi->cbGuiLanguage->findText(sGuiLanguage)) {
    m_pUi->cbGuiLanguage->setCurrentIndex(
        m_pUi->cbGuiLanguage->findText(sGuiLanguage));
  } else {
    m_pUi->cbGuiLanguage->setCurrentIndex(
        m_pUi->cbGuiLanguage->findText(QStringLiteral("auto")));
    m_pSettings->setGuiLanguage(QStringLiteral("auto"));
  }

  m_pUi->checkSystemBackground->setChecked(
      m_pSettings->getUseSystemBackground());
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::updateUiLang() {
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
      m_listMouseButtons.indexOf(m_pSettings->getMouseControls().at(0)));
  m_pUi->cbRotateBlockMouse->setCurrentIndex(
      m_listMouseButtons.indexOf(m_pSettings->getMouseControls().at(1)));
  m_pUi->cbFlipBlockMouse->setCurrentIndex(
      m_listMouseButtons.indexOf(m_pSettings->getMouseControls().at(2)));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto SettingsDialog::searchTranslations() -> QStringList {
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
  QDirIterator it2(m_pSettings->getSharePath() + "/lang",
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
