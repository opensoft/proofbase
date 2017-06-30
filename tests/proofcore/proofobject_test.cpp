// clazy:skip

#include "gtest/test_global.h"

#include "proofcore/proofobject.h"
#include "proofcore/taskchain.h"

#include <QThread>
#include <QDateTime>
#include <QTimer>

#include <atomic>

using namespace Proof;

class TestProofObject : public ProofObject
{
public:
    TestProofObject() : ProofObject(nullptr){}

    int multiply(int a, int b) {
        methodThread = QThread::currentThread();
        started = true;
        while (!canProceed)
            QThread::yieldCurrentThread();
        return a * b;
    }

    QThread *methodThread = nullptr;
    std::atomic_bool started {false};
    std::atomic_bool canProceed {false};
};

class TestQObject : public QObject
{
public:
    TestQObject() : QObject(nullptr){}

    int multiply(int a, int b) {
        methodThread = QThread::currentThread();
        started = true;
        while (!canProceed)
            QThread::yieldCurrentThread();
        return a * b;
    }

    QThread *methodThread = nullptr;
    std::atomic_bool started {false};
    std::atomic_bool canProceed {false};
};

class TestObject
{
public:
    TestObject() {}

    int multiply(int a, int b) {
        methodThread = QThread::currentThread();
        started = true;
        while (!canProceed)
            QThread::yieldCurrentThread();
        return a * b;
    }

    QThread *methodThread = nullptr;
    std::atomic_bool started {false};
    std::atomic_bool canProceed {false};
};

TEST(ProofObjectTest, NonBlockingCall)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    ProofObject::call(testObj, &TestProofObject::multiply, 42, 10);
    while (!testObj->started) {
        QThread::yieldCurrentThread();
        qApp->processEvents();
    }

    EXPECT_FALSE(testObj->canProceed);
    EXPECT_EQ(thread, testObj->methodThread);
    testObj->canProceed = true;

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete testObj;
}

