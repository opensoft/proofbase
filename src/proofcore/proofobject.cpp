/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
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
