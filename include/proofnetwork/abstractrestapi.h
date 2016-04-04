#ifndef ABSTRACTRESTAPI_H
#define ABSTRACTRESTAPI_H

#include "proofcore/proofobject.h"
#include "proofcore/taskchain.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"

#include <QNetworkReply>

namespace Proof {

struct PROOF_NETWORK_EXPORT RestApiError
{
    enum class Level {
        NoError,
        AuthCredentialsError,
        ClientError,
        ServerError,
        JsonParseError,
        JsonServerError,
        JsonDataError
    };
    RestApiError(Level _level = Level::NoError, qlonglong _code = 0,
                 long _proofModuleCode = NETWORK_MODULE_CODE, long _proofErrorCode = 0,
                 const QString &_message = QString(), bool _userFriendly = false)
        : level(_level), code(_code), proofModuleCode(_proofModuleCode), proofErrorCode(_proofErrorCode), message(_message), userFriendly(_userFriendly)
    {}

    QString toString() const;
    void reset();
    bool isNetworkError() const;
    QNetworkReply::NetworkError toNetworkError() const;

    Level level = Level::NoError;
    qlonglong code = 0;
    long proofModuleCode = NETWORK_MODULE_CODE;
    long proofErrorCode = 0;
    QString message;
    bool userFriendly = false;
};

class AbstractRestApiPrivate;
class PROOF_NETWORK_EXPORT AbstractRestApi : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractRestApi)
public:
    RestClientSP restClient() const;
    void setRestClient(const RestClientSP &client);

    virtual bool isLoggedOut() const;

    static qlonglong clientNetworkErrorOffset();
    static qlonglong clientSslErrorOffset();

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
        if (!callee->isLoggedOut()) {
            taskChain->addSignalWaiter(callee, signal, callback);
            taskChain->addSignalWaiter(callee, &Proof::AbstractRestApi::errorOccurred, generateErrorCallback(currentOperationId, error));
            currentOperationId = (*callee.*method)(args...);
            taskChain->fireSignalWaiters();
        } else {
            error.level = RestApiError::Level::AuthCredentialsError;
            error.code = 0;
            error.proofModuleCode = NETWORK_MODULE_CODE;
            error.proofErrorCode = NetworkErrorCode::AuthCredentialsError;
            error.message = "API is not logged in";
            error.userFriendly = false;
        }
        return error;
    }

    template <class Callee, class Result, class Method, class Signal, class ...Args>
    static RestApiError chainedApiCall(int attempts, const TaskChainSP &taskChain, Callee *callee, Method method,
                                       Signal signal, Result &result, Args... args)
    {
        Proof::RestApiError error;
        do {
            error = chainedApiCall(taskChain, callee, method, signal, result, args...);
            if (error.toNetworkError() != QNetworkReply::OperationCanceledError)
                break;
        } while(--attempts > 0);
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
        if (!callee->isLoggedOut()) {
            taskChain->addSignalWaiter(callee, signal, callback);
            taskChain->addSignalWaiter(callee, &Proof::AbstractRestApi::errorOccurred, generateErrorCallback(currentOperationId, error));
            currentOperationId = (*callee.*method)(args...);
            taskChain->fireSignalWaiters();
        } else {
            error.level = RestApiError::Level::AuthCredentialsError;
            error.code = 0;
            error.proofModuleCode = NETWORK_MODULE_CODE;
            error.proofErrorCode = NetworkErrorCode::AuthCredentialsError;
            error.message = "API is not logged in";
            error.userFriendly = false;
        }
        return error;
    }

    template <class Callee, class Method, class Signal, class ...Args>
    static RestApiError chainedApiCallWithoutResult(int attempts, const TaskChainSP &taskChain, Callee *callee,
                                                    Method method, Signal signal, Args... args)
    {
        Proof::RestApiError error;
        do {
            error = chainedApiCallWithoutResult(taskChain, callee, method, signal, args...);
            if (error.toNetworkError() != QNetworkReply::OperationCanceledError)
                break;
        } while(--attempts > 0);
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