TEST(ProofObjectTest, BlockingCall)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    int result = 0;
    auto chain = TaskChain::createChain();
    qlonglong taskId = chain->addTask([testObj]() {
        while (!testObj->started) {
            QThread::yieldCurrentThread();
            qApp->processEvents();
        }
        QThread::msleep(50);
        testObj->canProceed = true;
    });

    QTime timer;
    timer.start();

    bool runnerExeced = false;
    QTimer queuedRunner;
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced](){runnerExeced = true;}, Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::call(testObj, &TestProofObject::multiply, Call::Block, result, 42, 10);

    EXPECT_TRUE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(chain->waitForTask(taskId, 100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete testObj;
}

TEST(ProofObjectTest, EventsBlockingCall)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    int result = 0;
    auto chain = TaskChain::createChain();
    qlonglong taskId = chain->addTask([testObj]() {
        while (!testObj->started) {
            QThread::yieldCurrentThread();
            qApp->processEvents();
        }
        QThread::msleep(50);
        testObj->canProceed = true;
    });

    QTime timer;
    timer.start();

    bool runnerExeced = false;
    QTimer queuedRunner;
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced](){runnerExeced = true;}, Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::call(testObj, &TestProofObject::multiply, Call::BlockEvents, result, 42, 10);

    EXPECT_FALSE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(chain->waitForTask(taskId, 100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete testObj;
}

TEST(ProofObjectTest, NonBlockingCallQObject)
{
    TestQObject *testObj = new TestQObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    ProofObject::call(testObj, &TestQObject::multiply, 42, 10);
    while (!testObj->started) {
        QThread::yieldCurrentThread();
        qApp->processEvents();
    }

    EXPECT_FALSE(testObj->canProceed);
    EXPECT_EQ(thread, testObj->methodThread);
    testObj->canProceed = true;

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete testObj;
}

TEST(ProofObjectTest, BlockingCallQObject)
{
    TestQObject *testObj = new TestQObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    int result = 0;
    auto chain = TaskChain::createChain();
    qlonglong taskId = chain->addTask([testObj]() {
        while (!testObj->started) {
            QThread::yieldCurrentThread();
            qApp->processEvents();
        }
        QThread::msleep(50);
        testObj->canProceed = true;
    });

    QTime timer;
    timer.start();

    bool runnerExeced = false;
    QTimer queuedRunner;
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced](){runnerExeced = true;}, Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::call(testObj, &TestQObject::multiply, Call::Block, result, 42, 10);

    EXPECT_TRUE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(chain->waitForTask(taskId, 100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete testObj;
}

TEST(ProofObjectTest, EventsBlockingCallQObject)
{
    TestQObject *testObj = new TestQObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    int result = 0;
    auto chain = TaskChain::createChain();
    qlonglong taskId = chain->addTask([testObj]() {
        while (!testObj->started) {
            QThread::yieldCurrentThread();
            qApp->processEvents();
        }
        QThread::msleep(50);
        testObj->canProceed = true;
    });

    QTime timer;
    timer.start();

    bool runnerExeced = false;
    QTimer queuedRunner;
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced](){runnerExeced = true;}, Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::call(testObj, &TestQObject::multiply, Call::BlockEvents, result, 42, 10);

    EXPECT_FALSE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(chain->waitForTask(taskId, 100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete testObj;
}

TEST(ProofObjectTest, NonBlockingCallPlainObjectWithContext)
{
    TestObject *testObj = new TestObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    ProofObject::call(context, testObj, &TestObject::multiply, 42, 10);
    while (!testObj->started) {
        QThread::yieldCurrentThread();
        qApp->processEvents();
    }

    EXPECT_FALSE(testObj->canProceed);
    EXPECT_EQ(thread, testObj->methodThread);
    testObj->canProceed = true;

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete context;
    delete testObj;
}

TEST(ProofObjectTest, BlockingCallPlainObjectWithContext)
{
    TestObject *testObj = new TestObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    int result = 0;
    auto chain = TaskChain::createChain();
    qlonglong taskId = chain->addTask([testObj]() {
        while (!testObj->started) {
            QThread::yieldCurrentThread();
            qApp->processEvents();
        }
        QThread::msleep(50);
        testObj->canProceed = true;
    });

    QTime timer;
    timer.start();

    bool runnerExeced = false;
    QTimer queuedRunner;
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced](){runnerExeced = true;}, Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::call(context, testObj, &TestObject::multiply, Call::Block, result, 42, 10);

    EXPECT_TRUE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(chain->waitForTask(taskId, 100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete context;
    delete testObj;
}

TEST(ProofObjectTest, EventsBlockingCallPlainObjectWithContext)
{
    TestObject *testObj = new TestObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    int result = 0;
    auto chain = TaskChain::createChain();
    qlonglong taskId = chain->addTask([testObj]() {
        while (!testObj->started) {
            QThread::yieldCurrentThread();
            qApp->processEvents();
        }
        QThread::msleep(50);
        testObj->canProceed = true;
    });

    QTime timer;
    timer.start();

    bool runnerExeced = false;
    QTimer queuedRunner;
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced](){runnerExeced = true;}, Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::call(context, testObj, &TestObject::multiply, Call::BlockEvents, result, 42, 10);

    EXPECT_FALSE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(chain->waitForTask(taskId, 100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete context;
    delete testObj;
}

TEST(ProofObjectTest, NonBlockingCallWithContext)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    ProofObject::call(context, testObj, &TestProofObject::multiply, 42, 10);
    while (!testObj->started) {
        QThread::yieldCurrentThread();
        qApp->processEvents();
    }

    EXPECT_FALSE(testObj->canProceed);
    EXPECT_EQ(thread, testObj->methodThread);
    testObj->canProceed = true;

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete context;
    delete testObj;
}

TEST(ProofObjectTest, BlockingCallWithContext)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    int result = 0;
    auto chain = TaskChain::createChain();
    qlonglong taskId = chain->addTask([testObj]() {
        while (!testObj->started) {
            QThread::yieldCurrentThread();
            qApp->processEvents();
        }
        QThread::msleep(50);
        testObj->canProceed = true;
    });

    QTime timer;
    timer.start();

    bool runnerExeced = false;
    QTimer queuedRunner;
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced](){runnerExeced = true;}, Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::call(context, testObj, &TestProofObject::multiply, Call::Block, result, 42, 10);

    EXPECT_TRUE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(chain->waitForTask(taskId, 100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete context;
    delete testObj;
}

TEST(ProofObjectTest, EventsBlockingCallWithContext)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    int result = 0;
    auto chain = TaskChain::createChain();
    qlonglong taskId = chain->addTask([testObj]() {
        while (!testObj->started) {
            QThread::yieldCurrentThread();
            qApp->processEvents();
        }
        QThread::msleep(50);
        testObj->canProceed = true;
    });

    QTime timer;
    timer.start();

    bool runnerExeced = false;
    QTimer queuedRunner;
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced](){runnerExeced = true;}, Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::call(context, testObj, &TestProofObject::multiply, Call::BlockEvents, result, 42, 10);

    EXPECT_FALSE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(chain->waitForTask(taskId, 100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete context;
    delete testObj;
}
