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
    {
        TaskChainSP chain = TaskChain::createChain();
        chain->moveToThread(&thread);
        chainWeak = chain.toWeakRef();
        auto firstTask = TaskChain::createTask<>();
        auto secondTask = TaskChain::createTask<>();
        *firstTask = [&flag, chain, secondTask, &counter]() {
            while (!flag) {}
            ++counter;
            chain->addTask(*secondTask);
        };
        *secondTask = [&counter]() {
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
