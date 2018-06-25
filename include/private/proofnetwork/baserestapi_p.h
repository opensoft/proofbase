#ifndef BASERESTAPI_P_H
#define BASERESTAPI_P_H

#include "proofcore/future.h"
#include "proofcore/objectscache.h"
#include "proofcore/proofobject_p.h"

#include "proofnetwork/baserestapi.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/restclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace Proof {

class BaseRestApi;
class PROOF_NETWORK_EXPORT BaseRestApiPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(BaseRestApi)
public:
    BaseRestApiPrivate() : ProofObjectPrivate() {}

    CancelableFuture<RestApiReply> configureReply(CancelableFuture<QNetworkReply *> replyFuture);
    bool replyShouldBeHandledByError(QNetworkReply *reply) const;
    void rememberReply(const CancelableFuture<RestApiReply> &reply);

    RestClientSP restClient;

private:
    QHash<qint64, CancelableFuture<RestApiReply>> allReplies;
    SpinLock allRepliesLock;
};
} // namespace Proof
#endif // BASERESTAPI_P_H
