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

static const qlonglong DEFAULT_REPLY_TIMEOUT = 30000;

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
    int port = 443;
    QString scheme = QString("https");
    RestClient::AuthType authType = RestClient::AuthType::WithoutAuth;
    QHash<QNetworkReply *, QTimer *> replyTimeouts;
    qlonglong msecsForTimeout = DEFAULT_REPLY_TIMEOUT;

    QNetworkRequest createNetworkRequest(const QString &method, const QUrlQuery &query, const QByteArray &body) const;
    QByteArray generateWsseToken() const;
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

QNetworkReply *RestClient::get(const QString &method, const QUrlQuery &query)
{
    Q_D(RestClient);
    QNetworkReply *reply = d->qnam->get(d->createNetworkRequest(method, query, ""));
    d->handleReply(reply);
    return reply;
}

QNetworkReply *RestClient::post(const QString &method, const QUrlQuery &query, const QByteArray &body)
{
    Q_D(RestClient);
    QNetworkReply *reply = d->qnam->post(d->createNetworkRequest(method, query, body), body);
    d->handleReply(reply);
    return reply;
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

        //TODO: parse xml
        if (error.error == QJsonParseError::NoError)
            result.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        else
            result.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");
    }

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
    case RestClient::AuthType::WithoutAuth:
        break;
    }

    return result;
}

QByteArray RestClientPrivate::generateWsseToken() const
{
    QCryptographicHash passwordHasher(QCryptographicHash::Md5);
    passwordHasher.addData(password.toLatin1());

    QString createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString nonce = QUuid::createUuid().toString().replace("-", "");

    QCryptographicHash hasher(QCryptographicHash::Sha1);
    hasher.addData(nonce.toLatin1());
    hasher.addData(createdAt.toLatin1());
    hasher.addData(passwordHasher.result().toHex());

    return QString("UsernameToken Username=\"%1\", PasswordDigest=\"%2\", Nonce=\"%3\", Created=\"%4\"")
            .arg(userName)
            .arg(QString(hasher.result().toBase64()))
            .arg(QString(nonce.toLatin1().toBase64()))
            .arg(createdAt)
            .toLatin1();
}

void RestClientPrivate::handleReply(QNetworkReply *reply)
{
    Q_Q(RestClient);
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    replyTimeouts.insert(reply, timer);
    QObject::connect(timer, &QTimer::timeout, [timer, reply](){
        if (reply->isRunning())
            reply->abort();
        timer->deleteLater();
    });

    timer->start(msecsForTimeout);

    QObject::connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                     q, [this, reply](QNetworkReply::NetworkError) {cleanupReplyHandler(reply);});
    QObject::connect(reply, &QNetworkReply::finished,
                     q, [this, reply]() {cleanupReplyHandler(reply);});
}

void RestClientPrivate::cleanupReplyHandler(QNetworkReply *reply)
{
    if (replyTimeouts.contains(reply)) {
        QTimer *connectionTimer = replyTimeouts.take(reply);
        connectionTimer->stop();
        connectionTimer->deleteLater();
    }
}
