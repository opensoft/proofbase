#include "settingsgroup.h"

#include <QHash>

#include "settings.h"

namespace Proof {
class SettingsGroupPrivate
{
    Q_DECLARE_PUBLIC(SettingsGroup)
    SettingsGroup *q_ptr;

    QHash<QString, QVariant> m_values;
    QHash<QString, SettingsGroup *> m_groups;

    QString m_name;
};
}

using namespace Proof;

SettingsGroup::SettingsGroup(const QString &name, QObject *parent)
    : QObject(parent), d_ptr(new SettingsGroupPrivate())
{
    d_ptr->q_ptr = this;
    d_ptr->m_name = name;
}

SettingsGroup::~SettingsGroup()
{

}

QStringList SettingsGroup::groups() const
{
    return d_ptr->m_groups.keys();
}

QStringList SettingsGroup::values() const
{
    return d_ptr->m_values.keys();
}

SettingsGroup *SettingsGroup::group(const QString &groupName)
{
    return d_ptr->m_groups.value(groupName, 0);
}

QVariant SettingsGroup::value(const QString &key, const QVariant &defaultValue) const
{
    return d_ptr->m_values.value(key, defaultValue);
}

SettingsGroup *SettingsGroup::addGroup(const QString &groupName)
{
    SettingsGroup *newGroup = d_ptr->m_groups.value(groupName, 0);
    if (!newGroup) {
        newGroup = new SettingsGroup(groupName, this);
        d_ptr->m_groups[groupName] = newGroup;
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
    QVariant oldValue;
    if (d_ptr->m_values.contains(key))
        oldValue = d_ptr->m_values[key];

    if (oldValue != value) {
        if (value.isNull())
            d_ptr->m_values.remove(key);
        else
            d_ptr->m_values[key] = value;
        emit valueChanged({key}, value);
    }
}

void SettingsGroup::deleteGroup(const QString &groupName)
{
    if (d_ptr->m_groups.contains(groupName))
        d_ptr->m_groups.take(groupName)->deleteLater();
}

void SettingsGroup::deleteValue(const QString &key)
{
    setValue(key, QVariant());
}

QString SettingsGroup::name() const
{
    return d_ptr->m_name;
}
