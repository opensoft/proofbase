#ifndef PROOF_PROOFOBJECTPRIVATEPOINTER_H
#define PROOF_PROOFOBJECTPRIVATEPOINTER_H

#include "proofcore/proofcore_global.h"

#include <QScopedPointer>

namespace Proof {
class ProofObjectPrivate;

class PROOF_CORE_EXPORT ProofObjectPrivatePointer
{
public:
    ProofObjectPrivatePointer(ProofObjectPrivate *d);
    ~ProofObjectPrivatePointer();

    ProofObjectPrivate *data();
    const ProofObjectPrivate *data() const;

private:
    ProofObjectPrivate *m_pointer;
};

// Functions for macro Q_D. Works thanks to ADL
static inline void *qGetPtrHelper(ProofObjectPrivatePointer &p) { return p.data(); }
static inline const void *qGetPtrHelper(const ProofObjectPrivatePointer &p) { return p.data(); }

} // namespace Proof

#endif // PROOF_PROOFOBJECTPRIVATEPOINTER_H
