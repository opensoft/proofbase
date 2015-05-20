#ifndef ABSTRACTRESTAPI_H
#define ABSTRACTRESTAPI_H

#include "proofcore/proofobject.h"
#include "proofcore/taskchain.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {

struct PROOF_NETWORK_EXPORT RestApiError
{
    enum class Level {
        NoError,
        ClientError,
        ServerError,
        JsonParseError,
        JsonServerError,
        JsonDataError
    };
    RestApiError(Level _level = Level::NoError, qlonglong _code = 0, const QString &_message = QString())
        : level(_level), code(_code), message(_message)
    {}

    QString toString() const;
    void reset();

    Level level = Level::NoError;
    qlonglong code = 0;
    QString message;
};

class AbstractRestApiPrivate;
class PROOF_NETWORK_EXPORT AbstractRestApi : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractRestApi)
public:
    RestClientSP restClient() const;
    void setRestClient(const RestClientSP &client);

    using ErrorCallbackType = std::function<bool(qulonglong, const RestApiError &)>;
    static ErrorCallbackType generateErrorCallback(qulonglong &currentOperationId, RestApiError &error);
    static ErrorCallbackType generateErrorCallback(qulonglong &currentOperationId, QString &errorMessage);

    template <class Callee, class Result, class Method, class Signal, class ...Args>
    static RestApiError chainedApiCall(const TaskChainSP &taskChain, Callee *callee, Method method,
                                       Signal signal, Result &result, Args... args)
    {
        qulonglong currentOperationId = 0;
        std::function<bool(qulonglong, Result)> callback
                = [&result, &currentOperationId] (qulonglong operationId, Result received) {
            if (currentOperationId != operationId)
                return false;
            result = received;
            return true;
        };
        RestApiError error;
        taskChain->addSignalWaiter(callee, signal, callback);
        taskChain->addSignalWaiter(callee, &Proof::AbstractRestApi::errorOccurred, generateErrorCallback(currentOperationId, error));
        currentOperationId = (*callee.*method)(args...);
        if (currentOperationId)
            taskChain->fireSignalWaiters();
        else
            qApp->processEvents();
        return error;
    }

    template <class Callee, class Method, class Signal, class ...Args>
    static RestApiError chainedApiCallWithoutResult(const TaskChainSP &taskChain, Callee *callee, Method method,
                                                    Signal signal, Args... args)
    {
        qulonglong currentOperationId = 0;
        std::function<bool(qulonglong)> callback
                = [&currentOperationId] (qulonglong operationId) {
            return (currentOperationId == operationId);
        };
        RestApiError error;
        taskChain->addSignalWaiter(callee, signal, callback);
        taskChain->addSignalWaiter(callee, &Proof::AbstractRestApi::errorOccurred, generateErrorCallback(currentOperationId, error));
        currentOperationId = (*callee.*method)(args...);
        if (currentOperationId)
            taskChain->fireSignalWaiters();
        else
            qApp->processEvents();
        return error;
    }

signals:
    void errorOccurred(qulonglong operationId, const Proof::RestApiError &error);

protected:
    AbstractRestApi(const RestClientSP &restClient, AbstractRestApiPrivate &dd, QObject *parent = 0);
    //Called before actual set is done
    virtual void onRestClientChanging(const RestClientSP &client);
};
}

Q_DECLARE_METATYPE(Proof::RestApiError)

#endif // ABSTRACTRESTAPI_H
