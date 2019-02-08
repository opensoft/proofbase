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
#ifndef NETWORKDATAENTITYHELPERS_H
#define NETWORKDATAENTITYHELPERS_H

#include "proofcore/objectscache.h"

#include <QSharedPointer>
#include <QWeakPointer>

namespace Proof {
struct NetworkDataEntityHelpers
{
    NetworkDataEntityHelpers() = delete;
    NetworkDataEntityHelpers(const NetworkDataEntityHelpers &) = delete;
    NetworkDataEntityHelpers &operator=(const NetworkDataEntityHelpers &) = delete;
    NetworkDataEntityHelpers(NetworkDataEntityHelpers &&) = delete;
    NetworkDataEntityHelpers &operator=(NetworkDataEntityHelpers &&) = delete;
    ~NetworkDataEntityHelpers() = delete;

    template <typename Q, typename Entity, typename Key, typename KeyFunc, typename NotifySignal, typename Creator>
    static auto updateEntityField(QWeakPointer<Entity> &storedEntity, Key &storedKey, const Key &key,
                                  ObjectsCache<Key, Entity> &cache, KeyFunc &&keyFunc, Q *notifier,
                                  NotifySignal &&notifySignal, Creator &&creator)
        -> decltype(std::function<Key(Entity *)>(keyFunc)(cache.value(Key()).data()) == Key(),
                    std::function<void(Q *, QSharedPointer<Entity>)>(notifySignal)(notifier, storedEntity),
                    cache.add(key, creator()), QSharedPointer<Entity>())
    {
        auto castedKeyFunc = std::function<Key(Entity *)>(std::forward<KeyFunc>(keyFunc));
        auto castedSignal = std::function<void(Q *, QSharedPointer<Entity>)>(std::forward<NotifySignal>(notifySignal));
        QSharedPointer<Entity> result = storedEntity.toStrongRef();
        Key oldKey = result ? castedKeyFunc(result.data()) : storedKey;
        if (oldKey != key || !Entity::isValidAndDirty(result)) {
            result = cache.add(key, std::forward<Creator>(creator));
            storedEntity = result.toWeakRef();
            storedKey = castedKeyFunc(result.data());
            if (oldKey != key)
                emit castedSignal(notifier, result);
        }
        return result;
    }

    template <typename Q, typename Entity, typename Key, typename KeyFunc, typename NotifySignal>
    static auto updateEntityField(QWeakPointer<Entity> &storedEntity, Key &storedKey, const Key &key,
                                  ObjectsCache<Key, Entity> &cache, KeyFunc &&keyFunc, Q *notifier,
                                  NotifySignal &&notifySignal)
        -> decltype(std::function<Key(Entity *)>(keyFunc)(cache.value(Key()).data()) == Key(),
                    std::function<void(Q *, QSharedPointer<Entity>)>(notifySignal)(notifier, storedEntity),
                    QSharedPointer<Entity>())
    {
        return updateEntityField(storedEntity, storedKey, key, cache, std::forward<KeyFunc>(keyFunc), notifier,
                                 std::forward<NotifySignal>(notifySignal), std::bind(&Entity::create, key));
    }

    template <typename Q, typename Entity, typename Key, typename KeyFunc, typename NotifySignal, typename Creator>
    static auto updateEntityField(QSharedPointer<Entity> &storedEntity, const Key &key, ObjectsCache<Key, Entity> &cache,
                                  KeyFunc &&keyFunc, Q *notifier, NotifySignal &&notifySignal, Creator &&creator)
        -> decltype(std::function<Key(Entity *)>(keyFunc)(cache.value(Key()).data()) == Key(),
                    std::function<void(Q *, QSharedPointer<Entity>)>(notifySignal)(notifier, storedEntity),
                    cache.add(key, creator()), QSharedPointer<Entity>())
    {
        auto castedKeyFunc = std::function<Key(Entity *)>(std::forward<KeyFunc>(keyFunc));
        auto castedSignal = std::function<void(Q *, QSharedPointer<Entity>)>(std::forward<NotifySignal>(notifySignal));
        if (!Entity::isValidAndDirty(storedEntity) || castedKeyFunc(storedEntity.data()) != key) {
            QSharedPointer<Entity> newEntity = cache.add(key, std::forward<Creator>(creator));
            storedEntity = newEntity;
            emit castedSignal(notifier, storedEntity);
        }
        return storedEntity;
    }

    template <typename Q, typename Entity, typename Key, typename KeyFunc, typename NotifySignal>
    static auto updateEntityField(QSharedPointer<Entity> &storedEntity, const Key &key, ObjectsCache<Key, Entity> &cache,
                                  KeyFunc &&keyFunc, Q *notifier, NotifySignal &&notifySignal)
        -> decltype(std::function<Key(Entity *)>(keyFunc)(cache.value(Key()).data()) == Key(),
                    std::function<void(Q *, QSharedPointer<Entity>)>(notifySignal)(notifier, storedEntity),
                    QSharedPointer<Entity>())
    {
        return updateEntityField(storedEntity, key, cache, std::forward<KeyFunc>(keyFunc), notifier,
                                 std::forward<NotifySignal>(notifySignal), std::bind(&Entity::create, key));
    }
};
} // namespace Proof
#endif // NETWORKDATAENTITYHELPERS_H
