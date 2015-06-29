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

ProofObject::ProofObject(ProofObjectPrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{
    dd.q_ptr = this;
}

bool ProofObject::isDirty() const
{
    Q_D(const ProofObject);
    return d->dirtyFlag;
}

qulonglong ProofObject::nextQueuedCallId() const
{
    Q_D(const ProofObject);
    return ++d->nextDelayedCallId;
}


bool ProofObjectPrivate::isDirty() const
{
    return dirtyFlag || std::any_of(childrenDirtyCheckers.begin(), childrenDirtyCheckers.end(),
                                    [](const std::function<bool ()> &func) { return func(); });
}

bool ProofObjectPrivate::isDirtyItself() const
{
    return dirtyFlag;
}

void ProofObjectPrivate::setDirty(bool arg)
{
    dirtyFlag = arg;
}
