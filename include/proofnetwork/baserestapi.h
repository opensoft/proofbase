#ifndef BASERESTAPI_H
#define BASERESTAPI_H
#include "proofcore/proofobject.h"
#include "proofcore/future.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"

#include <QNetworkReply>

namespace Proof {
class BaseRestApiPrivate;
class PROOF_NETWORK_EXPORT BaseRestApi : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BaseRestApi)
public:
    RestClientSP restClient() const;

    virtual bool isLoggedOut() const;

    static qlonglong clientNetworkErrorOffset();
    static qlonglong clientSslErrorOffset();

    //TODO: remove after full switch to new network
    void itIsBase(){}

protected:
    BaseRestApi(const RestClientSP &restClient, BaseRestApiPrivate &dd, QObject *parent = nullptr);
};
}

#endif // BASERESTAPI_H
