#include "abstractrestapi.h"
#include "abstractrestapi_p.h"

#include "restclient.h"

#include <QNetworkReply>
#include <QThread>
#include <QDebug>

static const qlonglong NETWORK_SSL_ERROR_OFFSET = 1500;
static const qlonglong NETWORK_ERROR_OFFSET = 1000;

std::atomic<qulonglong> Proof::AbstractRestApiPrivate::lastUsedOperationId {0};

using namespace Proof;

AbstractRestApi::AbstractRestApi(const RestClientSP &restClient, AbstractRestApiPrivate &dd, QObject *parent)
    : ProofObject(dd, parent)
{
    qRegisterMetaType<Proof::AbstractRestApi::ErrorLevel>("Proof::AbstractRestApi::ErrorLevel");
    setRestClient(restClient);
}

RestClientSP AbstractRestApi::restClient()
{
    Q_D(AbstractRestApi);
    return d->restClient;
}

void AbstractRestApi::setRestClient(const RestClientSP &client)
{
    Q_D(AbstractRestApi);
    if (d->restClient == client)
        return;
    onRestClientChanging(client);
    d->restClient = client;
}

void AbstractRestApi::onRestClientChanging(const RestClientSP &client)
{
    Q_D(AbstractRestApi);
    if (d->replyFinishedConnection)
        QObject::disconnect(d->replyFinishedConnection);
    if (d->sslErrorsConnection)
        QObject::disconnect(d->sslErrorsConnection);
    if (!client)
        return;

    auto replyFinishedCaller = [d](QNetworkReply *reply) {
        if (!d->repliesIds.contains(reply))
            return;
        d->replyFinished(d->repliesIds[reply], reply);
    };
    auto sslErrorsOccurredCaller = [d](QNetworkReply *reply, const QList<QSslError> &errors) {
        if (!d->repliesIds.contains(reply))
            return;
        d->sslErrorsOccurred(d->repliesIds[reply], reply, errors);
    };

    d->replyFinishedConnection = QObject::connect(client.data(), &RestClient::finished, this, replyFinishedCaller);
    d->sslErrorsConnection = QObject::connect(client.data(), &RestClient::sslErrors, this, sslErrorsOccurredCaller);
}

QNetworkReply *AbstractRestApiPrivate::get(qulonglong &operationId, const QString &method, const QUrlQuery &query)
{
    Q_Q(AbstractRestApi);
    if (QThread::currentThread() != restClient->thread()) {
        qWarning() << "AbstractRestApi::get(): RestApi and RestClient should live in same thread."
                   << "\nRestClient object is in thread =" << restClient->thread()
                   << "\nRestApi is in thread =" << q->thread()
                   << "\nRunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->get(method, query);
    setupReply(operationId, reply);
    return reply;
}

QNetworkReply *AbstractRestApiPrivate::post(qulonglong &operationId, const QString &method, const QUrlQuery &query, const QByteArray &body)
{
    Q_Q(AbstractRestApi);
    if (QThread::currentThread() != restClient->thread()) {
        qWarning() << "AbstractRestApi::post(): RestApi and RestClient should live in same thread."
                   << "\nRestClient object is in thread =" << restClient->thread()
                   << "\nRestApi is in thread =" << q->thread()
                   << "\nrunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->post(method, query, body);
    setupReply(operationId, reply);
    return reply;
}

void AbstractRestApiPrivate::replyFinished(qulonglong operationId, QNetworkReply *reply)
{
    Q_Q(AbstractRestApi);
    if (reply->error() == QNetworkReply::NetworkError::NoError
            || (reply->error() >= 100 && (reply->error() % 100) != 99)) {
        int errorNumber = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (errorNumber != 200 && errorNumber != 201) {
            emit q->errorOccurred(operationId, AbstractRestApi::ErrorLevel::ServerError,
                                  errorNumber, reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
            cleanupReply(operationId, reply);
        }
    }
}

void AbstractRestApiPrivate::replyErrorOccurred(qulonglong operationId, QNetworkReply *reply)
{
    Q_Q(AbstractRestApi);
    if (reply->error() != QNetworkReply::NetworkError::NoError
            && (reply->error() < 100 || (reply->error() % 100) == 99)) {
        int errorNumber = NETWORK_ERROR_OFFSET
                + static_cast<int>(reply->error());
        emit q->errorOccurred(operationId, AbstractRestApi::ErrorLevel::ClientError,
                              errorNumber, reply->errorString());
        cleanupReply(operationId, reply);
    }
}


void AbstractRestApiPrivate::sslErrorsOccurred(qulonglong operationId, QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_Q(AbstractRestApi);
    for (const QSslError &error : errors) {
        if (error.error() != QSslError::SslError::NoError) {
            int errorNumber = NETWORK_SSL_ERROR_OFFSET + static_cast<int>(error.error());
            emit q->errorOccurred(operationId, AbstractRestApi::ErrorLevel::ClientError, errorNumber, error.errorString());
            cleanupReply(operationId, reply);
        }
    }
}

void AbstractRestApiPrivate::cleanupReply(qulonglong operationId, QNetworkReply *reply)
{
    Q_UNUSED(operationId);
    repliesIds.remove(reply);
}

void AbstractRestApiPrivate::setupReply(qulonglong &operationId, QNetworkReply *reply)
{
    Q_Q(AbstractRestApi);
    operationId = ++lastUsedOperationId;
    repliesIds[reply] = operationId;
    QObject::connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                     q, [this, reply, operationId](QNetworkReply::NetworkError) {replyErrorOccurred(operationId, reply);});
}
