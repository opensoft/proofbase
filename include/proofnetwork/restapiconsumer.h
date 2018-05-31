#ifndef PROOFNETWORK_RESTAPICONSUMER_H
#define PROOFNETWORK_RESTAPICONSUMER_H

#include "proofnetwork/abstractrestapi.h"
#include "proofnetwork/proofnetwork_global.h"

#include <functional>

namespace Proof {
struct RestApiError;

class PROOF_NETWORK_EXPORT RestApiConsumer
{
public:
    using ErrorCallbackType = Proof::AbstractRestApi::ErrorCallbackType;
    ErrorCallbackType generateErrorCallback(qulonglong &currentOperationId, QString &errorMessage) const;
    ErrorCallbackType generateErrorCallback(qulonglong &currentOperationId, RestApiError &error) const;

    template <class Callee, class Result, class Method, class Signal, class... Args>
    Proof::RestApiError chainedApiCall(const Proof::TaskChainSP &taskChain, Callee *callee, Method method,
                                       Signal signal, Result &&result, Args... args) const
    {
        return Proof::AbstractRestApi::chainedApiCall(taskChain, callee, method, signal, std::forward<Result>(result),
                                                      args...);
    }
    template <class Callee, class Result, class Method, class Signal, class... Args>
    RestApiError chainedApiCall(int attempts, const TaskChainSP &taskChain, Callee *callee, Method method,
                                Signal signal, Result &&result, Args... args)
    {
        return Proof::AbstractRestApi::chainedApiCall(attempts, taskChain, callee, method, signal,
                                                      std::forward<Result>(result), args...);
    }
    template <class Callee, class Method, class Signal, class... Args>
    RestApiError chainedApiCallWithoutResult(const TaskChainSP &taskChain, Callee *callee, Method method, Signal signal,
                                             Args... args) const
    {
        return Proof::AbstractRestApi::chainedApiCallWithoutResult(taskChain, callee, method, signal, args...);
    }
    template <class Callee, class Method, class Signal, class... Args>
    RestApiError chainedApiCallWithoutResult(int attempts, const TaskChainSP &taskChain, Callee *callee, Method method,
                                             Signal signal, Args... args)
    {
        return Proof::AbstractRestApi::chainedApiCallWithoutResult(attempts, taskChain, callee, method, signal, args...);
    }

protected:
    RestApiConsumer() {}
    virtual ~RestApiConsumer() {}
};
} // namespace Proof

#endif // PROOFNETWORK_RESTAPICONSUMER_H
