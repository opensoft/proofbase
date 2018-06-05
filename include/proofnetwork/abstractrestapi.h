#ifndef ABSTRACTRESTAPI_H
#define ABSTRACTRESTAPI_H

#include "proofcore/future.h"
#include "proofcore/proofobject.h"
#include "proofcore/taskchain.h"

#include "proofnetwork/baserestapi.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/restclient.h"

#include <QNetworkReply>

namespace Proof {

struct PROOF_NETWORK_EXPORT RestApiError
{
    enum class Level
    {
        NoError,
        AuthCredentialsError,
        ClientError,
        ServerError,
        JsonParseError,
        JsonServerError,
        JsonDataError
    };
    RestApiError(Level _level = Level::NoError, qlonglong _code = 0, long _proofModuleCode = NETWORK_MODULE_CODE,
                 long _proofErrorCode = 0, const QString &_message = QString(), bool _userFriendly = false)
        : level(_level), code(_code), proofModuleCode(_proofModuleCode), proofErrorCode(_proofErrorCode),
          message(_message), userFriendly(_userFriendly)
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

class PROOF_NETWORK_EXPORT AbstractRestApi
{
public:
    static qlonglong clientNetworkErrorOffset();
    static qlonglong clientSslErrorOffset();

    template <class Callee, class Result, class Method, class Signal, class... Args>
    static RestApiError chainedApiCall(const TaskChainSP &, Callee *callee, Method method, Signal, Result &&result,
                                       Args... args)
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

    template <class Callee, class Result, class Method, class Signal, class... Args>
    static RestApiError chainedApiCall(int attempts, const TaskChainSP &taskChain, Callee *callee, Method method,
                                       Signal signal, Result &&result, Args... args)
    {
        Proof::RestApiError error;
        do {
            error = chainedApiCall(taskChain, callee, method, signal, std::forward<Result>(result), args...);
            if (error.toNetworkError() != QNetworkReply::OperationCanceledError)
                break;
        } while (--attempts > 0);
        return error;
    }

    template <class Callee, class Method, class Signal, class... Args>
    static RestApiError chainedApiCallWithoutResult(const TaskChainSP &, Callee *callee, Method method, Signal,
                                                    Args... args)
    {
        auto f = (*callee.*method)(args...);
        f->wait();
        return f->failed() ? RestApiError::fromFailure(f->failureReason()) : RestApiError();
    }

    template <class Callee, class Method, class Signal, class... Args>
    static RestApiError chainedApiCallWithoutResult(int attempts, const TaskChainSP &taskChain, Callee *callee,
                                                    Method method, Signal signal, Args... args)
    {
        Proof::RestApiError error;
        do {
            error = chainedApiCallWithoutResult(taskChain, callee, method, signal, args...);
            if (error.toNetworkError() != QNetworkReply::OperationCanceledError)
                break;
        } while (--attempts > 0);
        return error;
    }
};
} // namespace Proof

Q_DECLARE_METATYPE(Proof::RestApiError)

#endif // ABSTRACTRESTAPI_H
