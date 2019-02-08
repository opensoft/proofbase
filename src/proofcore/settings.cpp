/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "proofcore/settings.h"

#include "proofcore/proofobject_p.h"
#include "proofcore/settingsgroup.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>
#include <QSharedPointer>
#include <QStandardPaths>

namespace Proof {
class SettingsPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(Settings)

    static QString filePath(Settings::Storage storage);
    void openSettings();
    void readSettings();
    void fillGroupFromSettings(SettingsGroup *groupToFill, const QSharedPointer<QSettings> &settings);
    void groupValueChanged(const QVector<QString> &key, const QVariant &value, const QSharedPointer<QSettings> &settings);

    SettingsGroup *mainLocalGroup = nullptr;
    QSharedPointer<QSettings> localSettings;
    SettingsGroup *mainGlobalGroup = nullptr;
    QSharedPointer<QSettings> globalSettings;
};
} // namespace Proof

using namespace Proof;

Settings::Settings(QObject *parent) : ProofObject(*new SettingsPrivate, parent)
{
    Q_D(Settings);
    d->mainGlobalGroup = new SettingsGroup(QString(), nullptr, nullptr);
    d->mainLocalGroup = new SettingsGroup(QString(), d->mainGlobalGroup, nullptr);
    connect(d->mainLocalGroup, &SettingsGroup::valueChanged, this,
            [d](const QVector<QString> &key, const QVariant &value, bool inherited) {
                if (!inherited)
                    d->groupValueChanged(key, value, d->localSettings);
            });
    connect(d->mainGlobalGroup, &SettingsGroup::valueChanged, this,
            [d](const QVector<QString> &key, const QVariant &value) {
                d->groupValueChanged(key, value, d->globalSettings);
            });
    d->readSettings();
}

Settings::~Settings()
{
    Q_D(Settings);
    d->localSettings.clear();
    d->globalSettings.clear();
    // It is safer to delete them manually to make sure global one will be removed after local
    delete d->mainLocalGroup;
    delete d->mainGlobalGroup;
}

void Settings::sync()
{
    Q_D(Settings);
    d->localSettings->sync();
    d->globalSettings->sync();
    d->readSettings();
}

SettingsGroup *Settings::mainGroup()
{
    Q_D(Settings);
    return d->mainLocalGroup;
}

QSet<QString> Settings::groups() const
{
    Q_D_CONST(Settings);
    return d->mainLocalGroup->groups();
}

SettingsGroup *Settings::group(const QString &groupName, NotFoundPolicy notFoundPolicy)
{
    Q_D(Settings);
    return d->mainLocalGroup->group(groupName, notFoundPolicy);
}

SettingsGroup *Settings::addGroup(const QString &groupName)
{
    Q_D(Settings);
    return d->mainLocalGroup->addGroup(groupName);
}

void Settings::deleteGroup(const QString &groupName, Storage storage)
{
    Q_D(Settings);
    d->mainLocalGroup->deleteGroup(groupName, storage);
}

QString Settings::filePath(Storage storage)
{
    return SettingsPrivate::filePath(storage);
}

QString SettingsPrivate::filePath(Settings::Storage storage)
{
    if (qApp->arguments().contains(QStringLiteral("-c"))) {
        int index = qApp->arguments().indexOf(QStringLiteral("-c")) + 1;
        if (index < qApp->arguments().count()) {
            QString configPath = qApp->arguments().at(index);
            if (QFileInfo::exists(configPath))
                return configPath;
        }
    }
    QString appName = storage == Settings::Storage::Local ? qApp->applicationName() : QStringLiteral("proof-common");
#if defined Q_OS_WIN
    //Windows already gives us org/app as part of conf location
    QString configPath =
        QStringLiteral("%1/%2/%3.conf")
            .arg(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation), qApp->organizationName(),
                 appName + (storage == Settings::Storage::Local ? QStringLiteral("/%1").arg(appName) : QString()));
#elif defined Q_OS_ANDROID
    QString configPath = QStringLiteral("%1/%2/%3.conf")
                             .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation),
                                  qApp->organizationName(), appName);
#else
    QString configPath = QStringLiteral("%1/%2/%3.conf")
                             .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation),
                                  qApp->organizationName(), appName);
#endif
    return configPath;
}

void SettingsPrivate::openSettings()
{
    QString localConfigPath = filePath(Settings::Storage::Local);
    QString globalConfigPath = filePath(Settings::Storage::Global);
    localSettings = QSharedPointer<QSettings>::create(localConfigPath, QSettings::IniFormat);
    globalSettings = QSharedPointer<QSettings>::create(globalConfigPath, QSettings::IniFormat);
    qCDebug(proofCoreSettingsLog) << "Application Settings at:" << localConfigPath;
    qCDebug(proofCoreSettingsLog) << "Common Proof Settings are at:" << globalConfigPath;
}

void SettingsPrivate::readSettings()
{
    if (!localSettings || !globalSettings)
        openSettings();
    fillGroupFromSettings(mainGlobalGroup, globalSettings);
    fillGroupFromSettings(mainLocalGroup, localSettings);
}

void SettingsPrivate::fillGroupFromSettings(SettingsGroup *groupToFill, const QSharedPointer<QSettings> &settings)
{
    QSet<QString> toRemove = groupToFill->values();
    const auto childKeys = settings->childKeys();
    for (const QString &key : childKeys) {
        toRemove.remove(key);
        groupToFill->setValue(key, settings->value(key));
    }
    for (const QString &key : qAsConst(toRemove))
        groupToFill->deleteValue(key);

    toRemove = groupToFill->groups();
    const auto childGroups = settings->childGroups();
    for (const QString &key : childGroups) {
        toRemove.remove(key);
        SettingsGroup *group = groupToFill->addGroup(key);
        settings->beginGroup(key);
        fillGroupFromSettings(group, settings);
        settings->endGroup();
    }
    for (const QString &key : qAsConst(toRemove))
        groupToFill->deleteGroup(key);
}

void SettingsPrivate::groupValueChanged(const QVector<QString> &key, const QVariant &value,
                                        const QSharedPointer<QSettings> &settings)
{
    Q_ASSERT_X(key.count(), Q_FUNC_INFO, "key list can't be empty");

    QString groupPathToRestore = settings->group();
    QVector<QString> groupsToRestore;
    while (!settings->group().isEmpty()) {
        settings->endGroup();
        QString newPath = settings->group();
        groupsToRestore.prepend(newPath.isEmpty() ? groupPathToRestore : groupPathToRestore.mid(newPath.length() + 1));
        groupPathToRestore = newPath;
    }

    for (int i = 0; i < key.count() - 1; ++i)
        settings->beginGroup(key[i]);

    if (value.isNull())
        settings->remove(key.last());
    else
        settings->setValue(key.last(), value);

    for (int i = 0; i < key.count() - 1; ++i)
        settings->endGroup();

    for (const QString &groupToRestore : qAsConst(groupsToRestore))
        settings->beginGroup(groupToRestore);
}
