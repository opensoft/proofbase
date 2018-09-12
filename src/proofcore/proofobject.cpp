#include "proofcore/proofobject.h"

#include "proofcore/proofobject_p.h"

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
    Q_D_CONST(ProofObject);
    return d->isDirtyItself() || algorithms::exists(d->childrenDirtyCheckers, [](const auto &f) { return f(); });
}

void ProofObject::markDirty(bool dirty) const
{
    Q_D_CONST(ProofObject);
    d->setDirty(d->dirtyFlag | dirty);
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
    Q_D_CONST(ProofObject);
    return ++d->nextDelayedCallId;
}

ProofObject *ProofObject::defaultInvoker()
{
    return ProofObjectPrivate::defaultInvoker;
}

void ProofObject::addDirtyChecker(const std::function<bool()> &checker) const
{
    Q_D_CONST(ProofObject);
    d->childrenDirtyCheckers << checker;
}

ProofObjectPrivate::ProofObjectPrivate()
{}

ProofObjectPrivate::~ProofObjectPrivate()
{}

bool ProofObjectPrivate::isDirtyItself() const
{
    return dirtyFlag;
}

void ProofObjectPrivate::setDirty(bool arg) const
{
    dirtyFlag = arg;
}
