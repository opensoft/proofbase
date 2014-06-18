#include "settings.h"

#include <QSettings>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

#include "settingsgroup.h"

namespace Proof {
class SettingsPrivate
{
    QSharedPointer<QSettings> openSettings();
    void groupValueChanged(const QStringList &key, const QVariant &value);

    Q_DECLARE_PUBLIC(Settings)
    Settings *q_ptr;

    bool m_isNativeFormatEnabled = false;

    SettingsGroup *m_mainGroup;

    QSharedPointer<QSettings> m_settings;
};

}

using namespace Proof;

Settings::Settings(QObject *parent)
    : QObject(parent), d_ptr(new SettingsPrivate())
{
    d_ptr->q_ptr = this;
    d_ptr->m_mainGroup = new SettingsGroup("", this);
    connect(d_ptr->m_mainGroup, &SettingsGroup::valueChanged, this,
            [this](const QStringList &key, const QVariant &value){d_ptr->groupValueChanged(key, value);});
    d_ptr->m_settings = d_ptr->openSettings();
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

void Settings::sync()
{
    d_ptr->m_settings->sync();
}

SettingsGroup *Settings::mainGroup()
{
    return d_ptr->m_mainGroup;
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

void SettingsPrivate::groupValueChanged(const QStringList &key, const QVariant &value)
{
    qDebug() << Q_FUNC_INFO << key << value;
    Q_ASSERT_X(key.count(), Q_FUNC_INFO, "key list can't be empty");
    for (int i = 0; i < key.count()-1; ++i)
        m_settings->beginGroup(key[i]);
    if (value.isNull())
        m_settings->remove(key.last());
    else
        m_settings->setValue(key.last(), value);
    for (int i = 0; i < key.count()-1; ++i)
        m_settings->endGroup();
}

#include "moc_settings.cpp"
