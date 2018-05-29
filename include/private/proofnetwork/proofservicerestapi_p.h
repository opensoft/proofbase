#ifndef PROOFSERVICERESTAPI_P_H
#define PROOFSERVICERESTAPI_P_H

#include "proofnetwork/abstractrestapi_p.h"
#include "proofnetwork/baserestapi_p.h"
#include "proofnetwork/proofservicerestapi.h"
#include "proofcore/errormessagesregistry_p.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {
namespace NetworkServices {

class ProofServiceRestApiOld;
class PROOF_NETWORK_EXPORT ProofServiceRestApiOldPrivate : public AbstractRestApiPrivate
{
    Q_DECLARE_PUBLIC(ProofServiceRestApiOld)
public:
    ProofServiceRestApiOldPrivate(const QSharedPointer<ErrorMessagesRegistry> &errorsRegistry);

    void replyFinished(qulonglong operationId, QNetworkReply *reply, bool forceUserFriendly = false) override;

    QSharedPointer<ErrorMessagesRegistry> errorsRegistry;
};

class ProofServiceRestApi;
class PROOF_NETWORK_EXPORT ProofServiceRestApiPrivate : public BaseRestApiPrivate
{
    Q_DECLARE_PUBLIC(ProofServiceRestApi)
public:
    ProofServiceRestApiPrivate(const QSharedPointer<ErrorMessagesRegistry> &errorsRegistry);
    void processSuccessfulReply(QNetworkReply *reply, const PromiseSP<QByteArray> &promise) override;

    QSharedPointer<ErrorMessagesRegistry> errorsRegistry;

};
} // namespace NetworkServices
} // namespace Proof

#endif // PROOFSERVICERESTAPI_P_H

