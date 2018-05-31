#include "3rdparty/qamqp/qamqpglobal.h"
#include "abstractrestapi.h"
#include "emailnotificationhandler.h"
#include "proofnetwork_global.h"
#include "proofnetwork_types.h"
#include "proofservicerestapi.h"
#include "smtpclient.h"

#include "proofcore/coreapplication.h"
#include "proofcore/errornotifier.h"
#include "proofcore/proofglobal.h"
#include "proofcore/settings.h"
#include "proofcore/settingsgroup.h"

Q_LOGGING_CATEGORY(proofNetworkMiscLog, "proof.network.misc")
Q_LOGGING_CATEGORY(proofNetworkExtraLog, "proof.network.extra")
Q_LOGGING_CATEGORY(proofNetworkAmqpLog, "proof.network.amqp")

PROOF_LIBRARY_INITIALIZER(libraryInit)
{
    // clang-format off
    qRegisterMetaType<Proof::RestApiError>("Proof::RestApiError");
    qRegisterMetaType<Proof::RestAuthType>("Proof::RestAuthType");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAMQP::Error>("QAMQP::Error");
    qRegisterMetaType<Proof::NetworkServices::VersionedEntityType>("Proof::NetworkServices::ApplicationType");
    // clang-format on

    Proof::CoreApplication::addInitializer([]() {
        Proof::SettingsGroup *notifierGroup = proofApp->settings()->group(QStringLiteral("error_notifier"),
                                                                          Proof::Settings::NotFoundPolicy::Add);

        QString appId = notifierGroup
                            ->value(QStringLiteral("app_id"), QStringLiteral(""), Proof::Settings::NotFoundPolicy::Add)
                            .toString();

        Proof::SettingsGroup *emailNotifierGroup = notifierGroup->group(QStringLiteral("email"),
                                                                        Proof::Settings::NotFoundPolicy::AddGlobal);
        if (emailNotifierGroup->value(QStringLiteral("enabled"), false, Proof::Settings::NotFoundPolicy::AddGlobal)
                .toBool()) {
            auto smtpClient = Proof::SmtpClientSP::create();

            QString from = emailNotifierGroup
                               ->value(QStringLiteral("from"), QStringLiteral(""),
                                       Proof::Settings::NotFoundPolicy::AddGlobal)
                               .toString();
            QString toString = emailNotifierGroup
                                   ->value(QStringLiteral("to"), QStringLiteral(""),
                                           Proof::Settings::NotFoundPolicy::AddGlobal)
                                   .toString();

            smtpClient->setHost(
                emailNotifierGroup
                    ->value(QStringLiteral("host"), QStringLiteral(""), Proof::Settings::NotFoundPolicy::AddGlobal)
                    .toString());
            smtpClient->setPort(
                emailNotifierGroup->value(QStringLiteral("port"), 25, Proof::Settings::NotFoundPolicy::AddGlobal).toInt());
            QString connectionType = emailNotifierGroup
                                         ->value(QStringLiteral("type"), QStringLiteral("starttls"),
                                                 Proof::Settings::NotFoundPolicy::AddGlobal)
                                         .toString()
                                         .toLower()
                                         .trimmed();
            if (connectionType == QLatin1String("ssl")) {
                smtpClient->setConnectionType(Proof::SmtpClient::ConnectionType::Ssl);
            } else if (connectionType == QLatin1String("starttls")) {
                smtpClient->setConnectionType(Proof::SmtpClient::ConnectionType::StartTls);
            } else {
                smtpClient->setConnectionType(Proof::SmtpClient::ConnectionType::Plain);
            }
            smtpClient->setUserName(
                emailNotifierGroup
                    ->value(QStringLiteral("username"), QStringLiteral(""), Proof::Settings::NotFoundPolicy::AddGlobal)
                    .toString());
            smtpClient->setPassword(
                emailNotifierGroup
                    ->value(QStringLiteral("password"), QStringLiteral(""), Proof::Settings::NotFoundPolicy::AddGlobal)
                    .toString());

            QStringList to;
            const auto splittedTo = toString.split(QStringLiteral("|"), QString::SkipEmptyParts);
            for (const auto &address : splittedTo) {
                QString trimmed = address.trimmed();
                if (!trimmed.isEmpty())
                    to << trimmed;
            }
            Proof::ErrorNotifier::instance()->registerHandler(
                new Proof::EmailNotificationHandler(smtpClient, from, to, appId));
        }
    });
}
