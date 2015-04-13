#include "proofobject.h"
#include "proofobject_p.h"

using namespace Proof;

ProofObject::ProofObject(QObject *parent)
    : ProofObject(*new ProofObjectPrivate, parent)
{
}

ProofObject::~ProofObject()
{
}

ProofObject::ProofObject(ProofObjectPrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

qulonglong ProofObject::nextQueuedCallId()
{
    Q_D(ProofObject);
    return ++d->nextDelayedCallId;
}
