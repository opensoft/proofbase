#ifndef PROOF_APICALL_H
#define PROOF_APICALL_H
#include "proofcore/future.h"
#include "proofcore/helpers/prooftypetraits.h"
#include "proofcore/tasks.h"
#include "proofnetwork/abstractrestapi.h"
#include "proofnetwork/baserestapi.h"

namespace Proof {
namespace detail {
template<typename Result>
struct ApiCallSuccessCallbackGenerator
{
    template <typename Signal>
    static std::function<bool(qulonglong, Result)>
    result(qulonglong &currentOperationId, Result &result, Signal)
    {
        return [&result, &currentOperationId] (qulonglong operationId, Result received) {
            if (currentOperationId != operationId)
                return false;
            result = received;
            return true;
        };
    }
};

template<typename ...Results>
struct ApiCallSuccessCallbackGenerator<std::tuple<Results...>>
{
    template<typename T, typename... Args>
    static std::function<bool(qulonglong, typename std::decay<Args>::type...)>
    result(qulonglong &currentOperationId, std::tuple<Results...> &results, void (T::*)(qulonglong, Args...))
    {
        return [&results, &currentOperationId] (qulonglong operationId, typename std::decay<Args>::type... received) {
            if (currentOperationId != operationId)
                return false;
            results = std::make_tuple(received...);
            return true;
        };
    }
};
}

//TODO: 2.0: Change whole network layer to Futures approach to make it more streamlined
template<typename Result>
struct ApiCall
{
    template <typename Callee, typename Method, typename Signal, typename ...Args>
    static auto exec(int attempts, Callee *callee, Method method, Signal signal, Args... args)
    -> decltype (callee->itIsBase(), FutureSP<Result>())
    {
        return (*callee.*method)(args...)->recoverWith([attempts, callee, method, signal, args...](const Failure &failure) -> FutureSP<Result> {
            if (attempts <= 1)
                return WithFailure(failure);
            else
                return exec(attempts - 1, callee, method, signal, args...);
        });
    }

    template <typename Callee, typename Method, typename Signal, typename ...Args>
    static auto exec(Callee *callee, Method method, Signal signal, Args... args)
    -> decltype (callee->itIsBase(), FutureSP<Result>())
    {
        return exec(1, callee, method, signal, args...);
    }

    template <typename Callee, typename Method, typename Signal, typename ...Args>
    static auto exec(int attempts, Callee *callee, Method method, Signal signal, Args... args)
    -> decltype (callee->abortRequest(42), FutureSP<Result>())
    {
        return tasks::run([callee, method, signal, args...](void) -> Result {
            if (callee->isLoggedOut())
                return WithFailure(QStringLiteral("API is not logged in"), NETWORK_MODULE_CODE, NetworkErrorCode::AuthCredentialsError);
            qulonglong currentOperationId = 0;
            Result result;
            auto callback = detail::ApiCallSuccessCallbackGenerator<Result>::result(currentOperationId, result, signal);
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
        }, tasks::RestrictionType::Custom, callee->restClient()->host())->recoverWith([attempts, callee, method, signal, args...](const Failure &failure) -> FutureSP<Result> {
            if (attempts <= 1)
                return WithFailure(failure);
            else
                return exec(attempts - 1, callee, method, signal, args...);
        });
    }

    template <typename Callee, typename Method, typename Signal, typename ...Args>
    static auto exec(Callee *callee, Method method, Signal signal, Args... args)
    -> decltype (callee->abortRequest(42), FutureSP<Result>())
    {
        return exec(1, callee, method, signal, args...);
    }
};

template<>
struct ApiCall<void>
{
    template <typename Callee, typename Method, typename Signal, typename ...Args>
    static auto exec(int attempts, Callee *callee, Method method, Signal signal, Args... args)
    -> decltype (callee->itIsBase(), FutureSP<bool>())
    {
        return (*callee.*method)(args...)->andThenValue(true)
                ->recoverWith([attempts, callee, method, signal, args...](const Failure &failure) -> FutureSP<bool> {
            if (attempts <= 1)
                return WithFailure(failure);
            else
                return exec(attempts - 1, callee, method, signal, args...);
        });
    }

    template <typename Callee, typename Method, typename Signal, typename ...Args>
    static auto exec(Callee *callee, Method method, Signal signal, Args... args)
    -> decltype (callee->itIsBase(), FutureSP<bool>())
    {
        return exec(1, callee, method, signal, args...);
    }

    template <typename Callee, typename Method, typename Signal, typename ...Args>
    static auto exec(int attempts, Callee *callee, Method method, Signal signal, Args... args)
    -> decltype (callee->abortRequest(42), FutureSP<bool>())
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
        }, tasks::RestrictionType::Custom, callee->restClient()->host())->recoverWith([attempts, callee, method, signal, args...](const Failure &failure) -> FutureSP<bool> {
            if (attempts <= 1)
                return WithFailure(failure);
            else
                return exec(attempts - 1, callee, method, signal, args...);
        });
    }

    template <typename Callee, typename Method, typename Signal, typename ...Args>
    static auto exec(Callee *callee, Method method, Signal signal, Args... args)
    -> decltype (callee->abortRequest(42), FutureSP<bool>())
    {
        return exec(1, callee, method, signal, args...);
    }
};
}
#endif // PROOF_APICALL_H
