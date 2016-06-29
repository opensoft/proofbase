#ifndef PROOFSERVICERESTAPI_P_H
#define PROOFSERVICERESTAPI_P_H

#include "proofcore/errormessagesregistry_p.h"
#include "proofnetwork/abstractrestapi_p.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {
namespace NetworkServices {

class ProofServiceRestApi;
class PROOF_NETWORK_EXPORT ProofServiceRestApiPrivate : public AbstractRestApiPrivate
{
    Q_DECLARE_PUBLIC(ProofServiceRestApi)
public:
    ProofServiceRestApiPrivate(const QSharedPointer<ErrorMessagesRegistry> & errorsRegistry);

    void replyFinished(qulonglong operationId, QNetworkReply *reply, bool forceUserFriendly = false) override;

    QSharedPointer<ErrorMessagesRegistry> errorsRegistry;
};

} // namespace NetworkServices
} // namespace Proof

#endif // PROOFSERVICERESTAPI_P_H

