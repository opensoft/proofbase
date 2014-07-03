#ifndef PROOFOBJECT_P_H
#define PROOFOBJECT_P_H

#include <QtGlobal>
#include "proofcore_global.h"

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
};
}

#endif // PROOFOBJECT_P_H
