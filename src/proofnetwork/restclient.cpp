#include "restclient.h"

#include "proofcore/proofglobal.h"
#include "proofcore/proofobject_p.h"
#include "proofcore/taskchain.h"
#include "proofcore/coreapplication.h"

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
#include <QHttpMultiPart>
#include <QNetworkInterface>

static const int DEFAULT_REPLY_TIMEOUT = 5 * 60 * 1000; //5 minutes

namespace Proof {
class NetworkScheduler
{
public:
    NetworkScheduler()
    {
        qnam = new QNetworkAccessManager;
        qnamThread = new QThread();
        qnamThread->start();
        qnam->moveToThread(qnamThread);
    }
    ~NetworkScheduler()
    {
        qnam->deleteLater();
        qnamThread->quit();
        qnamThread->wait(250);
        delete qnamThread;
    }

    static NetworkScheduler *instance()
    {
        static NetworkScheduler i;
        return &i;
    }

    CancelableFuture<QNetworkReply *> addRequest(const QString &host, std::function<QNetworkReply *(QNetworkAccessManager*)> &&request);

    QNetworkAccessManager *qnam = nullptr;
    QThread *qnamThread = nullptr;
private:
    using RequestDescriptor = std::pair<QString, std::function<void()>>;

    void schedule();
    void decreaseUsage(const QString &host);

    std::list<RequestDescriptor> requests;
    QHash<QString, int> usages;
    const int limit = 6;
    SpinLock requestsLock;
};

class RestClientPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(RestClient)
public:
    QUrl createUrl(QString method, const QUrlQuery &query) const;
    QNetworkRequest createNetworkRequest(const QUrl &url, const QByteArray &body, const QString &vendor);
    QByteArray generateWsseToken() const;

    void handleReply(QNetworkReply *reply);
    void cleanupReplyHandler(QNetworkReply *reply);
    void cleanupAll();
    QPair<QString, QString> parseHost(const QString &host);

    bool ignoreSslErrors = false;
    bool followRedirects = true;
    bool explicitPort = false;
    int port = 443;
    RestAuthType authType = RestAuthType::NoAuth;
    int msecsForTimeout = DEFAULT_REPLY_TIMEOUT;
    QString userName;
    QString password;
    QString clientName;
    QString host;
    QString postfix;
    QString token;
    QString scheme = QStringLiteral("https");
    QHash<QNetworkReply *, QTimer *> replyTimeouts;
    QHash<QByteArray, QByteArray> customHeaders;
    QHash<QString, QNetworkCookie> cookies;
};

}

using namespace Proof;

RestClient::RestClient(bool ignoreSslErrors)
    : ProofObject(*new RestClientPrivate)
{
    Q_D(RestClient);
    moveToThread(NetworkScheduler::instance()->qnamThread);
    d->ignoreSslErrors = ignoreSslErrors;
    connect(NetworkScheduler::instance()->qnam, &QNetworkAccessManager::finished, this, &RestClient::finished);
    if (!ignoreSslErrors) {
        connect(NetworkScheduler::instance()->qnam, &QNetworkAccessManager::sslErrors,
                this, [this, d](QNetworkReply *reply, const QList<QSslError> &errors) {
            d->cleanupReplyHandler(reply);
            emit sslErrors(reply, errors);
        });
    }
}

RestClient::~RestClient()
{
    Q_D(RestClient);
    d->cleanupAll();
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
    QPair<QString, QString> result = d->parseHost(arg);
    if (d->host != result.first) {
        d->host = result.first;
        emit hostChanged(result.first);
    }
    setPostfix(result.second);
}

QString RestClient::postfix() const
{
    Q_D(const RestClient);
    return d->postfix;
}

