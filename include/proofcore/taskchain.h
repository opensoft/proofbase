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
using TaskChainSP = QSharedPointer<TaskChain>;
using TaskChainWP = QWeakPointer<TaskChain>;

class TaskChainPrivate;
class PROOF_CORE_EXPORT TaskChain : public QThread // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TaskChain)
public:
    ~TaskChain();

    template<class ...Args>
    static QSharedPointer<std::function<void(Args...)>> createTask()
    {
        return QSharedPointer<std::function<void(Args...)>>::create();
    }

    // NOTE: Chain must be created (or moved after creation) in thread with Qt event loop and that thread must live all time that chain lives
    static TaskChainSP createChain();

    template<class Task, class ...Args>
    qlonglong addTask(Task &&task,
                      Args &&... args)
    {
        return addTaskPrivate(std::async(std::launch::async, std::forward<Task>(task), std::forward<Args>(args)...));
    }

    template<class SignalSender, class SignalType, class ...Args>
    void addSignalWaiter(SignalSender *sender,
                         SignalType signal,
                         std::function<bool(Args...)> callback)
    {
        std::function<void (const QSharedPointer<QEventLoop> &)> connector
            = [sender, signal, callback, this] (const QSharedPointer<QEventLoop> &eventLoop) {
                auto connection = QSharedPointer<QMetaObject::Connection>::create();
                auto eventLoopWeak = eventLoop.toWeakRef();
                std::function<void(Args...)> slot
                    = [callback, eventLoopWeak, connection, this] (Args... args) {
                        QSharedPointer<QEventLoop> eventLoop = eventLoopWeak.toStrongRef();
                        if (!eventLoop)
                            return;
                        if (!callback(args...))
                            return;
                        QObject::disconnect(*connection);
                        if (!eventLoopStarted())
                            clearEventLoop();
                        else
                            eventLoop->quit();
                    };
                *connection = QObject::connect(sender, signal, eventLoop.data(),
                                               slot, Qt::QueuedConnection);
            };
        qCDebug(proofCoreTaskChainExtraLog) << "Chain" << this << ": signal waiter added, sender:" << sender
                                       << "signal:" << &signal;
        addSignalWaiterPrivate(std::move(connector));
    }

    void fireSignalWaiters();

    bool waitForTask(qlonglong taskId, qlonglong msecs = 0);
    bool touchTask(qlonglong taskId);

protected:
    void run() override;

private:
    TaskChain();
    TaskChain(const TaskChain &other) = delete;
    TaskChain(TaskChain &&other) = delete;
    TaskChain &operator=(const TaskChain &other) = delete;
    TaskChain &operator=(TaskChain &&other) = delete;

    qlonglong addTaskPrivate(std::future<void> &&taskFuture);
    void addSignalWaiterPrivate(std::function<void (const QSharedPointer<QEventLoop> &)> &&connector);
    bool eventLoopStarted() const;
    void clearEventLoop();

    QScopedPointer<TaskChainPrivate> d_ptr;
};

}

#endif // TASKCHAIN_H
