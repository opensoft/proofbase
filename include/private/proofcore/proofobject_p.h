#ifndef PROOFOBJECT_P_H
#define PROOFOBJECT_P_H

#include "proofseed/proofalgorithms.h"

#include "proofcore/proofcore_global.h"
#include "proofcore/proofglobal.h"
#include "proofcore/proofobject.h"

#include <QtGlobal>

#include <algorithm>
#include <atomic>
#include <functional>
#include <tuple>

namespace Proof {
class PROOF_CORE_EXPORT ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(ProofObject)
public:
    ProofObjectPrivate();
    virtual ~ProofObjectPrivate();

    bool isDirtyItself() const;
    void setDirty(bool arg) const;

    template <class... Children>
    void registerChildren(const Children &... children) const
    {
        childrenDirtyCheckers << [v = std::make_tuple(&children...)]() { return ProofObject::dirtyCheck<0>(v); };
    }

    ProofObject *q_ptr = nullptr;

private:
    ProofObjectPrivate(const ProofObjectPrivate &other) = delete;
    ProofObjectPrivate &operator=(const ProofObjectPrivate &other) = delete;
    ProofObjectPrivate(const ProofObjectPrivate &&other) = delete;
    ProofObjectPrivate &operator=(const ProofObjectPrivate &&other) = delete;

    mutable QVector<std::function<bool()>> childrenDirtyCheckers;
    mutable std::atomic<qulonglong> nextDelayedCallId{0};
    static ProofObject *defaultInvoker;
    mutable bool dirtyFlag = false;
};
} // namespace Proof

#endif // PROOFOBJECT_P_H