void RestClient::setPostfix(const QString &arg)
{
    Q_D(RestClient);
    if (d->postfix != arg) {
        d->postfix = arg;
        emit postfixChanged(arg);
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
    d->explicitPort = true;
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

QString RestClient::token() const
{
    Q_D(const RestClient);
    return d->token;
}

void RestClient::setToken(const QString &arg)
{
    Q_D(RestClient);
    if (d->token != arg) {
        d->token = arg;
        emit tokenChanged(arg);
    }
}

RestAuthType RestClient::authType() const
{
    Q_D(const RestClient);
    return d->authType;
}

void RestClient::setAuthType(RestAuthType arg)
{
    Q_D(RestClient);
    if (d->authType != arg) {
        d->authType = arg;
        emit authTypeChanged(arg);
    }
}

int RestClient::msecsForTimeout() const
{
    Q_D(const RestClient);
    return d->msecsForTimeout;
}

void RestClient::setMsecsForTimeout(int arg)
{
    Q_D(RestClient);
    if (d->msecsForTimeout != arg) {
        d->msecsForTimeout = arg;
        emit msecsForTimeoutChanged(arg);
    }
}

bool RestClient::followRedirects() const
{
    Q_D(const RestClient);
    return d->followRedirects;
}

void RestClient::setFollowRedirects(bool arg)
{
    Q_D(RestClient);
    if (d->followRedirects != arg) {
        d->followRedirects = arg;
        emit followRedirectsChanged(arg);
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

void RestClient::setCookie(const QNetworkCookie &cookie)
{
    Q_D(RestClient);
    d->cookies[cookie.name()] = cookie;
}

QNetworkCookie RestClient::cookie(const QString &name) const
{
    Q_D(const RestClient);
    return d->cookies.value(name);
}

bool RestClient::containsCookie(const QString &name) const
{
    Q_D(const RestClient);
    return d->cookies.contains(name);
}

void RestClient::unsetCookie(const QString &name)
{
    Q_D(RestClient);
    d->cookies.remove(name);
}

CancelableFuture<QNetworkReply *> RestClient::get(const QString &method, const QUrlQuery &query, const QString &vendor)
{
    Q_D(RestClient);
    qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces);

    return NetworkScheduler::instance()->addRequest(d->host, [d, method, query, vendor](QNetworkAccessManager *qnam) {
        qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces) << "started";
        QNetworkReply *reply = qnam->get(d->createNetworkRequest(d->createUrl(method, query), "", vendor));
        d->handleReply(reply);
        return reply;
    });
}

CancelableFuture<QNetworkReply *> RestClient::post(const QString &method, const QUrlQuery &query, const QByteArray &body, const QString &vendor)
{
    Q_D(RestClient);
    qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces);

    return NetworkScheduler::instance()->addRequest(d->host, [d, method, query, body, vendor](QNetworkAccessManager *qnam) {
        qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces) << "started";
        QNetworkReply *reply = qnam->post(d->createNetworkRequest(d->createUrl(method, query), body, vendor), body);
        d->handleReply(reply);
        return reply;
    });
}

CancelableFuture<QNetworkReply *> RestClient::post(const QString &method, const QUrlQuery &query, QHttpMultiPart *multiParts)
{
    Q_D(RestClient);
    qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces);

    return NetworkScheduler::instance()->addRequest(d->host, [d, method, query, multiParts](QNetworkAccessManager *qnam) {
        qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces) << "started";
        QNetworkRequest request = d->createNetworkRequest(d->createUrl(method, query), "", QLatin1String(""));
        request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                          QStringLiteral("multipart/form-data; boundary=%1").arg(QString(multiParts->boundary())));
        QNetworkReply *reply = qnam->post(request, multiParts);
        qCDebug(proofNetworkMiscLog) << request.header(QNetworkRequest::KnownHeaders::ContentTypeHeader).toString();
        multiParts->setParent(reply);
        d->handleReply(reply);
        return reply;
    });
}

