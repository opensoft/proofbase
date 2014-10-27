#include "taskchain.h"

static const qlonglong TASK_ADDING_SPIN_SLEEP_TIME_IN_MSECS = 1;
static const qlonglong SELF_MANAGEMENT_SPIN_SLEEP_TIME_IN_MSECS = 5;
static const qlonglong SELF_MANAGEMENT_PAUSE_SLEEP_TIME_IN_MSECS = 1000;

namespace Proof {

class TaskChainPrivate
{
    Q_DECLARE_PUBLIC(TaskChain)

    void acquireFutures(qlonglong spinSleepTimeInMsecs);
    void releaseFutures();
    void startSelfManagementThreadIfNeeded();

    TaskChain *q_ptr = nullptr;

    std::vector<std::future<void>> futures;
    std::atomic_flag futuresLock = ATOMIC_FLAG_INIT;
    TaskChainSP selfPointer;
    bool wasStarted = false;

    QSharedPointer<QEventLoop> signalWaitersEventLoop;
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
    result->d_func()->selfPointer = result;

    auto connection = QSharedPointer<QMetaObject::Connection>::create();
    auto checker = [result, connection](){
        QObject::disconnect(*connection);
        result->d_func()->selfPointer.clear();
    };
    *connection = connect(result.data(), &QThread::finished, result.data(), checker);
    qCDebug(proofCoreTaskChainLog) << "New chain created" << result.data();

    return result;
}

void TaskChain::fireSignalWaiters()
{
    Q_D(TaskChain);
    if (!d->signalWaitersEventLoop)
        return;
    d->signalWaitersEventLoop->exec();
    d->signalWaitersEventLoop.clear();
    qCDebug(proofCoreTaskChainLog) << "Chain:" << this << " signal waiters fired";
}

void TaskChain::run()
{
    Q_D(TaskChain);
    Q_ASSERT(!d->wasStarted);
    d->wasStarted = true;

    qCDebug(proofCoreTaskChainLog) << "Chain:" << this << " thread started";
    bool deleteSelf = false;
    while (!deleteSelf) {
        d->acquireFutures(SELF_MANAGEMENT_SPIN_SLEEP_TIME_IN_MSECS);
        for (unsigned i = 0; i < d->futures.size(); ++i) {
            const std::future<void> &currentFuture = d->futures.at(i);
            bool removeIt = !currentFuture.valid();
            if (!removeIt)
                removeIt = currentFuture.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready;
            if (removeIt) {
                d->futures.erase(d->futures.begin() + i);
                --i;
            }
        }
        deleteSelf = !d->futures.size();
        d->releaseFutures();
        if (!deleteSelf)
            QThread::msleep(SELF_MANAGEMENT_PAUSE_SLEEP_TIME_IN_MSECS);
    }
}

void TaskChain::addTaskPrivate(std::future<void> &&taskFuture)
{
    Q_D(TaskChain);
    d->acquireFutures(TASK_ADDING_SPIN_SLEEP_TIME_IN_MSECS);
    qCDebug(proofCoreTaskChainLog) << "Chain" << this << ": task added" << &taskFuture;
    d->futures.push_back(std::move(taskFuture));
    d->releaseFutures();
    d->startSelfManagementThreadIfNeeded();
}

void TaskChain::addSignalWaiterPrivate(
        std::function<void (const QSharedPointer<QEventLoop> &)> &&connector)
{
    Q_D(TaskChain);
    if (!d->signalWaitersEventLoop)
        d->signalWaitersEventLoop.reset(new QEventLoop);
    connector(d->signalWaitersEventLoop);
}

void TaskChainPrivate::acquireFutures(qlonglong spinSleepTimeInMsecs)
{
    while (futuresLock.test_and_set(std::memory_order_acquire))
        QThread::msleep(spinSleepTimeInMsecs);
}

void TaskChainPrivate::releaseFutures()
{
    futuresLock.clear(std::memory_order_release);
}

void TaskChainPrivate::startSelfManagementThreadIfNeeded()
{
    Q_Q(TaskChain);
    if (!q->isRunning() && !wasStarted)
        q->start();
}
