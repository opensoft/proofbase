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

    d->updateFrom(other);
    d->setDirty(constOther->d_func()->isDirtyHimself());

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

void NetworkDataEntity::makeWeakSelf(const NetworkDataEntitySP &entity)
{
    const NetworkDataEntity *constEntity = entity.data();
    constEntity->d_func()->weakSelf = entity;
}

void NetworkDataEntityPrivate::updateFrom(const NetworkDataEntitySP &other)
{
    Q_Q(NetworkDataEntity);
    if (other->isFetched())
        q->setFetched(other->isFetched());
}
