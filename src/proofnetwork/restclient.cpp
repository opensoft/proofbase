#include "restclient.h"

#include "proofcore/proofobject_p.h"

#include <QAuthenticator>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCryptographicHash>
#include <QDateTime>
#include <QTimer>
#include <QUuid>
#include <QJsonParseError>
#include <QJsonObject>
#include <QThread>
#include <QBuffer>

static const qlonglong DEFAULT_REPLY_TIMEOUT = 60000;

namespace Proof {
class RestClientPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(RestClient)
public:
    QNetworkAccessManager *qnam;
    QString userName;
    QString password;
    QString clientName;
    QString host;
    QString quasiOAuth2Token;
    int port = 443;
    QString scheme = QString("https");
    RestClient::AuthType authType = RestClient::AuthType::WithoutAuth;
    QHash<QNetworkReply *, QTimer *> replyTimeouts;
    qlonglong msecsForTimeout = DEFAULT_REPLY_TIMEOUT;
    QHash<QByteArray, QByteArray> customHeaders;

    QNetworkRequest createNetworkRequest(const QString &method, const QUrlQuery &query, const QByteArray &body) const;
    QByteArray generateWsseToken() const;
    void requestQuasiOAuth2token(const QString &method = QString("/oauth2/token"));

    void handleReply(QNetworkReply *reply);
    void cleanupReplyHandler(QNetworkReply *reply);
};
}

using namespace Proof;

RestClient::RestClient()
    : ProofObject(*new RestClientPrivate)
{
    Q_D(RestClient);
    d->qnam = new QNetworkAccessManager(this);
    connect(d->qnam, &QNetworkAccessManager::authenticationRequired, this, &RestClient::authenticationRequired);
    connect(d->qnam, &QNetworkAccessManager::encrypted, this, &RestClient::encrypted);
    connect(d->qnam, &QNetworkAccessManager::finished, this, &RestClient::finished);
    connect(d->qnam, &QNetworkAccessManager::networkAccessibleChanged, this, &RestClient::networkAccessibleChanged);
    connect(d->qnam, &QNetworkAccessManager::proxyAuthenticationRequired, this, &RestClient::proxyAuthenticationRequired);
    connect(d->qnam, &QNetworkAccessManager::sslErrors, this, &RestClient::sslErrors);
}

QString RestClient::userName() const
{
    Q_D(const RestClient);
    return d->userName;
}


void RestClient::setUserName(const QString &arg)
{
    Q_D(RestClient);
    if (d->userName != arg) {
        d->userName = arg;
        emit userNameChanged(arg);
    }
}

QString RestClient::password() const
{
    Q_D(const RestClient);
    return d->password;
}

void RestClient::setPassword(const QString &arg)
{
    Q_D(RestClient);
    if (d->password != arg) {
        d->password = arg;
        emit passwordChanged(arg);
    }
}

QString RestClient::clientName() const
{
    Q_D(const RestClient);
    return d->clientName;
}

void RestClient::setClientName(const QString &arg)
{
    Q_D(RestClient);
    if (d->clientName != arg) {
        d->clientName = arg;
        emit clientNameChanged(arg);
    }
}

QString RestClient::host() const
{
    Q_D(const RestClient);
    return d->host;
}

void RestClient::setHost(const QString &arg)
{
    Q_D(RestClient);
    if (d->host != arg) {
        d->host = arg;
        emit hostChanged(arg);
    }
}

int RestClient::port() const
{
    Q_D(const RestClient);
    return d->port;
}

void RestClient::setPort(int arg)
{
    Q_D(RestClient);
    if (d->port != arg) {
        d->port = arg;
        emit portChanged(arg);
    }
}

QString RestClient::scheme() const
{
    Q_D(const RestClient);
    return d->scheme;
}

void RestClient::setScheme(const QString &arg)
{
    Q_D(RestClient);
    if (d->scheme != arg) {
        d->scheme = arg;
        emit schemeChanged(arg);
    }
}

RestClient::AuthType RestClient::authType() const
{
    Q_D(const RestClient);
    return d->authType;
}

