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
#ifndef PROOFOBJECT_P_H
#define PROOFOBJECT_P_H

#include "proofseed/proofalgorithms.h"

#include "proofcore/proofcore_global.h"
#include "proofcore/proofglobal.h"
#include "proofcore/proofobject.h"

#include <QtGlobal>

#include <algorithm>
#include <atomic>
#include <functional>
#include <tuple>

namespace Proof {
class PROOF_CORE_EXPORT ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(ProofObject)
public:
    ProofObjectPrivate();
    ProofObjectPrivate(const ProofObjectPrivate &other) = delete;
    ProofObjectPrivate &operator=(const ProofObjectPrivate &other) = delete;
    ProofObjectPrivate(ProofObjectPrivate &&other) = delete;
    ProofObjectPrivate &operator=(ProofObjectPrivate &&other) = delete;
    virtual ~ProofObjectPrivate();

    bool isDirtyItself() const;
    void setDirty(bool arg) const;

    template <class... Children>
    void registerChildren(const Children &... children) const
    {
        childrenDirtyCheckers << [v = std::make_tuple(&children...)]() { return ProofObject::dirtyCheck<0>(v); };
    }

    ProofObject *q_ptr = nullptr;

private:
    mutable QVector<std::function<bool()>> childrenDirtyCheckers;
    mutable std::atomic<qulonglong> nextDelayedCallId{0};
    static ProofObject *defaultInvoker;
    mutable bool dirtyFlag = false;
};
} // namespace Proof

#endif // PROOFOBJECT_P_H
