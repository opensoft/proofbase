#ifndef SETTINGSGROUP_H
#define SETTINGSGROUP_H

#include <QObject>
#include <QVariant>

#include "proofcore_global.h"

namespace Proof {

class Settings;
class SettingsGroupPrivate;

class PROOF_CORE_EXPORT SettingsGroup : public QObject
{
    Q_OBJECT
public:
    explicit SettingsGroup(const QString &name, QObject *parent = 0);
    ~SettingsGroup();

    QStringList groups() const;
    QStringList values() const;

    SettingsGroup *group(const QString &groupName);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant());

    SettingsGroup *createGroup(const QString &groupName);
    void setValue(const QString &key, const QVariant &value);

    void deleteGroup(const QString &groupName);
    void deleteValue(const QString &key);

    QString name() const;

signals:
    void groupAdded(const QString &groupName);
    void valueChanged(const QStringList &key, const QVariant &value);

private:
    Q_DECLARE_PRIVATE(SettingsGroup)
    QScopedPointer<SettingsGroupPrivate> d_ptr;
};

}

#endif // SETTINGSGROUP_H