CancelableFuture<QNetworkReply *> RestClient::put(const QString &method, const QUrlQuery &query, const QByteArray &body, const QString &vendor)
{
    Q_D(RestClient);
    qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces);

    return NetworkScheduler::instance()->addRequest(d->host, [d, method, query, body, vendor](QNetworkAccessManager *qnam) {
        qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces) << "started";
        QNetworkReply *reply = qnam->put(d->createNetworkRequest(d->createUrl(method, query), body, vendor), body);
        d->handleReply(reply);
        return reply;
    });
}

CancelableFuture<QNetworkReply *> RestClient::patch(const QString &method, const QUrlQuery &query, const QByteArray &body, const QString &vendor)
{
    Q_D(RestClient);
    qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces);

    return NetworkScheduler::instance()->addRequest(d->host, [d, method, query, body, vendor](QNetworkAccessManager *qnam) {
        qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces) << "started";
        QBuffer *bodyBuffer = new QBuffer;
        bodyBuffer->setData(body);
        QNetworkReply *reply = qnam->sendCustomRequest(d->createNetworkRequest(d->createUrl(method, query), body, vendor), "PATCH", bodyBuffer);
        d->handleReply(reply);
        bodyBuffer->setParent(reply);
        return reply;
    });
}

CancelableFuture<QNetworkReply *> RestClient::deleteResource(const QString &method, const QUrlQuery &query, const QString &vendor)
{
    Q_D(RestClient);
    qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces);

    return NetworkScheduler::instance()->addRequest(d->host, [d, method, query, vendor](QNetworkAccessManager *qnam) {
        qCDebug(proofNetworkMiscLog) << method << query.toString(QUrl::EncodeSpaces) << "started";
        QNetworkReply *reply = qnam->deleteResource(d->createNetworkRequest(d->createUrl(method, query), QByteArray(), vendor));
        d->handleReply(reply);
        return reply;
    });
}

CancelableFuture<QNetworkReply *> RestClient::get(const QUrl &url)
{
    Q_D(RestClient);
    qCDebug(proofNetworkMiscLog) << url;

    return NetworkScheduler::instance()->addRequest(url.host(), [d, url](QNetworkAccessManager *qnam) {
        qCDebug(proofNetworkMiscLog) << url << "started";
        QNetworkReply *reply = qnam->get(d->createNetworkRequest(url, QByteArray(), QStringLiteral()));
        d->handleReply(reply);
        return reply;
    });
}

QUrl RestClientPrivate::createUrl(QString method, const QUrlQuery &query) const
{
    QUrl url;
    url.setScheme(scheme);
    url.setHost(host);
    if (explicitPort)
        url.setPort(port);
    if (!method.startsWith('/'))
        method.prepend('/');
    url.setPath(postfix + method);
    url.setQuery(query);
    return url;
}

