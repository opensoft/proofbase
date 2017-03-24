#ifndef NETWORKDATAENTITY_P_H
#define NETWORKDATAENTITY_P_H

#include "proofcore/proofobject_p.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofcore/objectscache.h"
#include "proofcore/spinlock_p.h"

#include <QtGlobal>

#include <functional>

namespace Proof {
class PROOF_NETWORK_EXPORT NetworkDataEntityPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(NetworkDataEntity)
public:
    NetworkDataEntityPrivate() : ProofObjectPrivate() {}

    template<class T, class Entity, class EntityKey>
    QSharedPointer<Entity> updateEntityField(QWeakPointer<Entity> &storedEntity, EntityKey &storedKey, const EntityKey &key,
                                             ObjectsCache<EntityKey, Entity> &cache, std::function<EntityKey(Entity *)> &&keyFunction,
                                             T *notifySignalEmitter, std::function<void(T *, QSharedPointer<Entity>)> &&notifySignal,
                                             std::function<QSharedPointer<Entity>()> &&customEntityCreator)
    {
        QSharedPointer<Entity> result = storedEntity.toStrongRef();
        EntityKey oldKey = result ? keyFunction(result.data()) : storedKey;
        if (oldKey != key || !Entity::isValidAndDirty(result)) {
            result = cache.add(key, std::move(customEntityCreator));
            storedEntity = result.toWeakRef();
            storedKey = keyFunction(result.data());
            if (oldKey != key)
                emit notifySignal(notifySignalEmitter, result);
        }
        return result;
    }

    template<class T, class Entity, class EntityKey>
    QSharedPointer<Entity> updateEntityField(QWeakPointer<Entity> &storedEntity, EntityKey &storedKey, const EntityKey &key,
                                             ObjectsCache<EntityKey, Entity> &cache, std::function<EntityKey(Entity *)> &&keyFunction,
                                             T *notifySignalEmitter, std::function<void(T *, QSharedPointer<Entity>)> &&notifySignal)
    {
        return updateEntityField<T, Entity, EntityKey>(storedEntity, storedKey, key,
                                                       cache, std::move(keyFunction),
                                                       notifySignalEmitter, std::move(notifySignal),
                                                       std::bind(&Entity::create, key));
    }

    template<class T, class Entity, class EntityKey>
    QSharedPointer<Entity> updateEntityField(QSharedPointer<Entity> &storedEntity, const EntityKey &key,
                                             ObjectsCache<EntityKey, Entity> &cache, std::function<EntityKey(Entity *)> &&keyFunction,
                                             T *notifySignalEmitter, std::function<void(T *, QSharedPointer<Entity>)> &&notifySignal,
                                             std::function<QSharedPointer<Entity>()> &&customEntityCreator)
    {
        if (!Entity::isValidAndDirty(storedEntity) || keyFunction(storedEntity.data()) != key) {
            QSharedPointer<Entity> newEntity = cache.add(key, std::move(customEntityCreator));
            storedEntity = newEntity;
            emit notifySignal(notifySignalEmitter, storedEntity);
        }
        return storedEntity;
    }

    template<class T, class Entity, class EntityKey>
    QSharedPointer<Entity> updateEntityField(QSharedPointer<Entity> &storedEntity, const EntityKey &key,
                                             ObjectsCache<EntityKey, Entity> &cache, std::function<EntityKey(Entity *)> &&keyFunction,
                                             T *notifySignalEmitter, std::function<void(T *, QSharedPointer<Entity>)> &&notifySignal)
    {
        return updateEntityField<T, Entity, EntityKey>(storedEntity, key,
                                                       cache, std::move(keyFunction),
                                                       notifySignalEmitter, std::move(notifySignal),
                                                       std::bind(&Entity::create, key));
    }

    virtual void updateFrom(const Proof::NetworkDataEntitySP &other);

    bool isFetched = false;
    mutable NetworkDataEntityWP weakSelf;
    mutable SpinLock spinLock;
};
}

#endif // NETWORKDATAENTITY_P_H
