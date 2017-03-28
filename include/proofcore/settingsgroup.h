#ifndef SETTINGSGROUP_H
#define SETTINGSGROUP_H

#include "proofcore/settings.h"
#include "proofcore/proofobject.h"
#include "proofcore/proofcore_global.h"

#include <QVariant>

namespace Proof {

class SettingsGroupPrivate;
class PROOF_CORE_EXPORT SettingsGroup : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SettingsGroup)
public:
    QStringList groups() const;
    QStringList values() const;

    SettingsGroup *group(const QString &groupName,
                         Settings::NotFoundPolicy notFoundPolicy = Settings::NotFoundPolicy::DoNothing);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant(),
                   Settings::NotFoundPolicy notFoundPolicy = Settings::NotFoundPolicy::DoNothing);

    SettingsGroup *addGroup(const QString &groupName);
    void setValue(const QString &key, const QVariant &value);

    void deleteGroup(const QString &groupName);
    void deleteValue(const QString &key);

    //Destination must not overlap with current group
    void copyTo(SettingsGroup *destination);

    QString name() const;

signals:
    void groupAdded(const QString &groupName);
    void valueChanged(const QStringList &key, const QVariant &value);

private:
    friend class Settings;
    SettingsGroup() = delete;
    SettingsGroup(const QString &name, QObject *parent);
    ~SettingsGroup();
};

}

#endif // SETTINGSGROUP_H
