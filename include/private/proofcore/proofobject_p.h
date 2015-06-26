#ifndef PROOFOBJECT_P_H
#define PROOFOBJECT_P_H

#include "proofcore/proofcore_global.h"

#include <QtGlobal>

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

    template<class... Childs>
    void registerChilds(const Childs &... childs)
    {
        std::tuple<const Childs *...> pointerToChilds(&childs...);
        m_childsIsDirty << [pointerToChilds, this] { return isTupleDirty<0>(pointerToChilds); };
    }

    ProofObject *q_ptr = 0;

private:
    ProofObjectPrivate(const ProofObjectPrivate &other) = delete;
    ProofObjectPrivate &operator=(const ProofObjectPrivate &other) = delete;
    ProofObjectPrivate(const ProofObjectPrivate &&other) = delete;
    ProofObjectPrivate &operator=(const ProofObjectPrivate &&other) = delete;

    template<class T>
    bool isChildDirty(const QSharedPointer<T> &child)
    {
        return child.data()->isDirty();
    }

    template<class Container>
    bool isChildDirty(const Container &container)
    {
        return std::any_of(container.begin(), container.end(),
                           [this](decltype(*container.begin()) child) { return isChildDirty(child); });
    }

    template<std::size_t N, class Tuple>
    typename std::enable_if<(N != std::tuple_size<Tuple>::value - 1), bool>::type
    isTupleDirty(const Tuple &tuple)
    {
        return isChildDirty(*std::get<N>(tuple)) || isTupleDirty<N + 1>(tuple);
    }

    template<std::size_t N, class Tuple>
    typename std::enable_if<(N == std::tuple_size<Tuple>::value - 1), bool>::type
    isTupleDirty(const Tuple &tuple)
    {
        return isChildDirty(*std::get<N>(tuple));
    }

    QList<std::function<bool ()>> m_childsIsDirty;
    mutable QAtomicInteger<qulonglong> nextDelayedCallId {0};
    bool m_isDirty = false;
};
}

#endif // PROOFOBJECT_P_H
