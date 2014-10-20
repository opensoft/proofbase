#include "taskchain.h"

static const qlonglong taskAddingSpinSleepTimeInMSecs = 1;
static const qlonglong selfManagementSpinSleepTimeInMSecs = 5;
static const qlonglong selfManagementPauseSleepTimeInMSecs = 1000;

namespace Proof {

class TaskChainPrivate
{
    Q_DECLARE_PUBLIC(TaskChain)

    void acquireFutures(qlonglong spinSleepTimeInMsecs);
    void releaseFutures();
    void startSelfManagementThreadIfNeeded();

    TaskChain *q_ptr = nullptr;

    std::vector<std::future<void>> m_futures;
    std::atomic_flag m_futuresLock = ATOMIC_FLAG_INIT;
    TaskChainSP m_selfPointer;
    bool m_wasStarted = false;

    QSharedPointer<QEventLoop> m_signalWaitersEventLoop;
};

}

using namespace Proof;

TaskChain::TaskChain()
    : QThread(0), d_ptr(new TaskChainPrivate())
{
    Q_D(TaskChain);
    d->q_ptr = this;
}

TaskChain::~TaskChain()
{
}

TaskChainSP TaskChain::createChain()
{
    TaskChainSP result(new TaskChain());
    result->d_func()->m_selfPointer = result;

    auto connection = QSharedPointer<QMetaObject::Connection>::create();
    auto checker = [result, connection](){
        QObject::disconnect(*connection);
        result->d_func()->m_selfPointer.clear();
    };
    *connection = connect(result.data(), &QThread::finished, result.data(), checker);

    return result;
}

void TaskChain::fireSignalWaiters()
{
    Q_D(TaskChain);
    if (!d->m_signalWaitersEventLoop)
        return;
    d->m_signalWaitersEventLoop->exec();
    d->m_signalWaitersEventLoop.clear();
}

void TaskChain::run()
{
    Q_D(TaskChain);
    Q_ASSERT(!d->m_wasStarted);
    d->m_wasStarted = true;

    bool deleteSelf = false;
    while (!deleteSelf) {
        d->acquireFutures(selfManagementSpinSleepTimeInMSecs);
        for (unsigned i = 0; i < d->m_futures.size(); ++i) {
            const std::future<void> &currentFuture = d->m_futures.at(i);
            bool removeIt = !currentFuture.valid();
            if (!removeIt)
                removeIt = currentFuture.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready;
            if (removeIt) {
                d->m_futures.erase(d->m_futures.begin() + i);
                --i;
            }
        }
        deleteSelf = !d->m_futures.size();
        d->releaseFutures();
        if (!deleteSelf)
            QThread::msleep(selfManagementPauseSleepTimeInMSecs);
    }
}

void TaskChain::addFuture(std::future<void> &&taskFuture)
{
    Q_D(TaskChain);
    d->acquireFutures(taskAddingSpinSleepTimeInMSecs);
    d->m_futures.push_back(std::move(taskFuture));
    d->releaseFutures();
    d->startSelfManagementThreadIfNeeded();
}

void TaskChain::addSignalWaiterPrivate(
        std::function<void (const QSharedPointer<QEventLoop> &)> &&connector)
{
    Q_D(TaskChain);
    if (!d->m_signalWaitersEventLoop)
        d->m_signalWaitersEventLoop.reset(new QEventLoop);
    connector(d->m_signalWaitersEventLoop);
}

void TaskChainPrivate::acquireFutures(qlonglong spinSleepTimeInMsecs)
{
    while (m_futuresLock.test_and_set(std::memory_order_acquire))
        QThread::msleep(spinSleepTimeInMsecs);
}

void TaskChainPrivate::releaseFutures()
{
    m_futuresLock.clear(std::memory_order_release);
}

void TaskChainPrivate::startSelfManagementThreadIfNeeded()
{
    Q_Q(TaskChain);
    if (!q->isRunning() && !m_wasStarted)
        q->start();
}
