#ifndef PROOFSERVICERESTAPI_P_H
#define PROOFSERVICERESTAPI_P_H

#include "proofnetwork/baserestapi_p.h"
#include "proofnetwork/errormessagesregistry.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofservicerestapi.h"

namespace Proof {
namespace NetworkServices {
class ProofServiceRestApi;
class PROOF_NETWORK_EXPORT ProofServiceRestApiPrivate : public BaseRestApiPrivate
{
    Q_DECLARE_PUBLIC(ProofServiceRestApi)
public:
    ProofServiceRestApiPrivate(const QSharedPointer<ErrorMessagesRegistry> &errorsRegistry);
    QSharedPointer<ErrorMessagesRegistry> errorsRegistry;
};
} // namespace NetworkServices
} // namespace Proof

#endif // PROOFSERVICERESTAPI_P_H
