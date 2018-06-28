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
    SettingsGroup() = delete;
    SettingsGroup(const QString &name, SettingsGroup *globalGroup, QObject *parent);
    ~SettingsGroup();
};

} // namespace Proof

#endif // SETTINGSGROUP_H
