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
#include "proofnetwork/networkdataentity.h"

#include "proofnetwork/networkdataentity_p.h"

using namespace Proof;

NetworkDataEntity::NetworkDataEntity(NetworkDataEntityPrivate &dd) : ProofObject(dd, nullptr)
{}

bool NetworkDataEntity::isFetched() const
{
    Q_D_CONST(NetworkDataEntity);
    return d->isFetched;
}

NetworkDataEntity::NetworkDataEntity() : NetworkDataEntity(*new NetworkDataEntityPrivate)
{}

void NetworkDataEntity::updateFrom(const Proof::NetworkDataEntitySP &other)
{
    Q_ASSERT(other);

    const NetworkDataEntity *constOther = other.data();
    if (constOther == this)
        return;

    Q_D(NetworkDataEntity);

    forever {
        d->spinLock.lock();
        if (constOther->d_func()->spinLock.tryLock())
            break;
        d->spinLock.unlock();
        QThread::yieldCurrentThread();
    }

    updateSelf(other);
    d->setDirty(constOther->d_func()->isDirtyItself());

    constOther->d_func()->spinLock.unlock();
    d->spinLock.unlock();
}

void NetworkDataEntity::setFetched(bool fetched)
{
    Q_D(NetworkDataEntity);
    if (d->isFetched != fetched) {
        d->isFetched = fetched;
        emit isFetchedChanged(fetched);
    }
}

NetworkDataEntitySP NetworkDataEntity::selfPtr() const
{
    Q_D_CONST(NetworkDataEntity);
    return d->weakSelf.toStrongRef();
}

void NetworkDataEntity::initSelfWeakPtr(const NetworkDataEntitySP &entity)
{
    const NetworkDataEntity *constEntity = entity.data();
    constEntity->d_func()->weakSelf = entity;
}

void NetworkDataEntity::updateSelf(const NetworkDataEntitySP &other)
{
    if (other->isFetched())
        setFetched(other->isFetched());
}

NetworkDataEntityPrivate::NetworkDataEntityPrivate() : ProofObjectPrivate()
{}

NetworkDataEntityPrivate::~NetworkDataEntityPrivate()
{}
