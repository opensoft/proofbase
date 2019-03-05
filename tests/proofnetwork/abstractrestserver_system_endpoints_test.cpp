// clazy:skip

#include "proofcore/coreapplication.h"
#include "proofcore/memorystoragenotificationhandler.h"
#include "proofcore/proofglobal.h"

#include "proofnetwork/abstractrestserver.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/restclient.h"

#include "gtest/proof/test_global.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTest>

#include <tuple>

using testing::Test;
using testing::TestWithParam;

class SystemEndpointsTestRestServer : public Proof::AbstractRestServer
{
    Q_OBJECT
public:
    SystemEndpointsTestRestServer(const QString &pathPrefix = QString(), int port = 9091)
        : Proof::AbstractRestServer(pathPrefix, port)
    {
        setAuthType(Proof::RestAuthType::Basic);
        setUserName("username");
        setPassword("password");
    }

protected:
    Proof::Future<Proof::HealthStatusMap> healthStatus(bool) const override
    {
        return Proof::futures::successful(QMap<QString, QPair<QDateTime, QVariant>>{
            {"extra-param", qMakePair(QDateTime::currentDateTime(), QVariant(42))}});
    }
};

class RestServerSystemEndpointsTest : public Test
{
public:
    RestServerSystemEndpointsTest() {}

protected:
    void SetUp() override
    {
        restServerUT = new SystemEndpointsTestRestServer();
        ASSERT_EQ("", restServerUT->pathPrefix());
        ASSERT_EQ(9091, restServerUT->port());
        ASSERT_EQ("username", restServerUT->userName());
        ASSERT_EQ("password", restServerUT->password());
        ASSERT_EQ(Proof::RestAuthType::Basic, restServerUT->authType());
        restServerUT->startListen();

        QTime timer;
        timer.start();
        while (!restServerUT->isListening() && timer.elapsed() < 10000)
            QThread::msleep(50);
        ASSERT_TRUE(restServerUT->isListening());

        restClientUT = Proof::RestClientSP::create();
        restClientUT->setAuthType(Proof::RestAuthType::NoAuth);
        restClientUT->setHost("127.0.0.1");
        restClientUT->setPort(9091);
        restClientUT->setScheme("http");
        restClientUT->setClientName("Proof-test");
        Proof::ErrorNotifier::instance()->unregisterHandler<Proof::MemoryStorageNotificationHandler>();
        Proof::ErrorNotifier::instance()->registerHandler(
            new Proof::MemoryStorageNotificationHandler("RestServerSystemEndpointsTest"));
    }

    void TearDown() override { delete restServerUT; }

protected:
    Proof::RestClientSP restClientUT;
    SystemEndpointsTestRestServer *restServerUT = nullptr;
};

TEST_F(RestServerSystemEndpointsTest, recentErrors)
{
    ASSERT_TRUE(restServerUT->isListening());

    auto memoryErrorsStorage = Proof::ErrorNotifier::instance()->handler<Proof::MemoryStorageNotificationHandler>();
    ASSERT_TRUE(memoryErrorsStorage->messages().isEmpty());

    {
        QNetworkReply *reply = restClientUT->get("/system/recent-errors").result();
        QTime timer;
        timer.start();
        while (!reply->isFinished() && timer.elapsed() < 10000)
            QThread::msleep(5);
        ASSERT_TRUE(reply->isFinished());

        EXPECT_EQ(200, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

        const QString answer = QString(reply->readAll()).trimmed();
        QJsonDocument doc = QJsonDocument::fromJson(answer.toUtf8());
        EXPECT_TRUE(doc.isArray());
        EXPECT_TRUE(doc.array().isEmpty());

        delete reply;
    }
    Proof::ErrorNotifier::instance()->notify("message1");
    Proof::ErrorNotifier::instance()->notify("message2");
    {
        QNetworkReply *reply = restClientUT->get("/system/recent-errors").result();
        QTime timer;
        timer.start();
        while (!reply->isFinished() && timer.elapsed() < 10000)
            QThread::msleep(5);
        ASSERT_TRUE(reply->isFinished());

        EXPECT_EQ(200, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

        const QString answer = QString(reply->readAll()).trimmed();
        QJsonDocument doc = QJsonDocument::fromJson(answer.toUtf8());
        EXPECT_TRUE(doc.isArray());
        EXPECT_EQ(2, doc.array().count());
        QSet<QString> arrived{doc.array()[0].toObject().value("message").toString(),
                              doc.array()[1].toObject().value("message").toString()};
        EXPECT_EQ((QSet<QString>{"message1", "message2"}), arrived);

        delete reply;
    }
}

TEST_F(RestServerSystemEndpointsTest, healthStatus)
{
    ASSERT_TRUE(restServerUT->isListening());

    auto memoryErrorsStorage = Proof::ErrorNotifier::instance()->handler<Proof::MemoryStorageNotificationHandler>();
    ASSERT_TRUE(memoryErrorsStorage->messages().isEmpty());

    {
        QNetworkReply *reply = restClientUT->get("/system/status").result();
        QTime timer;
        timer.start();
        while (!reply->isFinished() && timer.elapsed() < 10000)
            QThread::msleep(5);
        ASSERT_TRUE(reply->isFinished());

        EXPECT_EQ(200, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

        const QString answer = QString(reply->readAll()).trimmed();
        QJsonDocument doc = QJsonDocument::fromJson(answer.toUtf8());
        EXPECT_TRUE(doc.isObject());
        QJsonObject obj = doc.object();
        EXPECT_EQ(qApp->applicationName(), obj.value("app_type").toString());
        EXPECT_EQ(qApp->applicationVersion(), obj.value("app_version").toString());
        EXPECT_EQ(Proof::proofVersion(), obj.value("proof_version").toString());
        EXPECT_EQ(proofApp->startedAt().toString(Qt::ISODate), obj.value("started_at").toString());
        EXPECT_EQ(QSysInfo::prettyProductName(), obj.value("os").toString());
        ASSERT_EQ(1, obj.value("health").toArray().count());
        EXPECT_EQ("extra-param", obj.value("health").toArray()[0].toObject().value("name").toString());
        EXPECT_EQ(42, obj.value("health").toArray()[0].toObject().value("value").toInt());
        EXPECT_TRUE(obj.value("health").toArray()[0].toObject().contains("updated_at"));
        EXPECT_TRUE(obj.contains("last_crash_at"));
        EXPECT_TRUE(obj.contains("network_addresses"));
        EXPECT_TRUE(obj.contains("generated_at"));
        EXPECT_TRUE(obj.contains("last_error"));
        EXPECT_TRUE(obj.value("last_error").isNull());

        delete reply;
    }
    Proof::ErrorNotifier::instance()->notify("error message");
    {
        QNetworkReply *reply = restClientUT->get("/system/status").result();
        QTime timer;
        timer.start();
        while (!reply->isFinished() && timer.elapsed() < 10000)
            QThread::msleep(5);
        ASSERT_TRUE(reply->isFinished());

        EXPECT_EQ(200, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

        const QString answer = QString(reply->readAll()).trimmed();
        QJsonDocument doc = QJsonDocument::fromJson(answer.toUtf8());
        EXPECT_TRUE(doc.isObject());
        QJsonObject obj = doc.object();
        EXPECT_TRUE(obj.value("last_error").isObject());
        EXPECT_EQ("error message", obj.value("last_error").toObject().value("message").toString());
        delete reply;
    }
}
#include "abstractrestserver_system_endpoints_test.moc"
