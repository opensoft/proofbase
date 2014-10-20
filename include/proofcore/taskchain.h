#ifndef TASKCHAIN_H
#define TASKCHAIN_H

#include "proofcore/proofcore_global.h"

#include <QObject>
#include <QEventLoop>
#include <QThread>
#include <QCoreApplication>
#include <QSharedPointer>

#include <functional>
#include <future>
#include <type_traits>
#include <atomic>

namespace Proof {

class TaskChain;
typedef QSharedPointer<TaskChain> TaskChainSP;
typedef QWeakPointer<TaskChain> TaskChainWP;

class TaskChainPrivate;
class PROOF_CORE_EXPORT TaskChain : public QThread
{
    Q_DECLARE_PRIVATE(TaskChain)
public:
    ~TaskChain();

    template<class ...Args>
    static QSharedPointer<std::function<void(Args...)>> createTask()
    {
        return QSharedPointer<std::function<void(Args...)>>::create();
    }

    static TaskChainSP createChain();

    template<class Task, class ...Args>
    void addTask(Task &&task,
                 Args &&... args)
    {
        addFuture(std::async(std::launch::async, task, std::forward<Args>(args)...));
    }

    //TODO: make it thread local to allow proper tree chain
    template<class SignalSender, class SignalType, class ...Args>
    void addSignalWaiter(SignalSender *sender,
                         SignalType signal,
                         std::function<bool(Args...)> callback)
    {
        std::function<void (const QSharedPointer<QEventLoop> &)> connector =
            [sender, signal, callback] (const QSharedPointer<QEventLoop> &eventLoop)
            {
                auto connection = QSharedPointer<QMetaObject::Connection>::create();
                auto eventLoopWeak = eventLoop.toWeakRef();
                std::function<void(Args...)> slot =
                    [callback, eventLoopWeak, connection] (Args... args)
                    {
                        QSharedPointer<QEventLoop> eventLoop = eventLoopWeak.toStrongRef();
                        if (!eventLoop)
                            return;
                        if (!callback(args...))
                            return;
                        QObject::disconnect(*connection);
                        eventLoop->quit();
                    };
                *connection = QObject::connect(sender, signal, eventLoop.data(),
                                               slot, Qt::QueuedConnection);
            };
        addSignalWaiterPrivate(std::move(connector));
    }

    void fireSignalWaiters();

protected:
    void run() override;

private:
    TaskChain();
    TaskChain(const TaskChain &other) = delete;
    TaskChain(TaskChain &&other) = delete;
    TaskChain &operator=(const TaskChain &other) = delete;
    TaskChain &operator=(TaskChain &&other) = delete;

    void addFuture(std::future<void> &&taskFuture);
    void addSignalWaiterPrivate(std::function<void (const QSharedPointer<QEventLoop> &)> &&connector);

    QScopedPointer<TaskChainPrivate> d_ptr;
};

}

#endif // TASKCHAIN_H