QNetworkRequest RestClientPrivate::createNetworkRequest(const QUrl &url, const QByteArray &body, const QString &vendor)
{
    QNetworkRequest result(url);
    result.setAttribute(QNetworkRequest::FollowRedirectsAttribute, followRedirects);

    if (!body.isEmpty()) {
        QJsonParseError error;
        QJsonDocument::fromJson(body, &error);

        QString contentTypePattern = vendor.isEmpty() ? QStringLiteral("application/%1") : QStringLiteral("application/vnd.%1+%2").arg(vendor);

        //We assume that if it is not json and not xml it's url encoded data
        if (error.error == QJsonParseError::NoError)
            result.setHeader(QNetworkRequest::ContentTypeHeader, contentTypePattern.arg(QStringLiteral("json")));
        else if (body.startsWith("<?xml"))
            result.setHeader(QNetworkRequest::ContentTypeHeader, vendor.isEmpty() ? QStringLiteral("text/xml") : contentTypePattern.arg(QStringLiteral("xml")));
        else
            result.setHeader(QNetworkRequest::ContentTypeHeader, contentTypePattern.arg(QStringLiteral("x-www-form-urlencoded")));

    } else {
        if (vendor.isEmpty())
            result.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
        else
            result.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/vnd.%1").arg(vendor));
    }

    for (const QNetworkCookie &cookie : qAsConst(cookies))
        result.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue(cookie));

    for (auto it = customHeaders.cbegin(); it != customHeaders.cend(); ++it)
        result.setRawHeader(it.key(), it.value());

    result.setRawHeader("Proof-Application", proofApp->prettifiedApplicationName().toLatin1());
    result.setRawHeader(QStringLiteral("Proof-%1-Version").arg(proofApp->prettifiedApplicationName()).toLatin1(), qApp->applicationVersion().toLatin1());
    result.setRawHeader(QStringLiteral("Proof-%1-Framework-Version").arg(proofApp->prettifiedApplicationName()).toLatin1(), Proof::proofVersion().toLatin1());

    QStringList ipAdresses;
    const auto allAddresses = QNetworkInterface::allAddresses();
    for (const auto &address : allAddresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost)
            ipAdresses << address.toString();
    }
    result.setRawHeader(QStringLiteral("Proof-IP-Addresses").toLatin1(), ipAdresses.join(QStringLiteral("; ")).toLatin1());

    switch (authType) {
    case RestAuthType::Wsse:
        result.setRawHeader("X-WSSE", generateWsseToken());
        result.setRawHeader("X-Client-Name", clientName.toLocal8Bit());
        result.setRawHeader("Authorization", "WSSE profile=\"UsernameToken\"");
        break;
    case RestAuthType::Basic:
        if (!clientName.isEmpty())
            result.setRawHeader("X-Client-Name", clientName.toLocal8Bit());
        result.setRawHeader("Authorization",
                            QStringLiteral("Basic %1")
                            .arg(QString(QStringLiteral("%1:%2").arg(userName, password).toLatin1().toBase64()))
                            .toLatin1());
        break;
    case RestAuthType::BearerToken:
        if (!clientName.isEmpty())
            result.setRawHeader("X-Client-Name", clientName.toLocal8Bit());
        result.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(token).toLatin1());
        break;
    case RestAuthType::NoAuth:
        if (!clientName.isEmpty())
            result.setRawHeader("X-Client-Name", clientName.toLocal8Bit());
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
    QString nonce = QUuid::createUuid().toString().replace(QLatin1String("-"), QLatin1String(""));

    QCryptographicHash hasher(QCryptographicHash::Sha1);
    hasher.addData(nonce.toLatin1());
    hasher.addData(createdAt.toLatin1());
    hasher.addData(hashedPassword);

    return QStringLiteral("UsernameToken Username=\"%1\", PasswordDigest=\"%2\", Nonce=\"%3\", Created=\"%4\"")
            .arg(userName,
                 QString(hasher.result().toBase64()),
                 QString(nonce.toLatin1().toBase64()),
                 createdAt)
            .toLatin1();
}

void RestClientPrivate::handleReply(QNetworkReply *reply)
{
    Q_Q(RestClient);
    if (ignoreSslErrors)
        reply->ignoreSslErrors();

    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    replyTimeouts.insert(reply, timer);
    QObject::connect(timer, &QTimer::timeout, q, [timer, reply]() {
        qCWarning(proofNetworkMiscLog) << "Timed out:" << reply->request().url().toDisplayString(QUrl::FormattingOptions(QUrl::FullyDecoded)) << reply->isRunning();
        if (reply->isRunning())
            reply->abort();
        timer->deleteLater();
    });

    timer->start(msecsForTimeout);

    QObject::connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                     q, [this, reply](QNetworkReply::NetworkError e) {
        qCWarning(proofNetworkMiscLog) << "Error occurred:" << reply->request().url().toDisplayString(QUrl::FormattingOptions(QUrl::FullyDecoded)) << e;
        cleanupReplyHandler(reply);
    });
    QObject::connect(reply, &QNetworkReply::finished, q, [this, reply]() {
        qCDebug(proofNetworkMiscLog) << "Finished:" << reply->request().url().toDisplayString(QUrl::FormattingOptions(QUrl::FullyDecoded)) << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        cleanupReplyHandler(reply);
    });
}

