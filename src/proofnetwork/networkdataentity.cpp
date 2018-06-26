#include "networkdataentity.h"

#include "networkdataentity_p.h"

using namespace Proof;

NetworkDataEntity::NetworkDataEntity(NetworkDataEntityPrivate &dd) : ProofObject(dd, nullptr)
{}

bool NetworkDataEntity::isFetched() const
{
    Q_D(const NetworkDataEntity);
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
    Q_D(const NetworkDataEntity);
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
