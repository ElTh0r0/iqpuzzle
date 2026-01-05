// SPDX-FileCopyrightText: 2014-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "./settings.h"

#include <QDebug>
#include <QDir>
#include <QGuiApplication>

Settings *Settings::instance() {
  static Settings _instance;
  return &_instance;
}

Settings::Settings(QObject *pParent)
    : QObject(pParent),
#if defined _WIN32
      m_settings(QSettings::IniFormat, QSettings::UserScope,
                 qApp->applicationName().toLower(),
                 qApp->applicationName().toLower())
#else
      m_settings(QSettings::NativeFormat, QSettings::UserScope,
                 qApp->applicationName().toLower(),
                 qApp->applicationName().toLower())
#endif
{
}

// ----------------------------------------------------------------------------

auto Settings::getSharePath() const -> QString { return m_sSharePath; }

void Settings::setSharePath(const QString &sPath) { m_sSharePath = sPath; }

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getMouseControls() -> QList<uint> {
  QList<uint> listMouseControls;

  m_settings.beginGroup(QStringLiteral("MouseControls"));
  listMouseControls
      << m_settings.value(QStringLiteral("MoveBlock"), Qt::LeftButton).toUInt();
  listMouseControls << m_settings
                           .value(QStringLiteral("RotateBlock"),
                                  (quint8(Qt::Vertical) | Settings::SHIFT))
                           .toUInt();
  listMouseControls << m_settings
                           .value(QStringLiteral("FlipBlock"), Qt::RightButton)
                           .toUInt();
  m_settings.endGroup();

  return listMouseControls;
}

void Settings::setMouseControls(const QList<uint> &mouseControls) {
  if (mouseControls.size() == 3) {
    m_settings.beginGroup(QStringLiteral("MouseControls"));
    m_settings.setValue(QStringLiteral("MoveBlock"), mouseControls[0]);
    m_settings.setValue(QStringLiteral("RotateBlock"), mouseControls[1]);
    m_settings.setValue(QStringLiteral("FlipBlock"), mouseControls[2]);
    m_settings.remove(QStringLiteral("Enabled"));
    m_settings.endGroup();
  } else {
    qWarning() << "Mouse controls couldn't be saved!";
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getGuiLanguage() -> QString {
  QString sGuiLanguage =
      m_settings.value(QStringLiteral("GuiLanguage"), QStringLiteral("auto"))
          .toString();

  if ("auto" == sGuiLanguage) {
#ifdef Q_OS_UNIX
    QByteArray lang = qgetenv("LANG");
    if (!lang.isEmpty()) {
      return QLocale(QString::fromLatin1(lang)).name();
    }
#endif
    return QLocale::system().name();
  }
  if (!QFile(":/" + qApp->applicationName().toLower() + "_" + sGuiLanguage +
             ".qm")
           .exists() &&
      !QFile(this->getSharePath() + "/lang/" +
             qApp->applicationName().toLower() + "_" + sGuiLanguage + ".qm")
           .exists()) {
    sGuiLanguage = QStringLiteral("en");
    m_settings.setValue(QStringLiteral("GuiLanguage"), sGuiLanguage);
    return sGuiLanguage;
  }
  return sGuiLanguage;
}

void Settings::setGuiLanguage(const QString &sGuiLanguage) {
  QString sOldGuiLang = this->getGuiLanguage();
  m_settings.setValue(QStringLiteral("GuiLanguage"), sGuiLanguage);
  if (sOldGuiLang != sGuiLanguage) {
    emit changeGuiLanguage(this->getGuiLanguage());
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getUseSystemBackground() const -> bool {
  return m_settings.value(QStringLiteral("UseSystemBackground"), false)
      .toBool();
}

void Settings::setUseSystemBackground(const bool bUseSystemBackground) {
  bool bOldValue = this->getUseSystemBackground();
  m_settings.setValue(QStringLiteral("UseSystemBackground"),
                      bUseSystemBackground);
  if (bOldValue != bUseSystemBackground) {
    emit updateUseSystemBackgroundColor(bUseSystemBackground);
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getLastOpenedDir() -> QString {
  return m_settings
      .value(QStringLiteral("LastOpenedDir"), QString(QDir::homePath()))
      .toString();
}

void Settings::setLastOpenedDir(const QString &sLastOpenedDir) {
  m_settings.setValue(QStringLiteral("LastOpenedDir"), sLastOpenedDir);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getThresholdEasy() const -> uint {
  uint nEasy =
      m_settings
          .value(QStringLiteral("ThresholdEasy"), Settings::THRESHOLD_EASY)
          .toUInt();
  if (0 == nEasy) nEasy = Settings::THRESHOLD_EASY;

  return nEasy;
}

auto Settings::getThresholdHard() const -> uint {
  uint nHard =
      m_settings
          .value(QStringLiteral("ThresholdHard"), Settings::THRESHOLD_HARD)
          .toUInt();
  if (0 == nHard) nHard = Settings::THRESHOLD_HARD;

  return nHard;
}
