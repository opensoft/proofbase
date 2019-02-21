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
#include "proofnetwork/qmlwrappers/networkdataentityqmlwrapper.h"

#include "proofcore/proofobject_p.h"

#include "proofnetwork/networkdataentity.h"
#include "proofnetwork/qmlwrappers/networkdataentityqmlwrapper_p.h"

using namespace Proof;

NetworkDataEntityQmlWrapper::NetworkDataEntityQmlWrapper(const QSharedPointer<NetworkDataEntity> &networkDataEntity,
                                                         NetworkDataEntityQmlWrapperPrivate &dd, QObject *parent)
    : ProofObject(dd, parent)
{
    Q_D(NetworkDataEntityQmlWrapper);
    d->dataEntity = networkDataEntity;
    d->entityConnectContext = new QObject(this);
}

NetworkDataEntityQmlWrapper::NetworkDataEntityQmlWrapper(const QSharedPointer<NetworkDataEntity> &networkDataEntity,
                                                         QObject *parent)
    : NetworkDataEntityQmlWrapper(networkDataEntity, *new NetworkDataEntityQmlWrapperPrivate, parent)
{}

QObject *NetworkDataEntityQmlWrapper::entityConnectContext() const
{
    Q_D_CONST(NetworkDataEntityQmlWrapper);
    return d->entityConnectContext;
}

bool NetworkDataEntityQmlWrapper::isFetched() const
{
    Q_D_CONST(NetworkDataEntityQmlWrapper);
    Q_ASSERT(d->dataEntity);
    return d->dataEntity->isFetched();
}

QSharedPointer<NetworkDataEntity> NetworkDataEntityQmlWrapper::entity()
{
    Q_D_CONST(NetworkDataEntityQmlWrapper);
    return d->dataEntity;
}

const QSharedPointer<NetworkDataEntity> NetworkDataEntityQmlWrapper::entity() const
{
    Q_D_CONST(NetworkDataEntityQmlWrapper);
    return d->dataEntity;
}

void NetworkDataEntityQmlWrapper::setEntity(const QSharedPointer<NetworkDataEntity> &networkDataEntity)
{
    if (safeCall(this, &NetworkDataEntityQmlWrapper::setEntity, Call::Block, networkDataEntity))
        return;
    Q_D(NetworkDataEntityQmlWrapper);
    if (d->dataEntity)
        disconnect(d->dataEntity.data(), nullptr, this, nullptr);
    d->entityConnectContext->deleteLater();
    d->entityConnectContext = new QObject(this);
    QSharedPointer<NetworkDataEntity> old = d->dataEntity;
    d->dataEntity = networkDataEntity;
    setupEntity(old);
}
