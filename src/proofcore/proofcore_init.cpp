#include "proofcore_global.h"
#include "logs.h"
#include "proofglobal.h"
#include "proofcore/settings.h"
#include "proofcore/settingsgroup.h"
#include "proofcore/coreapplication.h"
#include "proofcore/errornotifier.h"
#include "proofcore/memorystoragenotificationhandler.h"
#include "proofcore/helpers/versionhelper.h"

Q_LOGGING_CATEGORY(proofCoreSettingsLog, "proof.core.settings")
Q_LOGGING_CATEGORY(proofCoreCrashLog, "proof.core.crash")
Q_LOGGING_CATEGORY(proofCoreLoggerLog, "proof.core.logger")
Q_LOGGING_CATEGORY(proofCoreUpdatesLog, "proof.core.updates")
Q_LOGGING_CATEGORY(proofCoreMiscLog, "proof.core.misc")
Q_LOGGING_CATEGORY(proofCoreTaskChainExtraLog, "proof.core.taskchain.extra")
Q_LOGGING_CATEGORY(proofCoreTaskChainStatsLog, "proof.core.taskchain.stats")
Q_LOGGING_CATEGORY(proofCoreCacheLog, "proof.core.cache")

namespace {
//TODO: move to Proof::Settings
void moveSettingsGroup(Proof::SettingsGroup *oldGroup, Proof::SettingsGroup *newGroup)
{
    if (!oldGroup || !newGroup)
        return;
    for (const QString &name : oldGroup->values()) {
        newGroup->setValue(name, oldGroup->value(name));
        oldGroup->setValue(name, QVariant());
    }

    for (const QString &groupName : oldGroup->groups())
        moveSettingsGroup(oldGroup->group(groupName), newGroup->addGroup(groupName));
}

void removeSettingsGroup(Proof::SettingsGroup *group)
{
    if (!group)
        return;
    for (const QString &name : group->values())
        group->setValue(name, QVariant());

    for (const QString &groupName : group->groups())
        removeSettingsGroup(group->group(groupName));
}
}

PROOF_LIBRARY_INITIALIZER(libraryInit)
{
    //clang-format off
    //clang-format on

    Proof::CoreApplication::addInitializer([]() {
        Proof::SettingsGroup *notifierGroup = proofApp->settings()->group("error_notifier", Proof::Settings::NotFoundPolicy::Add);
        QString appId = notifierGroup->value("app_id", "", Proof::Settings::NotFoundPolicy::Add).toString();
        Proof::ErrorNotifier::instance()->registerHandler(new Proof::MemoryStorageNotificationHandler(appId));
    });

    //error_notifier settings group migration
    Proof::CoreApplication::addMigration(Proof::packVersion(0, 17, 2, 7),
                                        [](quint64, quint64 oldProofVersion, Proof::Settings *settings) {
        if (oldProofVersion > Proof::packVersion(0, 17, 2, 7))
            return;

        auto allGroups = settings->groups();
        if (!allGroups.contains("error_notifier")) {
            auto errorNotifierGroup = settings->addGroup("error_notifier");
            if (allGroups.contains("errors_notifier")) {
                moveSettingsGroup(settings->group("errors_notifier"), errorNotifierGroup);
            } else if (allGroups.contains("email_notifier")) {
                auto emailGroup = errorNotifierGroup->addGroup("email");
                moveSettingsGroup(settings->group("email_notifier"), emailGroup);
                errorNotifierGroup->setValue("app_id", emailGroup->value("app_id"));
                emailGroup->setValue("app_id", QVariant());
            }
        }
        removeSettingsGroup(settings->group("errors_notifier"));
        removeSettingsGroup(settings->group("email_notifier"));
    });
}
