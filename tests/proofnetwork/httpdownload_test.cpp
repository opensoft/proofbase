// clazy:skip

#include "proofnetwork/httpdownloader.h"
#include "proofnetwork/restclient.h"

#include "gtest/proof/test_global.h"

#include <QTemporaryFile>

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

TEST_F(HttpDownloaderTest, downloadTo)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());
    serverRunner->setServerAnswer(QByteArray("file body"));

    QTemporaryFile output;
    output.open();

    Future<QIODevice *> future = httpDownloaderUT->downloadTo(QUrl("http://127.0.0.1:9091/test.jpg"), &output);
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
    EXPECT_EQ(&output, future.result());
    output.flush();
    QFile outputReader(output.fileName());
    ASSERT_TRUE(outputReader.open(QIODevice::ReadOnly));
    EXPECT_EQ("file body", outputReader.readAll());
    outputReader.close();
}

TEST_F(HttpDownloaderTest, bigDownloadTo)
{
    QByteArray fullData;
    for (int i = 0; i < 1024 * 1024 * 30; ++i)
        fullData.append(static_cast<char>(i % 128));
    ASSERT_TRUE(serverRunner->serverIsRunning());
    serverRunner->setServerAnswer(fullData);

    QTemporaryFile output;
    output.open();

    Future<QIODevice *> future = httpDownloaderUT->downloadTo(QUrl("http://127.0.0.1:9091/test.jpg"), &output);
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
    EXPECT_EQ(&output, future.result());
    output.flush();
    QFile outputReader(output.fileName());
    ASSERT_TRUE(outputReader.open(QIODevice::ReadOnly));
    QByteArray result = outputReader.readAll();
    EXPECT_EQ(fullData.count(), result.count());
    EXPECT_TRUE(fullData == result);
    outputReader.close();
}

TEST_F(HttpDownloaderTest, bigDownloadToWithSomething)
{
    QByteArray fullData;
    for (int i = 0; i < 1024 * 1024 * 15 + 1024 * 15; ++i)
        fullData.append(static_cast<char>(i % 128));
    ASSERT_TRUE(serverRunner->serverIsRunning());
    serverRunner->setServerAnswer(fullData);

    QTemporaryFile output;
    output.open();

    Future<QIODevice *> future = httpDownloaderUT->downloadTo(QUrl("http://127.0.0.1:9091/test.jpg"), &output);
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
    EXPECT_EQ(&output, future.result());
    output.flush();
    QFile outputReader(output.fileName());
    ASSERT_TRUE(outputReader.open(QIODevice::ReadOnly));
    QByteArray result = outputReader.readAll();
    EXPECT_EQ(fullData.count(), result.count());
    EXPECT_TRUE(fullData == result);
    outputReader.close();
}

TEST_F(HttpDownloaderTest, failDownloadTo)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());
    httpDownloaderUT->restClient()->setMsecsForTimeout(TIMEOUT / 2);

    QTemporaryFile output;
    output.open();

    Future<QIODevice *> future = httpDownloaderUT->downloadTo(QUrl("http://127.0.0.1:9000/test.jpg"), &output);
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

TEST_F(HttpDownloaderTest, failDownloadToNotFound)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());
    serverRunner->setResultCode(404, "Not Found");

    QTemporaryFile output;
    output.open();

    Future<QIODevice *> future = httpDownloaderUT->downloadTo(QUrl("http://127.0.0.1:9091/test.jpg"), &output);
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

TEST_F(HttpDownloaderTest, wrongUrlInDownloadTo)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());

    QTemporaryFile output;
    output.open();

    Future<QIODevice *> future = httpDownloaderUT->downloadTo(QUrl(), &output);
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

TEST_F(HttpDownloaderTest, nullDeviceDownloadTo)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());

    Future<QIODevice *> future = httpDownloaderUT->downloadTo(QUrl("http://127.0.0.1:9091/test.jpg"), nullptr);
    QTime timer;
    timer.start();

    while (!future.isCompleted() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    ASSERT_TRUE(future.isCompleted());
    EXPECT_FALSE(future.isSucceeded());
    EXPECT_TRUE(future.isFailed());
    Failure failure = future.failureReason();
    EXPECT_EQ(failure.errorCode, NetworkErrorCode::InternalError);
}

TEST_F(HttpDownloaderTest, notOpenedDeviceDownloadTo)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());

    QTemporaryFile output;

    Future<QIODevice *> future = httpDownloaderUT->downloadTo(QUrl("http://127.0.0.1:9091/test.jpg"), &output);
    QTime timer;
    timer.start();

    while (!future.isCompleted() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    ASSERT_TRUE(future.isCompleted());
    EXPECT_FALSE(future.isSucceeded());
    EXPECT_TRUE(future.isFailed());
    Failure failure = future.failureReason();
    EXPECT_EQ(failure.errorCode, NetworkErrorCode::InternalError);
}

TEST_F(HttpDownloaderTest, nonWritableDeviceDownloadTo)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());

    QFile output(qApp->applicationFilePath());
    output.open(QIODevice::ReadOnly);

    Future<QIODevice *> future = httpDownloaderUT->downloadTo(QUrl("http://127.0.0.1:9091/test.jpg"), &output);
    QTime timer;
    timer.start();

    while (!future.isCompleted() && timer.elapsed() < TIMEOUT)
        qApp->processEvents();

    ASSERT_TRUE(future.isCompleted());
    EXPECT_FALSE(future.isSucceeded());
    EXPECT_TRUE(future.isFailed());
    Failure failure = future.failureReason();
    EXPECT_EQ(failure.errorCode, NetworkErrorCode::InternalError);
}

TEST_F(HttpDownloaderTest, failDownloadCustomTimeout)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());

    Future<QByteArray> future = httpDownloaderUT->download(QUrl("http://127.0.0.1:9000/test.jpg"), TIMEOUT / 2);
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

TEST_F(HttpDownloaderTest, failDownloadToCustomTimeout)
{
    ASSERT_TRUE(serverRunner->serverIsRunning());

    QTemporaryFile output;
    output.open();

    Future<QIODevice *> future = httpDownloaderUT->downloadTo(QUrl("http://127.0.0.1:9000/test.jpg"), &output,
                                                              TIMEOUT / 2);
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
