#include "proofobject.h"

#include "proofobject_p.h"

#include <algorithm>

using namespace Proof;

ProofObject *ProofObjectPrivate::defaultInvoker = new ProofObject(nullptr);

ProofObject::ProofObject(QObject *parent) : ProofObject(*new ProofObjectPrivate, parent)
{}

ProofObject::ProofObject(ProofObjectPrivate &dd, QObject *parent) : QObject(parent), d_ptr(&dd)
{
    dd.q_ptr = this;
}

ProofObject::~ProofObject()
{}

bool ProofObject::isDirty() const
{
    Q_D(const ProofObject);
    return d->isDirty();
}

void ProofObject::emitError(const Failure &failure, Failure::Hints forceHints)
{
    emit errorOccurred(failure.moduleCode, failure.errorCode, failure.message,
                       (failure.hints | forceHints) & Failure::UserFriendlyHint,
                       (failure.hints | forceHints) & Failure::CriticalHint);
}

std::function<void(const Failure &)> ProofObject::simpleFailureHandler(Failure::Hints forceHints)
{
    return [this, forceHints](const Failure &failure) { emitError(failure, forceHints); };
}

qulonglong ProofObject::nextQueuedCallId() const
{
    Q_D(const ProofObject);
    return ++d->nextDelayedCallId;
}

ProofObject *ProofObject::defaultInvoker()
{
    return ProofObjectPrivate::defaultInvoker;
}

ProofObjectPrivate::ProofObjectPrivate()
{}

ProofObjectPrivate::~ProofObjectPrivate()
{}

bool ProofObjectPrivate::isDirty() const
{
    return dirtyFlag
           || std::any_of(childrenDirtyCheckers.begin(), childrenDirtyCheckers.end(),
                          [](const std::function<bool()> &func) { return func(); });
}

bool ProofObjectPrivate::isDirtyItself() const
{
    return dirtyFlag;
}

void ProofObjectPrivate::setDirty(bool arg)
{
    dirtyFlag = arg;
}
