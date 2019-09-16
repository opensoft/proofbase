// clazy:skip

#include "proofnetwork/papertrailnotificationhandler.h"

#include "gtest/proof/test_global.h"

#include <QTime>

using namespace Proof;
using testing::Test;

static const int TIMEOUT = 2000; //msec

class PapertrailNotificationHandlerTest : public Test
{
public:
    PapertrailNotificationHandlerTest() {}

protected:
    void SetUp() override
    {
        serverRunner = new FakeServerRunner;
        serverRunner->runServer();
        QTime timer;
        timer.start();
        while (!serverRunner->serverIsRunning() && timer.elapsed() < 10000)
            QThread::msleep(50);
        ASSERT_TRUE(serverRunner->serverIsRunning());

        papertrailHandlerUT = new Proof::PapertrailNotificationHandler("127.0.0.1", 9091, QString("senderName"),
                                                                       QString("appId"));
    }

    void TearDown() override
    {
        delete papertrailHandlerUT;
        delete serverRunner;
    }

protected:
    PapertrailNotificationHandler *papertrailHandlerUT;
    FakeServerRunner *serverRunner;
};

TEST_F(PapertrailNotificationHandlerTest, notify)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());

    papertrailHandlerUT->notify("TestMessage", Proof::ErrorNotifier::Severity::Info);

    QTime timer;
    timer.start();
    while (serverRunner->lastQueryRaw().isEmpty() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    ASSERT_FALSE(serverRunner->lastQueryRaw().isEmpty());
    EXPECT_TRUE(serverRunner->lastQueryRaw().contains("senderName appId INFO | TestMessage"));
}
