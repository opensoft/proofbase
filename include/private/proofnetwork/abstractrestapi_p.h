#ifndef ABSTRACTRESTAPI_P_H
#define ABSTRACTRESTAPI_P_H

#include "proofcore/proofobject_p.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/proofnetwork_global.h"

#include <QSslError>
#include <QHash>
#include <QJsonParseError>

#include <atomic>

class QTimer;
class QNetworkReply;

namespace Proof {
class AbstractRestApi;
class PROOF_NETWORK_EXPORT AbstractRestApiPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(AbstractRestApi)
public:
    AbstractRestApiPrivate() : ProofObjectPrivate() {}

    QNetworkReply *get(qulonglong &operationId, const QString &method, const QUrlQuery &query = QUrlQuery());
    QNetworkReply *post(qulonglong &operationId, const QString &method, const QUrlQuery &query = QUrlQuery(),
                        const QByteArray &body = "");
    QNetworkReply *patch(qulonglong &operationId, const QString &method, const QUrlQuery &query = QUrlQuery(),
                        const QByteArray &body = "");

    virtual void replyFinished(qulonglong operationId, QNetworkReply *reply);
    virtual void replyErrorOccurred(qulonglong operationId, QNetworkReply *reply);
    virtual void sslErrorsOccurred(qulonglong operationId, QNetworkReply *reply, const QList<QSslError> &errors);
    virtual void cleanupReply(qulonglong operationId, QNetworkReply *reply);

    void notifyAboutJsonParseError(qulonglong operationId, const QJsonParseError &error);

    QMetaObject::Connection replyFinishedConnection;
    QMetaObject::Connection sslErrorsConnection;
    RestClientSP restClient;

private:
    void setupReply(qulonglong &operationId, QNetworkReply *reply);

    static std::atomic<qulonglong> lastUsedOperationId;
    QHash<QNetworkReply *, qulonglong> repliesIds;
};
}
#endif // ABSTRACTRESTAPI_P_H
