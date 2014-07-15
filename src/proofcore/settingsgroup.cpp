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

SettingsGroup *SettingsGroup::group(const QString &groupName, bool createIfNotExists)
{
    Q_D(SettingsGroup);
    SettingsGroup *result = d->groups.value(groupName, 0);
    if (!result && createIfNotExists)
        result = addGroup(groupName);
    return result;
}

QVariant SettingsGroup::value(const QString &key, const QVariant &defaultValue, bool createIfNotExists)
{
    Q_D(SettingsGroup);
    if (!d->values.contains(key)) {
        if (createIfNotExists)
            setValue(key, defaultValue);
        return defaultValue;
    }
    return d->values[key];
}

SettingsGroup *SettingsGroup::addGroup(const QString &groupName)
{
    Q_D(SettingsGroup);
    SettingsGroup *newGroup = d->groups.value(groupName, 0);
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
    }
}

void SettingsGroup::deleteGroup(const QString &groupName)
{
    Q_D(SettingsGroup);
    if (d->groups.contains(groupName))
        d->groups.take(groupName)->deleteLater();
}

void SettingsGroup::deleteValue(const QString &key)
{
    setValue(key, QVariant());
}

QString SettingsGroup::name() const
{
    Q_D(const SettingsGroup);
    return d->name;
}
