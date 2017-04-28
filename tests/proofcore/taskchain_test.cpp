// clazy:skip

#include "gtest/test_global.h"

#include "proofcore/taskchain.h"

#include <QThread>
#include <QDateTime>
#include <QTimer>

using namespace Proof;

TEST(TaskChainTest, memoryCheckWithNoChainCapture)
{
    TaskChainWP chainWeak;
    QThread thread;
    thread.start();
    std::atomic<bool> flag(false);
    {
        TaskChainSP chain = TaskChain::createChain();
        chain->moveToThread(&thread);
        chainWeak = chain.toWeakRef();
        auto task = TaskChain::createTask<>();
        *task = [&flag]() {
            while (!flag) {}
        };
        chain->addTask(*task);
    }
    EXPECT_FALSE(chainWeak.isNull());
    flag = true;
    for (int i = 0; i < 2000; ++i) {
        if (chainWeak.isNull())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    thread.quit();
    thread.wait(100);
    EXPECT_TRUE(chainWeak.isNull());
}

TEST(TaskChainTest, memoryCheckWithChainCapture)
{
    TaskChainWP chainWeak;
    QWeakPointer<std::function<void()>> secondTaskWeak;
    QThread thread;
    thread.start();
    std::atomic<bool> flag(false);
    {
        TaskChainSP chain = TaskChain::createChain();
        chain->moveToThread(&thread);
        chainWeak = chain.toWeakRef();
        auto firstTask = TaskChain::createTask<>();
        auto secondTask = TaskChain::createTask<>();
        *firstTask = [&flag, chain, secondTask]() {
            while (!flag) {}
            chain->addTask(*secondTask);
        };
        *secondTask = []() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        };
        secondTaskWeak = secondTask.toWeakRef();
        chain->addTask(*firstTask);
    }
    EXPECT_FALSE(chainWeak.isNull());
    EXPECT_FALSE(secondTaskWeak.isNull());
    flag = true;
    for (int i = 0; i < 2000; ++i) {
        if (chainWeak.isNull())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    thread.quit();
    thread.wait(100);
    EXPECT_TRUE(chainWeak.isNull());
    EXPECT_TRUE(secondTaskWeak.isNull());
}

TEST(TaskChainTest, stepsPerforming)
{
    TaskChainWP chainWeak;
    QThread thread;
    thread.start();
    std::atomic<int> counter(0);
    std::atomic<bool> flag(false);
    auto mainThreadId = std::this_thread::get_id();
    std::thread::id firstTaskThreadId;
    std::thread::id secondTaskThreadId;
    {
        TaskChainSP chain = TaskChain::createChain();
        chain->moveToThread(&thread);
        chainWeak = chain.toWeakRef();
        auto firstTask = TaskChain::createTask<>();
        auto secondTask = TaskChain::createTask<>();
        *firstTask = [&flag, chain, secondTask, &counter, &firstTaskThreadId]() {
            while (!flag) {}
            firstTaskThreadId = std::this_thread::get_id();
            ++counter;
            qlonglong secondId = chain->addTask(*secondTask);
            chain->waitForTask(secondId);
        };
        *secondTask = [&counter, &secondTaskThreadId]() {
            secondTaskThreadId = std::this_thread::get_id();
            ++counter;
        };
        chain->addTask(*firstTask);
    }
    EXPECT_FALSE(chainWeak.isNull());
    flag = true;
    for (int i = 0; i < 2000; ++i) {
        if (chainWeak.isNull())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    thread.quit();
    thread.wait(100);
    EXPECT_TRUE(chainWeak.isNull());
    EXPECT_NE(mainThreadId, firstTaskThreadId);
    EXPECT_NE(mainThreadId, secondTaskThreadId);
    EXPECT_NE(firstTaskThreadId, secondTaskThreadId);
    EXPECT_EQ(2, counter);
}

TEST(TaskChainTest, stepsPerformingWithArgs)
{
    TaskChainWP chainWeak;
    QThread thread;
    thread.start();
    std::atomic<int> counter(0);
    std::atomic<bool> flag(false);
    {
        TaskChainSP chain = TaskChain::createChain();
        chain->moveToThread(&thread);
        chainWeak = chain.toWeakRef();
        auto firstTask = TaskChain::createTask<>();
        auto secondTask = TaskChain::createTask<std::atomic<int> *>();
        *firstTask = [&flag, chain, secondTask, &counter]() {
            while (!flag) {}
            ++counter;
            chain->addTask(*secondTask, &counter);
        };
        *secondTask = [](std::atomic<int> *counterPtr) {
            ++(*counterPtr);
        };
        chain->addTask(*firstTask);
    }
    EXPECT_FALSE(chainWeak.isNull());
    flag = true;
    for (int i = 0; i < 2000; ++i) {
        if (chainWeak.isNull())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    thread.quit();
    thread.wait(100);
    EXPECT_TRUE(chainWeak.isNull());
    EXPECT_EQ(2, counter);
}

TEST(TaskChainTest, signalWaiting)
{
    TaskChainWP chainWeak;
    QThread thread;
    thread.start();
    std::atomic<bool> flag(false);
    std::atomic<bool> result(false);
    std::atomic<bool> finished(false);
    QTimer *timer = new QTimer();
    timer = new QTimer;
    timer->setSingleShot(true);
    timer->moveToThread(&thread);
    {
        TaskChainSP chain = TaskChain::createChain();
        chain->moveToThread(&thread);
        chainWeak = chain.toWeakRef();
        auto firstTask = TaskChain::createTask<>();
        *firstTask = [&flag, chain, timer, &result, &finished]() {
            std::function<bool()> timerCallback = [&result]() {
                result = true;
                return true;
            };
            chain->addSignalWaiter(timer, &QTimer::timeout, timerCallback);
            flag = true;
            chain->fireSignalWaiters();
            delete timer;
            finished = true;
        };
        chain->addTask(*firstTask);
    }
    EXPECT_FALSE(chainWeak.isNull());
    while (!flag) {}
    EXPECT_FALSE(finished);
    QMetaObject::invokeMethod(timer, "start", Qt::BlockingQueuedConnection, Q_ARG(int, 100));
    for (int i = 0; i < 2000; ++i) {
        if (chainWeak.isNull())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    thread.quit();
    thread.wait(100);
    EXPECT_TRUE(result);
    EXPECT_TRUE(finished);
    EXPECT_TRUE(chainWeak.isNull());
}

TEST(TaskChainTest, tasksTree)
{
    TaskChainWP chainWeak;
    QThread thread;
    thread.start();
    std::atomic<bool> startedFlag1(false);
    std::atomic<bool> startedFlag2(false);
    std::atomic<bool> result1(false);
    std::atomic<bool> result2(false);
    std::atomic<bool> finished1(false);
    std::atomic<bool> finished2(false);
    std::atomic_ullong evLoopThread1(0);
    std::atomic_ullong evLoopThread2(0);
    QTimer *timer = new QTimer();
    timer = new QTimer;
    timer->setSingleShot(true);
    timer->moveToThread(&thread);
    {
        TaskChainSP chain = TaskChain::createChain();
        chain->moveToThread(&thread);
        chainWeak = chain.toWeakRef();
        auto secondTask = TaskChain::createTask<>();
        auto thirdTask = TaskChain::createTask<>();
        auto firstTask = TaskChain::createTask<>();
        *firstTask = [chain, secondTask, thirdTask]() {
            chain->addTask(*secondTask);
            chain->addTask(*thirdTask);
        };

        *secondTask = [chain, timer, &result1, &finished1, &startedFlag1, &evLoopThread1]() {
            std::function<bool()> timerCallback = [&result1, &evLoopThread1]() {
                result1 = true;
                evLoopThread1 = reinterpret_cast<unsigned long long>(QThread::currentThread());
                return true;
            };
            chain->addSignalWaiter(timer, &QTimer::timeout, timerCallback);
            startedFlag1 = true;
            chain->fireSignalWaiters();
            finished1 = true;
        };
        *thirdTask = [chain, timer, &result2, &finished2, &startedFlag2, &evLoopThread2]() {
            std::function<bool()> timerCallback = [&result2, &evLoopThread2]() {
                result2 = true;
                evLoopThread2 = reinterpret_cast<unsigned long long>(QThread::currentThread());
                return true;
            };
            chain->addSignalWaiter(timer, &QTimer::timeout, timerCallback);
            startedFlag2 = true;
            chain->fireSignalWaiters();
            finished2 = true;
        };

        chain->addTask(*firstTask);
    }
    EXPECT_FALSE(chainWeak.isNull());
    while (!startedFlag1 && !startedFlag2) {}
    EXPECT_FALSE(finished1);
    EXPECT_FALSE(finished2);
    QMetaObject::invokeMethod(timer, "start", Qt::BlockingQueuedConnection, Q_ARG(int, 100));
    for (int i = 0; i < 2000; ++i) {
        if (chainWeak.isNull())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    thread.quit();
    thread.wait(100);
    delete timer;
    EXPECT_TRUE(result1);
    EXPECT_TRUE(finished1);
    EXPECT_TRUE(result2);
    EXPECT_TRUE(finished2);
    EXPECT_NE(0ull, (unsigned long long)evLoopThread1);
    EXPECT_NE(0ull, (unsigned long long)evLoopThread2);
    EXPECT_NE((unsigned long long)evLoopThread1, (unsigned long long)evLoopThread2);
    EXPECT_TRUE(chainWeak.isNull());
}

TEST(TaskChainTest, tasksTreeWaiting)
{
    QThread thread;
    thread.start();
    std::atomic_ullong rootTaskEndedTime(0);
    std::atomic_ullong child1TaskEndedTime(0);
    std::atomic_ullong child2TaskEndedTime(0);
    qlonglong rootTaskId = 0;

    TaskChainSP chain = TaskChain::createChain();
    chain->moveToThread(&thread);
    {
        auto secondTask = TaskChain::createTask<>();
        auto thirdTask = TaskChain::createTask<>();
        auto firstTask = TaskChain::createTask<>();
        *firstTask = [chain, secondTask, thirdTask, &rootTaskEndedTime]() {
            qlonglong firstId = chain->addTask(*secondTask);
            qlonglong secondId = chain->addTask(*thirdTask);
            chain->waitForTask(firstId);
            chain->waitForTask(secondId);
            rootTaskEndedTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        };

        *secondTask = [chain, &child1TaskEndedTime]() {
            QThread::msleep(30);
            child1TaskEndedTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            QThread::msleep(10);
        };
        *thirdTask = [chain, &child2TaskEndedTime]() {
            QThread::msleep(30);
            child2TaskEndedTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            QThread::msleep(10);
        };

        rootTaskId = chain->addTask(*firstTask);
    }
    EXPECT_NE(0ll, rootTaskId);
    chain->waitForTask(rootTaskId);
    thread.quit();
    thread.wait(100);
    EXPECT_GT((unsigned long long)rootTaskEndedTime, (unsigned long long)child1TaskEndedTime);
    EXPECT_GT((unsigned long long)rootTaskEndedTime, (unsigned long long)child2TaskEndedTime);
}

TEST(TaskChainTest, tasksTreeWaitingTimeout)
{
    QThread thread;
    thread.start();
    std::atomic_ullong rootTaskEndedTime(0);
    std::atomic_ullong child1TaskEndedTime(0);
    std::atomic_ullong child2TaskEndedTime(0);
    std::atomic_ullong rootCounter(0);
    qlonglong rootTaskId = 0;

    TaskChainSP chain = TaskChain::createChain();
    chain->moveToThread(&thread);
    {
        auto secondTask = TaskChain::createTask<>();
        auto thirdTask = TaskChain::createTask<>();
        auto firstTask = TaskChain::createTask<>();
        *firstTask = [chain, secondTask, thirdTask, &rootTaskEndedTime]() {
            qlonglong firstId = chain->addTask(*secondTask);
            qlonglong secondId = chain->addTask(*thirdTask);
            while (!chain->waitForTask(firstId, 1)) {}
            while (!chain->waitForTask(secondId, 1)) {}
            rootTaskEndedTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        };

        *secondTask = [chain, &child1TaskEndedTime]() {
            QThread::msleep(30);
            child1TaskEndedTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            QThread::msleep(10);
        };
        *thirdTask = [chain, &child2TaskEndedTime]() {
            QThread::msleep(30);
            child2TaskEndedTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            QThread::msleep(10);
        };

        rootTaskId = chain->addTask(*firstTask);
    }
    EXPECT_NE(0ll, rootTaskId);
    while (!chain->waitForTask(rootTaskId, 1)) {++rootCounter;}
    thread.quit();
    thread.wait(100);
    EXPECT_GT((unsigned long long)rootTaskEndedTime, (unsigned long long)child1TaskEndedTime);
    EXPECT_GT((unsigned long long)rootTaskEndedTime, (unsigned long long)child2TaskEndedTime);
    EXPECT_GT((unsigned long long)rootCounter, 0ull);
}
