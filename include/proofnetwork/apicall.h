#ifndef PROOF_APICALL_H
#define PROOF_APICALL_H

#include "proofseed/future.h"

#include "proofnetwork/proofnetwork_global.h"

#include <QNetworkReply>

namespace Proof {
//TODO: Replace completely
//As it looks like - we don't need level (we use it only to check if error happened and if it was server error)
// and everything else can be easily covered with Failure directly
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

    Failure toFailure() const;
    static RestApiError fromFailure(const Failure &f);

    Level level = Level::NoError;
    qlonglong code = 0;
    long proofModuleCode = NETWORK_MODULE_CODE;
    long proofErrorCode = 0;
    QString message;
    bool userFriendly = false;
};

//TODO: deprecated, remove when all clients will be moved to proper futures usage
//Can be used anywhere instead of chainedApiCall because they already require a background thread
template <typename T>
RestApiError runApiCall(const FutureSP<T> &caller, T &result)
{
    caller->wait();
    if (caller->failed()) {
        return RestApiError::fromFailure(caller->failureReason());
    } else {
        result = caller->result();
        return RestApiError();
    }
}

template <typename T>
RestApiError runApiCall(const FutureSP<T> &caller)
{
    caller->wait();
    return caller->failed() ? RestApiError::fromFailure(caller->failureReason()) : RestApiError();
}

template <typename T>
RestApiError runApiCall(const CancelableFuture<T> &caller, T &result)
{
    return runApiCall(caller.future(), result);
}

template <typename T>
RestApiError runApiCall(const CancelableFuture<T> &caller)
{
    return runApiCall(caller.future());
}

} // namespace Proof

Q_DECLARE_METATYPE(Proof::RestApiError)
#endif // PROOF_APICALL_H
