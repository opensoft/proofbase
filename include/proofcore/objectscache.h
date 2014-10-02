#ifndef OBJECTSCACHE_H
#define OBJECTSCACHE_H

#include "proofobject.h"

#include <QHash>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QReadWriteLock>

namespace Proof {

template<class Key, class T>
class WeakObjectsCache;
template<class Key, class T>
class StrongObjectsCache;

template<class Key, class T>
class ObjectsCache
{
public:
    virtual void add(const Key &key, const QSharedPointer<T> &object) = 0;
    virtual void remove(const Key &key) = 0;
    virtual void clear() = 0;
    virtual bool isEmpty() const = 0;
    virtual bool contains(const Key &key) const = 0;
    virtual QSharedPointer<T> value(const Key &key, bool useOtherCaches = true) = 0;

protected:
    ObjectsCache() {}
    virtual ~ObjectsCache() {}
    ObjectsCache(const ObjectsCache &other) = delete;
    ObjectsCache &operator=(const ObjectsCache &other) = delete;
    ObjectsCache(const ObjectsCache &&other) = delete;
    ObjectsCache &operator=(const ObjectsCache &&other) = delete;
};


template<class Key, class T>
class WeakObjectsCache : public ObjectsCache<Key, T>
{
public:
    static ObjectsCache<Key, T> &instance()
    {
        static WeakObjectsCache<Key, T> inst;
        return inst;
    }

    void add(const Key &key, const QSharedPointer<T> &object) override
    {
        if (!object || key == Key())
            return;
        m_cacheLock.lockForWrite();
        m_cache[key] = object.toWeakRef();
        m_cacheLock.unlock();
    }

    void remove(const Key &key) override
    {
        m_cacheLock.lockForWrite();
        m_cache.remove(key);
        m_cacheLock.unlock();
    }

    void clear() override;
    bool isEmpty() const override;

    bool contains(const Key &key) const override
    {
        m_cacheLock.lockForRead();
        bool result = m_cache.contains(key);
        m_cacheLock.unlock();
        return result;
    }

    QSharedPointer<T> value(const Key &key, bool useOtherCaches = true) override
    {
        QSharedPointer<T> foundValue;
        bool removeFromCache = false;
        m_cacheLock.lockForRead();
        if (!m_cache.contains(key)) {
            m_cacheLock.unlock();
            if (useOtherCaches)
                foundValue = StrongObjectsCache<Key, T>::instance().value(key, false);
        } else {
            QWeakPointer<T> cachedValue = m_cache[key];
            m_cacheLock.unlock();
            foundValue = cachedValue.toStrongRef();
            removeFromCache = !foundValue;
        }
        if (!foundValue && key == Key())
            foundValue = T::defaultObject();
        if (removeFromCache) {
            m_cacheLock.lockForWrite();
            if (m_cache.contains(key) && !m_cache[key])
                m_cache.remove(key);
            m_cacheLock.unlock();
        }
        return foundValue;
    }

private:
    WeakObjectsCache() : ObjectsCache<Key, T>() {}
    ~WeakObjectsCache() {}
    WeakObjectsCache(const WeakObjectsCache &other) = delete;
    WeakObjectsCache &operator=(const WeakObjectsCache &other) = delete;
    WeakObjectsCache(const WeakObjectsCache &&other) = delete;
    WeakObjectsCache &operator=(const WeakObjectsCache &&other) = delete;

    QHash<Key, QWeakPointer<T>> m_cache;
    mutable QReadWriteLock m_cacheLock;
};

template<class Key, class T>
class StrongObjectsCache : public ObjectsCache<Key, T>
{
public:
    static ObjectsCache<Key, T> &instance()
    {
        static StrongObjectsCache<Key, T> inst;
        return inst;
    }

    void add(const Key &key, const QSharedPointer<T> &object) override
    {
        if (!object || key == Key())
            return;
        m_cacheLock.lockForWrite();
        m_cache[key] = object;
        m_cacheLock.unlock();
    }

    void remove(const Key &key) override
    {
        m_cacheLock.lockForWrite();
        m_cache.remove(key);
        m_cacheLock.unlock();
    }

    void clear() override;
    bool isEmpty() const override;

    bool contains(const Key &key) const override
    {
        m_cacheLock.lockForRead();
        bool result = m_cache.contains(key);
        m_cacheLock.unlock();
        return result;
    }

    QSharedPointer<T> value(const Key &key, bool useOtherCaches = true) override
    {
        m_cacheLock.lockForRead();
        QSharedPointer<T> foundValue = m_cache.value(key, QSharedPointer<T>());
        m_cacheLock.unlock();
        if (!foundValue && useOtherCaches)
            foundValue = WeakObjectsCache<Key, T>::instance().value(key, false);
        if (!foundValue && key == Key())
            foundValue = T::defaultObject();
        return foundValue;
    }

private:
    StrongObjectsCache() : ObjectsCache<Key, T>() {}
    ~StrongObjectsCache() {}
    StrongObjectsCache(const StrongObjectsCache &other) = delete;
    StrongObjectsCache &operator=(const StrongObjectsCache &other) = delete;
    StrongObjectsCache(const StrongObjectsCache &&other) = delete;
    StrongObjectsCache &operator=(const StrongObjectsCache &&other) = delete;

    QHash<Key, QSharedPointer<T>> m_cache;
    mutable QReadWriteLock m_cacheLock;
};

}

#endif // OBJECTSCACHE_H
