#ifndef OBJECTSCACHE_H
#define OBJECTSCACHE_H

#include "proofcore/proofobject.h"
#include "proofcore/expirator.h"

#include <QHash>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QReadWriteLock>

#include <type_traits>

/* YOU MUST USE special functions to obtain instance of cache of Proof's exported types.
 * DO NOT USE methods 'instance' for Proof's exported types
 */
//TODO: make contains+add pair atomic (proper way is something to think about)
namespace Proof {

template<class Key, class T>
class WeakObjectsCache;
template<class Key, class T>
class StrongObjectsCache;
template<class Key, class T, class Enable = void>
class GuaranteedLifeTimeObjectsCache;

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
    virtual QList<Key> keys() const = 0;

protected:
    ObjectsCache() {}
    virtual ~ObjectsCache() {}
    ObjectsCache(const ObjectsCache &other) = delete;
    ObjectsCache &operator=(const ObjectsCache &other) = delete;
    ObjectsCache(const ObjectsCache &&other) = delete;
    ObjectsCache &operator=(const ObjectsCache &&other) = delete;

    template<typename U = T>
    QString valueTypeName(typename std::enable_if<!std::is_base_of<QObject, U>::value>::type * = 0) const
    {
        return QString();
    }

    template<typename U = T>
    QString valueTypeName(typename std::enable_if<std::is_base_of<QObject, U>::value>::type * = 0) const
    {
        return U::staticMetaObject.className();
    }

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
        qCDebug(proofCoreCacheLog) << "Adding object" << object.data() << "to"
                                   << this->valueTypeName()
                                   << "weakcache with key" << key;
        m_cacheLock.unlock();
    }

    void remove(const Key &key) override
    {
        m_cacheLock.lockForWrite();
        m_cache.remove(key);
        qCDebug(proofCoreCacheLog) << "Removing object with key" << key << "from"
                                   << this->valueTypeName()
                                   << "weakcache";
        m_cacheLock.unlock();
    }

    void clear()
    {
        m_cacheLock.lockForWrite();
        m_cache.clear();
        qCDebug(proofCoreCacheLog) << "Clearing"
                                   << this->valueTypeName()
                                   << "weakcache";
        m_cacheLock.unlock();
    }

    bool isEmpty() const
    {
        m_cacheLock.lockForRead();
        bool result = m_cache.isEmpty();
        m_cacheLock.unlock();
        return result;
    }

    bool contains(const Key &key) const override
    {
        m_cacheLock.lockForRead();
        bool result = m_cache.contains(key) && m_cache[key];
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
                foundValue = valueFromOtherCaches(key);
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
        qCDebug(proofCoreCacheLog) << "Returning value"
                                   << foundValue.data()
                                   << "from"
                                   << this->valueTypeName()
                                   << "weakcache"
                                   << "for key"
                                   << key;
        return foundValue;
    }

    QList<Key> keys() const override
    {
        return m_cache.keys();
    }

protected:
    virtual QSharedPointer<T> valueFromOtherCaches(const Key &key)
    {
        QSharedPointer<T> foundValue = StrongObjectsCache<Key, T>::instance().value(key, false);
        if (!foundValue)
            foundValue = GuaranteedLifeTimeObjectsCache<Key, T>::instance().value(key, false);
        return foundValue;
    }

    WeakObjectsCache() : ObjectsCache<Key, T>() {}
    ~WeakObjectsCache() {}

private:
    WeakObjectsCache(const WeakObjectsCache &other) = delete;
    WeakObjectsCache &operator=(const WeakObjectsCache &other) = delete;
    WeakObjectsCache(const WeakObjectsCache &&other) = delete;
    WeakObjectsCache &operator=(const WeakObjectsCache &&other) = delete;

    QHash<Key, QWeakPointer<T>> m_cache;
    mutable QReadWriteLock m_cacheLock;
};

//Dummy class for objects that are not based on ProofObject but need to be put in weak or strong cache
//Such objects of course can not be placed in this cache (due to nature of Expirator) but can be placed in other caches
template<class Key, class T>
class GuaranteedLifeTimeObjectsCache
        <Key, T, typename std::enable_if<!std::is_base_of<ProofObject, T>::value>::type>
        : public ObjectsCache<Key, T>
{
public:
    static ObjectsCache<Key, T> &instance()
    {
        static GuaranteedLifeTimeObjectsCache<Key, T> inst;
        return inst;
    }
    void add(const Key &, const QSharedPointer<T> &) override
    {
        Q_ASSERT_X(false, "GuaranteedLifeTimeObjectsCache", "Should not be called for non-ProofObject values");
    }
    void remove(const Key &) override
    {
        Q_ASSERT_X(false, "GuaranteedLifeTimeObjectsCache", "Should not be called for non-ProofObject values");
    }
    void clear() override
    {
        Q_ASSERT_X(false, "GuaranteedLifeTimeObjectsCache", "Should not be called for non-ProofObject values");
    }
    bool isEmpty() const override {return true;}
    bool contains(const Key &) const override {return false;}
    QSharedPointer<T> value(const Key &, bool = true) override {return QSharedPointer<T>();}
    QList<Key> keys() const override {return QList<Key>();}
private:
    GuaranteedLifeTimeObjectsCache() : ObjectsCache<Key, T>() {}
    ~GuaranteedLifeTimeObjectsCache() {}
    GuaranteedLifeTimeObjectsCache(const GuaranteedLifeTimeObjectsCache &other) = delete;
    GuaranteedLifeTimeObjectsCache &operator=(const GuaranteedLifeTimeObjectsCache &other) = delete;
    GuaranteedLifeTimeObjectsCache(const GuaranteedLifeTimeObjectsCache &&other) = delete;
    GuaranteedLifeTimeObjectsCache &operator=(const GuaranteedLifeTimeObjectsCache &&other) = delete;
};

