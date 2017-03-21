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
#include "proofcore/notifier.h"

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
        Proof::SettingsGroup *notifierGroup = proofApp->settings()->group("errors_notifier", Proof::Settings::NotFoundPolicy::Add);

        QString appId = notifierGroup->value("app_id", "", Proof::Settings::NotFoundPolicy::Add).toString();

        Proof::SettingsGroup *emailNotifierGroup = notifierGroup->group("email", Proof::Settings::NotFoundPolicy::Add);
        auto smtpClient = Proof::SmtpClientSP::create();

        QString from = emailNotifierGroup->value("from", "", Proof::Settings::NotFoundPolicy::Add).toString();
        QString toString = emailNotifierGroup->value("to", "", Proof::Settings::NotFoundPolicy::Add).toString();

        smtpClient->setHost(emailNotifierGroup->value("host", "", Proof::Settings::NotFoundPolicy::Add).toString());
        smtpClient->setPort(emailNotifierGroup->value("port", 25, Proof::Settings::NotFoundPolicy::Add).toInt());
        QString connectionType = emailNotifierGroup->value("type", "ssl", Proof::Settings::NotFoundPolicy::Add).toString().toLower().trimmed();
        if (connectionType == "ssl") {
            smtpClient->setConnectionType(Proof::SmtpClient::ConnectionType::Ssl);
        } else if (connectionType == "starttls") {
            smtpClient->setConnectionType(Proof::SmtpClient::ConnectionType::StartTls);
        } else {
            smtpClient->setConnectionType(Proof::SmtpClient::ConnectionType::Plain);
            emailNotifierGroup->setValue("type", "plain");
        }
        smtpClient->setUserName(emailNotifierGroup->value("username", "", Proof::Settings::NotFoundPolicy::Add).toString());
        smtpClient->setPassword(emailNotifierGroup->value("password", "", Proof::Settings::NotFoundPolicy::Add).toString());

        if (emailNotifierGroup->value("enabled", false, Proof::Settings::NotFoundPolicy::Add).toBool()) {
            QStringList to;
            for (const auto &address : toString.split("|", QString::SkipEmptyParts)) {
                QString trimmed = address.trimmed();
                if (!trimmed.isEmpty())
                    to << trimmed;
            }
            Proof::Notifier::instance()->registerHandler(new Proof::EmailNotificationHandler(smtpClient, from, to, appId));
        }
    });
}
