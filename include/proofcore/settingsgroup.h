#ifndef SETTINGSGROUP_H
#define SETTINGSGROUP_H

#include "proofcore/proofobject.h"
#include "proofcore/proofcore_global.h"

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

    SettingsGroup *group(const QString &groupName, bool createIfNotExists = false);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant(), bool createIfNotExists = false);

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
