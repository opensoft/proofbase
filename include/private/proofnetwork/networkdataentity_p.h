#ifndef NETWORKDATAENTITY_P_H
#define NETWORKDATAENTITY_P_H

#include "proofcore/proofobject_p.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofcore/objectscache.h"

#include <QtGlobal>

#include <functional>

namespace Proof {
class PROOF_NETWORK_EXPORT NetworkDataEntityPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(NetworkDataEntity)
public:
    NetworkDataEntityPrivate() : ProofObjectPrivate() {}

    template<class T, class Entity, class EntityKey>
    QSharedPointer<Entity> updateEntityWeakPtr(QWeakPointer<Entity> &storedEntity, EntityKey &storedKey, const EntityKey &key,
                                               ObjectsCache<EntityKey, Entity> &cache, std::function<EntityKey(Entity *)> &&keyFunction,
                                               T *notifySignalEmitter, std::function<void(T *, QSharedPointer<Entity>)> &&notifySignal,
                                               std::function<QSharedPointer<Entity>()> &&customEntityCreator)
    {
        QSharedPointer<Entity> result = storedEntity.toStrongRef();
        qlonglong oldKey = result ? keyFunction(result.data()) : storedKey;
        if (oldKey != key || !result || result == Entity::defaultObject()) {
            result = cache.value(key);
            if (!result) {
                result = customEntityCreator();
                cache.add(key, result);
            }
            storedEntity = result.toWeakRef();
            storedKey = result->id();
            if (oldKey != key)
                emit notifySignal(notifySignalEmitter, result);
        }
        return result;
    }

    template<class T, class Entity, class EntityKey>
    QSharedPointer<Entity> updateEntityWeakPtr(QWeakPointer<Entity> &storedEntity, EntityKey &storedKey, const EntityKey &key,
                                               ObjectsCache<EntityKey, Entity> &cache, std::function<EntityKey(Entity *)> &&keyFunction,
                                               T *notifySignalEmitter, std::function<void(T *, QSharedPointer<Entity>)> &&notifySignal)
    {
        return updateEntityWeakPtr<T, Entity, EntityKey>(storedEntity, storedKey, key,
                                                         cache, std::move(keyFunction),
                                                         notifySignalEmitter, std::move(notifySignal),
                                                         std::bind(&Entity::create, key));
    }

    template<class T, class Entity, class EntityKey>
    QSharedPointer<Entity> updateEntitySharedPtr(QSharedPointer<Entity> &storedEntity, const EntityKey &key,
                                                 ObjectsCache<EntityKey, Entity> &cache, std::function<EntityKey(Entity *)> &&keyFunction,
                                                 T *notifySignalEmitter, std::function<void(T *, QSharedPointer<Entity>)> &&notifySignal,
                                                 std::function<QSharedPointer<Entity>()> &&customEntityCreator)
    {
        if (!storedEntity || storedEntity == Entity::defaultObject() || keyFunction(storedEntity.data()) != key) {
            QSharedPointer<Entity> newEntity = cache.value(key);
            if (!newEntity) {
                newEntity = customEntityCreator();
                cache.add(key, newEntity);
            }
            storedEntity = newEntity;
            emit notifySignal(notifySignalEmitter, storedEntity);
        }
        return storedEntity;
    }

    template<class T, class Entity, class EntityKey>
    QSharedPointer<Entity> updateEntitySharedPtr(QSharedPointer<Entity> &storedEntity, const EntityKey &key,
                                                 ObjectsCache<EntityKey, Entity> &cache, std::function<EntityKey(Entity *)> &&keyFunction,
                                                 T *notifySignalEmitter, std::function<void(T *, QSharedPointer<Entity>)> &&notifySignal)
    {
        return updateEntitySharedPtr<T, Entity, EntityKey>(storedEntity, key,
                                                           cache, std::move(keyFunction),
                                                           notifySignalEmitter, std::move(notifySignal),
                                                           std::bind(&Entity::create, key));
    }

    bool isFetched = false;
    NetworkDataEntityWP weakSelf;
};
}

#endif // NETWORKDATAENTITY_P_H
