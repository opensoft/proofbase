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
    template<class ChainLink, class ...Args>
    static TaskChainSP forge(ChainLink &&firstChainLink,
                             Args &&... args)
    {
        TaskChainSP result(new TaskChain());
        result->m_selfPointer = result;

        auto connection = QSharedPointer<QMetaObject::Connection>::create();
        auto checker = [result, connection](){
            QObject::disconnect(*connection);
            result->m_selfPointer.clear();
        };
        *connection = connect(result.data(), &QThread::finished, result.data(), checker);

        result->addChainLink(firstChainLink, args...);
        result->start();
        return result;
    }

    template<class ChainLink, class ...Args>
    void addChainLink(ChainLink &&chainLink,
                      Args &&... args)
    {
        while (m_futureLock.test_and_set(std::memory_order_acquire)) {
            QCoreApplication::processEvents();
            QThread::msleep(1);
        }
        m_future = std::async(std::launch::async, chainLink, args...);
        m_futureLock.clear(std::memory_order_release);
    }

    template<class SignalSender, class SignalType, class CallbackType, class ...Args>
    typename std::result_of<CallbackType(Args...)>::type
    waitForSignal(SignalSender *sender,
                  SignalType &&signal,
                  CallbackType &&callback)
    {
        QEventLoop eventLoop;
        typename std::result_of<CallbackType(Args...)>::type result;
        QObject *context = new QObject;
        auto slot = [&result, &callback, &eventLoop] (Args ... args) {
            result = callback(args...);
            eventLoop.quit();
        };
        QObject::connect(sender, signal, context, slot, Qt::QueuedConnection);
        eventLoop.exec();
        delete context;
        return result;
    }

protected:
    void run() override
    {
        Q_ASSERT(!m_wasStarted);
        m_wasStarted = true;

        bool deleteSelf = false;
        while (!deleteSelf) {
            while (m_futureLock.test_and_set(std::memory_order_acquire)) {
                QCoreApplication::processEvents();
                QThread::msleep(100);
            }
            if (m_future.valid())
                deleteSelf = m_future.wait_for(std::chrono::milliseconds(1)) ==std::future_status::ready;
            else
                deleteSelf = true;
            m_futureLock.clear(std::memory_order_release);
        }
    }

private:
    TaskChain()
        : QThread(0)
    {
        m_futureLock.clear(std::memory_order_release);
    }
    TaskChain(const TaskChain &other) = delete;
    TaskChain(TaskChain &&other) = delete;
    TaskChain &operator=(const TaskChain &other) = delete;
    TaskChain &operator=(TaskChain &&other) = delete;

    std::future<void> m_future;
    std::future<void> m_checkFuture;
    std::atomic_flag m_futureLock;
    TaskChainSP m_selfPointer;
    bool m_wasStarted = false;
};

}

#endif // TASKCHAIN_H
