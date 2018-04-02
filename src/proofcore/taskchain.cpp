#include "taskchain.h"

#include <QTime>

#include <QMap>
#include <QCoreApplication>
#include <QMutex>
#include <QWaitCondition>

static const qlonglong TASK_ADDING_SPIN_SLEEP_TIME_IN_MSECS = 1;
static const qlonglong WAIT_FOR_TASK_SPIN_SLEEP_TIME_IN_MSECS = 1;


namespace Proof {

class TaskChainPrivate
{
    Q_DECLARE_PUBLIC(TaskChain)

    void acquireFutures(qlonglong spinSleepTimeInMsecs);
    void releaseFutures();
    bool waitForFuture(const FutureSP<bool> &future, qlonglong msecs = 0);

    TaskChain *q_ptr = nullptr;

    QMap<qlonglong, FutureSP<bool>> futures;
    std::atomic_flag futuresLock = ATOMIC_FLAG_INIT;
    std::atomic_llong lastUsedId {0};
    static std::atomic_llong chainsCounter;
};

std::atomic_llong TaskChainPrivate::chainsCounter {0};
}

using namespace Proof;

TaskChain::TaskChain()
    : QThread(0), d_ptr(new TaskChainPrivate)
{
    Q_D(TaskChain);
    d->q_ptr = this;
    ++TaskChainPrivate::chainsCounter;
    qCDebug(proofCoreTaskChainStatsLog) << "Chains in use:" << TaskChainPrivate::chainsCounter;
}

TaskChain::~TaskChain()
{
    --TaskChainPrivate::chainsCounter;
    qCDebug(proofCoreTaskChainStatsLog) << "Chains in use:" << TaskChainPrivate::chainsCounter;
}

TaskChainSP TaskChain::createChain(bool)
{
    TaskChainSP result(new TaskChain());
    qCDebug(proofCoreTaskChainExtraLog) << "New chain created" << result.data();
    return result;
}

void TaskChain::fireSignalWaiters()
{
    tasks::fireSignalWaiters();
}

bool TaskChain::waitForTask(qlonglong taskId, qlonglong msecs)
{
    Q_D(TaskChain);
    d->acquireFutures(WAIT_FOR_TASK_SPIN_SLEEP_TIME_IN_MSECS);
    FutureSP<bool> futureToWait = d->futures.value(taskId);
    d->releaseFutures();
    if (!futureToWait)
        return true;

    bool result = d->waitForFuture(futureToWait, msecs);
    if (result) {
        d->acquireFutures(WAIT_FOR_TASK_SPIN_SLEEP_TIME_IN_MSECS);
        d->futures.remove(taskId);
        d->releaseFutures();
    }
    return result;
}

bool TaskChain::touchTask(qlonglong taskId)
{
    Q_D(TaskChain);
    d->acquireFutures(WAIT_FOR_TASK_SPIN_SLEEP_TIME_IN_MSECS);
    FutureSP<bool> futureToWait = d->futures.value(taskId);
    d->releaseFutures();
    if (!futureToWait)
        return true;
    bool result = futureToWait->completed();
    if (result) {
        d->acquireFutures(WAIT_FOR_TASK_SPIN_SLEEP_TIME_IN_MSECS);
        d->futures.remove(taskId);
        d->releaseFutures();
    }
    return result;
}

qlonglong TaskChain::addTaskPrivate(const FutureSP<bool> &taskFuture)
{
    Q_D(TaskChain);
    d->acquireFutures(TASK_ADDING_SPIN_SLEEP_TIME_IN_MSECS);
    qlonglong id = ++d->lastUsedId;
    qCDebug(proofCoreTaskChainExtraLog) << "Chain" << this << ": task added" << &taskFuture;
    d->futures[id] = taskFuture;
    d->releaseFutures();
    return id;
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

bool TaskChainPrivate::waitForFuture(const FutureSP<bool> &future, qlonglong msecs)
{
    if (future->completed())
        return true;
    bool waitForever = msecs < 1;
    //We need to maintain gui thread event loop while waiting
    bool maintainEvents = QThread::currentThread() == qApp->thread();
    if (maintainEvents || !waitForever) {
        QTime timer;
        if (!waitForever)
            timer.start();
        while (waitForever || (timer.elapsed() <= msecs)) {
            if (future->completed())
                return true;
            if (maintainEvents)
                QCoreApplication::processEvents();
            QThread::msleep(1);
        }
    } else {
        QTime timer;
        timer.start();
        QMutex mutex;
        QWaitCondition waiter;
        mutex.lock();
        future->recover([](const auto &){return false;})->onSuccess([&waiter](bool) {waiter.wakeAll();});
        waiter.wait(&mutex);
        mutex.unlock();
    }
    return future->completed();
}
