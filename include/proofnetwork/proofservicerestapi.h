#ifndef PROOF_NETWORKSERVICES_PROOFSERVICERESTAPI_H
#define PROOF_NETWORKSERVICES_PROOFSERVICERESTAPI_H

#include "proofnetwork/abstractrestapi.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {
namespace NetworkServices {

class ProofServiceRestApiPrivate;
class PROOF_NETWORK_EXPORT ProofServiceRestApi : public AbstractRestApi
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ProofServiceRestApi)
protected:
    ProofServiceRestApi(const RestClientSP &restClient, ProofServiceRestApiPrivate &dd, QObject *parent = 0);
};

} // namespace NetworkServices
} // namespace Proof

#endif // PROOF_NETWORKSERVICES_PROOFSERVICERESTAPI_H