void RestClientPrivate::cleanupReplyHandler(QNetworkReply *reply)
{
    //This call is only for compatibility with old stations where restclient was explicitly moved to some other thread
    //In proper workflow this call with not do anything since restclient is in the same thread
    if (ProofObject::call(NetworkScheduler::instance()->qnam, this, &RestClientPrivate::cleanupReplyHandler, Call::Block, reply))
        return;
    if (replyTimeouts.contains(reply)) {
        QTimer *connectionTimer = replyTimeouts.take(reply);
        connectionTimer->stop();
        connectionTimer->deleteLater();
    }
}

void RestClientPrivate::cleanupAll()
{
    if (ProofObject::call(NetworkScheduler::instance()->qnam, this, &RestClientPrivate::cleanupAll, Call::BlockEvents))
        return;
    algorithms::forEach(replyTimeouts, [](QNetworkReply *, QTimer *timer) {
        timer->stop();
        timer->deleteLater();
    });
    replyTimeouts.clear();
}

QPair<QString, QString> RestClientPrivate::parseHost(const QString &host)
{
    QStringList parts = host.split('/', QString::SkipEmptyParts);
    if (parts.isEmpty())
        return {host, QString()};

    int hostIndex = parts[0].endsWith(':') ? 1 : 0;
    QString newHost = parts[hostIndex];

    parts.removeFirst();
    if (hostIndex > 0)
        parts.removeFirst();

    QString postfix;
    if (!parts.isEmpty())
        postfix = '/' + parts.join('/');
    return {newHost, postfix};
}

CancelableFuture<QNetworkReply *> NetworkScheduler::addRequest(const QString &host, std::function<QNetworkReply *(QNetworkAccessManager *)> &&request)
{
    auto promise = PromiseSP<QNetworkReply *>::create();
    promise->future()->flatMap([this, host](QNetworkReply *reply) {
        auto checker = PromiseSP<bool>::create();
        QObject::connect(reply, &QNetworkReply::finished, reply, [checker](){checker->success(true);});
        QObject::connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), reply, [checker](){checker->success(true);});
        QObject::connect(reply, &QNetworkReply::sslErrors, reply, [checker](){checker->success(true);});
        return checker->future();
    })->onSuccess([this, host](bool) {
        decreaseUsage(host);
        schedule();
    });

    requestsLock.lock();
    qCDebug(proofNetworkExtraLog) << "Adding request for" << host << "with current usage =" << usages.value(host, 0);
    requests.emplace_back(host, [this, host, request, promise]() {
        if (promise->filled()) {
            qCDebug(proofNetworkExtraLog) << "Request for" << host << "was ready to be sent, but is already canceled, skipping it";
            decreaseUsage(host);
            return;
        }
        qCDebug(proofNetworkExtraLog) << "Sending request for" << host;
        promise->success(request(qnam));
    });
    requestsLock.unlock();

    schedule();
    return CancelableFuture<QNetworkReply *>(promise);
}

void NetworkScheduler::schedule()
{
    if (ProofObject::call(qnam, this, &NetworkScheduler::schedule))
        return;
    qCDebug(proofNetworkExtraLog) << "Scheduling network requests with queue size =" << requests.size();
    while (requests.size()) {
        bool found = false;
        requestsLock.lock();
        RequestDescriptor candidate;
        for (auto it = requests.begin(); it != requests.end(); ++it) {
            if (usages[it->first] < limit) {
                candidate = *it;
                found = true;
                if (!candidate.first.isEmpty())
                    ++usages[candidate.first];
                requests.erase(it);
                break;
            }
        }
        requestsLock.unlock();
        if (!found)
            break;
        candidate.second();
    }
}

void NetworkScheduler::decreaseUsage(const QString &host)
{
    if (!host.isEmpty()) {
        requestsLock.lock();
        --usages[host];
        if (usages[host] <= 0)
            usages.remove(host);
        requestsLock.unlock();
    }
}
