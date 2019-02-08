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
#ifndef SETTINGSGROUP_H
#define SETTINGSGROUP_H

#include "proofcore/proofcore_global.h"
#include "proofcore/proofobject.h"
#include "proofcore/settings.h"

#include <QVariant>

namespace Proof {

class SettingsGroupPrivate;
class PROOF_CORE_EXPORT SettingsGroup : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SettingsGroup)
public:
    SettingsGroup() = delete;
    SettingsGroup(const SettingsGroup &) = delete;
    SettingsGroup(SettingsGroup &&) = delete;
    SettingsGroup &operator=(const SettingsGroup &) = delete;
    SettingsGroup &operator=(SettingsGroup &&) = delete;

    QSet<QString> groups() const;
    QSet<QString> values() const;

    SettingsGroup *group(const QString &groupName,
                         Settings::NotFoundPolicy notFoundPolicy = Settings::NotFoundPolicy::DoNothing);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant(),
                   Settings::NotFoundPolicy notFoundPolicy = Settings::NotFoundPolicy::DoNothing);

    SettingsGroup *addGroup(const QString &groupName);
    void setValue(const QString &key, const QVariant &value, Settings::Storage storage = Settings::Storage::Local);

    void deleteGroup(const QString &groupName, Settings::Storage storage = Settings::Storage::Local);
    void deleteValue(const QString &key, Settings::Storage storage = Settings::Storage::Local);

    //Destination must not overlap with current group
    void copyTo(SettingsGroup *destination);

    QString name() const;

signals:
    void groupAdded(const QString &groupName);
    void groupRemoved(const QString &groupName);
    void valueChanged(const QVector<QString> &key, const QVariant &value, bool inherited);

private:
    friend class Settings;
    SettingsGroup(const QString &name, SettingsGroup *globalGroup, QObject *parent);
    ~SettingsGroup();
};

} // namespace Proof

#endif // SETTINGSGROUP_H
