#include "taskchain.h"

#include <QTime>

#include <QMap>
#include <QCoreApplication>
#include <QMutex>
#include <QWaitCondition>

static const qulonglong TASK_ADDING_SPIN_SLEEP_TIME_IN_MSECS = 1;
static const qulonglong WAIT_FOR_TASK_SPIN_SLEEP_TIME_IN_MSECS = 1;


namespace Proof {

class TaskChainPrivate
{
    Q_DECLARE_PUBLIC(TaskChain)

    void acquireFutures(qulonglong spinSleepTimeInMsecs);
    void releaseFutures();

    TaskChain *q_ptr = nullptr;

    TaskChainWP weakSelf;

    QMap<qlonglong, FutureSP<bool>> futures;
    std::atomic_flag futuresLock = ATOMIC_FLAG_INIT;
    std::atomic_llong lastUsedId {0};
    static std::atomic_llong chainsCounter;
};

std::atomic_llong TaskChainPrivate::chainsCounter {0};
}

using namespace Proof;

TaskChain::TaskChain()
    : QThread(nullptr), d_ptr(new TaskChainPrivate)
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
    result->d_func()->weakSelf = result.toWeakRef();
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

    bool result = futureToWait->wait(msecs);
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
    taskFuture->recover([](const auto &){return true;})->onSuccess([self = d->weakSelf, id](bool) {
        auto strongSelf = self.toStrongRef();
        if (!strongSelf)
            return;
        strongSelf->d_ptr->acquireFutures(WAIT_FOR_TASK_SPIN_SLEEP_TIME_IN_MSECS);
        strongSelf->d_ptr->futures.remove(id);
        strongSelf->d_ptr->releaseFutures();
    });
    return id;
}

void TaskChainPrivate::acquireFutures(qulonglong spinSleepTimeInMsecs)
{
    while (futuresLock.test_and_set(std::memory_order_acquire))
        QThread::msleep(spinSleepTimeInMsecs);
}

void TaskChainPrivate::releaseFutures()
{
    futuresLock.clear(std::memory_order_release);
}
