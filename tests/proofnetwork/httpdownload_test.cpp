// clazy:skip

#include "proofnetwork/httpdownloader.h"
#include "proofnetwork/restclient.h"

#include "gtest/proof/test_global.h"

using namespace Proof;
using testing::Test;

static const int TIMEOUT = 2000; //msec

class HttpDownloaderTest : public Test
{
public:
    HttpDownloaderTest() {}

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

        httpDownloaderUT = new Proof::HttpDownloader();
    }

    void TearDown() override
    {
        delete httpDownloaderUT;
        delete serverRunner;
    }

protected:
    HttpDownloader *httpDownloaderUT;
    FakeServerRunner *serverRunner;
};

TEST_F(HttpDownloaderTest, download)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());
    serverRunner->setServerAnswer(QByteArray("file body"));

    Future<QByteArray> future = httpDownloaderUT->download(QUrl("http://127.0.0.1:9091/test.jpg"));
    QTime timer;
    timer.start();

    while (!future.isCompleted() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    EXPECT_EQ(FakeServer::Method::Get, serverRunner->lastQueryMethod());
    EXPECT_EQ(QUrl("/test.jpg"), serverRunner->lastQueryUrl());
    EXPECT_TRUE(serverRunner->lastQueryBody().isEmpty());

    ASSERT_TRUE(future.isCompleted());
    EXPECT_TRUE(future.isSucceeded());
    EXPECT_FALSE(future.isFailed());
    EXPECT_EQ("file body", future.result());
}

TEST_F(HttpDownloaderTest, failDownload)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());
    httpDownloaderUT->restClient()->setMsecsForTimeout(TIMEOUT / 2);

    Future<QByteArray> future = httpDownloaderUT->download(QUrl("http://127.0.0.1:9000/test.jpg"));
    QTime timer;
    timer.start();

    while (!future.isCompleted() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    ASSERT_TRUE(future.isCompleted());
    EXPECT_FALSE(future.isSucceeded());
    EXPECT_TRUE(future.isFailed());
    Failure failure = future.failureReason();
    EXPECT_EQ(0, failure.data.toInt());
    EXPECT_EQ(NetworkErrorCode::ServerError, failure.errorCode);
    EXPECT_EQ("Download failed", failure.message);
}

TEST_F(HttpDownloaderTest, failDownloadNotFound)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());
    serverRunner->setResultCode(404, "Not Found");

    Future<QByteArray> future = httpDownloaderUT->download(QUrl("http://127.0.0.1:9091/test.jpg"));
    QTime timer;
    timer.start();

    while (!future.isCompleted() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    EXPECT_EQ(FakeServer::Method::Get, serverRunner->lastQueryMethod());
    EXPECT_EQ(QUrl("/test.jpg"), serverRunner->lastQueryUrl());
    EXPECT_TRUE(serverRunner->lastQueryBody().isEmpty());

    ASSERT_TRUE(future.isCompleted());
    EXPECT_FALSE(future.isSucceeded());
    EXPECT_TRUE(future.isFailed());
    Failure failure = future.failureReason();
    EXPECT_EQ(404, failure.data.toInt());
    EXPECT_EQ(NetworkErrorCode::ServerError, failure.errorCode);
    EXPECT_EQ("Download failed: Not Found", failure.message);
}

TEST_F(HttpDownloaderTest, wrongUrl)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());

    Future<QByteArray> future = httpDownloaderUT->download(QUrl());
    QTime timer;
    timer.start();

    while (!future.isCompleted() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    ASSERT_TRUE(future.isCompleted());
    EXPECT_FALSE(future.isSucceeded());
    EXPECT_TRUE(future.isFailed());
    Failure failure = future.failureReason();
    EXPECT_EQ(failure.errorCode, NetworkErrorCode::InvalidUrl);
}
