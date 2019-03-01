// clazy:skip

#include "proofseed/asynqro_extra.h"

#include "proofcore/proofobject.h"

#include "gtest/proof/test_global.h"

#include <QDateTime>
#include <QThread>
#include <QTimer>

#include <atomic>

using namespace Proof;

class TestProofObject : public ProofObject
{
    Q_OBJECT
public:
    TestProofObject() : ProofObject(nullptr) {}

    void decorateFutureWithFailureHandler(const Future<bool> &f, Failure::Hints hints)
    {
        f.onFailure(simpleFailureHandler(hints));
    }

    void shouldNotHappen() { shouldNotHappenHappened = true; }

    int multiply(int a, int b)
    {
        methodThread = QThread::currentThread();
        started = true;
        while (!canProceed)
            QThread::yieldCurrentThread();
        return a * b;
    }

    QThread *methodThread = nullptr;
    std::atomic_bool started{false};
    std::atomic_bool canProceed{false};

    std::atomic_bool shouldNotHappenHappened{false};
};

class TestQObject : public QObject
{
    Q_OBJECT
public:
    TestQObject() : QObject(nullptr) {}

    void shouldNotHappen() { shouldNotHappenHappened = true; }

    int multiply(int a, int b)
    {
        methodThread = QThread::currentThread();
        started = true;
        while (!canProceed)
            QThread::yieldCurrentThread();
        return a * b;
    }

    QThread *methodThread = nullptr;
    std::atomic_bool started{false};
    std::atomic_bool canProceed{false};

    std::atomic_bool shouldNotHappenHappened{false};
};

class TestObject
{
public:
    TestObject() {}

    int multiply(int a, int b)
    {
        methodThread = QThread::currentThread();
        started = true;
        while (!canProceed)
            QThread::yieldCurrentThread();
        return a * b;
    }

    QThread *methodThread = nullptr;
    std::atomic_bool started{false};
    std::atomic_bool canProceed{false};
};

TEST(ProofObjectTest, nonBlockingSafeCall)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    ProofObject::safeCall(testObj, &TestProofObject::multiply, 42, 10);
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

