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

PROOF_LIBRARY_INITIALIZER(libraryInit)
{
    //clang-format off
    //clang-format on

    Proof::CoreApplication::addInitializer([]() {
        Proof::SettingsGroup *notifierGroup = proofApp->settings()->group(QStringLiteral("error_notifier"), Proof::Settings::NotFoundPolicy::Add);
        QString appId = notifierGroup->value(QStringLiteral("app_id"), QStringLiteral(""), Proof::Settings::NotFoundPolicy::Add).toString();
        Proof::ErrorNotifier::instance()->registerHandler(new Proof::MemoryStorageNotificationHandler(appId));
    });

    //error_notifier settings group migration
    Proof::CoreApplication::addMigration(Proof::packVersion(0, 17, 4, 10),
                                         [](quint64, quint64 oldProofVersion, Proof::Settings *settings) {
        if (oldProofVersion >= Proof::packVersion(0, 17, 4, 10))
            return;

        auto allGroups = settings->groups();
        if (!allGroups.contains(QStringLiteral("error_notifier"))) {
            auto errorNotifierGroup = settings->addGroup(QStringLiteral("error_notifier"));
            if (allGroups.contains(QStringLiteral("errors_notifier"))) {
                settings->group(QStringLiteral("errors_notifier"))->copyTo(errorNotifierGroup);
            } else if (allGroups.contains(QStringLiteral("email_notifier"))) {
                auto emailGroup = errorNotifierGroup->addGroup(QStringLiteral("email"));
                settings->group(QStringLiteral("email_notifier"))->copyTo(emailGroup);
                errorNotifierGroup->setValue(QStringLiteral("app_id"), emailGroup->value(QStringLiteral("app_id")));
                emailGroup->deleteValue(QStringLiteral("app_id"));
            }
        }
        settings->deleteGroup(QStringLiteral("errors_notifier"));
        settings->deleteGroup(QStringLiteral("email_notifier"));
    });

    //proof-common config population
    Proof::CoreApplication::addMigration(Proof::packVersion(0, 17, 7, 14),
                                         [](quint64, quint64 oldProofVersion, Proof::Settings *settings) {
        if (oldProofVersion >= Proof::packVersion(0, 17, 7, 14))
            return;

        const QStringList keys = {
            "authenticator/enabled",
            "authenticator/host",
            "authenticator/company",
            "logs/custom_storage_path",
            "mms/host",
            "mms/scheme",
            "mms/port",
            "mms/enabled",
            "barcode_scanner/prefix",
            "barcode_scanner/postfix",
            "barcode_scanner/min_length",
            "barcode_scanner/wait_timeout_in_msecs",
            "barcode_scanner/enabled",
            "demo_mode/windowed",
            "error_notifier/email/from",
            "error_notifier/email/to",
            "error_notifier/email/host",
            "error_notifier/email/port",
            "error_notifier/email/type",
            "error_notifier/email/username",
            "error_notifier/email/password",
            "error_notifier/email/enabled",
            "profit/use_https",
            "profit/ignore_ssl_errors",
            "profit/host",
            "profit/photos_url",
            "profit_reports/host",
            "ums/host",
            "network_proxy/type",
            "network_proxy/host",
            "network_proxy/port",
            "network_proxy/username",
            "network_proxy/password",
            "network_proxy/url",
            "network_proxy/excludes",
            "updates/sources_list_file"
        };

        for (const QString &fullKey : keys) {
            QStringList keyPath = fullKey.split("/");
            auto group = settings->mainGroup();
            for (int i = 0; i < keyPath.count() - 1 && group; ++i)
                group = group->group(keyPath[i]);
            if (!group)
                continue;

            const QString key = keyPath.constLast();
            if (!group->values().contains(key))
                continue;

            QVariant value = group->value(key);
            if (!value.isValid())
                continue;

            group->deleteValue(key, Proof::Settings::Storage::Local);
            if (group->values().contains(key))
                continue;

            group->setValue(key, value, Proof::Settings::Storage::Global);
        }
    });

    //label printer migration
    Proof::CoreApplication::addMigration(Proof::packVersion(0, 17, 11, 14),
                                         [](quint64, quint64 oldProofVersion, Proof::Settings *settings) {
        if (oldProofVersion >= Proof::packVersion(0, 17, 11, 14))
            return;

        auto printerGroup = settings->group("label_printer", Proof::Settings::NotFoundPolicy::DoNothing);

        if (!printerGroup)
            return;

        auto commonPrinters = settings->mainGroup()->value("label_printers", "", Proof::Settings::NotFoundPolicy::DoNothing).toString().split("|", QString::SkipEmptyParts);

        auto name = printerGroup->value("name", "", Proof::Settings::NotFoundPolicy::DoNothing).toString();
        if (name.isEmpty()) {
            settings->deleteGroup("label_printer", Proof::Settings::Storage::Local);
            return;
        }

        settings->mainGroup()->setValue("selected_printers", name, Proof::Settings::Storage::Local);

        if (commonPrinters.contains(name)) {
            settings->deleteGroup("label_printer", Proof::Settings::Storage::Local);
            return;
        }

        commonPrinters.append(name);
        settings->mainGroup()->setValue("label_printers", commonPrinters.join("|"), Proof::Settings::Storage::Global);
        auto newPrinterGroup = settings->group(name, Proof::Settings::NotFoundPolicy::AddGlobal);
        auto host = printerGroup->value("host", "", Proof::Settings::NotFoundPolicy::DoNothing).toString();
        auto port = printerGroup->value("port", "", Proof::Settings::NotFoundPolicy::DoNothing).toInt();
        auto binariesCheck = printerGroup->value("binaries_check", "", Proof::Settings::NotFoundPolicy::DoNothing).toBool();
        auto forceServiceUsage = printerGroup->value("force_service_usage", "", Proof::Settings::NotFoundPolicy::DoNothing).toBool();

        newPrinterGroup->setValue("name", name);
        newPrinterGroup->setValue("host", host);
        newPrinterGroup->setValue("port", port);
        newPrinterGroup->setValue("binaries_check", binariesCheck);
        newPrinterGroup->setValue("force_service_usage", forceServiceUsage);

        settings->deleteGroup("label_printer", Proof::Settings::Storage::Local);
    });
}
