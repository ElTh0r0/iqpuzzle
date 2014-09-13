/**
 * \file CSettings.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012-2014 Thorsten Roth <elthoro@gmx.de>
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
#include <QDir>
#include <QMessageBox>

#include "./CSettings.h"
#include "ui_CSettings.h"

CSettings::CSettings(const QString &sSharePath, QWidget *pParent)
    : QDialog(pParent),
      m_sSharePath(sSharePath),
      m_nSHIFT(0xF0) {
    qDebug() << "Calling" << Q_FUNC_INFO;

    m_pUi = new Ui::CSettingsDialog();
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
    m_listMouseButtons << Qt::LeftButton << Qt::MiddleButton << Qt::RightButton;
    m_pUi->cbMoveBlockMouse->addItems(m_sListMouseButtons);
    m_sListMouseButtons << trUtf8("First X") << trUtf8("Second X")
                        << trUtf8("Verical wheel") << trUtf8("Horizontal wheel");
    m_listMouseButtons << Qt::XButton1 << Qt::XButton2
                       << (Qt::Vertical|m_nSHIFT) << (Qt::Horizontal|m_nSHIFT);
    m_pUi->cbRotateBlockMouse->addItems(m_sListMouseButtons);
    m_pUi->cbFlipBlockMouse->addItems(m_sListMouseButtons);

    // TODO: Add keyboard cpntrols
    m_pUi->radioKeyboardControls->setEnabled(false);

    connect(m_pUi->radioMouseControls, SIGNAL(toggled(bool)),
            this, SLOT(changedControls()));
    connect(m_pUi->radioKeyboardControls, SIGNAL(toggled(bool)),
            this, SLOT(changedControls()));
    connect(m_pUi->buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    this->readSettings();
}

CSettings::~CSettings() {
    if (m_pUi) {
        delete m_pUi;
        m_pUi = NULL;
    }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CSettings::accept() {
    qDebug() << "Calling" << Q_FUNC_INFO;

    QString sOldGuiLang = m_sGuiLanguage;
    m_sGuiLanguage = m_pUi->cbGuiLanguage->currentText();
    m_pSettings->setValue("GuiLanguage", m_sGuiLanguage);

    m_bMouseControl = m_pUi->radioMouseControls->isChecked();
    m_listMouseControls[0] = m_listMouseButtons[m_pUi->cbMoveBlockMouse->currentIndex()];
    m_listMouseControls[1] = m_listMouseButtons[m_pUi->cbRotateBlockMouse->currentIndex()];
    m_listMouseControls[2] = m_listMouseButtons[m_pUi->cbFlipBlockMouse->currentIndex()];
    m_pSettings->beginGroup("MouseControls");
    m_pSettings->setValue("Enabled", m_bMouseControl);
    m_pSettings->setValue("MoveBlock", m_listMouseControls[0]);
    m_pSettings->setValue("RotateBlock", m_listMouseControls[1]);
    m_pSettings->setValue("FlipBlock", m_listMouseControls[2]);
    m_pSettings->endGroup();

    m_bKeyboardControl = m_pUi->radioKeyboardControls->isChecked();
    m_pSettings->beginGroup("KeyboardControls");
    m_pSettings->setValue("Enabled", m_bKeyboardControl);
    m_pSettings->endGroup();

    if (sOldGuiLang != m_sGuiLanguage) {
        QMessageBox::information(0, this->windowTitle(),
                                 trUtf8("The game has to be restarted for "
                                        "applying the changes."));
    }

    QDialog::accept();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CSettings::readSettings() {
    m_sGuiLanguage = m_pSettings->value("GuiLanguage", "auto").toString();
    QStringList sListGuiLanguages;
    sListGuiLanguages << "auto" << "en";
    QDir appDir(m_sSharePath + "/lang");
    QFileInfoList fiListFiles = appDir.entryInfoList(
                QDir::NoDotAndDotDot | QDir::Files);
    foreach (QFileInfo fi, fiListFiles) {
        if ("qm" == fi.suffix() && fi.baseName().startsWith(qAppName() + "_")) {
            sListGuiLanguages << fi.baseName().remove(qAppName() + "_");
        }
    }
    m_pUi->cbGuiLanguage->addItems(sListGuiLanguages);
    if (-1 != m_pUi->cbGuiLanguage->findText(m_sGuiLanguage)) {
        m_pUi->cbGuiLanguage->setCurrentIndex(
                    m_pUi->cbGuiLanguage->findText(m_sGuiLanguage));
    } else {
        m_pUi->cbGuiLanguage->setCurrentIndex(
                    m_pUi->cbGuiLanguage->findText("auto"));
    }
    m_sGuiLanguage = m_pUi->cbGuiLanguage->currentText();

    m_listMouseControls.clear();
    m_listMouseControls << 0 << 0 << 0;
    m_pSettings->beginGroup("MouseControls");
    m_bMouseControl = m_pSettings->value("Enabled", true).toBool();
    m_pUi->radioMouseControls->setChecked(m_bMouseControl);
    m_listMouseControls[0] = m_pSettings->value("MoveBlock",
                                                Qt::LeftButton).toUInt();
    m_listMouseControls[1] = m_pSettings->value("RotateBlock",
                                                quint8(Qt::Vertical | m_nSHIFT)).toUInt();
    m_listMouseControls[2] = m_pSettings->value("FlipBlock",
                                                Qt::RightButton).toUInt();
    m_pUi->cbMoveBlockMouse->setCurrentIndex(
                m_listMouseButtons.indexOf(m_listMouseControls[0]));
    m_pUi->cbRotateBlockMouse->setCurrentIndex(
                m_listMouseButtons.indexOf(m_listMouseControls[1]));
    m_pUi->cbFlipBlockMouse->setCurrentIndex(
                m_listMouseButtons.indexOf(m_listMouseControls[2]));
    m_pSettings->endGroup();

    m_pSettings->beginGroup("KeyboardControls");
    m_bKeyboardControl = m_pSettings->value("Enabled", false).toBool();
    m_pUi->radioKeyboardControls->setChecked(m_bKeyboardControl);
    m_pSettings->endGroup();
    this->changedControls();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CSettings::changedControls() {
    if (m_pUi->radioMouseControls->isChecked()) {
        m_pUi->lblMoveBlock->setEnabled(true);
        m_pUi->cbMoveBlockMouse->setEnabled(true);
        m_pUi->lblRotateBlock->setEnabled(true);
        m_pUi->cbRotateBlockMouse->setEnabled(true);
        m_pUi->lblFlipBlock->setEnabled(true);
        m_pUi->cbFlipBlockMouse->setEnabled(true);

        m_pUi->lblSelectBlock->setEnabled(false);
        m_pUi->leSelectBlockKey->setEnabled(false);
        m_pUi->lblMoveLeft->setEnabled(false);
        m_pUi->leMoveLeftKey->setEnabled(false);
        m_pUi->lblMoveUp->setEnabled(false);
        m_pUi->leMoveUpKey->setEnabled(false);
        m_pUi->lblMoveRight->setEnabled(false);
        m_pUi->leMoveRightKey->setEnabled(false);
        m_pUi->lblMoveDown->setEnabled(false);
        m_pUi->leMoveDownKey->setEnabled(false);
        m_pUi->lblRotateBlockKey->setEnabled(false);
        m_pUi->leRotateBlockKey->setEnabled(false);
        m_pUi->lblFlipBlockKey->setEnabled(false);
        m_pUi->leFlipBlockKey->setEnabled(false);
    } else if (m_pUi->radioKeyboardControls->isChecked()) {
        m_pUi->lblMoveBlock->setEnabled(false);
        m_pUi->cbMoveBlockMouse->setEnabled(false);
        m_pUi->lblRotateBlock->setEnabled(false);
        m_pUi->cbRotateBlockMouse->setEnabled(false);
        m_pUi->lblFlipBlock->setEnabled(false);
        m_pUi->cbFlipBlockMouse->setEnabled(false);

        m_pUi->lblSelectBlock->setEnabled(true);
        m_pUi->leSelectBlockKey->setEnabled(true);
        m_pUi->lblMoveLeft->setEnabled(true);
        m_pUi->leMoveLeftKey->setEnabled(true);
        m_pUi->lblMoveUp->setEnabled(true);
        m_pUi->leMoveUpKey->setEnabled(true);
        m_pUi->lblMoveRight->setEnabled(true);
        m_pUi->leMoveRightKey->setEnabled(true);
        m_pUi->lblMoveDown->setEnabled(true);
        m_pUi->leMoveDownKey->setEnabled(true);
        m_pUi->lblRotateBlockKey->setEnabled(true);
        m_pUi->leRotateBlockKey->setEnabled(true);
        m_pUi->lblFlipBlockKey->setEnabled(true);
        m_pUi->leFlipBlockKey->setEnabled(true);
    }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

bool CSettings::getUseMouse() const {
    return m_bMouseControl;
}

quint8 CSettings::getShift() const {
    return m_nSHIFT;
}

QList<quint8> CSettings::getMouseControls() const {
    return m_listMouseControls;
}

bool CSettings::getUseKeyboard() const {
    return m_bKeyboardControl;
}