template<class Key, class T>
class GuaranteedLifeTimeObjectsCache
        <Key, T, typename std::enable_if<std::is_base_of<ProofObject, T>::value>::type>
        : public WeakObjectsCache<Key, T>
{
public:
    static GuaranteedLifeTimeObjectsCache<Key, T> &instance()
    {
        static GuaranteedLifeTimeObjectsCache<Key, T> inst;
        return inst;
    }

    void setObjectsMinLifeTime(qulonglong secs)
    {
        m_objectsMinLifeTimeInSeconds = secs;
    }

    void add(const Key &key, const QSharedPointer<T> &object) override
    {
        if (!object || key == Key())
            return;
        addObjectToExpirator(qSharedPointerCast<ProofObject>(object));
        WeakObjectsCache<Key, T>::add(key, object);
    }

protected:
    QSharedPointer<T> valueFromOtherCaches(const Key &key) override
    {
        QSharedPointer<T> foundValue = StrongObjectsCache<Key, T>::instance().value(key, false);
        if (!foundValue)
            foundValue = WeakObjectsCache<Key, T>::instance().value(key, false);
        return foundValue;
    }

private:
    GuaranteedLifeTimeObjectsCache() : WeakObjectsCache<Key, T>() {}
    ~GuaranteedLifeTimeObjectsCache() {}
    GuaranteedLifeTimeObjectsCache(const GuaranteedLifeTimeObjectsCache &other) = delete;
    GuaranteedLifeTimeObjectsCache &operator=(const GuaranteedLifeTimeObjectsCache &other) = delete;
    GuaranteedLifeTimeObjectsCache(const GuaranteedLifeTimeObjectsCache &&other) = delete;
    GuaranteedLifeTimeObjectsCache &operator=(const GuaranteedLifeTimeObjectsCache &&other) = delete;

    void addObjectToExpirator(const QSharedPointer<ProofObject> &object)
    {
        if (m_objectsMinLifeTimeInSeconds) {
            Expirator::instance()->addObject(object, QDateTime::currentDateTime().addSecs(m_objectsMinLifeTimeInSeconds));
            qCDebug(proofCoreCacheLog) << "Object"
                                       << this->valueTypeName()
                                       << "added" << object.data() << "to expirator";
        }
    }

    qulonglong m_objectsMinLifeTimeInSeconds = 8 * 60 * 60; //Default value is 8 hours
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
        qCDebug(proofCoreCacheLog) << "Adding object" << object.data() << "to"
                                   << this->valueTypeName()
                                   << "strongcache with key" << key;
        m_cacheLock.unlock();
    }

    void remove(const Key &key) override
    {
        m_cacheLock.lockForWrite();
        m_cache.remove(key);
        qCDebug(proofCoreCacheLog) << "Removing object with key" << key << "from"
                                   << this->valueTypeName()
                                   << "strongcache";
        m_cacheLock.unlock();
    }

    void clear()
    {
        m_cacheLock.lockForWrite();
        m_cache.clear();
        qCDebug(proofCoreCacheLog) << "Clearing"
                                   << this->valueTypeName()
                                   << "strongcache";
        m_cacheLock.unlock();
    }

    bool isEmpty() const
    {
        m_cacheLock.lockForRead();
        bool result = m_cache.isEmpty();
        m_cacheLock.unlock();
        return result;
    }

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
        if (!foundValue && useOtherCaches) {
            foundValue = WeakObjectsCache<Key, T>::instance().value(key, false);
            if (!foundValue)
                foundValue = GuaranteedLifeTimeObjectsCache<Key, T>::instance().value(key, false);
        }
        if (!foundValue && key == Key())
            foundValue = T::defaultObject();
        qCDebug(proofCoreCacheLog) << "Returning value"
                                   << foundValue.data()
                                   << "from"
                                   << this->valueTypeName()
                                   << "strongcache"
                                   << "for key"
                                   << key;
        return foundValue;
    }

    QList<Key> keys() const override
    {
        return m_cache.keys();
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
