#ifndef OBJECTSCACHE_H
#define OBJECTSCACHE_H

#include "proofcore/expirator.h"
#include "proofcore/proofobject.h"

#include <QHash>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QWeakPointer>

#include <functional>
#include <type_traits>

/* YOU MUST USE special functions to obtain instance of cache of Proof's exported types.
 * DO NOT USE methods 'instance' for Proof's exported types
 */
namespace Proof {

template <class Key, class T>
class WeakObjectsCache;
template <class Key, class T>
class StrongObjectsCache;
template <class Key, class T, class Enable = void>
class GuaranteedLifeTimeObjectsCache;

template <class Key, class T>
class ObjectsCache
{
public:
    using key_type = Key;
    using cached_type = T;
    using Creator = std::function<QSharedPointer<T>()>;

    virtual QSharedPointer<T> add(const Key &key, const QSharedPointer<T> &object, bool overwriteCurrent = false) = 0;
    virtual QSharedPointer<T> add(const Key &key, Creator creator) = 0;
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

    template <typename U = T>
    QString valueTypeName(typename std::enable_if<!std::is_base_of<QObject, U>::value>::type * = nullptr) const
    {
        return QString();
    }

    template <typename U = T>
    QString valueTypeName(typename std::enable_if<std::is_base_of<QObject, U>::value>::type * = nullptr) const
    {
        return U::staticMetaObject.className();
    }
};

template <class Key, class T>
class WeakObjectsCache : public ObjectsCache<Key, T>
{
public:
    using typename ObjectsCache<Key, T>::Creator;

    static ObjectsCache<Key, T> &instance()
    {
        static WeakObjectsCache<Key, T> inst;
        return inst;
    }

    QSharedPointer<T> add(const Key &key, const QSharedPointer<T> &object, bool overwriteCurrent = false) override
    {
        if (!object || key == Key())
            return object;
        return addPrivate(key, [&object]() { return object; }, overwriteCurrent);
    }

    QSharedPointer<T> add(const Key &key, Creator creator) override
    {
        if (key == Key())
            return creator();
        return addPrivate(key, std::move(creator), false);
    }

    void remove(const Key &key) override
    {
        m_cacheLock.lockForWrite();
        m_cache.remove(key);
        qCDebug(proofCoreCacheLog) << "Removing object with key" << key << "from" << this->valueTypeName() << "weakcache";
        m_cacheLock.unlock();
    }

    void clear() override
    {
        m_cacheLock.lockForWrite();
        m_cache.clear();
        qCDebug(proofCoreCacheLog) << "Clearing" << this->valueTypeName() << "weakcache";
        m_cacheLock.unlock();
    }

    bool isEmpty() const override
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
            foundValue = T::create();
        if (removeFromCache) {
            m_cacheLock.lockForWrite();
            if (m_cache.contains(key) && !m_cache[key])
                m_cache.remove(key);
            m_cacheLock.unlock();
        }
        qCDebug(proofCoreCacheLog) << "Returning value" << foundValue.data() << "from" << this->valueTypeName()
                                   << "weakcache"
                                   << "for key" << key;
        return foundValue;
    }

