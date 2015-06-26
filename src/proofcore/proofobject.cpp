#include "proofobject.h"
#include "proofobject_p.h"

#include <algorithm>

using namespace Proof;

ProofObject::ProofObject(QObject *parent)
    : ProofObject(*new ProofObjectPrivate, parent)
{
}

ProofObject::~ProofObject()
{
}

bool ProofObject::isDirty() const
{
    Q_D(const ProofObject);
    return d->m_isDirty;
}

ProofObject::ProofObject(ProofObjectPrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{
    dd.q_ptr = this;
}

qulonglong ProofObject::nextQueuedCallId() const
{
    Q_D(const ProofObject);
    return ++d->nextDelayedCallId;
}


bool ProofObjectPrivate::isDirty() const
{
    return m_isDirty || std::any_of(m_childsIsDirty.begin(), m_childsIsDirty.end(),
                                    [](const std::function<bool ()> &func) { return func(); });
}

bool ProofObjectPrivate::isDirtyItself() const
{
    return m_isDirty;
}

void ProofObjectPrivate::setDirty(bool arg)
{
    m_isDirty = arg;
}
