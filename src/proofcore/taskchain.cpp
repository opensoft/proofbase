/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "proofcore/taskchain.h"

#include <QCoreApplication>
#include <QMap>
#include <QMutex>
#include <QTime>
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
    std::atomic_llong lastUsedId{0};
    static std::atomic_llong chainsCounter;
};

std::atomic_llong TaskChainPrivate::chainsCounter{0};
} // namespace Proof

using namespace Proof;

TaskChain::TaskChain() : QThread(nullptr), d_ptr(new TaskChainPrivate)
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
    taskFuture->recover([](const auto &) { return true; })->onSuccess([self = d->weakSelf, id](bool) {
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
