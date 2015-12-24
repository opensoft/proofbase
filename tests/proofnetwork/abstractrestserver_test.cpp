#include "gtest/test_global.h"

#include "proofnetwork/abstractrestserver.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QSignalSpy>
#include <QNetworkReply>
#include <QTest>

#include <tuple>

using testing::Test;
using testing::TestWithParam;

class TestRestServer : public Proof::AbstractRestServer
{
    Q_OBJECT
public:
    TestRestServer(const QString &pathPrefix = QString(), int port = 9091)
        : Proof::AbstractRestServer("username", "password", pathPrefix, port) {}

public slots:
    void rest_get_TestMethod(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                             const QUrlQuery &queryParams, const QByteArray &body)
    {
        Q_UNUSED(headers)
        Q_UNUSED(body)
        sendAnswer(socket, __func__, "text/plain", 200, methodVariableParts.join('/') + "|" + queryParams.toString());
    }

    void rest_get_Testmethod(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                             const QUrlQuery &queryParams, const QByteArray &body)
    {
        Q_UNUSED(headers)
        Q_UNUSED(methodVariableParts)
        Q_UNUSED(queryParams)
        Q_UNUSED(body)
        sendAnswer(socket, __func__, "text/plain");
    }

    void rest_get_TestMethod_SubMethod(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                                       const QUrlQuery &queryParams, const QByteArray &body)
    {
        Q_UNUSED(headers)
        Q_UNUSED(methodVariableParts)
        Q_UNUSED(queryParams)
        Q_UNUSED(body)
        sendAnswer(socket, __func__, "text/plain");
    }

    void rest_post_TestMethod(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                              const QUrlQuery &queryParams, const QByteArray &body)
    {
        Q_UNUSED(headers)
        Q_UNUSED(body)
        sendAnswer(socket, __func__, "text/plain", 200, methodVariableParts.join('/') + "|" + queryParams.toString());
    }
};

class TestRestServerWithoutAuth : public Proof::AbstractRestServer
{
    Q_OBJECT
public:
    TestRestServerWithoutAuth()
        : Proof::AbstractRestServer("", 9092, Proof::RestAuthType::NoAuth) {}

public slots:
    void rest_get_TestMethod(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                             const QUrlQuery &queryParams, const QByteArray &body)
    {
        Q_UNUSED(headers)
        Q_UNUSED(methodVariableParts)
        Q_UNUSED(queryParams)
        Q_UNUSED(body)
        sendAnswer(socket, __func__, "text/plain");
    }
};

class TestRestServerWithPathPrefix : public TestRestServer
{
    Q_OBJECT
public:
    TestRestServerWithPathPrefix()
        : TestRestServer("/api", 9093) {}
};

class RestServerMethodsTest : public TestWithParam<std::tuple<QString, QString, int, bool>>
{
public:
    RestServerMethodsTest()
    {
    }

    static void SetUpTestCase()
    {
        restServerUT = new TestRestServer();
        restServerUT->startListen();
        QTest::qWait(300);

        restServerWithoutAuthUT = new TestRestServerWithoutAuth();
        restServerWithoutAuthUT->startListen();
        QTest::qWait(300);

        restServerWithPathPrefixUT = new TestRestServerWithPathPrefix();
        restServerWithPathPrefixUT->startListen();
        QTest::qWait(300);
    }

    static void TearDownTestCase()
    {
        delete restServerWithPathPrefixUT;
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

        restClientWithoutAuthUT = Proof::RestClientSP::create();
        restClientWithoutAuthUT->setAuthType(Proof::RestAuthType::NoAuth);
        restClientWithoutAuthUT->setHost("127.0.0.1");
        restClientWithoutAuthUT->setPort(9092);
        restClientWithoutAuthUT->setScheme("http");
        restClientWithoutAuthUT->setClientName("Proof-test");

        restClientWithPrefixUT = Proof::RestClientSP::create();
        restClientWithPrefixUT->setAuthType(Proof::RestAuthType::Basic);
        restClientWithPrefixUT->setUserName("username");
        restClientWithPrefixUT->setPassword("password");
        restClientWithPrefixUT->setHost("127.0.0.1");
        restClientWithPrefixUT->setPort(9093);
        restClientWithPrefixUT->setScheme("http");
        restClientWithPrefixUT->setClientName("Proof-test");
    }

protected:
    Proof::RestClientSP restClientUT;
    static TestRestServer *restServerUT;
    Proof::RestClientSP restClientWithoutAuthUT;
    static TestRestServerWithoutAuth *restServerWithoutAuthUT;
    Proof::RestClientSP restClientWithPrefixUT;
    static TestRestServerWithPathPrefix *restServerWithPathPrefixUT;

};

