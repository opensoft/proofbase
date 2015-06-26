#include "proofobjectprivatepointer.h"

#include "proofobject_p.h"

namespace Proof {

ProofObjectPrivatePointer::ProofObjectPrivatePointer(ProofObjectPrivate *d)
    : m_pointer(d)
{
}

ProofObjectPrivatePointer::~ProofObjectPrivatePointer()
{
    delete m_pointer;
}

void *ProofObjectPrivatePointer::data()
{
    m_pointer->setDirty(true);
    return m_pointer;
}

const void *ProofObjectPrivatePointer::data() const
{
    return m_pointer;
}

} // namespace Proof
