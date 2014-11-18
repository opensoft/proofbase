#ifndef ABSTRACTRESTAPI_H
#define ABSTRACTRESTAPI_H

#include "proofcore/proofobject.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {

class AbstractRestApiPrivate;

struct RestApiError;

class PROOF_NETWORK_EXPORT AbstractRestApi : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractRestApi)
public:
    RestClientSP restClient() const;
    void setRestClient(const RestClientSP &client);

signals:
    void errorOccurred(qulonglong operationId, const Proof::RestApiError &error);

protected:
    AbstractRestApi(const RestClientSP &restClient, AbstractRestApiPrivate &dd, QObject *parent = 0);
    //Called before actual set is done
    virtual void onRestClientChanging(const RestClientSP &client);
};

struct RestApiError
{
    enum ErrorLevel {
        ClientError,
        ServerError,
        JsonParseError,
        JsonServerError,
        JsonDataError
    };
    ErrorLevel level;
    qlonglong code;
    QString message;
};
}

Q_DECLARE_METATYPE(Proof::RestApiError)

#endif // ABSTRACTRESTAPI_H
