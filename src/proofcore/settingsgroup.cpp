#include "settingsgroup.h"

#include "proofobject_p.h"
#include "settings.h"

#include <QHash>

namespace Proof {
class SettingsGroupPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(SettingsGroup)

    QHash<QString, QVariant> values;
    QHash<QString, SettingsGroup *> groups;

    QString name;
};
}

using namespace Proof;

SettingsGroup::SettingsGroup(const QString &name, QObject *parent)
    : ProofObject(*new SettingsGroupPrivate, parent)
{
    Q_D(SettingsGroup);
    d->name = name;
}

SettingsGroup::~SettingsGroup()
{
}

QStringList SettingsGroup::groups() const
{
    Q_D(const SettingsGroup);
    return d->groups.keys();
}

QStringList SettingsGroup::values() const
{
    Q_D(const SettingsGroup);
    return d->values.keys();
}

SettingsGroup *SettingsGroup::group(const QString &groupName, Settings::NotFoundPolicy notFoundPolicy)
{
    Q_D(SettingsGroup);
    SettingsGroup *result = d->groups.value(groupName, nullptr);
    if (!result && notFoundPolicy == Settings::NotFoundPolicy::Add)
        result = addGroup(groupName);
    return result;
}

QVariant SettingsGroup::value(const QString &key, const QVariant &defaultValue, Settings::NotFoundPolicy notFoundPolicy)
{
    Q_D(SettingsGroup);
    if (!d->values.contains(key)) {
        if (notFoundPolicy == Settings::NotFoundPolicy::Add)
            setValue(key, defaultValue);
        return defaultValue;
    }
    return d->values[key];
}

SettingsGroup *SettingsGroup::addGroup(const QString &groupName)
{
    Q_D(SettingsGroup);
    SettingsGroup *newGroup = d->groups.value(groupName, nullptr);
    if (!newGroup) {
        newGroup = new SettingsGroup(groupName, this);
        d->groups[groupName] = newGroup;
        connect(newGroup, &SettingsGroup::valueChanged,
                this, [this, groupName](const QStringList &key, const QVariant &value) {
            QStringList newKey {groupName};
            newKey.append(key);
            emit valueChanged(newKey, value);
        });
        emit groupAdded(groupName);
        qCDebug(proofCoreSettingsLog) << "Group" << groupName << "was added as child of" << d->name;
    }

    return newGroup;
}

void SettingsGroup::setValue(const QString &key, const QVariant &value)
{
    Q_D(SettingsGroup);
    QVariant oldValue;
    if (d->values.contains(key))
        oldValue = d->values[key];

    if (oldValue != value) {
        if (value.isNull())
            d->values.remove(key);
        else
            d->values[key] = value;
        emit valueChanged({key}, value);
        qCDebug(proofCoreSettingsLog) << "Group" << d->name << ": new value for key" << key
                                        << "is" << value << "old value was" << oldValue;
    }
}

void SettingsGroup::deleteGroup(const QString &groupName)
{
    Q_D(SettingsGroup);
    if (d->groups.contains(groupName)) {
        auto groupToDelete = d->groups[groupName];
        for (const QString &valueName : groupToDelete->values())
            groupToDelete->deleteValue(valueName);

        for (const QString &groupName : groupToDelete->groups())
            groupToDelete->deleteGroup(groupName);

        d->groups.take(groupName)->deleteLater();
        qCDebug(proofCoreSettingsLog) << "Group" << groupName << "was deleted";
    }
}

void SettingsGroup::deleteValue(const QString &key)
{
    Q_D(SettingsGroup);
    setValue(key, QVariant());
    qCDebug(proofCoreSettingsLog) << "Group:" << d->name << "value for key" << key << "was deleted";
}

void SettingsGroup::copyTo(SettingsGroup *destination)
{
    Q_D(SettingsGroup);
    if (!destination || destination == this)
        return;
    for (const QString &name : values())
        destination->setValue(name, value(name));

    for (SettingsGroup *group : d->groups)
        group->copyTo(destination->addGroup(group->name()));
}

QString SettingsGroup::name() const
{
    Q_D(const SettingsGroup);
    return d->name;
}
