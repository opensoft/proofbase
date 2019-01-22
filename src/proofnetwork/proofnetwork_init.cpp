/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "proofcore/coreapplication.h"
#include "proofcore/errornotifier.h"
#include "proofcore/proofglobal.h"
#include "proofcore/settings.h"
#include "proofcore/settingsgroup.h"

#include "proofnetwork/apicall.h"
#include "proofnetwork/emailnotificationhandler.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofservicerestapi.h"
#include "proofnetwork/smtpclient.h"

#include "3rdparty/qamqp/src/qamqpglobal.h"

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

        QString appId = proofApp->settings()
                            ->mainGroup()
                            ->value(QStringLiteral("app_id"), QString(), Proof::Settings::NotFoundPolicy::Add)
                            .toString();

        Proof::SettingsGroup *emailNotifierGroup = notifierGroup->group(QStringLiteral("email"),
                                                                        Proof::Settings::NotFoundPolicy::AddGlobal);
        if (emailNotifierGroup->value(QStringLiteral("enabled"), false, Proof::Settings::NotFoundPolicy::AddGlobal)
                .toBool()) {
            auto smtpClient = Proof::SmtpClientSP::create();

            QString from = emailNotifierGroup
                               ->value(QStringLiteral("from"), QString(), Proof::Settings::NotFoundPolicy::AddGlobal)
                               .toString();
            QString toString = emailNotifierGroup
                                   ->value(QStringLiteral("to"), QString(), Proof::Settings::NotFoundPolicy::AddGlobal)
                                   .toString();

            smtpClient->setHost(
                emailNotifierGroup->value(QStringLiteral("host"), QString(), Proof::Settings::NotFoundPolicy::AddGlobal)
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
                    ->value(QStringLiteral("username"), QString(), Proof::Settings::NotFoundPolicy::AddGlobal)
                    .toString());
            smtpClient->setPassword(
                emailNotifierGroup
                    ->value(QStringLiteral("password"), QString(), Proof::Settings::NotFoundPolicy::AddGlobal)
                    .toString());

            QStringList to;
            const auto splittedTo = toString.split(QStringLiteral("|"), QString::SkipEmptyParts);
            to.reserve(splittedTo.count());
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
