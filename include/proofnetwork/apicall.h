#ifndef PROOF_APICALL_H
#define PROOF_APICALL_H

#include "proofcore/future.h"

#include "proofnetwork/abstractrestapi.h"

namespace Proof {
//TODO: 1.0: Change whole network layer to Futures approach to make it more streamlined
template <typename Result>
struct ApiCall
{
    template <typename Callee, typename Method, typename Signal, typename... Args>
    static FutureSP<Result> exec(int attempts, Callee *callee, Method method, Signal signal, Args... args)
    {
        return (*callee.*method)(args...)->recoverWith(
            [attempts, callee, method, signal, args...](const Failure &failure) -> FutureSP<Result> {
                if (attempts <= 1)
                    return WithFailure(failure);
                else
                    return exec(attempts - 1, callee, method, signal, args...);
            });
    }

    template <typename Callee, typename Method, typename Signal, typename... Args>
    static FutureSP<Result> exec(Callee *callee, Method method, Signal signal, Args... args)
    {
        return exec(1, callee, method, signal, args...);
    }
};

template <>
struct ApiCall<void>
{
    template <typename Callee, typename Method, typename Signal, typename... Args>
    static FutureSP<bool> exec(int attempts, Callee *callee, Method method, Signal signal, Args... args)
    {
        return (*callee.*method)(args...)->andThenValue(true)->recoverWith(
            [attempts, callee, method, signal, args...](const Failure &failure) -> FutureSP<bool> {
                if (attempts <= 1)
                    return WithFailure(failure);
                else
                    return exec(attempts - 1, callee, method, signal, args...);
            });
    }

    template <typename Callee, typename Method, typename Signal, typename... Args>
    static FutureSP<bool> exec(Callee *callee, Method method, Signal signal, Args... args)
    {
        return exec(1, callee, method, signal, args...);
    }
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
#endif // PROOF_APICALL_H
