#include "gtest/test_global.h"

#include "proofcore/taskchain.h"

#include <QThread>

using namespace Proof;
using testing::Test;

class TaskChainTest: public Test
{
public:
    TaskChainTest()
    {
    }

protected:
    void SetUp() override
    {
    }
};

TEST_F(TaskChainTest, memoryCheckWithNoChainCapture)
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
        *task = [&flag](){
            while (!flag) {}
        };
        chain->addTask(*task);
        chain.clear();
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