TEST(ProofObjectTest, blockingSafeCall)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    int result = 0;
    auto task = tasks::run([testObj]() {
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
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced]() { runnerExeced = true; },
                     Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    bool invoked = ProofObject::safeCall(testObj, &TestProofObject::multiply, Call::Block, result, 42, 10);

    EXPECT_TRUE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);
    EXPECT_TRUE(invoked);

    EXPECT_TRUE(task.wait(100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete testObj;
}

TEST(ProofObjectTest, eventsBlockingSafeCall)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    int result = 0;
    auto task = tasks::run([testObj]() {
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
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced]() { runnerExeced = true; },
                     Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::safeCall(testObj, &TestProofObject::multiply, Call::BlockEvents, result, 42, 10);

    EXPECT_FALSE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(task.wait(100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete testObj;
}

TEST(ProofObjectTest, nonBlockingSafeCallQObject)
{
    TestQObject *testObj = new TestQObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    ProofObject::safeCall(testObj, &TestQObject::multiply, 42, 10);
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

TEST(ProofObjectTest, blockingSafeCallQObject)
{
    TestQObject *testObj = new TestQObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    int result = 0;
    auto task = tasks::run([testObj]() {
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
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced]() { runnerExeced = true; },
                     Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::safeCall(testObj, &TestQObject::multiply, Call::Block, result, 42, 10);

    EXPECT_TRUE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(task.wait(100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete testObj;
}

TEST(ProofObjectTest, eventsBlockingSafeCallQObject)
{
    TestQObject *testObj = new TestQObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    int result = 0;
    auto task = tasks::run([testObj]() {
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
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced]() { runnerExeced = true; },
                     Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::safeCall(testObj, &TestQObject::multiply, Call::BlockEvents, result, 42, 10);

    EXPECT_FALSE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(task.wait(100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete testObj;
}

TEST(ProofObjectTest, nonBlockingSafeCallPlainObjectWithContext)
{
    TestObject *testObj = new TestObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    ProofObject::safeCall(context, testObj, &TestObject::multiply, 42, 10);
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

TEST(ProofObjectTest, blockingSafeCallPlainObjectWithContext)
{
    TestObject *testObj = new TestObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    int result = 0;
    auto task = tasks::run([testObj]() {
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
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced]() { runnerExeced = true; },
                     Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::safeCall(context, testObj, &TestObject::multiply, Call::Block, result, 42, 10);

    EXPECT_TRUE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(task.wait(100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete context;
    delete testObj;
}

TEST(ProofObjectTest, eventsBlockingSafeCallPlainObjectWithContext)
{
    TestObject *testObj = new TestObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    int result = 0;
    auto task = tasks::run([testObj]() {
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
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced]() { runnerExeced = true; },
                     Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::safeCall(context, testObj, &TestObject::multiply, Call::BlockEvents, result, 42, 10);

    EXPECT_FALSE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(task.wait(100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete context;
    delete testObj;
}

TEST(ProofObjectTest, nonBlockingSafeCallWithContext)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    ProofObject::safeCall(context, testObj, &TestProofObject::multiply, 42, 10);
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

TEST(ProofObjectTest, blockingSafeCallWithContext)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    int result = 0;
    auto task = tasks::run([testObj]() {
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
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced]() { runnerExeced = true; },
                     Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::safeCall(context, testObj, &TestProofObject::multiply, Call::Block, result, 42, 10);

    EXPECT_TRUE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(task.wait(100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete context;
    delete testObj;
}

TEST(ProofObjectTest, eventsBlockingSafeCallWithContext)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    QObject *context = new QObject;
    context->moveToThread(thread);
    thread->start();

    int result = 0;
    auto task = tasks::run([testObj]() {
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
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced]() { runnerExeced = true; },
                     Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::safeCall(context, testObj, &TestProofObject::multiply, Call::BlockEvents, result, 42, 10);

    EXPECT_FALSE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(task.wait(100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete context;
    delete testObj;
}

TEST(ProofObjectTest, call)
{
    TestProofObject *testObj = new TestProofObject;
    QThread *thread = new QThread;
    testObj->moveToThread(thread);
    thread->start();

    int result = 0;
    auto task = tasks::run([testObj]() {
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
    QObject::connect(&queuedRunner, &QTimer::timeout, &queuedRunner, [&runnerExeced]() { runnerExeced = true; },
                     Qt::QueuedConnection);
    queuedRunner.setSingleShot(true);
    queuedRunner.start(0);

    EXPECT_FALSE(runnerExeced);
    ProofObject::call(testObj, &TestProofObject::multiply, Call::Block, result, 42, 10);

    EXPECT_TRUE(runnerExeced);
    EXPECT_LE(50, timer.elapsed());
    EXPECT_EQ(420, result);
    EXPECT_EQ(thread, testObj->methodThread);

    EXPECT_TRUE(task.wait(100));

    thread->quit();
    thread->wait(1000);
    delete thread;
    delete testObj;
}

TEST(ProofObjectTest, callSameThread)
{
    TestProofObject *testObj = new TestProofObject;
    testObj->canProceed = true;
    int result = 0;
    ProofObject::call(testObj, &TestProofObject::multiply, Call::Block, result, 42, 10);
    EXPECT_EQ(420, result);
    EXPECT_EQ(QThread::currentThread(), testObj->methodThread);
    delete testObj;
}

TEST(ProofObjectTest, safeCallSameThread)
{
    TestProofObject *testObj = new TestProofObject;
    bool invoked = ProofObject::safeCall(testObj, &TestProofObject::shouldNotHappen, Call::Block);
    EXPECT_FALSE(invoked);
    EXPECT_FALSE(testObj->shouldNotHappenHappened);
    delete testObj;
}

TEST(ProofObjectTest, failureHandlerDefault)
{
    TestProofObject *testObj = new TestProofObject;
    QString message;
    long module = 0;
    long error = 0;
    bool wasFatal = false;
    bool wasUserFriendly = false;
    QObject::connect(testObj, &ProofObject::errorOccurred, testObj,
                     [&message, &module, &error, &wasFatal, &wasUserFriendly](long moduleCode, long errorCode,
                                                                              const QString &errorMessage,
                                                                              bool userFriendly, bool fatal) {
                         message = errorMessage;
                         module = moduleCode;
                         error = errorCode;
                         wasUserFriendly = userFriendly;
                         wasFatal = fatal;
                     });
    Promise<bool> p;
    testObj->decorateFutureWithFailureHandler(p.future(), Failure::Hints::NoHint);
    p.failure(Failure("Message", 42, 10));
    EXPECT_EQ("Message", message);
    EXPECT_EQ(42, module);
    EXPECT_EQ(10, error);
    EXPECT_FALSE(wasUserFriendly);
    EXPECT_FALSE(wasFatal);
    delete testObj;
}

TEST(ProofObjectTest, failureHandlerFriendly)
{
    TestProofObject *testObj = new TestProofObject;
    QString message;
    long module = 0;
    long error = 0;
    bool wasFatal = false;
    bool wasUserFriendly = false;
    QObject::connect(testObj, &ProofObject::errorOccurred, testObj,
                     [&message, &module, &error, &wasFatal, &wasUserFriendly](long moduleCode, long errorCode,
                                                                              const QString &errorMessage,
                                                                              bool userFriendly, bool fatal) {
                         message = errorMessage;
                         module = moduleCode;
                         error = errorCode;
                         wasUserFriendly = userFriendly;
                         wasFatal = fatal;
                     });
    Promise<bool> p;
    testObj->decorateFutureWithFailureHandler(p.future(), Failure::Hints::NoHint);
    p.failure(Failure("Message", 42, 10, Failure::Hints::UserFriendlyHint));
    EXPECT_EQ("Message", message);
    EXPECT_EQ(42, module);
    EXPECT_EQ(10, error);
    EXPECT_TRUE(wasUserFriendly);
    EXPECT_FALSE(wasFatal);
    delete testObj;
}

TEST(ProofObjectTest, failureHandlerCritical)
{
    TestProofObject *testObj = new TestProofObject;
    QString message;
    long module = 0;
    long error = 0;
    bool wasFatal = false;
    bool wasUserFriendly = false;
    QObject::connect(testObj, &ProofObject::errorOccurred, testObj,
                     [&message, &module, &error, &wasFatal, &wasUserFriendly](long moduleCode, long errorCode,
                                                                              const QString &errorMessage,
                                                                              bool userFriendly, bool fatal) {
                         message = errorMessage;
                         module = moduleCode;
                         error = errorCode;
                         wasUserFriendly = userFriendly;
                         wasFatal = fatal;
                     });
    Promise<bool> p;
    testObj->decorateFutureWithFailureHandler(p.future(), Failure::Hints::NoHint);
    p.failure(Failure("Message", 42, 10, Failure::Hints::CriticalHint));
    EXPECT_EQ("Message", message);
    EXPECT_EQ(42, module);
    EXPECT_EQ(10, error);
    EXPECT_FALSE(wasUserFriendly);
    EXPECT_TRUE(wasFatal);
    delete testObj;
}

TEST(ProofObjectTest, failureHandlerForceFriendly)
{
    TestProofObject *testObj = new TestProofObject;
    QString message;
    long module = 0;
    long error = 0;
    bool wasFatal = false;
    bool wasUserFriendly = false;
    QObject::connect(testObj, &ProofObject::errorOccurred, testObj,
                     [&message, &module, &error, &wasFatal, &wasUserFriendly](long moduleCode, long errorCode,
                                                                              const QString &errorMessage,
                                                                              bool userFriendly, bool fatal) {
                         message = errorMessage;
                         module = moduleCode;
                         error = errorCode;
                         wasUserFriendly = userFriendly;
                         wasFatal = fatal;
                     });
    Promise<bool> p;
    testObj->decorateFutureWithFailureHandler(p.future(), Failure::Hints::UserFriendlyHint);
    p.failure(Failure("Message", 42, 10));
    EXPECT_EQ("Message", message);
    EXPECT_EQ(42, module);
    EXPECT_EQ(10, error);
    EXPECT_TRUE(wasUserFriendly);
    EXPECT_FALSE(wasFatal);
    delete testObj;
}

TEST(ProofObjectTest, failureHandlerForceCritical)
{
    TestProofObject *testObj = new TestProofObject;
    QString message;
    long module = 0;
    long error = 0;
    bool wasFatal = false;
    bool wasUserFriendly = false;
    QObject::connect(testObj, &ProofObject::errorOccurred, testObj,
                     [&message, &module, &error, &wasFatal, &wasUserFriendly](long moduleCode, long errorCode,
                                                                              const QString &errorMessage,
                                                                              bool userFriendly, bool fatal) {
                         message = errorMessage;
                         module = moduleCode;
                         error = errorCode;
                         wasUserFriendly = userFriendly;
                         wasFatal = fatal;
                     });
    Promise<bool> p;
    testObj->decorateFutureWithFailureHandler(p.future(), Failure::Hints::CriticalHint);
    p.failure(Failure("Message", 42, 10));
    EXPECT_EQ("Message", message);
    EXPECT_EQ(42, module);
    EXPECT_EQ(10, error);
    EXPECT_FALSE(wasUserFriendly);
    EXPECT_TRUE(wasFatal);
    delete testObj;
}
#include "proofobject_test.moc"
