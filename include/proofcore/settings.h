#ifndef SETTINGS_H
#define SETTINGS_H

#include "proofcore/proofcore_global.h"
#include "proofcore/proofobject.h"

namespace Proof {

class SettingsGroup;
class SettingsPrivate;

class PROOF_CORE_EXPORT Settings : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Settings)
public:
    enum class NotFoundPolicy
    {
        DoNothing,
        Add,
        AddGlobal
    };

    enum class Storage
    {
        Local,
        Global
    };

    explicit Settings(QObject *parent = nullptr);
    ~Settings();

    void sync();
    SettingsGroup *mainGroup();
    QSet<QString> groups() const;
    SettingsGroup *group(const QString &groupName, NotFoundPolicy notFoundPolicy = NotFoundPolicy::DoNothing);
    SettingsGroup *addGroup(const QString &groupName);
    void deleteGroup(const QString &groupName, Settings::Storage storage = Settings::Storage::Local);
    static QString filePath(Storage storage = Storage::Local);
};

} // namespace Proof

#endif // SETTINGS_H
