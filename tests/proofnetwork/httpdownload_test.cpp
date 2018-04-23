// clazy:skip

#include "gtest/test_global.h"

#include "proofnetwork/httpdownloader.h"

using namespace Proof;
using testing::Test;

static const int TIMEOUT = 2000; //msec

class HttpDownloaderTest: public Test
{
public:
    HttpDownloaderTest()
    {
    }

protected:
    void SetUp() override
    {
        serverRunner = new FakeServerRunner;
        serverRunner->runServer();

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

    FutureSP<QByteArray> future = httpDownloaderUT->download(QUrl("http://127.0.0.1:9091/test.jpg"));
    QTime timer;
    timer.start();
    while (!future->completed() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    ASSERT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_EQ("file body", future->result());
}

TEST_F(HttpDownloaderTest, failDownload)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());

    FutureSP<QByteArray> future = httpDownloaderUT->download(QUrl("http://127.0.0.1:9000/test.jpg"));
    QTime timer;
    timer.start();
    while (!future->completed() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    Failure failure = future->failureReason();
    EXPECT_EQ(0, failure.data.toInt());
    EXPECT_EQ(NetworkErrorCode::ServerError, failure.errorCode);
    EXPECT_EQ("Download failed", failure.message);
}


TEST_F(HttpDownloaderTest, failDownloadNotFound)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());
    serverRunner->setResultCode(404, "Not Found");

    FutureSP<QByteArray> future = httpDownloaderUT->download(QUrl("http://127.0.0.1:9091/test.jpg"));
    QTime timer;
    timer.start();
    while (!future->completed() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    Failure failure = future->failureReason();
    EXPECT_EQ(404, failure.data.toInt());
    EXPECT_EQ(NetworkErrorCode::ServerError, failure.errorCode);
    EXPECT_EQ("Download failed: Not Found", failure.message);
}

TEST_F(HttpDownloaderTest, wrongUrl)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());

    FutureSP<QByteArray> future = httpDownloaderUT->download(QUrl());
    QTime timer;
    timer.start();
    while (!future->completed() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    Failure failure = future->failureReason();
    EXPECT_EQ(failure.errorCode, NetworkErrorCode::InvalidUrl);
}
