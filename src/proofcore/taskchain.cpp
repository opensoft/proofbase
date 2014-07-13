#include "taskchain.h"

using namespace Proof;

TaskChain::TaskChain()
    : QThread(0)
{
}

TaskChain::~TaskChain()
{
}

TaskChainSP TaskChain::createChain()
{
    TaskChainSP result(new TaskChain());
    result->m_selfPointer = result;

    auto connection = QSharedPointer<QMetaObject::Connection>::create();
    auto checker = [result, connection](){
        QObject::disconnect(*connection);
        result->m_selfPointer.clear();
    };
    *connection = connect(result.data(), &QThread::finished, result.data(), checker);

    return result;
}

void TaskChain::fireSignalWaiters()
{
    if (!m_signalWaitersEventLoop)
        return;
    m_signalWaitersEventLoop->exec();
    m_signalWaitersEventLoop.clear();
}

void TaskChain::run()
{
    Q_ASSERT(!m_wasStarted);
    m_wasStarted = true;

    bool deleteSelf = false;
    while (!deleteSelf) {
        acquireFutures(selfManagementSpinSleepTimeInMSecs);
        for (unsigned i = 0; i < m_futures.size(); ++i) {
            const std::future<void> &currentFuture = m_futures.at(i);
            bool removeIt = !currentFuture.valid();
            if (!removeIt)
                removeIt = currentFuture.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready;
            if (removeIt) {
                m_futures.erase(m_futures.begin() + i);
                --i;
            }
        }
        deleteSelf = !m_futures.size();
        releaseFutures();
        if (!deleteSelf)
            QThread::msleep(selfManagementPauseSleepTimeInMSecs);
    }
}

void TaskChain::acquireFutures(qlonglong spinSleepTimeInMsecs)
{
    while (m_futuresLock.test_and_set(std::memory_order_acquire))
        QThread::msleep(spinSleepTimeInMsecs);
}

void TaskChain::releaseFutures()
{
    m_futuresLock.clear(std::memory_order_release);
}

void TaskChain::startSelfManagementThreadIfNeeded()
{
    if (!isRunning() && !m_wasStarted)
        start();
}
