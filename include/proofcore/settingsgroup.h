#ifndef SETTINGSGROUP_H
#define SETTINGSGROUP_H

#include "proofobject.h"
#include "proofcore_global.h"

#include <QVariant>

namespace Proof {

class Settings;
class SettingsGroupPrivate;

class PROOF_CORE_EXPORT SettingsGroup : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SettingsGroup)
public:
    explicit SettingsGroup(const QString &name, QObject *parent = 0);

    QStringList groups() const;
    QStringList values() const;

    SettingsGroup *group(const QString &groupName);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    SettingsGroup *addGroup(const QString &groupName);
    void setValue(const QString &key, const QVariant &value);

    void deleteGroup(const QString &groupName);
    void deleteValue(const QString &key);

    QString name() const;

signals:
    void groupAdded(const QString &groupName);
    void valueChanged(const QStringList &key, const QVariant &value);
};

}

#endif // SETTINGSGROUP_H
