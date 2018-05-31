#ifndef TASKCHAIN_H
#define TASKCHAIN_H

#include "proofcore/future.h"
#include "proofcore/proofcore_global.h"
#include "proofcore/tasks.h"

#include <QSharedPointer>
#include <QThread>

//TODO: 1.0: deprecated, use tasks
namespace Proof {

class TaskChain;
using TaskChainSP = QSharedPointer<TaskChain>;
using TaskChainWP = QWeakPointer<TaskChain>;

class TaskChainPrivate;
class PROOF_CORE_EXPORT TaskChain : public QThread // clazy:exclude=ctor-missing-parent-argument
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
