#include "abstractrestapi.h"
#include "proofnetwork_global.h"
#include "proofnetwork_types.h"
#include "proofservicerestapi.h"
#include "emailnotificationhandler.h"
#include "smtpclient.h"
#include "3rdparty/qamqp/qamqpglobal.h"
#include "proofcore/proofglobal.h"
#include "proofcore/settings.h"
#include "proofcore/settingsgroup.h"
#include "proofcore/coreapplication.h"
#include "proofcore/errornotifier.h"

Q_LOGGING_CATEGORY(proofNetworkMiscLog, "proof.network.misc")
Q_LOGGING_CATEGORY(proofNetworkAmqpLog, "proof.network.amqp")

PROOF_LIBRARY_INITIALIZER(libraryInit)
{
    //clang-format off
    qRegisterMetaType<Proof::RestApiError>("Proof::RestApiError");
    qRegisterMetaType<Proof::RestAuthType>("Proof::RestAuthType");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAMQP::Error>("QAMQP::Error");
    qRegisterMetaType<Proof::NetworkServices::VersionedEntityType>("Proof::NetworkServices::ApplicationType");
    //clang-format on

    Proof::CoreApplication::addInitializer([]() {
        Proof::SettingsGroup *notifierGroup = proofApp->settings()->group(QStringLiteral("error_notifier"), Proof::Settings::NotFoundPolicy::Add);

        QString appId = notifierGroup->value(QStringLiteral("app_id"), QStringLiteral(""), Proof::Settings::NotFoundPolicy::Add).toString();

        Proof::SettingsGroup *emailNotifierGroup = notifierGroup->group(QStringLiteral("email"), Proof::Settings::NotFoundPolicy::Add);
        auto smtpClient = Proof::SmtpClientSP::create();

        QString from = emailNotifierGroup->value(QStringLiteral("from"), QStringLiteral(""), Proof::Settings::NotFoundPolicy::Add).toString();
        QString toString = emailNotifierGroup->value(QStringLiteral("to"), QStringLiteral(""), Proof::Settings::NotFoundPolicy::Add).toString();

        smtpClient->setHost(emailNotifierGroup->value(QStringLiteral("host"), QStringLiteral(""), Proof::Settings::NotFoundPolicy::Add).toString());
        smtpClient->setPort(emailNotifierGroup->value(QStringLiteral("port"), 25, Proof::Settings::NotFoundPolicy::Add).toInt());
        QString connectionType = emailNotifierGroup->value(QStringLiteral("type"), QStringLiteral("ssl"), Proof::Settings::NotFoundPolicy::Add).toString().toLower().trimmed();
        if (connectionType == QLatin1String("ssl")) {
            smtpClient->setConnectionType(Proof::SmtpClient::ConnectionType::Ssl);
        } else if (connectionType == QLatin1String("starttls")) {
            smtpClient->setConnectionType(Proof::SmtpClient::ConnectionType::StartTls);
        } else {
            smtpClient->setConnectionType(Proof::SmtpClient::ConnectionType::Plain);
            emailNotifierGroup->setValue(QStringLiteral("type"), QStringLiteral("plain"));
        }
        smtpClient->setUserName(emailNotifierGroup->value(QStringLiteral("username"), QStringLiteral(""), Proof::Settings::NotFoundPolicy::Add).toString());
        smtpClient->setPassword(emailNotifierGroup->value(QStringLiteral("password"), QStringLiteral(""), Proof::Settings::NotFoundPolicy::Add).toString());

        if (emailNotifierGroup->value(QStringLiteral("enabled"), false, Proof::Settings::NotFoundPolicy::Add).toBool()) {
            QStringList to;
            const auto splittedTo = toString.split(QStringLiteral("|"), QString::SkipEmptyParts);
            for (const auto &address : splittedTo) {
                QString trimmed = address.trimmed();
                if (!trimmed.isEmpty())
                    to << trimmed;
            }
            Proof::ErrorNotifier::instance()->registerHandler(new Proof::EmailNotificationHandler(smtpClient, from, to, appId));
        }
    });
}
