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
#ifndef TASKCHAIN_H
#define TASKCHAIN_H

#include "proofseed/future.h"
#include "proofseed/tasks.h"

#include "proofcore/proofcore_global.h"

#include <QSharedPointer>
#include <QThread>

//TODO: 1.0: deprecated, use tasks
namespace Proof {

class TaskChain;
using TaskChainSP = QSharedPointer<TaskChain>;
using TaskChainWP = QWeakPointer<TaskChain>;

class TaskChainPrivate;
class PROOF_CORE_EXPORT TaskChain : public QThread
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TaskChain)
public:
    ~TaskChain();

    template <class... Args>
    static QSharedPointer<std::function<void(Args...)>> createTask()
    {
        return QSharedPointer<std::function<void(Args...)>>::create();
    }

    static TaskChainSP createChain(bool selfDestroyable = true);

    template <class Task, class... Args>
    qlonglong addTask(Task &&task, Args &&... args)
    {
        return addTaskPrivate(tasks::run([task = std::forward<Task>(task), args...] { task(args...); }));
    }

    template <class SignalSender, class SignalType, class... Args>
    void addSignalWaiter(SignalSender *sender, SignalType signal, std::function<bool(Args...)> callback)
    {
        tasks::addSignalWaiter(sender, signal, std::move(callback));
    }

    void fireSignalWaiters();

    bool waitForTask(qlonglong taskId, qlonglong msecs = 0);
    bool touchTask(qlonglong taskId);

private:
    TaskChain();
    TaskChain(const TaskChain &other) = delete;
    TaskChain(TaskChain &&other) = delete;
    TaskChain &operator=(const TaskChain &other) = delete;
    TaskChain &operator=(TaskChain &&other) = delete;

    qlonglong addTaskPrivate(const FutureSP<bool> &taskFuture);

    QScopedPointer<TaskChainPrivate> d_ptr;
};

} // namespace Proof

#endif // TASKCHAIN_H
