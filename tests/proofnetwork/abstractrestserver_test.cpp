// clazy:skip

#include "proofcore/coreapplication.h"

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

class TestRestServer : public Proof::AbstractRestServer
{
    Q_OBJECT
public:
    TestRestServer(const QString &pathPrefix = QString(), int port = 9091) : Proof::AbstractRestServer(pathPrefix, port)
    {
        setAuthType(Proof::RestAuthType::Basic);
        setUserName("username");
        setPassword("password");
    }

public slots:
    NO_AUTH_REQUIRED void rest_get_TestPublicMethod(QTcpSocket *socket, const QStringList &,
                                                    const QStringList &methodVariableParts,
                                                    const QUrlQuery &queryParams, const QByteArray &)
    {
        sendAnswer(socket, __func__, "text/plain", 200, methodVariableParts.join('/') + "|" + queryParams.toString());
    }

    void rest_get_TestMethod(QTcpSocket *socket, const QStringList &, const QStringList &methodVariableParts,
                             const QUrlQuery &queryParams, const QByteArray &)
    {
        sendAnswer(socket, __func__, "text/plain", 200, methodVariableParts.join('/') + "|" + queryParams.toString());
    }
};

class TestRestServerWithoutAuth : public Proof::AbstractRestServer
{
    Q_OBJECT
public:
    TestRestServerWithoutAuth() : Proof::AbstractRestServer(9092) {}

public slots:
    void rest_get_TestMethod(QTcpSocket *socket, const QStringList &, const QStringList &, const QUrlQuery &,
                             const QByteArray &)
    {
        sendAnswer(socket, __func__, "text/plain");
    }
    void rest_get_Slow_TestMethod(QTcpSocket *socket, const QStringList &, const QStringList &, const QUrlQuery &,
                                  const QByteArray &)
    {
        QThread::msleep(500);
        sendAnswer(socket, __func__, "text/plain");
    }

    void rest_get_TestMethodWithCustomHeader(QTcpSocket *socket, const QStringList &, const QStringList &,
                                             const QUrlQuery &, const QByteArray &)
    {
        sendAnswer(socket, __func__, "text/plain", QHash<QString, QString>{{"ExtraHeader", "extra header value"}});
    }

    void rest_get_Error_TestMethod(QTcpSocket *socket, const QStringList &, const QStringList &, const QUrlQuery &,
                                   const QByteArray &)
    {
        sendErrorCode(socket, 500, "Some reason here", 42, {"arg1", "arg2"});
    }

    void rest_get_Error_BadRequest(QTcpSocket *socket, const QStringList &, const QStringList &, const QUrlQuery &,
                                   const QByteArray &)
    {
        sendBadRequest(socket);
    }
    void rest_get_Error_NotFound(QTcpSocket *socket, const QStringList &, const QStringList &, const QUrlQuery &,
                                 const QByteArray &)
    {
        sendNotFound(socket);
    }
    void rest_get_Error_Conflict(QTcpSocket *socket, const QStringList &, const QStringList &, const QUrlQuery &,
                                 const QByteArray &)
    {
        sendConflict(socket);
    }
    void rest_get_Error_InternalError(QTcpSocket *socket, const QStringList &, const QStringList &, const QUrlQuery &,
                                      const QByteArray &)
    {
        sendInternalError(socket);
    }
    void rest_get_Error_NotImplemented(QTcpSocket *socket, const QStringList &, const QStringList &, const QUrlQuery &,
                                       const QByteArray &)
    {
        sendNotImplemented(socket);
    }
};

class RestServerTest : public Test
{
public:
    RestServerTest() {}

    static void SetUpTestCase()
    {
        restServerUT = new TestRestServer();
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

        restServerWithoutAuthUT = new TestRestServerWithoutAuth();
        restServerWithoutAuthUT->startListen();
        timer.start();
        while (!restServerWithoutAuthUT->isListening() && timer.elapsed() < 10000)
            QThread::msleep(50);
        ASSERT_TRUE(restServerWithoutAuthUT->isListening());
    }

