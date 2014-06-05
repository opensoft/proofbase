#include "settings.h"

#include <QSettings>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QCoreApplication>

namespace Proof {
class SettingsPrivate
{
    explicit SettingsPrivate(Settings *settings);

    QSharedPointer<QSettings> openSettings();

    Q_DECLARE_PUBLIC(Settings)
    Settings *q_ptr;

    bool m_isNativeFormatEnabled;
    bool m_isAutoSaveEnabled;
};

}

using namespace Proof;

Settings::Settings(QObject *parent)
    : QObject(parent), d_ptr(new SettingsPrivate(this))
{
}

Settings::~Settings()
{
}

bool Settings::isNativeFormatEnabled() const
{
    return d_ptr->m_isNativeFormatEnabled;
}

void Settings::setNativeFormatEnabled(bool arg)
{
    if (d_ptr->m_isNativeFormatEnabled != arg) {
        d_ptr->m_isNativeFormatEnabled = arg;
        emit nativeFormatEnabledChanged(arg);
    }
}

bool Settings::isAutoSaveEnabled() const
{
    return d_ptr->m_isAutoSaveEnabled;
}

void Settings::setAutoSaveEnabled(bool arg)
{
    if (d_ptr->m_isAutoSaveEnabled != arg) {
        d_ptr->m_isAutoSaveEnabled = arg;
        emit autoSaveEnabledChanged(arg);
    }
}

void Settings::save()
{

}

void Settings::load()
{

}

SettingsPrivate::SettingsPrivate(Settings *settings)
    : m_isNativeFormatEnabled(false), m_isAutoSaveEnabled(true)
{
    q_ptr = settings;
    openSettings();
}

QSharedPointer<QSettings> SettingsPrivate::openSettings()
{
    QSettings *settingsPtr;

    if (m_isNativeFormatEnabled) {
        settingsPtr = new QSettings();
    } else {
        //TODO: check at all platforms
        QString configPath = QString("%1/%2/%3.conf")
                .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                .arg(QCoreApplication::organizationName())
                .arg(QCoreApplication::applicationName());
        settingsPtr = new QSettings(configPath, QSettings::IniFormat);
    }

    return QSharedPointer<QSettings>(settingsPtr);
}
