#include "gtest/test_global.h"

#include "proofcore/taskchain.h"

#include <QThread>
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
            chain->addTask(*secondTask);
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