    static void TearDownTestCase()
    {
        delete restServerWithoutAuthUT;
        delete restServerUT;
    }

protected:
    void SetUp() override
    {
        restClientUT = Proof::RestClientSP::create();
        restClientUT->setAuthType(Proof::RestAuthType::Basic);
        restClientUT->setUserName("username");
        restClientUT->setPassword("password");
        restClientUT->setHost("127.0.0.1");
        restClientUT->setPort(9091);
        restClientUT->setScheme("http");
        restClientUT->setClientName("Proof-test");

        restClientForNoAuthTagUT = Proof::RestClientSP::create();
        restClientForNoAuthTagUT->setAuthType(Proof::RestAuthType::NoAuth);
        restClientForNoAuthTagUT->setHost("127.0.0.1");
        restClientForNoAuthTagUT->setPort(9091);
        restClientForNoAuthTagUT->setScheme("http");
        restClientForNoAuthTagUT->setClientName("Proof-test");

        restClientWithoutAuthUT = Proof::RestClientSP::create();
        restClientWithoutAuthUT->setAuthType(Proof::RestAuthType::NoAuth);
        restClientWithoutAuthUT->setHost("127.0.0.1");
        restClientWithoutAuthUT->setPort(9092);
        restClientWithoutAuthUT->setScheme("http");
        restClientWithoutAuthUT->setClientName("Proof-test");
    }

protected:
    Proof::RestClientSP restClientForNoAuthTagUT;
    Proof::RestClientSP restClientUT;
    static TestRestServer *restServerUT;
    Proof::RestClientSP restClientWithoutAuthUT;
    static TestRestServerWithoutAuth *restServerWithoutAuthUT;
};

TestRestServer *RestServerTest::restServerUT = nullptr;
TestRestServerWithoutAuth *RestServerTest::restServerWithoutAuthUT = nullptr;

TEST_F(RestServerTest, defaultCtor)
{
    std::unique_ptr<Proof::AbstractRestServer> server(new Proof::AbstractRestServer());
    EXPECT_EQ(80, server->port());
    EXPECT_EQ("", server->pathPrefix());
    EXPECT_EQ("", server->userName());
    EXPECT_EQ("", server->password());
    EXPECT_EQ(Proof::RestAuthType::NoAuth, server->authType());
}

TEST_F(RestServerTest, customServer)
{
    {
        std::unique_ptr<Proof::AbstractRestServer> server(new Proof::AbstractRestServer(4096));
        EXPECT_EQ(4096, server->port());
        EXPECT_EQ("", server->pathPrefix());
        EXPECT_EQ("", server->userName());
        EXPECT_EQ("", server->password());
        EXPECT_EQ(Proof::RestAuthType::NoAuth, server->authType());
    }
    {
        std::unique_ptr<Proof::AbstractRestServer> server(new Proof::AbstractRestServer("api/internal", 4096));
        EXPECT_EQ(4096, server->port());
        EXPECT_EQ("api/internal", server->pathPrefix());
        EXPECT_EQ("", server->userName());
        EXPECT_EQ("", server->password());
        EXPECT_EQ(Proof::RestAuthType::NoAuth, server->authType());
    }
    {
        std::unique_ptr<Proof::AbstractRestServer> server(new Proof::AbstractRestServer());
        server->setPort(5120);
        EXPECT_EQ(5120, server->port());
        server->setPathPrefix("api/internal");
        EXPECT_EQ("api/internal", server->pathPrefix());
        server->setUserName("user");
        EXPECT_EQ("user", server->userName());
        server->setPassword("secret phrase");
        EXPECT_EQ("secret phrase", server->password());
        server->setAuthType(Proof::RestAuthType::Basic);
        EXPECT_EQ(Proof::RestAuthType::Basic, server->authType());

        EXPECT_EQ(5120, server->port());
        EXPECT_EQ("api/internal", server->pathPrefix());
        EXPECT_EQ("user", server->userName());
        EXPECT_EQ("secret phrase", server->password());
    }
}

