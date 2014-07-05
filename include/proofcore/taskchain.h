#ifndef TASKCHAIN_H
#define TASKCHAIN_H

#include "proofcore_global.h"

#include <QObject>
#include <QEventLoop>
#include <QThread>
#include <QCoreApplication>
#include <QSharedPointer>
#include <QDebug>

#include <functional>
#include <future>
#include <type_traits>
#include <atomic>

namespace Proof {

class TaskChain;
typedef QSharedPointer<TaskChain> TaskChainSP;

class PROOF_CORE_EXPORT TaskChain : public QThread
{
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
        acquireFutures(taskAddingSpinSleepTimeInMSecs);
        m_futures.push_back(std::async(std::launch::async, task, args...));
        releaseFutures();
        startSelfManagementThreadIfNeeded();
    }

    //TODO: make it thread local to allow proper tree chain
    template<class SignalSender, class SignalType, class ...Args>
    void addSignalWaiter(SignalSender *sender,
                         SignalType &&signal,
                         const std::function<bool(Args...)> &callback)
    {
        if (!m_signalWaitersEventLoop)
            m_signalWaitersEventLoop.reset(new QEventLoop);
        QWeakPointer<QEventLoop> eventLoopWeak = m_signalWaitersEventLoop.toWeakRef();
        auto connection = QSharedPointer<QMetaObject::Connection>::create();
        std::function<void(Args...)> slot = [&callback, eventLoopWeak, connection] (Args... args) {
            QSharedPointer<QEventLoop> eventLoop = eventLoopWeak.toStrongRef();
            if (!eventLoop)
                return;
            if (!callback(args...))
                return;
            QObject::disconnect(*connection);
            eventLoop->quit();
        };
        *connection = QObject::connect(sender, signal, m_signalWaitersEventLoop.data(), slot, Qt::QueuedConnection);
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

    void acquireFutures(qlonglong spinSleepTimeInMsecs);
    void releaseFutures();
    void startSelfManagementThreadIfNeeded();

    std::vector<std::future<void>> m_futures;
    std::atomic_flag m_futuresLock;
    TaskChainSP m_selfPointer;
    bool m_wasStarted = false;

    QSharedPointer<QEventLoop> m_signalWaitersEventLoop;

    static const qlonglong taskAddingSpinSleepTimeInMSecs = 1;
    static const qlonglong selfManagementSpinSleepTimeInMSecs = 5;
    static const qlonglong selfManagementPauseSleepTimeInMSecs = 1000;
};

}

#endif // TASKCHAIN_H
