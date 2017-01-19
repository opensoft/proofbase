#ifndef PROOFOBJECT_P_H
#define PROOFOBJECT_P_H

#include "proofcore/proofcore_global.h"

#include <QtGlobal>

#include <atomic>
#include <tuple>
#include <functional>
#include <algorithm>

namespace Proof {
class ProofObject;
class PROOF_CORE_EXPORT ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(ProofObject)
public:
    ProofObjectPrivate() {}
    virtual ~ProofObjectPrivate() {}

    bool isDirty() const;
    bool isDirtyItself() const;
    void setDirty(bool arg);

    template<class... Children>
    void registerChildren(const Children &... children)
    {
        std::tuple<const Children *...> childrenPointers(&children...);
        childrenDirtyCheckers << [childrenPointers, this]() { return isDirty<0>(childrenPointers); };
    }

    ProofObject *q_ptr = 0;

private:
    ProofObjectPrivate(const ProofObjectPrivate &other) = delete;
    ProofObjectPrivate &operator=(const ProofObjectPrivate &other) = delete;
    ProofObjectPrivate(const ProofObjectPrivate &&other) = delete;
    ProofObjectPrivate &operator=(const ProofObjectPrivate &&other) = delete;

    template<class T>
    bool isDirty(const QSharedPointer<T> &child)
    {
        return child.data()->isDirty();
    }

    template<class Container>
    bool isDirty(const Container &container)
    {
        return std::any_of(container.begin(), container.end(),
                           [this](decltype(*container.begin()) child) { return isDirty(child); });
    }

    template<std::size_t N, class Tuple>
    typename std::enable_if<(N != std::tuple_size<Tuple>::value - 1), bool>::type
    isDirty(const Tuple &tuple)
    {
        return isDirty(*std::get<N>(tuple)) || isDirty<N + 1>(tuple);
    }

    template<std::size_t N, class Tuple>
    typename std::enable_if<(N == std::tuple_size<Tuple>::value - 1), bool>::type
    isDirty(const Tuple &tuple)
    {
        return isDirty(*std::get<N>(tuple));
    }

    QList<std::function<bool ()>> childrenDirtyCheckers;
    mutable std::atomic<qulonglong> nextDelayedCallId {0};
    bool dirtyFlag = false;
};
}

#endif // PROOFOBJECT_P_H
