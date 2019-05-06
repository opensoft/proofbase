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
#ifndef ENTITY_H
#define ENTITY_H

#include "proofcore/proofobject.h"

#include "proofnetwork/networkdataentityhelpers.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

namespace Proof {
class NetworkDataEntityQmlWrapper;
class NetworkDataEntityPrivate;
//TODO: make NDEs thread-safe when and if is needed
class PROOF_NETWORK_EXPORT NetworkDataEntity : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(NetworkDataEntity)
public:
    using NDE = NetworkDataEntityHelpers;
    bool isFetched() const;

    template <class Argument>
    static typename std::enable_if<std::is_base_of<NetworkDataEntity, Argument>::value, bool>::type
    isValidAndDirty(const QSharedPointer<Argument> &sp)
    {
        return sp && sp->isDirty();
    }

    void updateFrom(const Proof::NetworkDataEntitySP &other);
    virtual NetworkDataEntityQmlWrapper *toQmlWrapper(QObject *parent = nullptr) const = 0;

signals:
    void isFetchedChanged(bool arg);

protected:
    NetworkDataEntity();
    explicit NetworkDataEntity(NetworkDataEntityPrivate &dd);
    void setFetched(bool fetched);

    virtual void updateSelf(const Proof::NetworkDataEntitySP &other);

    NetworkDataEntitySP selfPtr() const;

    template <typename Entity>
    QSharedPointer<Entity> castedSelfPtr() const
    {
        auto self = selfPtr();
        return self ? qSharedPointerCast<Entity>(self) : QSharedPointer<Entity>();
    }

    static void initSelfWeakPtr(const NetworkDataEntitySP &entity);
};
} // namespace Proof

#endif // ENTITY_H
