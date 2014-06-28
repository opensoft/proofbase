#ifndef TASKCHAIN_H
#define TASKCHAIN_H

#include <QObject>
#include <QEventLoop>
#include <QThread>
#include <QCoreApplication>
#include <QSharedPointer>
#include <QDebug>
#include <QThread>

#include <functional>
#include <future>
#include <type_traits>
#include <atomic>

namespace Proof {

class TaskChain;
typedef QSharedPointer<TaskChain> TaskChainSP;

class TaskChain : public QThread
{
public:
    ~TaskChain()
    {
    }

    static TaskChainSP startChain()
    {
        TaskChainSP result(new TaskChain());
        result->m_selfPointer = result;

        auto connection = QSharedPointer<QMetaObject::Connection>::create();
        auto checker = [result, connection](){
            QObject::disconnect(*connection);
            result->m_selfPointer.clear();
        };
        *connection = connect(result.data(), &QThread::finished, result.data(), checker);

        return result;
    }

    template<class Task, class ...Args>
    void addTask(Task &&task,
                 Args &&... args)
    {
        while (m_futuresLock.test_and_set(std::memory_order_acquire)) {
            QCoreApplication::processEvents();
            QThread::msleep(1);
        }
        m_futures.push_back(std::async(std::launch::async, task, args...));
        m_futuresLock.clear(std::memory_order_release);

        if (!isRunning() && !m_wasStarted)
            start();
    }

    template<class SignalSender, class SignalType, class ...Args>
    void addSignalWaiter(SignalSender *sender,
                         SignalType &&signal,
                         const std::function<void(Args...)> &callback)
    {
        if (!m_signalWaitersEventLoop)
            m_signalWaitersEventLoop.reset(new QEventLoop);
        QWeakPointer<QEventLoop> eventLoopWeak = m_signalWaitersEventLoop.toWeakRef();
        auto connection = QSharedPointer<QMetaObject::Connection>::create();
        std::function<void(Args...)> slot = [&callback, eventLoopWeak, connection] (Args... args) {
            QSharedPointer<QEventLoop> eventLoop = eventLoopWeak.toStrongRef();
            if (!eventLoop)
                return;
            callback(args...);
            QObject::disconnect(*connection);
            eventLoop->quit();
        };
        *connection = QObject::connect(sender, signal, eventLoopWeak.data(), slot, Qt::QueuedConnection);
    }

    void fireSignalWaiters()
    {
        if (!m_signalWaitersEventLoop)
            return;
        m_signalWaitersEventLoop->exec();
        m_signalWaitersEventLoop.clear();
    }

protected:
    void run() override
    {
        Q_ASSERT(!m_wasStarted);
        m_wasStarted = true;

        bool deleteSelf = false;
        while (!deleteSelf) {
            while (m_futuresLock.test_and_set(std::memory_order_acquire))
                QThread::msleep(5);
            for (unsigned i = 0; i < m_futures.size(); ++i) {
                const std::future<void> &currentFuture = m_futures.at(i);
                bool removeIt = !currentFuture.valid();
                if (!removeIt)
                    removeIt = currentFuture.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready;
                if (removeIt) {
                    m_futures.erase(m_futures.begin()+i);
                    --i;
                }
            }
            deleteSelf = !m_futures.size();
            m_futuresLock.clear(std::memory_order_release);
            if (!deleteSelf)
                QThread::msleep(1000);
        }
    }

private:
    TaskChain()
        : QThread(0)
    {
        m_futuresLock.clear(std::memory_order_release);
    }
    TaskChain(const TaskChain &other) = delete;
    TaskChain(TaskChain &&other) = delete;
    TaskChain &operator=(const TaskChain &other) = delete;
    TaskChain &operator=(TaskChain &&other) = delete;

    std::vector<std::future<void>> m_futures;
    std::atomic_flag m_futuresLock;
    TaskChainSP m_selfPointer;
    bool m_wasStarted = false;

    QSharedPointer<QEventLoop> m_signalWaitersEventLoop;
};

}

#endif // TASKCHAIN_H
