#ifndef ABSTRACTRESTAPI_P_H
#define ABSTRACTRESTAPI_P_H

#include "proofcore/proofobject_p.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/proofnetwork_global.h"

#include <QNetworkReply>

class QTimer;

namespace Proof {
class AbstractRestApi;
class PROOF_NETWORK_EXPORT AbstractRestApiPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(AbstractRestApi)
public:
    AbstractRestApiPrivate() : ProofObjectPrivate() {}

    QNetworkReply *get(const QString &method, const QUrlQuery &query = QUrlQuery());
    QNetworkReply *post(const QString &method, const QUrlQuery &query = QUrlQuery(), const QByteArray &body = "");

    virtual void replyFinished(QNetworkReply *reply);
    virtual void replyErrorOccurred(QNetworkReply *reply);
    virtual void sslErrorsOccurred(QNetworkReply *reply, const QList<QSslError> &errors);
    virtual void cleanupReply(QNetworkReply *reply) = 0;

    QMetaObject::Connection replyFinishedConnection;
    QMetaObject::Connection sslErrorsConnection;
    RestClientSP restClient;
};
}
#endif // ABSTRACTRESTAPI_P_H