    QList<Key> keys() const override { return m_cache.keys(); }

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
    template <class F>
    QSharedPointer<T> addPrivate(const Key &key, F &&creator, bool overwriteCurrent)
    {
        m_cacheLock.lockForWrite();
        if (!overwriteCurrent && m_cache.contains(key)) {
            QSharedPointer<T> current = m_cache[key].toStrongRef();
            if (current) {
                m_cacheLock.unlock();
                return current;
            }
        }
        QSharedPointer<T> object{creator()};
        m_cache[key] = object.toWeakRef();
        qCDebug(proofCoreCacheLog) << "Adding object" << object.data() << "to" << this->valueTypeName()
                                   << "weakcache with key" << key;
        m_cacheLock.unlock();
        return object;
    }

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
template <class Key, class T>
class GuaranteedLifeTimeObjectsCache<Key, T, typename std::enable_if<!std::is_base_of<ProofObject, T>::value>::type>
    : public ObjectsCache<Key, T>
{
public:
    using typename ObjectsCache<Key, T>::Creator;

    static ObjectsCache<Key, T> &instance()
    {
        static GuaranteedLifeTimeObjectsCache<Key, T> inst;
        return inst;
    }
    QSharedPointer<T> add(const Key &, const QSharedPointer<T> &, bool = false) override
    {
        Q_ASSERT_X(false, "GuaranteedLifeTimeObjectsCache", "Should not be called for non-ProofObject values");
        return QSharedPointer<T>();
    }
    QSharedPointer<T> add(const Key &, Creator) override
    {
        Q_ASSERT_X(false, "GuaranteedLifeTimeObjectsCache", "Should not be called for non-ProofObject values");
        return QSharedPointer<T>();
    }
    void remove(const Key &) override
    {
        Q_ASSERT_X(false, "GuaranteedLifeTimeObjectsCache", "Should not be called for non-ProofObject values");
    }
    void clear() override
    {
        Q_ASSERT_X(false, "GuaranteedLifeTimeObjectsCache", "Should not be called for non-ProofObject values");
    }
    bool isEmpty() const override { return true; }
    bool contains(const Key &) const override { return false; }
    QSharedPointer<T> value(const Key &, bool = true) override { return QSharedPointer<T>(); }
    QList<Key> keys() const override { return QList<Key>(); }

private:
    GuaranteedLifeTimeObjectsCache() : ObjectsCache<Key, T>() {}
    ~GuaranteedLifeTimeObjectsCache() {}
    GuaranteedLifeTimeObjectsCache(const GuaranteedLifeTimeObjectsCache &other) = delete;
    GuaranteedLifeTimeObjectsCache &operator=(const GuaranteedLifeTimeObjectsCache &other) = delete;
    GuaranteedLifeTimeObjectsCache(const GuaranteedLifeTimeObjectsCache &&other) = delete;
    GuaranteedLifeTimeObjectsCache &operator=(const GuaranteedLifeTimeObjectsCache &&other) = delete;
};

template <class Key, class T>
class GuaranteedLifeTimeObjectsCache<Key, T, typename std::enable_if<std::is_base_of<ProofObject, T>::value>::type>
    : public WeakObjectsCache<Key, T>
{
public:
    using typename ObjectsCache<Key, T>::Creator;

    static GuaranteedLifeTimeObjectsCache<Key, T> &instance()
    {
        static GuaranteedLifeTimeObjectsCache<Key, T> inst;
        return inst;
    }

    void setObjectsMinLifeTime(qulonglong secs) { m_objectsMinLifeTimeInSeconds = secs; }

    QSharedPointer<T> add(const Key &key, const QSharedPointer<T> &object, bool overwriteCurrent = false) override
    {
        if (!object || key == Key())
            return object;
        QSharedPointer<T> result = WeakObjectsCache<Key, T>::add(key, object, overwriteCurrent);
        if (result == object)
            addObjectToExpirator(qSharedPointerCast<ProofObject>(object));
        return result;
    }

    QSharedPointer<T> add(const Key &key, Creator creator) override
    {
        if (key == Key())
            return creator();
        bool isCreatorCalled = false;
        QSharedPointer<T> result = WeakObjectsCache<Key, T>::add(key, [creator = std::move(creator), &isCreatorCalled]() {
            isCreatorCalled = true;
            return creator();
        });
        if (isCreatorCalled)
            addObjectToExpirator(result);
        return result;
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
            qCDebug(proofCoreCacheLog) << "Object" << this->valueTypeName() << "added" << object.data() << "to expirator";
        }
    }

    qulonglong m_objectsMinLifeTimeInSeconds = 8 * 60 * 60; //Default value is 8 hours
};

template <class Key, class T>
class StrongObjectsCache : public ObjectsCache<Key, T>
{
public:
    using typename ObjectsCache<Key, T>::Creator;

    static ObjectsCache<Key, T> &instance()
    {
        static StrongObjectsCache<Key, T> inst;
        return inst;
    }

    QSharedPointer<T> add(const Key &key, const QSharedPointer<T> &object, bool overwriteCurrent = false) override
    {
        if (!object || key == Key())
            return object;
        return addPrivate(key, [&object]() { return object; }, overwriteCurrent);
    }

    QSharedPointer<T> add(const Key &key, Creator creator) override
    {
        if (key == Key())
            return creator();
        return addPrivate(key, std::move(creator), false);
    }

    void remove(const Key &key) override
    {
        m_cacheLock.lockForWrite();
        m_cache.remove(key);
        qCDebug(proofCoreCacheLog) << "Removing object with key" << key << "from" << this->valueTypeName()
                                   << "strongcache";
        m_cacheLock.unlock();
    }

    void clear() override
    {
        m_cacheLock.lockForWrite();
        m_cache.clear();
        qCDebug(proofCoreCacheLog) << "Clearing" << this->valueTypeName() << "strongcache";
        m_cacheLock.unlock();
    }

    bool isEmpty() const override
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
            foundValue = T::create();
        qCDebug(proofCoreCacheLog) << "Returning value" << foundValue.data() << "from" << this->valueTypeName()
                                   << "strongcache"
                                   << "for key" << key;
        return foundValue;
    }

    QList<Key> keys() const override { return m_cache.keys(); }

private:
    StrongObjectsCache() : ObjectsCache<Key, T>() {}
    ~StrongObjectsCache() {}
    StrongObjectsCache(const StrongObjectsCache &other) = delete;
    StrongObjectsCache &operator=(const StrongObjectsCache &other) = delete;
    StrongObjectsCache(const StrongObjectsCache &&other) = delete;
    StrongObjectsCache &operator=(const StrongObjectsCache &&other) = delete;

    template <class F>
    QSharedPointer<T> addPrivate(const Key &key, F &&creator, bool overwriteCurrent)
    {
        m_cacheLock.lockForWrite();
        if (!overwriteCurrent && m_cache.contains(key)) {
            QSharedPointer<T> current = m_cache[key];
            m_cacheLock.unlock();
            return current;
        }
        QSharedPointer<T> object{creator()};
        m_cache[key] = object;
        qCDebug(proofCoreCacheLog) << "Adding object" << object.data() << "to" << this->valueTypeName()
                                   << "strongcache with key" << key;
        m_cacheLock.unlock();
        return object;
    }

private:
    QHash<Key, QSharedPointer<T>> m_cache;
    mutable QReadWriteLock m_cacheLock;
};

} // namespace Proof

#endif // OBJECTSCACHE_H
