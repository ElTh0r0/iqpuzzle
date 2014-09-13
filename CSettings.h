/**
 * \file CSettings.h
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
 * Class definition for settings.
 */

#ifndef IQPUZZLE_CSETTINGS_H_
#define IQPUZZLE_CSETTINGS_H_

#include <QDialog>
#include <QSettings>

#include "./CSettings.h"

namespace Ui {
    class CSettingsDialog;
}

/**
 * \class CSettings
 * \brief Settings dialog.
 */
class CSettings : public QDialog {
    Q_OBJECT

  public:
    explicit CSettings(const QString &sSharePath, QWidget *pParent = 0);
    virtual ~CSettings();

    bool getUseMouse() const;
    QList<quint8> getMouseControls() const;
    quint8 getShift() const;
    bool getUseKeyboard() const;

  public slots:
    void accept();

  private slots:
    void changedControls();

  private:
    void readSettings();

    QWidget *m_pParent;
    Ui::CSettingsDialog *m_pUi;
    QSettings *m_pSettings;

    QString m_sSharePath;
    QString m_sGuiLanguage;
    bool m_bMouseControl;
    const quint8 m_nSHIFT;
    QStringList m_sListMouseButtons;
    QList<quint8> m_listMouseButtons;
    QList<quint8> m_listMouseControls;
    bool m_bKeyboardControl;
};

#endif  // IQPUZZLE_CSETTINGS_H_
