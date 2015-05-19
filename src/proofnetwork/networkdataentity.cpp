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
    Q_D(NetworkDataEntity);

    if (other == d->weakSelf)
        return;

    forever {
        d->spinLock.lock();
        if (other->d_func()->spinLock.tryLock())
            break;
        d->spinLock.unlock();
        d->spinLock.sleep();
    }

    d->updateFrom(other);

    other->d_func()->spinLock.unlock();
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

void NetworkDataEntityPrivate::updateFrom(const NetworkDataEntitySP &other)
{
    Q_Q(NetworkDataEntity);
    if (other->isFetched())
        q->setFetched(other->isFetched());
}
