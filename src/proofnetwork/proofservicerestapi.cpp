#include "proofservicerestapi.h"

#include "proofservicerestapi_p.h"

#include "proofnetwork_types.h"

#include <QJsonDocument>
#include <QJsonObject>

static const QHash<QString, Proof::NetworkServices::VersionedEntityType> VERSIONED_ENTITY_TYPES = {
    {"station", Proof::NetworkServices::VersionedEntityType::Station},
    {"service", Proof::NetworkServices::VersionedEntityType::Service},
    {"framework", Proof::NetworkServices::VersionedEntityType::Framework}
};
namespace Proof {
namespace NetworkServices {

ProofServiceRestApi::ProofServiceRestApi(const RestClientSP &restClient, ProofServiceRestApiPrivate &dd, QObject *parent)
    : AbstractRestApi(restClient, dd, parent)
{
}

ProofServiceRestApiPrivate::ProofServiceRestApiPrivate(const QSharedPointer<ErrorMessagesRegistry> &errorsRegistry)
    : AbstractRestApiPrivate(), errorsRegistry(errorsRegistry)
{
}

void ProofServiceRestApiPrivate::replyFinished(qulonglong operationId, QNetworkReply *reply, bool forceUserFriendly)
{
    Q_Q(ProofServiceRestApi);
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
            QString typeString = (versionHeaderRegExp.cap(3).isEmpty() ? versionHeaderRegExp.cap(2) : versionHeaderRegExp.cap(3)).toLower();
            auto type = VERSIONED_ENTITY_TYPES.value(typeString, VersionedEntityType::Unknown);
            emit q->versionFetched(type, versionHeaderRegExp.cap(1).toLower(), header.second);
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
    if (!serviceName.isEmpty())
        qCDebug(proofNetworkMiscLog).nospace().noquote() << "Response from " << serviceName << " v." << serviceVersion << " (Proof v." << serviceFrameworkVersion << ")";

    if (400 <= errorCode && errorCode < 600) {
        QJsonParseError jsonError;
        QJsonDocument content = QJsonDocument::fromJson(reply->readAll(), &jsonError);
        if (jsonError.error == QJsonParseError::NoError && content.isObject()) {
            QJsonValue serviceErrorCode = content.object().value(QStringLiteral("error_code"));
            if (serviceErrorCode.isDouble()) {
                QJsonArray jsonArgs = content.object().value(QStringLiteral("message_args")).toArray();
                QStringList args;
                for (const auto &arg : jsonArgs)
                    args << arg.toString();
                ErrorInfo errorInfo = errorsRegistry->infoForCode(serviceErrorCode.toInt(), args);
                qCDebug(proofNetworkMiscLog) << "Error in JSON parsing occurred for" << operationId
                                             << reply->request().url().toDisplayString(QUrl::FormattingOptions(QUrl::FullyDecoded))
                                             << ": " << errorCode
                                             << errorInfo.proofModuleCode << errorInfo.proofErrorCode << errorInfo.message;
                emit q->apiErrorOccurred(operationId, RestApiError{RestApiError::Level::ServerError, errorCode,
                                                                errorInfo.proofModuleCode, errorInfo.proofErrorCode,
                                                                errorInfo.message, forceUserFriendly ? true : errorInfo.userFriendly});
                cleanupReply(operationId, reply);
                return;
            }
        }
    }
    AbstractRestApiPrivate::replyFinished(operationId, reply, forceUserFriendly);
}

} // namespace NetworkServices
} // namespace Proof

