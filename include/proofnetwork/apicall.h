#ifndef PROOF_APICALL_H
#define PROOF_APICALL_H
#include "proofcore/future.h"
#include "proofcore/tasks.h"
#include "proofnetwork/abstractrestapi.h"

namespace Proof {
//TODO: 2.0: Change whole network layer to Futures approach to make it more streamlined
template<typename Result>
struct ApiCall
{
    template <class Callee, class Method, class Signal, class ...Args>
    static FutureSP<Result> exec(int attempts, Callee *callee, Method method, Signal signal, Args... args)
    {
        return tasks::run([callee, method, signal, args...](void) -> Result {
            if (callee->isLoggedOut())
                return WithFailure(QStringLiteral("API is not logged in"), NETWORK_MODULE_CODE, NetworkErrorCode::AuthCredentialsError);
            qulonglong currentOperationId = 0;
            Result result;
            auto callback = generateSuccessCallback(currentOperationId, result, signal);
            RestApiError error;
            tasks::addSignalWaiter(callee, signal, callback);
            tasks::addSignalWaiter(callee, &Proof::AbstractRestApi::apiErrorOccurred, AbstractRestApi::generateErrorCallback(currentOperationId, error));
            currentOperationId = (*callee.*method)(args...);
            if (!currentOperationId)
                return WithFailure(QStringLiteral("Invalid arguments"), NETWORK_MODULE_CODE, NetworkErrorCode::InvalidRequest);
            tasks::fireSignalWaiters();
            if (error.level != RestApiError::Level::NoError)
                return WithFailure(error.toFailure());
            else
                return result;
        }, tasks::RestrictionType::Http, callee->restClient()->host())->recoverWith([attempts, callee, method, signal, args...](const Failure &failure) -> FutureSP<Result> {
            if (attempts <= 1)
                return WithFailure(failure);
            else
                return exec(attempts - 1, callee, method, signal, args...);
        });
    }

    template <class Callee, class Method, class Signal, class ...Args>
    static FutureSP<Result> exec(Callee *callee, Method method, Signal signal, Args... args)
    {
        return exec(1, callee, method, signal, args...);
    }

private:
    template<class Signal>
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
    generateSuccessCallback(qulonglong &currentOperationId, std::tuple<Types...> &results, void (T::*)(qulonglong, Args...))
    {
        return [results, &currentOperationId] (qulonglong operationId, typename std::decay<Args>::type... received) mutable {
            if (currentOperationId != operationId)
                return false;
            results = std::make_tuple(received...);
            return true;
        };
    }
};

template<>
struct ApiCall<void>
{
    template <class Callee, class Method, class Signal, class ...Args>
    static FutureSP<bool> exec(int attempts, Callee *callee, Method method, Signal signal, Args... args)
    {
        return tasks::run([callee, method, signal, args...](void) -> bool {
            if (callee->isLoggedOut())
                return WithFailure(QStringLiteral("API is not logged in"), NETWORK_MODULE_CODE, NetworkErrorCode::AuthCredentialsError);
            qulonglong currentOperationId = 0;
            std::function<bool(qulonglong)> callback
                    = [&currentOperationId] (qulonglong operationId) {
                return (currentOperationId == operationId);
            };
            RestApiError error;
            tasks::addSignalWaiter(callee, signal, callback);
            tasks::addSignalWaiter(callee, &Proof::AbstractRestApi::apiErrorOccurred, AbstractRestApi::generateErrorCallback(currentOperationId, error));
            currentOperationId = (*callee.*method)(args...);
            if (!currentOperationId)
                return WithFailure(QStringLiteral("Invalid arguments"), NETWORK_MODULE_CODE, NetworkErrorCode::InvalidRequest);
            tasks::fireSignalWaiters();
            if (error.level != RestApiError::Level::NoError)
                return WithFailure(error.toFailure());
            else
                return true;
        }, tasks::RestrictionType::Http, callee->restClient()->host())->recoverWith([attempts, callee, method, signal, args...](const Failure &failure) -> FutureSP<bool> {
            if (attempts <= 1)
                return WithFailure(failure);
            else
                return exec(attempts - 1, callee, method, signal, args...);
        });
    }

    template <class Callee, class Method, class Signal, class ...Args>
    static FutureSP<bool> exec(Callee *callee, Method method, Signal signal, Args... args)
    {
        return exec(1, callee, method, signal, args...);
    }
};
}
#endif // PROOF_APICALL_H
