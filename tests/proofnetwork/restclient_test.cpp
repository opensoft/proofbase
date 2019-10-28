// clazy:skip

#include "proofseed/asynqro_extra.h"

#include "proofnetwork/restclient.h"

#include "gtest/proof/test_global.h"

#include <QNetworkReply>
#include <QRegExp>
#include <QScopedPointer>

#include <functional>
#include <tuple>

using testing::Test;
using testing::TestWithParam;

using HttpMethodCall =
    std::function<Proof::CancelableFuture<QNetworkReply *>(Proof::RestClient &, const QByteArray & /*body*/)>;
using HttpMethodsTestParam = std::tuple<HttpMethodCall, QString /*fileOfBody*/, QString /*contentType*/>;

class RestClientTest : public TestWithParam<HttpMethodsTestParam>
{
public:
    RestClientTest() {}

protected:
    void SetUp() override
    {
        serverRunner = new FakeServerRunner();
        serverRunner->runServer();
        QTime timer;
        timer.start();
        while (!serverRunner->serverIsRunning() && timer.elapsed() < 10000)
            QThread::msleep(50);
        ASSERT_TRUE(serverRunner->serverIsRunning());

        restClient = Proof::RestClientSP::create();
        restClient->setAuthType(Proof::RestAuthType::NoAuth);
        restClient->setHost("127.0.0.1");
        restClient->setPort(9091);
        restClient->setScheme("http");
        restClient->setClientName("Proof-test");
    }

    void TearDown() override { delete serverRunner; }

protected:
    FakeServerRunner *serverRunner;
    Proof::RestClientSP restClient;
};

using namespace std::placeholders;

TEST(RestClientBasicsTest, fieldsSanity)
{
    Proof::RestClientSP restClient = Proof::RestClientSP::create();
    restClient->setHost("127.0.0.1");
    EXPECT_EQ("127.0.0.1", restClient->host());
    restClient->setPort(9091);
    EXPECT_EQ(9091, restClient->port());
    restClient->setScheme("https");
    EXPECT_EQ("https", restClient->scheme());
    restClient->setClientName("Proof-test");
    EXPECT_EQ("Proof-test", restClient->clientName());
    restClient->setAuthType(Proof::RestAuthType::Basic);
    EXPECT_EQ(Proof::RestAuthType::Basic, restClient->authType());
    restClient->setToken("some token");
    EXPECT_EQ("some token", restClient->token());
    restClient->setUserName("user");
    EXPECT_EQ("user", restClient->userName());
    restClient->setPassword("secret phrase");
    EXPECT_EQ("secret phrase", restClient->password());

    EXPECT_EQ("127.0.0.1", restClient->host());
    EXPECT_EQ(9091, restClient->port());
    EXPECT_EQ("https", restClient->scheme());
    EXPECT_EQ("Proof-test", restClient->clientName());
    EXPECT_EQ(Proof::RestAuthType::Basic, restClient->authType());
    EXPECT_EQ("some token", restClient->token());
    EXPECT_EQ("user", restClient->userName());

    restClient->setHost("127.0.0.1/api");
    EXPECT_EQ("127.0.0.1", restClient->host());
    EXPECT_EQ("/api", restClient->postfix());
    restClient->setPostfix("");
    EXPECT_EQ("", restClient->postfix());
    restClient->setPostfix("v2");
    EXPECT_EQ("v2", restClient->postfix());
}

