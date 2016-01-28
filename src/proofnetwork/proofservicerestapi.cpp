#include "proofservicerestapi.h"

#include "proofservicerestapi_p.h"

#include "proofnetwork_types.h"

#include <QJsonDocument>
#include <QJsonObject>

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

void ProofServiceRestApiPrivate::replyFinished(qulonglong operationId, QNetworkReply *reply)
{
    Q_Q(ProofServiceRestApi);
    int errorCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (400 <= errorCode && errorCode < 600) {
        QJsonParseError jsonError;
        QJsonDocument content = QJsonDocument::fromJson(reply->readAll(), &jsonError);
        if (jsonError.error == QJsonParseError::NoError && content.isObject()) {
            QJsonValue serviceErrorCode = content.object().value("error_code");
            if (serviceErrorCode.isDouble()) {
                QJsonArray jsonArgs = content.object().value("message_args").toArray();
                QStringList args;
                for (const auto &arg : jsonArgs)
                    args << arg.toString();
                ErrorInfo errorInfo = errorsRegistry->infoForCode(serviceErrorCode.toInt(), args);
                qCDebug(proofNetworkMiscLog) << "Error in JSON parsing occurred for" << operationId
                                             << reply->request().url().path()
                                             << reply->request().url().query()
                                             << ": " << errorCode
                                             << errorInfo.proofModuleCode << errorInfo.proofErrorCode << errorInfo.message;
                emit q->errorOccurred(operationId, RestApiError{RestApiError::Level::ServerError, errorCode,
                                                                errorInfo.proofModuleCode, errorInfo.proofErrorCode,
                                                                errorInfo.message, errorInfo.userFriendly});
                cleanupReply(operationId, reply);
                return;
            }
        }
    }
    AbstractRestApiPrivate::replyFinished(operationId, reply);
}

} // namespace NetworkServices
} // namespace Proof