void RestClient::setAuthType(AuthType arg)
{
    Q_D(RestClient);
    if (d->authType != arg) {
        d->authType = arg;
        emit authTypeChanged(arg);
    }
}

qlonglong RestClient::msecsForTimeout() const
{
    Q_D(const RestClient);
    return d->msecsForTimeout;
}

void RestClient::setMsecsForTimeout(qlonglong arg)
{
    Q_D(RestClient);
    if (d->msecsForTimeout != arg) {
        d->msecsForTimeout = arg;
        emit msecsForTimeoutChanged(arg);
    }
}

void RestClient::setCustomHeader(const QByteArray &header, const QByteArray &value)
{
    Q_D(RestClient);
    d->customHeaders[header] = value;
}

QByteArray RestClient::customHeader(const QByteArray &header) const
{
    Q_D(const RestClient);
    return d->customHeaders.value(header);
}

bool RestClient::containsCustomHeader(const QByteArray &header) const
{
    Q_D(const RestClient);
    return d->customHeaders.contains(header);
}

void RestClient::unsetCustomHeader(const QByteArray &header)
{
    Q_D(RestClient);
    d->customHeaders.remove(header);
}

QNetworkReply *RestClient::get(const QString &method, const QUrlQuery &query)
{
    Q_D(RestClient);
    qCDebug(proofNetworkMiscLog) << method << query.toString();
    QNetworkReply *reply = d->qnam->get(d->createNetworkRequest(method, query, ""));
    d->handleReply(reply);
    return reply;
}

QNetworkReply *RestClient::post(const QString &method, const QUrlQuery &query, const QByteArray &body)
{
    Q_D(RestClient);
    qCDebug(proofNetworkMiscLog) << method << query.toString() << body;
    QNetworkReply *reply = d->qnam->post(d->createNetworkRequest(method, query, body), body);
    d->handleReply(reply);
    return reply;
}

QNetworkReply *RestClient::patch(const QString &method, const QUrlQuery &query, const QByteArray &body)
{
    Q_D(RestClient);
    QBuffer *bodyBuffer = new QBuffer;
    bodyBuffer->setData(body);
    qCDebug(proofNetworkMiscLog) << method << query.toString() << body;
    QNetworkReply *reply = d->qnam->sendCustomRequest(d->createNetworkRequest(method, query, body), "PATCH", bodyBuffer);
    d->handleReply(reply);
    bodyBuffer->setParent(reply);
    return reply;
}

void RestClient::authenticate()
{
    Q_D(RestClient);
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, __func__, Qt::QueuedConnection);
    } else {
        if (authType() == RestClient::AuthType::QuasiOAuth2Auth)
            d->requestQuasiOAuth2token();
    }
}


QNetworkRequest RestClientPrivate::createNetworkRequest(const QString &method, const QUrlQuery &query, const QByteArray &body) const
{
    QNetworkRequest result;

    QUrl url;
    url.setScheme(scheme);
    url.setHost(host);
    url.setPort(port);
    url.setPath(method);
    url.setQuery(query);

    result.setUrl(url);

    if (!body.isEmpty()) {
        QJsonParseError error;
        QJsonDocument::fromJson(body, &error);

        //NOTE: We assume that if it is not json then it is xml
        if (error.error == QJsonParseError::NoError)
            result.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        else
            result.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");
    } else {
        result.setHeader(QNetworkRequest::ContentTypeHeader, "plain/text");
    }

    for (const QByteArray &header : customHeaders.keys())
        result.setRawHeader(header, customHeaders[header]);

    switch (authType) {
    case RestClient::AuthType::WsseAuth:
        result.setRawHeader("X-WSSE", generateWsseToken());
        result.setRawHeader("X-Client-Name", clientName.toLocal8Bit());
        result.setRawHeader("Authorization", "WSSE profile=\"UsernameToken\"");
        break;
    case RestClient::AuthType::BasicAuth:
        result.setRawHeader("Authorization",
                            QString("Basic %1")
                            .arg(QString(QString("%1:%2")
                                 .arg(userName)
                                 .arg(password)
                                 .toLatin1().toBase64()))
                            .toLatin1());
        break;
    case RestClient::AuthType::QuasiOAuth2Auth:
        result.setRawHeader("Authorization", QString("Bearer %1").arg(quasiOAuth2Token).toLatin1());
        break;
    case RestClient::AuthType::WithoutAuth:
        break;
    }

    return result;
}