TEST(RestClientBasicsTest, customHeadersSanity)
{
    Proof::RestClientSP restClient = Proof::RestClientSP::create();
    EXPECT_FALSE(restClient->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ(QString(), restClient->customHeader("SpecialHeader"));
    restClient->setCustomHeader("SpecialHeader", "Some Value");
    EXPECT_TRUE(restClient->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ("Some Value", restClient->customHeader("SpecialHeader"));
    restClient->unsetCustomHeader("SpecialHeader");
    EXPECT_FALSE(restClient->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ(QString(), restClient->customHeader("SpecialHeader"));
    restClient->setCustomHeader("SpecialHeader", "Some Value");
    restClient->setCustomHeader("OtherSpecialHeader", "Some Another Value");
    EXPECT_TRUE(restClient->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ("Some Value", restClient->customHeader("SpecialHeader"));
    EXPECT_TRUE(restClient->containsCustomHeader("OtherSpecialHeader"));
    EXPECT_EQ("Some Another Value", restClient->customHeader("OtherSpecialHeader"));
    restClient->unsetCustomHeader("SpecialHeader");
    EXPECT_FALSE(restClient->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ(QString(), restClient->customHeader("SpecialHeader"));
    EXPECT_TRUE(restClient->containsCustomHeader("OtherSpecialHeader"));
    EXPECT_EQ("Some Another Value", restClient->customHeader("OtherSpecialHeader"));
    restClient->unsetCustomHeader("OtherSpecialHeader");
    EXPECT_FALSE(restClient->containsCustomHeader("SpecialHeader"));
    EXPECT_EQ(QString(), restClient->customHeader("SpecialHeader"));
    EXPECT_FALSE(restClient->containsCustomHeader("OtherSpecialHeader"));
    EXPECT_EQ(QString(), restClient->customHeader("OtherSpecialHeader"));
}

TEST(RestClientBasicsTest, cookiesSanity)
{
    Proof::RestClientSP restClient = Proof::RestClientSP::create();
    EXPECT_FALSE(restClient->containsCookie("cookie1"));
    EXPECT_EQ(QString(), restClient->cookie("cookie1").value());
    restClient->setCookie(QNetworkCookie("cookie1", "value1"));
    EXPECT_TRUE(restClient->containsCookie("cookie1"));
    EXPECT_EQ("value1", restClient->cookie("cookie1").value());
    restClient->unsetCookie("cookie1");
    EXPECT_FALSE(restClient->containsCookie("cookie1"));
    EXPECT_EQ(QString(), restClient->cookie("cookie1").value());
    restClient->setCookie(QNetworkCookie("cookie1", "value1"));
    restClient->setCookie(QNetworkCookie("cookie2", "value2"));
    EXPECT_TRUE(restClient->containsCookie("cookie1"));
    EXPECT_EQ("value1", restClient->cookie("cookie1").value());
    EXPECT_TRUE(restClient->containsCookie("cookie2"));
    EXPECT_EQ("value2", restClient->cookie("cookie2").value());
    restClient->unsetCookie("cookie1");
    EXPECT_FALSE(restClient->containsCookie("cookie1"));
    EXPECT_EQ(QString(), restClient->cookie("cookie1").value());
    EXPECT_TRUE(restClient->containsCookie("cookie2"));
    EXPECT_EQ("value2", restClient->cookie("cookie2").value());
    restClient->unsetCookie("cookie2");
    EXPECT_FALSE(restClient->containsCookie("cookie1"));
    EXPECT_EQ(QString(), restClient->cookie("cookie1").value());
    EXPECT_FALSE(restClient->containsCookie("cookie2"));
    EXPECT_EQ(QString(), restClient->cookie("cookie2").value());
}

INSTANTIATE_TEST_CASE_P(
    RestClientTestInstance, RestClientTest,
    testing::Values(
        // Without vendor, without body
        HttpMethodsTestParam(std::bind(static_cast<Proof::CancelableFuture<QNetworkReply *> (Proof::RestClient::*)(
                                           const QString &, const QUrlQuery &, const QString &)>(&Proof::RestClient::get),
                                       _1, QStringLiteral("/"), QUrlQuery(), QString()),
                             "", "text/plain"),
        HttpMethodsTestParam(
            std::bind(static_cast<Proof::CancelableFuture<QNetworkReply *> (Proof::RestClient::*)(const QUrl &, int)>(
                          &Proof::RestClient::get),
                      _1, QUrl("http://127.0.0.1:9091/"), -1),
            "", "text/plain"),
        HttpMethodsTestParam(
            std::bind(static_cast<Proof::CancelableFuture<QNetworkReply *> (Proof::RestClient::*)(const QUrl &, int)>(
                          &Proof::RestClient::get),
                      _1, QUrl("http://127.0.0.1:9091/"), 10000),
            "", "text/plain"),
        HttpMethodsTestParam(std::bind(static_cast<Proof::CancelableFuture<QNetworkReply *> (Proof::RestClient::*)(
                                           const QString &, const QUrlQuery &, const QByteArray &, const QString &)>(
                                           &Proof::RestClient::post),
                                       _1, "/", QUrlQuery(), _2, QString()),
                             "", "text/plain"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::put, _1, "/", QUrlQuery(), _2, QString()), "", "text/plain"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::patch, _1, "/", QUrlQuery(), _2, QString()), "", "text/plain"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::deleteResource, _1, "/", QUrlQuery(), QString()), "",
                             "text/plain"),
        // With vendor, without body
        HttpMethodsTestParam(std::bind(static_cast<Proof::CancelableFuture<QNetworkReply *> (Proof::RestClient::*)(
                                           const QString &, const QUrlQuery &, const QString &)>(&Proof::RestClient::get),
                                       _1, QStringLiteral("/"), QUrlQuery(), "opensoft"),
                             "", "application/vnd.opensoft"),
        HttpMethodsTestParam(std::bind(static_cast<Proof::CancelableFuture<QNetworkReply *> (Proof::RestClient::*)(
                                           const QString &, const QUrlQuery &, const QByteArray &, const QString &)>(
                                           &Proof::RestClient::post),
                                       _1, "/", QUrlQuery(), _2, "opensoft"),
                             "", "application/vnd.opensoft"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::put, _1, "/", QUrlQuery(), _2, "opensoft"), "",
                             "application/vnd.opensoft"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::patch, _1, "/", QUrlQuery(), _2, "opensoft"), "",
                             "application/vnd.opensoft"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::deleteResource, _1, "/", QUrlQuery(), "opensoft"), "",
                             "application/vnd.opensoft"),
        // Without vendor, with json body
        HttpMethodsTestParam(std::bind(static_cast<Proof::CancelableFuture<QNetworkReply *> (Proof::RestClient::*)(
                                           const QString &, const QUrlQuery &, const QByteArray &, const QString &)>(
                                           &Proof::RestClient::post),
                                       _1, "/", QUrlQuery(), _2, QString()),
                             ":/data/vendor_test_body.json", "application/json"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::put, _1, "/", QUrlQuery(), _2, QString()),
                             ":/data/vendor_test_body.json", "application/json"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::patch, _1, "/", QUrlQuery(), _2, QString()),
                             ":/data/vendor_test_body.json", "application/json"),
        // Without vendor, with xml body
        HttpMethodsTestParam(std::bind(static_cast<Proof::CancelableFuture<QNetworkReply *> (Proof::RestClient::*)(
                                           const QString &, const QUrlQuery &, const QByteArray &, const QString &)>(
                                           &Proof::RestClient::post),
                                       _1, "/", QUrlQuery(), _2, QString()),
                             ":/data/vendor_test_body.xml", "text/xml"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::put, _1, "/", QUrlQuery(), _2, QString()),
                             ":/data/vendor_test_body.xml", "text/xml"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::patch, _1, "/", QUrlQuery(), _2, QString()),
                             ":/data/vendor_test_body.xml", "text/xml"),
        // With vendor, with json body
        HttpMethodsTestParam(std::bind(static_cast<Proof::CancelableFuture<QNetworkReply *> (Proof::RestClient::*)(
                                           const QString &, const QUrlQuery &, const QByteArray &, const QString &)>(
                                           &Proof::RestClient::post),
                                       _1, "/", QUrlQuery(), _2, "opensoft"),
                             ":/data/vendor_test_body.json", "application/vnd.opensoft+json"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::put, _1, "/", QUrlQuery(), _2, "opensoft"),
                             ":/data/vendor_test_body.json", "application/vnd.opensoft+json"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::patch, _1, "/", QUrlQuery(), _2, "opensoft"),
                             ":/data/vendor_test_body.json", "application/vnd.opensoft+json"),
        // With vendor, with xml body
        HttpMethodsTestParam(std::bind(static_cast<Proof::CancelableFuture<QNetworkReply *> (Proof::RestClient::*)(
                                           const QString &, const QUrlQuery &, const QByteArray &, const QString &)>(
                                           &Proof::RestClient::post),
                                       _1, "/", QUrlQuery(), _2, "opensoft"),
                             ":/data/vendor_test_body.xml", "application/vnd.opensoft+xml"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::put, _1, "/", QUrlQuery(), _2, "opensoft"),
                             ":/data/vendor_test_body.xml", "application/vnd.opensoft+xml"),
        HttpMethodsTestParam(std::bind(&Proof::RestClient::patch, _1, "/", QUrlQuery(), _2, "opensoft"),
                             ":/data/vendor_test_body.xml", "application/vnd.opensoft+xml")));

TEST_P(RestClientTest, vendorTest)
{
    const auto methodCall = std::get<0>(GetParam());
    const auto file = std::get<1>(GetParam());
    const auto expected = std::get<2>(GetParam());
    const QRegExp expectedRegExp(QString("Content-Type:(\\s*)([^\r\n]*)\\r\\n"));

    QByteArray body;
    if (!file.isEmpty()) {
        body = dataFromFile(file);
        ASSERT_FALSE(body.isEmpty());
    }

    QScopedPointer<QNetworkReply> reply(methodCall(*restClient, body).result());

    ASSERT_NE(nullptr, reply);

    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 10000)
        QThread::msleep(5);
    ASSERT_TRUE(reply->isFinished());

    const auto query = QString::fromLatin1(serverRunner->lastQueryRaw());
    const int position = expectedRegExp.indexIn(query);

    ASSERT_NE(-1, position);
    EXPECT_EQ(expected, expectedRegExp.cap(2));
}
