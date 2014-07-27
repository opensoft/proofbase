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
}
bool NetworkDataEntityQmlWrapper::isFetched() const
{
    Q_D(const NetworkDataEntityQmlWrapper);
    Q_ASSERT(d->dataEntity);
    return d->dataEntity->isFetched();
}

void NetworkDataEntityQmlWrapper::setEntity(const QSharedPointer<NetworkDataEntity> &networkDataEntity)
{
    Q_D(NetworkDataEntityQmlWrapper);
    d->dataEntity = networkDataEntity;
    setupEntity();
}