TEST_F(RestServerTest, customHeadersSanity)
{
    std::unique_ptr<Proof::AbstractRestServer> server(new Proof::AbstractRestServer());
    EXPECT_FALSE(server->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ(QString(), server->customHeader("SpecialHeader"));
    server->setCustomHeader("SpecialHeader", "Some Value");
    EXPECT_TRUE(server->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ("Some Value", server->customHeader("SpecialHeader"));
    server->unsetCustomHeader("SpecialHeader");
    EXPECT_FALSE(server->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ(QString(), server->customHeader("SpecialHeader"));
    server->setCustomHeader("SpecialHeader", "Some Value");
    server->setCustomHeader("OtherSpecialHeader", "Some Another Value");
    EXPECT_TRUE(server->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ("Some Value", server->customHeader("SpecialHeader"));
    EXPECT_TRUE(server->containsCustomHeader("OtherSpecialHeader"));
    EXPECT_EQ("Some Another Value", server->customHeader("OtherSpecialHeader"));
    server->unsetCustomHeader("SpecialHeader");
    EXPECT_FALSE(server->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ(QString(), server->customHeader("SpecialHeader"));
    EXPECT_TRUE(server->containsCustomHeader("OtherSpecialHeader"));
    EXPECT_EQ("Some Another Value", server->customHeader("OtherSpecialHeader"));
    server->unsetCustomHeader("OtherSpecialHeader");
    EXPECT_FALSE(server->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ(QString(), server->customHeader("SpecialHeader"));
    EXPECT_FALSE(server->containsCustomHeader("OtherSpecialHeader"));
    EXPECT_EQ(QString(), server->customHeader("OtherSpecialHeader"));
}

TEST_F(RestServerTest, withoutAuth)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());

    QNetworkReply *reply = restClientWithoutAuthUT->get("/test-method").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());

    EXPECT_EQ(200, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    const QString answer = QString(reply->readAll()).trimmed();
    EXPECT_EQ("rest_get_TestMethod", answer);

    delete reply;
}

TEST_F(RestServerTest, withAuth)
{
    ASSERT_TRUE(restServerUT->isListening());

    QNetworkReply *reply = restClientUT->get("/test-method").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());

    EXPECT_EQ(200, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    const QString answer = QString(reply->readAll()).trimmed();
    EXPECT_EQ("rest_get_TestMethod", answer);

    delete reply;
}

TEST_F(RestServerTest, noAuthTag)
{
    ASSERT_TRUE(restServerUT->isListening());

    QNetworkReply *reply = restClientForNoAuthTagUT->get("/test-public-method").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());

    EXPECT_EQ(200, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    const QString answer = QString(reply->readAll()).trimmed();
    EXPECT_EQ("rest_get_TestPublicMethod", answer);

    delete reply;
}

TEST_F(RestServerTest, noAuthTagNegative)
{
    ASSERT_TRUE(restServerUT->isListening());

    QNetworkReply *reply = restClientForNoAuthTagUT->get("/test-method").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());

    EXPECT_EQ(401, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    delete reply;
}

TEST_F(RestServerTest, doubleRequest)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());

    QNetworkReply *slowReply = restClientWithoutAuthUT->get("/slow/test-method").result();
    QNetworkReply *reply = restClientWithoutAuthUT->get("/test-method").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    EXPECT_TRUE(reply->isFinished());
    while (!slowReply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    EXPECT_TRUE(slowReply->isFinished());

    delete reply;
    delete slowReply;
}

TEST_F(RestServerTest, dynamicHeaderRetrieve)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());

    QNetworkReply *reply = restClientWithoutAuthUT->get("/test-method-with-custom-header").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());

    EXPECT_EQ(200, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    const QString answer = QString(reply->readAll()).trimmed();
    EXPECT_EQ("rest_get_TestMethodWithCustomHeader", answer);
    EXPECT_TRUE(reply->hasRawHeader("ExtraHeader"));
    EXPECT_EQ("extra header value", reply->rawHeader("ExtraHeader"));

    delete reply;
}

TEST_F(RestServerTest, predefinedHeaderRetrieve)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());
    restServerWithoutAuthUT->setCustomHeader("CustomHeader", "custom header value");

    QNetworkReply *reply = restClientWithoutAuthUT->get("/test-method").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());

    EXPECT_EQ(200, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    const QString answer = QString(reply->readAll()).trimmed();
    EXPECT_EQ("rest_get_TestMethod", answer);
    EXPECT_TRUE(reply->hasRawHeader("CustomHeader"));
    EXPECT_EQ("custom header value", reply->rawHeader("CustomHeader"));

    delete reply;
}