TestRestServer *RestServerMethodsTest::restServerUT = nullptr;
TestRestServerWithoutAuth *RestServerMethodsTest::restServerWithoutAuthUT = nullptr;
TestRestServerWithPathPrefix *RestServerMethodsTest::restServerWithPathPrefixUT = nullptr;

class PathPrefixServerMethodsTest : public RestServerMethodsTest
{
};

class AnotherRestServerMethodsTest : public RestServerMethodsTest
{
};

class SomeMoreRestServerMethodsTest : public RestServerMethodsTest
{
};

TEST_F(RestServerMethodsTest, withoutAuth)
{
    ASSERT_TRUE(restServerWithoutAuthUT->isListening());

    QNetworkReply *reply = restClientWithoutAuthUT->get("/test-method");

    QSignalSpy spy(reply, SIGNAL(finished()));

    ASSERT_TRUE(spy.wait());
    EXPECT_EQ(1, spy.count());

    EXPECT_EQ(200, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    const QString answer = QString(reply->readAll()).trimmed();
    EXPECT_EQ("rest_get_TestMethod", answer);

    delete reply;
}

TEST_P(RestServerMethodsTest, methodsNames)
{
    QString method = std::get<0>(GetParam());
    QString serverMethodName = std::get<1>(GetParam());
    int resultCode = std::get<2>(GetParam());
    bool isPost = std::get<3>(GetParam());

    ASSERT_TRUE(restServerUT->isListening());

    QNetworkReply *reply = isPost ? restClientUT->post(method) : restClientUT->get(method);

    QSignalSpy spy(reply, SIGNAL(finished()));

    ASSERT_TRUE(spy.wait());
    EXPECT_EQ(1, spy.count());

    EXPECT_EQ(resultCode, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    if (resultCode == 200) {
        QString methodName = QString(reply->readAll()).trimmed();
        EXPECT_EQ(methodName, serverMethodName);
    }
    delete reply;
}

INSTANTIATE_TEST_CASE_P(RestServerMethodsTestInstance,
                        RestServerMethodsTest,
                        testing::Values(std::tuple<QString, QString, int, bool>("/test-method", "rest_get_TestMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-met-hod", "",  404, false),
                                        std::tuple<QString, QString, int, bool>("/tEst-meThoD", "rest_get_TestMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/testmethod", "rest_get_Testmethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method/sub-method", "rest_get_TestMethod_SubMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method/sub-method/subsub", "rest_get_TestMethod_SubMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method",  "rest_post_TestMethod", 200, true),
                                        std::tuple<QString, QString, int, bool>("/wrong-method", "", 404, false),
                                        std::tuple<QString, QString, int, bool>("/wrong-method", "", 404, true)));

TEST_P(PathPrefixServerMethodsTest, methodsNames)
{
    QString method = QString("%1/%2").arg(restServerWithPathPrefixUT->pathPrefix(), std::get<0>(GetParam()));
    QString serverMethodName = std::get<1>(GetParam());
    int resultCode = std::get<2>(GetParam());
    bool isPost = std::get<3>(GetParam());

    ASSERT_TRUE(restServerWithPathPrefixUT->isListening());

    QNetworkReply *reply = isPost ? restClientWithPrefixUT->post(method) : restClientWithPrefixUT->get(method);

    QSignalSpy spy(reply, SIGNAL(finished()));

    ASSERT_TRUE(spy.wait());
    EXPECT_EQ(1, spy.count());

    EXPECT_EQ(resultCode, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    if (resultCode == 200) {
        QString methodName = QString(reply->readAll()).trimmed();
        EXPECT_EQ(methodName, serverMethodName);
    }
    delete reply;
}

INSTANTIATE_TEST_CASE_P(PathPrefixServerMethodsTestInstance,
                        PathPrefixServerMethodsTest,
                        testing::Values(std::tuple<QString, QString, int, bool>("/test-method", "rest_get_TestMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-met-hod", "",  404, false),
                                        std::tuple<QString, QString, int, bool>("/tEst-meThoD", "rest_get_TestMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/testmethod", "rest_get_Testmethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method/sub-method", "rest_get_TestMethod_SubMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method/sub-method/subsub", "rest_get_TestMethod_SubMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method",  "rest_post_TestMethod", 200, true),
                                        std::tuple<QString, QString, int, bool>("/wrong-method", "", 404, false),
                                        std::tuple<QString, QString, int, bool>("/wrong-method", "", 404, true)));

TEST_P(AnotherRestServerMethodsTest, methodsParams)
{
    QString method = std::get<0>(GetParam());
    QString params = std::get<1>(GetParam());
    int resultCode = std::get<2>(GetParam());
    bool isPost = std::get<3>(GetParam());

    ASSERT_TRUE(restServerUT->isListening());

    QUrlQuery query(params);

    QNetworkReply *reply = isPost ? restClientUT->post(method, query): restClientUT->get(method, query);

    QSignalSpy spy(reply, SIGNAL(finished()));

    ASSERT_TRUE(spy.wait());
    EXPECT_EQ(1, spy.count());

    EXPECT_EQ(resultCode, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    QStringList reasonResult = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString().trimmed().split('|');

    ASSERT_EQ(2, reasonResult.count());

    EXPECT_TRUE(method.endsWith(reasonResult.at(0)));
    EXPECT_EQ(query, QUrlQuery(reasonResult.at(1)));

    delete reply;
}

INSTANTIATE_TEST_CASE_P(AnotherRestServerMethodsTestInstance,
                        AnotherRestServerMethodsTest,
                        testing::Values(std::tuple<QString, QString, int, bool>("/test-method/123", "", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method", "param=123&another_param=true", 200, true),
                                        std::tuple<QString, QString, int, bool>("/test-method/", "param=hello&another_param=true", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method/123/sub-method", "param=321&some_param=false", 200, true)
                                        ));

TEST_P(SomeMoreRestServerMethodsTest, methodsVariableParts)
{
    QString method = std::get<0>(GetParam());
    QString variablePart = std::get<1>(GetParam());
    int resultCode = std::get<2>(GetParam());
    bool isPost = std::get<3>(GetParam());

    ASSERT_TRUE(restServerUT->isListening());

    QNetworkReply *reply = isPost ? restClientUT->post(method): restClientUT->get(method);

    QSignalSpy spy(reply, SIGNAL(finished()));

    ASSERT_TRUE(spy.wait());
    EXPECT_EQ(1, spy.count());

    EXPECT_EQ(resultCode, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    QStringList reasonResult = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString().trimmed().split('|');

    ASSERT_EQ(2, reasonResult.count());

    EXPECT_EQ(variablePart, reasonResult.at(0));

    delete reply;
}

INSTANTIATE_TEST_CASE_P(SomeMoreRestServerMethodsTestInstance,
                        SomeMoreRestServerMethodsTest,
                        testing::Values(std::tuple<QString, QString, int, bool>("/test-method/123", "123", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method", "", 200, true),
                                        std::tuple<QString, QString, int, bool>("/test-method/", "", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method////", "", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method/\\`123", "%5C%60123", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method/123/sub-method", "123/sub-method", 200, true),
                                        std::tuple<QString, QString, int, bool>("/test-method/CaSetEST", "CaSetEST", 200, false)
                                        ));

#include "abstractrestserver_test.moc"
