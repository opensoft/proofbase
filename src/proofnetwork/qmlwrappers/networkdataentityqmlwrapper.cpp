#include "networkdataentityqmlwrapper.h"

#include "networkdataentityqmlwrapper_p.h"

#include "proofcore/proofobject_p.h"

#include "proofnetwork/networkdataentity.h"

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
    if (call(this, &NetworkDataEntityQmlWrapper::setEntity, Call::Block, networkDataEntity))
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
