// clazy:skip

#include "proofcore/errornotifier.h"
#include "proofcore/memorystoragenotificationhandler.h"

#include "gtest/proof/test_global.h"

using namespace Proof;

TEST(MemoryStorageNotificationHandlerTest, registerUnregister)
{
    MemoryStorageNotificationHandler *handler = ErrorNotifier::instance()->handler<MemoryStorageNotificationHandler>();
    ASSERT_TRUE(handler);
    EXPECT_TRUE(handler->messages().isEmpty());

    ErrorNotifier::instance()->unregisterHandler<MemoryStorageNotificationHandler>();
    handler = ErrorNotifier::instance()->handler<MemoryStorageNotificationHandler>();
    EXPECT_FALSE(handler);

    MemoryStorageNotificationHandler *newHandler = new MemoryStorageNotificationHandler("testsHandler");
    ErrorNotifier::instance()->registerHandler(newHandler);
    handler = ErrorNotifier::instance()->handler<MemoryStorageNotificationHandler>();
    ASSERT_TRUE(handler);
    EXPECT_EQ(newHandler, handler);
    EXPECT_EQ("testsHandler", handler->appId());
    EXPECT_TRUE(handler->messages().isEmpty());
}

TEST(MemoryStorageNotificationHandlerTest, notify)
{
    MemoryStorageNotificationHandler *handler = ErrorNotifier::instance()->handler<MemoryStorageNotificationHandler>();
    ASSERT_TRUE(handler);
    ASSERT_TRUE(handler->messages().isEmpty());

    QDateTime timestamp = QDateTime::currentDateTime();
    ErrorNotifier::instance()->notify("error message");
    {
        ASSERT_EQ(1, handler->messages().size());
        auto all = handler->messages();
        auto last = handler->lastMessage();
        EXPECT_NEAR(last.first.toSecsSinceEpoch(), timestamp.toSecsSinceEpoch(), 10);
        EXPECT_EQ("error message", last.second);
        EXPECT_EQ(last.first, all.firstKey());
        EXPECT_EQ(last.second, all.first());
    }

    QThread::msleep(100);
    timestamp = QDateTime::currentDateTime();
    ErrorNotifier::instance()->notify("error message 2");
    {
        ASSERT_EQ(2, handler->messages().size());
        auto all = handler->messages();
        auto last = handler->lastMessage();
        EXPECT_NEAR(last.first.toSecsSinceEpoch(), timestamp.toSecsSinceEpoch(), 10);
        EXPECT_EQ("error message 2", last.second);
        EXPECT_EQ(last.first, all.lastKey());
        EXPECT_EQ(last.second, all.last());
    }
}
