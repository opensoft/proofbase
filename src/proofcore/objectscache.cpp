#include "objectscache.h"

using namespace Proof;

void WeakObjectsCache::clear()
{
    m_cacheLock.lockForWrite();
    m_cache.clear();
    m_cacheLock.unlock();
}

bool WeakObjectsCache::isEmpty() const
{
    m_cacheLock.lockForRead();
    bool result = m_cache.isEmpty();
    m_cacheLock.unlock();
    return result;
}

void StrongObjectsCache::clear()
{
    m_cacheLock.lockForWrite();
    m_cache.clear();
    m_cacheLock.unlock();
}

bool StrongObjectsCache::isEmpty() const
{
    m_cacheLock.lockForRead();
    bool result = m_cache.isEmpty();
    m_cacheLock.unlock();
    return result;
}
