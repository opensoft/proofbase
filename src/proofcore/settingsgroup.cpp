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
#include "proofcore/settingsgroup.h"

#include "proofcore/proofobject_p.h"
#include "proofcore/settings.h"

#include <QHash>

namespace Proof {
class SettingsGroupPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(SettingsGroup)

    QHash<QString, QVariant> values;
    QHash<QString, SettingsGroup *> groups;

    QString name;
    SettingsGroup *globalGroup = nullptr;

    QSet<QString> valuesNames;
    QSet<QString> groupsNames;
};
} // namespace Proof

using namespace Proof;

SettingsGroup::SettingsGroup(const QString &name, SettingsGroup *globalGroup, QObject *parent)
    : ProofObject(*new SettingsGroupPrivate, parent)
{
    Q_D(SettingsGroup);
    d->name = name;
    d->globalGroup = globalGroup;

    if (d->globalGroup) {
        d->valuesNames = d->globalGroup->values();
        d->groupsNames = d->globalGroup->groups();
        connect(d->globalGroup, &Proof::SettingsGroup::valueChanged, this,
                [this, d](const QVector<QString> &key, const QVariant &value) {
                    // We don't care about sub values
                    if (key.count() > 1)
                        return;
                    // Add if changed, remove if deleted and doesn't exist in this group
                    bool emitNeeded = false;
                    if (value.isValid()) {
                        d->valuesNames << key.constFirst();
                        emitNeeded = !d->values.contains(key.constFirst());
                    } else if (!d->values.contains(key.constFirst())) {
                        d->valuesNames.remove(key.constFirst());
                        emitNeeded = true;
                    }
                    if (emitNeeded)
                        emit valueChanged(key, value, true);
                });
        connect(d->globalGroup, &Proof::SettingsGroup::groupAdded, this, [this, d](const QString &groupName) {
            d->groupsNames << groupName;
            if (!d->groups.contains(groupName))
                emit groupAdded(groupName);
        });
        connect(d->globalGroup, &Proof::SettingsGroup::groupRemoved, this, [this, d](const QString &groupName) {
            if (!d->groups.contains(groupName)) {
                d->groupsNames.remove(groupName);
                emit groupRemoved(groupName);
            } else if (d->groups[groupName]->groups().isEmpty() && d->groups[groupName]->values().isEmpty()) {
                deleteGroup(groupName);
            }
        });
    }
}

SettingsGroup::~SettingsGroup()
{}

QSet<QString> SettingsGroup::groups() const
{
    Q_D_CONST(SettingsGroup);
    return d->groupsNames;
}

QSet<QString> SettingsGroup::values() const
{
    Q_D_CONST(SettingsGroup);
    return d->valuesNames;
}

SettingsGroup *SettingsGroup::group(const QString &groupName, Settings::NotFoundPolicy notFoundPolicy)
{
    SettingsGroup *result;
    if (ProofObject::safeCall(this, &SettingsGroup::group, Proof::Call::Block, result, groupName, notFoundPolicy))
        return result;
    Q_D(SettingsGroup);
    result = d->groups.value(groupName, nullptr);
    if (!result && d->globalGroup) {
        result = d->globalGroup->group(groupName, notFoundPolicy == Settings::NotFoundPolicy::AddGlobal
                                                      ? Settings::NotFoundPolicy::Add
                                                      : Settings::NotFoundPolicy::DoNothing);
        if (result)
            result = addGroup(groupName);
    }
    if (!result && notFoundPolicy == Settings::NotFoundPolicy::Add)
        result = addGroup(groupName);
    return result;
}

QVariant SettingsGroup::value(const QString &key, const QVariant &defaultValue, Settings::NotFoundPolicy notFoundPolicy)
{
    QVariant result;
    if (ProofObject::safeCall(this, &SettingsGroup::value, Proof::Call::Block, result, key, defaultValue, notFoundPolicy))
        return result;
    Q_D(SettingsGroup);
    if (!d->values.contains(key)) {
        if (d->globalGroup) {
            if (d->globalGroup->values().contains(key))
                return d->globalGroup->value(key);
            if (notFoundPolicy == Settings::NotFoundPolicy::AddGlobal)
                return d->globalGroup->value(key, defaultValue, Settings::NotFoundPolicy::Add);
        }

        if (notFoundPolicy == Settings::NotFoundPolicy::Add)
            setValue(key, defaultValue);
        return defaultValue;
    }
    return d->values[key];
}

