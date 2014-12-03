#include "taskchain.h"

#include <map>

static const qlonglong TASK_ADDING_SPIN_SLEEP_TIME_IN_MSECS = 1;
static const qlonglong WAIT_FOR_TASK_SPIN_SLEEP_TIME_IN_MSECS = 1;
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

    std::map<qlonglong, std::future<void>> futures;
    std::atomic_flag futuresLock = ATOMIC_FLAG_INIT;
    TaskChainSP selfPointer;
    bool wasStarted = false;
    std::atomic_llong lastUsedId {0};

    static thread_local QSharedPointer<QEventLoop> signalWaitersEventLoop;
};

thread_local QSharedPointer<QEventLoop> TaskChainPrivate::signalWaitersEventLoop;
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

bool TaskChain::waitForTask(qlonglong taskId, qlonglong msecs)
{
    Q_D(TaskChain);
    d->acquireFutures(WAIT_FOR_TASK_SPIN_SLEEP_TIME_IN_MSECS);
    //We have it true by default to skip already deleted tasks
    bool result = true;
    std::future<void> futureToWait;
    auto task = d->futures.find(taskId);
    if (task != d->futures.cend()) {
        if (!task->second.valid()) {
            result = true;
        } else if (msecs < 1) {
            futureToWait = std::move(task->second);
            d->futures.erase(task);
        } else {
            result = task->second.wait_for(std::chrono::milliseconds(msecs)) == std::future_status::ready;
        }
    }
    d->releaseFutures();
    if (msecs < 1)
        futureToWait.wait();
    return result;
}

bool TaskChain::touchTask(qlonglong taskId)
{
    return waitForTask(taskId, 1);
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
        std::map<qlonglong, std::future<void>>::iterator it = d->futures.begin();
        while (it != d->futures.end()) {
            bool removeIt = !it->second.valid();
            if (!removeIt)
                removeIt = it->second.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready;
            if (removeIt) {
                bool startOver = it == d->futures.begin();
                d->futures.erase(it);
                startOver &= d->futures.size();
                if (startOver)
                    it = d->futures.begin();
                else
                    break;
            } else {
                ++it;
            }
        }
        deleteSelf = !d->futures.size();
        d->releaseFutures();
        if (!deleteSelf)
            QThread::msleep(SELF_MANAGEMENT_PAUSE_SLEEP_TIME_IN_MSECS);
    }
}

qlonglong TaskChain::addTaskPrivate(std::future<void> &&taskFuture)
{
    Q_D(TaskChain);
    d->acquireFutures(TASK_ADDING_SPIN_SLEEP_TIME_IN_MSECS);
    qlonglong id = ++d->lastUsedId;
    qCDebug(proofCoreTaskChainLog) << "Chain" << this << ": task added" << &taskFuture;
    d->futures.insert(std::move(std::pair<qlonglong, std::future<void>>(id, std::move(taskFuture))));
    d->releaseFutures();
    d->startSelfManagementThreadIfNeeded();
    return id;
}

void TaskChain::addSignalWaiterPrivate(std::function<void (const QSharedPointer<QEventLoop> &)> &&connector)
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
