#ifndef PROOFOBJECT_P_H
#define PROOFOBJECT_P_H

#include "proofcore/proofcore_global.h"

#include <QtGlobal>

namespace Proof {
class ProofObject;
class PROOF_CORE_EXPORT ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(ProofObject)
public:
    ProofObjectPrivate() {}
    virtual ~ProofObjectPrivate() {}

    ProofObject *q_ptr = 0;

private:
    ProofObjectPrivate(const ProofObjectPrivate &other) = delete;
    ProofObjectPrivate &operator=(const ProofObjectPrivate &other) = delete;
    ProofObjectPrivate(const ProofObjectPrivate &&other) = delete;
    ProofObjectPrivate &operator=(const ProofObjectPrivate &&other) = delete;

    QAtomicInteger<qulonglong> nextDelayedCallId {0};
};
}

#endif // PROOFOBJECT_P_H
