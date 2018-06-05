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
    template <class Callee, class Result, class Method, class Signal, class... Args>
    Proof::RestApiError chainedApiCall(const Proof::TaskChainSP &taskChain, Callee *callee, Method method,
                                       Signal signal, Result &&result, Args... args) const
    {
        return AbstractRestApi::chainedApiCall(taskChain, callee, method, signal, std::forward<Result>(result), args...);
    }
    template <class Callee, class Result, class Method, class Signal, class... Args>
    RestApiError chainedApiCall(int attempts, const TaskChainSP &taskChain, Callee *callee, Method method,
                                Signal signal, Result &&result, Args... args)
    {
        return AbstractRestApi::chainedApiCall(attempts, taskChain, callee, method, signal,
                                               std::forward<Result>(result), args...);
    }
    template <class Callee, class Method, class Signal, class... Args>
    RestApiError chainedApiCallWithoutResult(const TaskChainSP &taskChain, Callee *callee, Method method, Signal signal,
                                             Args... args) const
    {
        return AbstractRestApi::chainedApiCallWithoutResult(taskChain, callee, method, signal, args...);
    }
    template <class Callee, class Method, class Signal, class... Args>
    RestApiError chainedApiCallWithoutResult(int attempts, const TaskChainSP &taskChain, Callee *callee, Method method,
                                             Signal signal, Args... args)
    {
        return AbstractRestApi::chainedApiCallWithoutResult(attempts, taskChain, callee, method, signal, args...);
    }

protected:
    RestApiConsumer() {}
    virtual ~RestApiConsumer();
};
} // namespace Proof

#endif // PROOFNETWORK_RESTAPICONSUMER_H