QByteArray RestClientPrivate::generateWsseToken() const
{
    QByteArray hashedPassword;
    if (!password.isEmpty()) {
        QCryptographicHash passwordHasher(QCryptographicHash::Md5);
        passwordHasher.addData(password.toLatin1());
        hashedPassword = passwordHasher.result().toHex();
    }

    QString createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString nonce = QUuid::createUuid().toString().replace("-", "");

    QCryptographicHash hasher(QCryptographicHash::Sha1);
    hasher.addData(nonce.toLatin1());
    hasher.addData(createdAt.toLatin1());
    hasher.addData(hashedPassword);

    return QString("UsernameToken Username=\"%1\", PasswordDigest=\"%2\", Nonce=\"%3\", Created=\"%4\"")
            .arg(userName)
            .arg(QString(hasher.result().toBase64()))
            .arg(QString(nonce.toLatin1().toBase64()))
            .arg(createdAt)
            .toLatin1();
}

void RestClientPrivate::requestQuasiOAuth2token(const QString &method)
{
    Q_Q(RestClient);
    QUrl url;
    url.setScheme(scheme);
    url.setHost(host);
    url.setPort(port);
    url.setPath(method);
    QString quasiOAuth2TokenRequestData = QString("grant_type=password&username=%1&password=%2")
            .arg(userName)
            .arg(password);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    auto reply = qnam->post(request, QUrlQuery(quasiOAuth2TokenRequestData).toString().toLatin1());
    handleReply(reply);
    QObject::connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                     q, [q, reply](QNetworkReply::NetworkError) {
        emit q->authenticationErrorOccurred("Can't connect to Scissorhands service.\nPlease check your internet connection.");
    });
    QObject::connect(reply, &QNetworkReply::finished,
                     q, [this, q, reply]() {
        QJsonParseError error;
        QJsonObject response = QJsonDocument::fromJson(reply->readAll(), &error).object();

        if (error.error == QJsonParseError::NoError) {
            quasiOAuth2Token = response.value("access_token").toString();
            if (quasiOAuth2Token.isEmpty())
                emit q->authenticationErrorOccurred("Wrong Scissorhands service authentication.\nPlease check your authentication settings.");
            else
                emit q->authenticationSucceed();

        } else {
            emit q->authenticationErrorOccurred("Wrong Scissorhands service answer.\nPlease check your host settings.");
        }
    });
}

void RestClientPrivate::handleReply(QNetworkReply *reply)
{
    Q_Q(RestClient);
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    replyTimeouts.insert(reply, timer);
    QObject::connect(timer, &QTimer::timeout, [timer, reply](){
        qCDebug(proofNetworkMiscLog) << "Timed out:" << reply->request().url().path() << reply->request().url().query() << reply->isRunning();
        if (reply->isRunning())
            reply->abort();
        timer->deleteLater();
    });

    timer->start(msecsForTimeout);

    QObject::connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                     q, [this, reply](QNetworkReply::NetworkError) {
        qCDebug(proofNetworkMiscLog) << "Error occured:" << reply->request().url().path() << reply->request().url().query();
        cleanupReplyHandler(reply);
    });
    QObject::connect(reply, &QNetworkReply::finished,
                     q, [this, reply]() {
        qCDebug(proofNetworkMiscLog) << "Finished:" << reply->request().url().path() << reply->request().url().query();
        cleanupReplyHandler(reply);
    });
}

void RestClientPrivate::cleanupReplyHandler(QNetworkReply *reply)
{
    if (replyTimeouts.contains(reply)) {
        QTimer *connectionTimer = replyTimeouts.take(reply);
        connectionTimer->stop();
        connectionTimer->deleteLater();
    }
}
