#ifndef ABSTRACTRESTAPI_H
#define ABSTRACTRESTAPI_H

#include "proofcore/proofobject.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {

class AbstractRestApiPrivate;

class PROOF_NETWORK_EXPORT AbstractRestApi : public ProofObject
{
    Q_OBJECT
    Q_ENUMS(ErrorLevel)
    Q_DECLARE_PRIVATE(AbstractRestApi)
public:
    enum ErrorLevel {
        ClientError,
        ServerError,
        JsonParseError,
        JsonServerError,
        JsonDataError
    };

    RestClientSP restClient();
    void setRestClient(const RestClientSP &client);

signals:
    void errorOccurred(Proof::AbstractRestApi::ErrorLevel errorLevel, qlonglong errorNumber, const QString &errorString);

protected:
    AbstractRestApi(const RestClientSP &restClient, AbstractRestApiPrivate &dd, QObject *parent = 0);
    //Called before actual set is done
    virtual void onRestClientChanging(const RestClientSP &client);
};

}

#endif // ABSTRACTRESTAPI_H