SettingsGroup *SettingsGroup::addGroup(const QString &groupName)
{
    SettingsGroup *result;
    if (ProofObject::safeCall(this, &SettingsGroup::addGroup, Proof::Call::Block, result, groupName))
        return result;
    Q_D(SettingsGroup);
    SettingsGroup *newGroup = d->groups.value(groupName, nullptr);
    if (!newGroup) {
        bool globalMatchingGroupWillBeCreated = d->globalGroup && !d->globalGroup->group(groupName);
        newGroup = new SettingsGroup(groupName,
                                     d->globalGroup ? d->globalGroup->group(groupName, Settings::NotFoundPolicy::Add)
                                                    : nullptr,
                                     this);
        d->groups[groupName] = newGroup;
        d->groupsNames << groupName;
        connect(newGroup, &SettingsGroup::valueChanged, this,
                [this, groupName](const QVector<QString> &key, const QVariant &value, bool inherited) {
                    QVector<QString> newKey{groupName};
                    newKey.append(key);
                    emit valueChanged(newKey, value, inherited);
                });

        // We don't need to emit this signal since it will be emitted by global group creation
        if (!globalMatchingGroupWillBeCreated)
            emit groupAdded(groupName);
        qCDebug(proofCoreSettingsLog) << "Group" << groupName << "was added as child of" << d->name;
    }

    return newGroup;
}

void SettingsGroup::setValue(const QString &key, const QVariant &value, Settings::Storage storage)
{
    if (ProofObject::safeCall(this, &SettingsGroup::setValue, Proof::Call::Block, key, value, storage))
        return;
    Q_D(SettingsGroup);
    if (storage == Settings::Storage::Global && d->globalGroup) {
        d->globalGroup->setValue(key, value);
        return;
    }
    QVariant oldValue;
    if (d->values.contains(key))
        oldValue = d->values[key];

    if (oldValue != value) {
        bool emitNeeded = true;
        if (value.isNull()) {
            d->values.remove(key);
            if (!d->globalGroup || !d->globalGroup->values().contains(key))
                d->valuesNames.remove(key);
        } else {
            emitNeeded = !d->globalGroup || d->globalGroup->value(key) != value;
            d->valuesNames << key;
            d->values[key] = value;
        }
        if (emitNeeded)
            emit valueChanged({key}, value, false);
        qCDebug(proofCoreSettingsLog) << "Group" << d->name << ": new value for key" << key << "is" << value
                                      << "old value was" << oldValue;
    }
}

void SettingsGroup::deleteGroup(const QString &groupName, Settings::Storage storage)
{
    if (ProofObject::safeCall(this, &SettingsGroup::deleteGroup, Proof::Call::Block, groupName, storage))
        return;
    Q_D(SettingsGroup);
    if (storage == Settings::Storage::Global && d->globalGroup) {
        d->globalGroup->deleteGroup(groupName);
        return;
    }
    if (d->groups.contains(groupName)) {
        auto groupToDelete = d->groups[groupName];
        const auto valuesToDelete = groupToDelete->values();
        for (const QString &toDelete : valuesToDelete)
            groupToDelete->deleteValue(toDelete);

        const auto subgroupsToDelete = groupToDelete->groups();
        for (const QString &toDelete : subgroupsToDelete)
            groupToDelete->deleteGroup(toDelete);

        //if global group with this name was empty then we can safely remove it
        if (d->globalGroup && d->globalGroup->group(groupName) && !d->globalGroup->group(groupName)->values().count()
            && !d->globalGroup->group(groupName)->groups().count()) {
            d->globalGroup->deleteGroup(groupName);
        }

        // It can be removed by globalGroup->deleteGroup
        if (d->groups.contains(groupName)) {
            d->groups.take(groupName)->deleteLater();
            if (!d->globalGroup || !d->globalGroup->groups().contains(groupName)) {
                d->groupsNames.remove(groupName);
                emit groupRemoved(groupName);
            }

            qCDebug(proofCoreSettingsLog) << "Group" << groupName << "was deleted";
        }
    }
}

void SettingsGroup::deleteValue(const QString &key, Settings::Storage storage)
{
    if (ProofObject::safeCall(this, &SettingsGroup::deleteValue, Proof::Call::Block, key, storage))
        return;
    Q_D(SettingsGroup);
    if (storage == Settings::Storage::Global && d->globalGroup) {
        d->globalGroup->deleteValue(key);
        return;
    }
    setValue(key, QVariant());
    qCDebug(proofCoreSettingsLog) << "Group:" << d->name << "value for key" << key << "was deleted";
}

void SettingsGroup::copyTo(SettingsGroup *destination)
{
    if (ProofObject::safeCall(this, &SettingsGroup::copyTo, Proof::Call::Block, destination))
        return;
    Q_D(SettingsGroup);
    if (!destination || destination == this)
        return;
    const auto srcValues = values();
    for (const QString &name : srcValues)
        destination->setValue(name, value(name));

    for (SettingsGroup *group : qAsConst(d->groups))
        group->copyTo(destination->addGroup(group->name()));
}

QString SettingsGroup::name() const
{
    Q_D_CONST(SettingsGroup);
    return d->name;
}
