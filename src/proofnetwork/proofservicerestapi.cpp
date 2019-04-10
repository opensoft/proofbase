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
#include "proofnetwork/proofservicerestapi.h"

#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofservicerestapi_p.h"

#include <QJsonDocument>
#include <QJsonObject>

using namespace Proof;
using namespace Proof::NetworkServices;

using EntityTypeDict = QHash<QString, VersionedEntityType>;
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
Q_GLOBAL_STATIC_WITH_ARGS(EntityTypeDict, VERSIONED_ENTITY_TYPES,
                          ({{"station", VersionedEntityType::Station},
                            {"service", VersionedEntityType::Service},
                            {"framework", VersionedEntityType::Framework}}))

ProofServiceRestApi::ProofServiceRestApi(const RestClientSP &restClient,
                                         const QSharedPointer<ErrorMessagesRegistry> &errorsRegistry, QObject *parent)
    : ProofServiceRestApi(restClient, *new ProofServiceRestApiPrivate(errorsRegistry), parent)
{}

ProofServiceRestApi::ProofServiceRestApi(const RestClientSP &restClient, ProofServiceRestApiPrivate &dd, QObject *parent)
    : BaseRestApi(restClient, dd, parent)
{}

void ProofServiceRestApi::processSuccessfulReply(QNetworkReply *reply, const Promise<RestApiReply> &promise)
{
    Q_D(ProofServiceRestApi);
    int errorCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString serviceName;
    QString serviceVersion;
    QString serviceFrameworkVersion;
    for (const auto &header : reply->rawHeaderPairs()) {
        QString headerName = header.first;
        if (!headerName.startsWith(QLatin1String("proof-"), Qt::CaseInsensitive))
            continue;
        if (!headerName.compare(QLatin1String("proof-application"), Qt::CaseInsensitive)) {
            serviceName = header.second;
            continue;
        }
        QRegExp versionHeaderRegExp("proof-(.*-(service|station))(?:-(framework))?-version");
        versionHeaderRegExp.setCaseSensitivity(Qt::CaseInsensitive);
        if (versionHeaderRegExp.exactMatch(headerName)) {
            QString typeString =
                (versionHeaderRegExp.cap(3).isEmpty() ? versionHeaderRegExp.cap(2) : versionHeaderRegExp.cap(3)).toLower();
            auto type = VERSIONED_ENTITY_TYPES->value(typeString, VersionedEntityType::Unknown);
            emit versionFetched(type, versionHeaderRegExp.cap(1).toLower(), header.second);
            switch (type) {
            case VersionedEntityType::Service:
                serviceVersion = header.second;
                break;
            case VersionedEntityType::Framework:
                serviceFrameworkVersion = header.second;
                break;
            default:
                break;
            }
        }
    }
    if (!serviceName.isEmpty()) {
        qCDebug(proofNetworkMiscLog).nospace().noquote() << "Response from " << serviceName << " v." << serviceVersion
                                                         << " (Proof v." << serviceFrameworkVersion << ")";
    }

    if (400 <= errorCode && errorCode < 600) {
        QJsonParseError jsonError{};
        QJsonDocument content = QJsonDocument::fromJson(reply->readAll(), &jsonError);
        if (jsonError.error == QJsonParseError::NoError && content.isObject()) {
            QJsonValue serviceErrorCode = content.object().value(QStringLiteral("error_code"));
            if (serviceErrorCode.isDouble()) {
                QJsonArray jsonArgs = content.object().value(QStringLiteral("message_args")).toArray();
                auto args = algorithms::map(jsonArgs, [](const auto &x) { return x.toString(); }, QVector<QString>());
                ErrorInfo error = d->errorsRegistry->infoForCode(serviceErrorCode.toInt(), args);
                qCWarning(proofNetworkMiscLog)
                    << "Error occurred"
                    << reply->request().url().toDisplayString(QUrl::FormattingOptions(QUrl::FullyDecoded)) << ":"
                    << errorCode << error.proofModuleCode << error.proofErrorCode << error.message;
                promise.failure(Failure(error.message, error.proofModuleCode, error.proofErrorCode,
                                        error.userFriendly ? Failure::UserFriendlyHint : Failure::NoHint, errorCode));
                return;
            }
        }
    }
    BaseRestApi::processSuccessfulReply(reply, promise);
}

std::function<bool(const RestApiReply &)> ProofServiceRestApi::boolResultUnmarshaller()
{
    return [](const RestApiReply &reply) -> bool {
        QJsonParseError jsonError{};
        QJsonDocument doc = QJsonDocument::fromJson(reply.data, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            return WithFailure(QStringLiteral("JSON error: %1").arg(jsonError.errorString()), NETWORK_MODULE_CODE,
                               NetworkErrorCode::InvalidReply, Failure::NoHint, jsonError.error);
        }
        if (!doc.isObject()) {
            return WithFailure(QStringLiteral("Object is not found in document"), NETWORK_MODULE_CODE,
                               NetworkErrorCode::InvalidReply);
        }
        QJsonObject object = doc.object();
        if (!object[QStringLiteral("result")].isBool()) {
            return WithFailure(object[QStringLiteral("error")].toString(), NETWORK_MODULE_CODE,
                               NetworkErrorCode::InvalidReply);
        }
        return true;
    };
}

ProofServiceRestApiPrivate::ProofServiceRestApiPrivate(const QSharedPointer<ErrorMessagesRegistry> &errorsRegistry)
    : BaseRestApiPrivate(), errorsRegistry(errorsRegistry)
{}
