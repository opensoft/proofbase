#ifndef ABSTRACTRESTAPI_H
#define ABSTRACTRESTAPI_H

#include "proofcore/proofobject.h"
#include "proofcore/future.h"
#include "proofcore/taskchain.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/baserestapi.h"

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

    //TODO: Replace completely
    //As it looks like - we don't need level (we use it only to check if error happened and if it was server error)
    // and everything else can be easily covered with Failure directly
    Failure toFailure() const;
    static RestApiError fromFailure(const Failure &f);

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

    void abortRequest(qulonglong operationId);

    virtual bool isLoggedOut() const;

    static qlonglong clientNetworkErrorOffset();
    static qlonglong clientSslErrorOffset();

    template<class Result, class Signal>
    static std::function<bool(qulonglong, Result)> generateSuccessCallback(qulonglong &currentOperationId, Result &result, Signal)
    {
        return [&result, &currentOperationId] (qulonglong operationId, Result received) {
            if (currentOperationId != operationId)
                return false;
            result = received;
            return true;
        };
    }
    template<class... Types, class T, class... Args>
    static std::function<bool(qulonglong, typename std::decay<Args>::type...)>
    generateSuccessCallback(qulonglong &currentOperationId, std::tuple<Types &...> results, void (T::*)(qulonglong, Args...))
    {
        return [results, &currentOperationId] (qulonglong operationId, typename std::decay<Args>::type... received) mutable {
            if (currentOperationId != operationId)
                return false;
            results = std::tie(received...);
            return true;
        };
    }

    using ErrorCallbackType = std::function<bool(qulonglong, const RestApiError &)>;
    static ErrorCallbackType generateErrorCallback(qulonglong &currentOperationId, RestApiError &error);
    static ErrorCallbackType generateErrorCallback(qulonglong &currentOperationId, QString &errorMessage);

    template <class Callee, class Result, class Method, class Signal, class ...Args>
    static auto chainedApiCall(const TaskChainSP &, Callee *callee, Method method, Signal, Result &&result, Args... args)
    -> decltype (callee->itIsBase(), RestApiError())
    {
        auto f = (*callee.*method)(args...);
        f->wait();
        if (f->failed()) {
            return RestApiError::fromFailure(f->failureReason());
        } else {
            result = f->result();
            return RestApiError();
        }
    }

    template <class Callee, class Result, class Method, class Signal, class ...Args>
    static auto chainedApiCall(const TaskChainSP &taskChain, Callee *callee, Method method,
                               Signal signal, Result &&result, Args... args)
    -> decltype (callee->abortRequest(42), RestApiError())
    {
        qulonglong currentOperationId = 0;
        auto callback = generateSuccessCallback(currentOperationId, std::forward<Result>(result), signal);
        RestApiError error;
        if (!callee->isLoggedOut()) {
            taskChain->addSignalWaiter(callee, signal, callback);
            taskChain->addSignalWaiter(callee, &Proof::AbstractRestApi::apiErrorOccurred, generateErrorCallback(currentOperationId, error));
            currentOperationId = (*callee.*method)(args...);
            if (!currentOperationId) {
                error.level = RestApiError::Level::ClientError;
                error.code = 0;
                error.proofModuleCode = NETWORK_MODULE_CODE;
                error.proofErrorCode = NetworkErrorCode::InvalidRequest;
                error.message = QStringLiteral("Invalid arguments");
                error.userFriendly = false;
            } else {
                taskChain->fireSignalWaiters();
            }
        } else {
            error.level = RestApiError::Level::AuthCredentialsError;
            error.code = 0;
            error.proofModuleCode = NETWORK_MODULE_CODE;
            error.proofErrorCode = NetworkErrorCode::AuthCredentialsError;
            error.message = QStringLiteral("API is not logged in");
            error.userFriendly = false;
        }
        return error;
    }

    template <class Callee, class Result, class Method, class Signal, class ...Args>
    static RestApiError chainedApiCall(int attempts, const TaskChainSP &taskChain, Callee *callee, Method method,
                                       Signal signal, Result &&result, Args... args)
    {
        Proof::RestApiError error;
        do {
            error = chainedApiCall(taskChain, callee, method, signal, std::forward<Result>(result), args...);
            if (error.toNetworkError() != QNetworkReply::OperationCanceledError)
                break;
        } while(--attempts > 0);
        return error;
    }

    template <class Callee, class Method, class Signal, class ...Args>
    static auto chainedApiCallWithoutResult(const TaskChainSP &, Callee *callee, Method method, Signal, Args... args)
    -> decltype (callee->itIsBase(), RestApiError())
    {
        auto f = (*callee.*method)(args...);
        f->wait();
        return f->failed() ? RestApiError::fromFailure(f->failureReason()) : RestApiError();
    }

    template <class Callee, class Method, class Signal, class ...Args>
    static auto chainedApiCallWithoutResult(const TaskChainSP &taskChain, Callee *callee, Method method,
                                            Signal signal, Args... args)
    -> decltype (callee->abortRequest(42), RestApiError())
    {
        qulonglong currentOperationId = 0;
        std::function<bool(qulonglong)> callback
                = [&currentOperationId] (qulonglong operationId) {
            return (currentOperationId == operationId);
        };
        RestApiError error;
        if (!callee->isLoggedOut()) {
            taskChain->addSignalWaiter(callee, signal, callback);
            taskChain->addSignalWaiter(callee, &Proof::AbstractRestApi::apiErrorOccurred, generateErrorCallback(currentOperationId, error));
            currentOperationId = (*callee.*method)(args...);
            if (!currentOperationId) {
                error.level = RestApiError::Level::ClientError;
                error.code = 0;
                error.proofModuleCode = NETWORK_MODULE_CODE;
                error.proofErrorCode = NetworkErrorCode::InvalidRequest;
                error.message = QStringLiteral("Invalid arguments");
                error.userFriendly = false;
            } else {
                taskChain->fireSignalWaiters();
            }
        } else {
            error.level = RestApiError::Level::AuthCredentialsError;
            error.code = 0;
            error.proofModuleCode = NETWORK_MODULE_CODE;
            error.proofErrorCode = NetworkErrorCode::AuthCredentialsError;
            error.message = QStringLiteral("API is not logged in");
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
    void apiErrorOccurred(qulonglong operationId, const Proof::RestApiError &error);

protected:
    AbstractRestApi(const RestClientSP &restClient, AbstractRestApiPrivate &dd, QObject *parent = nullptr);
    //Called before actual set is done
    virtual void onRestClientChanging(const RestClientSP &client);
};
}

Q_DECLARE_METATYPE(Proof::RestApiError)

#endif // ABSTRACTRESTAPI_H