TEST_F(RestServerTest, bothHeadersRetrieve)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());
    restServerWithoutAuthUT->setCustomHeader("CustomHeader", "custom header value");

    QNetworkReply *reply = restClientWithoutAuthUT->get("/test-method-with-custom-header").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());

    EXPECT_EQ(200, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    const QString answer = QString(reply->readAll()).trimmed();
    EXPECT_EQ("rest_get_TestMethodWithCustomHeader", answer);
    EXPECT_TRUE(reply->hasRawHeader("CustomHeader"));
    EXPECT_EQ("custom header value", reply->rawHeader("CustomHeader"));
    EXPECT_TRUE(reply->hasRawHeader("ExtraHeader"));
    EXPECT_EQ("extra header value", reply->rawHeader("ExtraHeader"));

    delete reply;
}

TEST_F(RestServerTest, errorCode)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());

    QNetworkReply *reply = restClientWithoutAuthUT->get("/error/test-method").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());

    EXPECT_EQ(500, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    EXPECT_EQ("Some reason here", reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());

    const QString answer = QString(reply->readAll()).trimmed();
    QJsonDocument doc = QJsonDocument::fromJson(answer.toUtf8());
    EXPECT_TRUE(doc.isObject());
    EXPECT_EQ(42, doc.object().value("error_code").toInt());
    QJsonArray args = doc.object().value("message_args").toArray();
    ASSERT_EQ(2, args.count());
    EXPECT_EQ("arg1", args[0].toString());
    EXPECT_EQ("arg2", args[1].toString());

    delete reply;
}

TEST_F(RestServerTest, badRequestHttpCode)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());
    QNetworkReply *reply = restClientWithoutAuthUT->get("/error/bad-request").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());
    EXPECT_EQ(400, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    EXPECT_EQ("Bad Request", reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
    delete reply;
}

TEST_F(RestServerTest, notFoundHttpCode)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());
    QNetworkReply *reply = restClientWithoutAuthUT->get("/error/not-found").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());
    EXPECT_EQ(404, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    EXPECT_EQ("Not Found", reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
    delete reply;
}

TEST_F(RestServerTest, conflictHttpCode)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());
    QNetworkReply *reply = restClientWithoutAuthUT->get("/error/conflict").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());
    EXPECT_EQ(409, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    EXPECT_EQ("Conflict", reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
    delete reply;
}

TEST_F(RestServerTest, internalErrorHttpCode)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());
    QNetworkReply *reply = restClientWithoutAuthUT->get("/error/internal-error").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());
    EXPECT_EQ(500, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    EXPECT_EQ("Internal Server Error", reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
    delete reply;
}

TEST_F(RestServerTest, notImplementedHttpCode)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());
    QNetworkReply *reply = restClientWithoutAuthUT->get("/error/not-implemented").result();
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());
    EXPECT_EQ(501, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    EXPECT_EQ("Not Implemented", reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
    delete reply;
}

#include "abstractrestserver_test.moc"
