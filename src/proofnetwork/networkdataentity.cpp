#include "networkdataentity.h"
#include "networkdataentity_p.h"

using namespace Proof;

NetworkDataEntity::NetworkDataEntity(NetworkDataEntityPrivate &dd, QObject *parent)
    : ProofObject(dd, parent)
{
}

bool NetworkDataEntity::isFetched() const
{
    Q_D(const NetworkDataEntity);
    return d->isFetched;
}

void NetworkDataEntity::updateFrom(const Proof::NetworkDataEntitySP &other)
{
    Q_ASSERT(other);
    if (other->isFetched())
        setFetched(other->isFetched());
}

void NetworkDataEntity::setFetched(bool fetched)
{
    Q_D(NetworkDataEntity);
    if (d->isFetched != fetched) {
        d->isFetched = fetched;
        emit isFetchedChanged(fetched);
    }
}

