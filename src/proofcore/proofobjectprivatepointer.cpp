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

ProofObjectPrivate *ProofObjectPrivatePointer::data()
{
    m_pointer->setDirty(true);
    return m_pointer;
}

const ProofObjectPrivate *ProofObjectPrivatePointer::data() const
{
    return m_pointer;
}

} // namespace Proof
