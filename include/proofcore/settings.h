#ifndef SETTINGS_H
#define SETTINGS_H

#include "proofcore/proofobject.h"
#include "proofcore/proofcore_global.h"

namespace Proof {

class SettingsGroup;
class SettingsPrivate;

class PROOF_CORE_EXPORT Settings : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Settings)
public:
    enum class NotFoundPolicy {
        DoNothing,
        Add
    };

    explicit Settings(QObject *parent = nullptr);
    ~Settings();

    void sync();
    SettingsGroup *mainGroup();
    QStringList groups() const;
    SettingsGroup *group(const QString &groupName, NotFoundPolicy notFoundPolicy = NotFoundPolicy::DoNothing);
    SettingsGroup *addGroup(const QString &groupName);
    static QString filePath();

};

}

#endif // SETTINGS_H
