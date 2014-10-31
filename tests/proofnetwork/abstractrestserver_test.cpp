#include "gtest/test_global.h"

#include "proofnetwork/abstractrestserver.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QSignalSpy>
#include <QNetworkReply>

#include <tuple>

using testing::Test;
using testing::TestWithParam;

class TestRestServer : public Proof::AbstractRestServer
{
    Q_OBJECT
public:
    TestRestServer() :
        Proof::AbstractRestServer("username", "password", "api", 9091) {}

public slots:
    void get_TestMethod(QTcpSocket *socket, const QStringList &headers, const QByteArray &body)
    {
        Q_UNUSED(headers)
        Q_UNUSED(body)
        sendAnswer(socket, __func__, "plain/text");
    }

    void get_Testmethod(QTcpSocket *socket, const QStringList &headers, const QByteArray &body)
    {
        Q_UNUSED(headers)
        Q_UNUSED(body)
        sendAnswer(socket, __func__, "plain/text");
    }

    void get_TestMethod_SubMethod(QTcpSocket *socket, const QStringList &headers, const QByteArray &body)
    {
        Q_UNUSED(headers)
        Q_UNUSED(body)
        sendAnswer(socket, __func__, "plain/text");
    }

    void post_TestMethod(QTcpSocket *socket, const QStringList &headers, const QByteArray &body)
    {
        Q_UNUSED(headers)
        Q_UNUSED(body)
        sendAnswer(socket, __func__, "plain/text");
    }
};

class RestServerMethodsTest: public TestWithParam<std::tuple<QString, QString, int, bool>>
{
public:
    RestServerMethodsTest()
    {
    }

    static void SetUpTestCase()
    {
        restServerUT = new TestRestServer();
        restServerUT->startListen();
    }

    static void TearDownTestCase()
    {
        restServerUT->close();
        delete restServerUT;
    }

protected:
    void SetUp() override
    {
        restClientUT = Proof::RestClientSP::create();
        restClientUT->setAuthType(Proof::RestClient::AuthType::BasicAuth);
        restClientUT->setUserName("username");
        restClientUT->setPassword("password");
        restClientUT->setHost("127.0.0.1");
        restClientUT->setPort(9091);
        restClientUT->setScheme("http");
        restClientUT->setClientName("Proof-test");
    }

protected:
    Proof::RestClientSP restClientUT;
    static TestRestServer *restServerUT;

};

TestRestServer *RestServerMethodsTest::restServerUT = nullptr;

TEST_P(RestServerMethodsTest, methodsNames)
{
    QString method = std::get<0>(GetParam());
    QString serverMethodName = std::get<1>(GetParam());
    int resultCode = std::get<2>(GetParam());
    bool isPost = std::get<3>(GetParam());

    ASSERT_TRUE(restServerUT->isListening());

    QNetworkReply *reply = isPost ? restClientUT->post(method): restClientUT->get(method);

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
                        testing::Values(std::tuple<QString, QString, int, bool>("/test-method", "get_TestMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-met-hod", "",  404, false),
                                        std::tuple<QString, QString, int, bool>("/tEst-meThoD", "get_TestMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/testmethod", "get_Testmethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method/sub-method", "get_TestMethod_SubMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method/sub-method/subsub", "get_TestMethod_SubMethod", 200, false),
                                        std::tuple<QString, QString, int, bool>("/test-method",  "post_TestMethod", 200, true),
                                        std::tuple<QString, QString, int, bool>("/wrong-method", "", 404, false),
                                        std::tuple<QString, QString, int, bool>("/wrong-method", "", 404, true)));

#include "abstractrestserver_test.moc"
