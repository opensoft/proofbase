#ifndef OBJECTSCACHE_H
#define OBJECTSCACHE_H

#include <QHash>
#include <QSharedPointer>
#include <QWeakPointer>

namespace Proof {

template<class Key, class T>
class WeakObjectsCache;
template<class Key, class T>
class StrongObjectsCache;

//TODO: make caches thread-safe
//TODO: add time-based cache
template<class Key, class T>
class ObjectsCache
{
public:
    virtual void add(const Key &key, const QSharedPointer<T> &object) = 0;
    virtual void remove(const Key &key) = 0;
    virtual void clear() = 0;
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
        m_cache[key] = object.toWeakRef();
    }

    void remove(const Key &key) override
    {
        m_cache.remove(key);
    }

    void clear() override
    {
        m_cache.clear();
    }

    bool contains(const Key &key) const override
    {
        return m_cache.contains(key);
    }

    QSharedPointer<T> value(const Key &key, bool useOtherCaches = true) override
    {
        QSharedPointer<T> foundValue;
        if (!contains(key)) {
            if (useOtherCaches)
                foundValue = StrongObjectsCache<Key, T>::instance().value(key, false);
        } else {
            foundValue = m_cache[key].toStrongRef();
            if (!foundValue)
                m_cache.remove(key);
        }
        if (!foundValue && key == Key())
            foundValue = T::defaultEntity();
        return foundValue;
    }

private:
    WeakObjectsCache() : ObjectsCache<Key, T>() {}
    ~WeakObjectsCache() {}
    WeakObjectsCache(const WeakObjectsCache &other) = delete;
    WeakObjectsCache &operator=(const WeakObjectsCache &other) = delete;
    WeakObjectsCache(const WeakObjectsCache &&other) = delete;
    WeakObjectsCache &operator=(const WeakObjectsCache &&other) = delete;

    friend class ObjectsCache<Key, T>;

    QHash<Key, QWeakPointer<T>> m_cache;

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
        m_cache[key] = object;
    }

    void remove(const Key &key) override
    {
        m_cache.remove(key);
    }

    void clear() override
    {
        m_cache.clear();
    }

    bool contains(const Key &key) const override
    {
        return m_cache.contains(key);
    }

    QSharedPointer<T> value(const Key &key, bool useOtherCaches = true) override
    {
        QSharedPointer<T> foundValue = m_cache.value(key, QSharedPointer<T>());
        if (!foundValue && useOtherCaches)
            foundValue = WeakObjectsCache<Key, T>::instance().value(key, false);
        if (!foundValue && key == Key())
            foundValue = T::defaultEntity();
        return foundValue;
    }

private:
    StrongObjectsCache() : ObjectsCache<Key, T>() {}
    ~StrongObjectsCache() {}
    StrongObjectsCache(const StrongObjectsCache &other) = delete;
    StrongObjectsCache &operator=(const StrongObjectsCache &other) = delete;
    StrongObjectsCache(const StrongObjectsCache &&other) = delete;
    StrongObjectsCache &operator=(const StrongObjectsCache &&other) = delete;

    friend class ObjectsCache<Key, T>;

    QHash<Key, QSharedPointer<T>> m_cache;
};

}

#endif // OBJECTSCACHE_H
