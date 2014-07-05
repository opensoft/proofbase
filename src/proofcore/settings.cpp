#include "settings.h"

#include "proofobject_p.h"
#include "settingsgroup.h"

#include <QSettings>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

namespace Proof {
class SettingsPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(Settings)

    QSharedPointer<QSettings> openSettings();
    void groupValueChanged(const QStringList &key, const QVariant &value);

    bool isNativeFormatEnabled = false;
    SettingsGroup *mainGroup;
    QSharedPointer<QSettings> settings;
};
}

using namespace Proof;

Settings::Settings(QObject *parent)
    : ProofObject(*new SettingsPrivate, parent)
{
    Q_D(Settings);
    d->mainGroup = new SettingsGroup("", this);
    connect(d->mainGroup, &SettingsGroup::valueChanged, this,
            [d](const QStringList &key, const QVariant &value){d->groupValueChanged(key, value);});
    d->settings = d->openSettings();
}

bool Settings::isNativeFormatEnabled() const
{
    Q_D(const Settings);
    return d->isNativeFormatEnabled;
}

void Settings::setNativeFormatEnabled(bool arg)
{
    Q_D(Settings);
    if (d->isNativeFormatEnabled != arg) {
        d->isNativeFormatEnabled = arg;
        emit nativeFormatEnabledChanged(arg);
    }
}

void Settings::sync()
{
    Q_D(Settings);
    d->settings->sync();
}

SettingsGroup *Settings::mainGroup()
{
    Q_D(Settings);
    return d->mainGroup;
}

QSharedPointer<QSettings> SettingsPrivate::openSettings()
{
    QSettings *settingsPtr;

    if (isNativeFormatEnabled) {
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

    for (int i = 0; i < key.count() - 1; ++i)
        settings->beginGroup(key[i]);

    if (value.isNull())
        settings->remove(key.last());
    else
        settings->setValue(key.last(), value);

    for (int i = 0; i < key.count() - 1; ++i)
        settings->endGroup();
}

#include "moc_settings.cpp"
