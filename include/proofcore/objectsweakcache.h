#ifndef OBJECTSWEAKCACHE_H
#define OBJECTSWEAKCACHE_H

#include <QHash>
#include <QSharedPointer>
#include <QWeakPointer>

namespace Proof {

template<class Key, class T>
class ObjectsWeakCache
{
public:
    static ObjectsWeakCache<Key, T> &instance()
    {
        static ObjectsWeakCache<Key, T> inst;
        return inst;
    }

    void add(const Key &key, QSharedPointer<T> object)
    {
        if (!object)
            return;
        m_cache[key] = object.toWeakRef();
    }

    QSharedPointer<T> value(const Key &key)
    {
        if (!m_cache.contains(key))
            return QSharedPointer<T>();
        QSharedPointer<T> shared = m_cache[key].toStrongRef();
        if (!shared)
            m_cache.remove(key);
        return shared;
    }

private:
    ObjectsWeakCache() {}
    Q_DISABLE_COPY(ObjectsWeakCache)

    QHash<Key, QWeakPointer<T>> m_cache;
};

}

#endif // OBJECTSWEAKCACHE_H
