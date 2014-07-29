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

    void openSettings();
    void readSettings();
    void fillGroupFromSettings(SettingsGroup *groupToFill);
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
    d->readSettings();
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

SettingsGroup *Settings::group(const QString &groupName, NotFoundPolicy notFoundPolicy)
{
    Q_D(Settings);
    return d->mainGroup->group(groupName, notFoundPolicy);
}

void SettingsPrivate::openSettings()
{
    if (isNativeFormatEnabled) {
        settings = QSharedPointer<QSettings>::create();
    } else {
        //TODO: check at all platforms
        QString configPath = QString("%1/%2/%3.conf")
                .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                .arg(QCoreApplication::organizationName())
                .arg(QCoreApplication::applicationName());
        settings = QSharedPointer<QSettings>::create(configPath, QSettings::IniFormat);
        qDebug() << Q_FUNC_INFO << "Settings at:" << configPath;
    }
}

void SettingsPrivate::readSettings()
{
    if (!settings)
        openSettings();
    fillGroupFromSettings(mainGroup);
}

void SettingsPrivate::fillGroupFromSettings(SettingsGroup *groupToFill)
{
    QSet<QString> toRemove = groupToFill->values().toSet();
    for (const QString &key : settings->childKeys()) {
        toRemove.remove(key);
        groupToFill->setValue(key, settings->value(key));
    }
    for (const QString &key : toRemove)
        groupToFill->deleteValue(key);

    toRemove = groupToFill->groups().toSet();
    for (const QString &key : settings->childGroups()) {
        toRemove.remove(key);
        SettingsGroup *group = groupToFill->addGroup(key);
        settings->beginGroup(key);
        fillGroupFromSettings(group);
        settings->endGroup();
    }
    for (const QString &key : toRemove)
        groupToFill->deleteGroup(key);
}

void SettingsPrivate::groupValueChanged(const QStringList &key, const QVariant &value)
{
    Q_ASSERT_X(key.count(), Q_FUNC_INFO, "key list can't be empty");

    QStringList groupsToRestore;
    while (!settings->group().isEmpty()) {
        groupsToRestore.prepend(settings->group());
        settings->endGroup();
    }

    for (int i = 0; i < key.count() - 1; ++i)
        settings->beginGroup(key[i]);

    if (value.isNull())
        settings->remove(key.last());
    else
        settings->setValue(key.last(), value);

    for (int i = 0; i < key.count() - 1; ++i)
        settings->endGroup();

    for (const QString &group : groupsToRestore)
        settings->beginGroup(group);
}

#include "moc_settings.cpp"
