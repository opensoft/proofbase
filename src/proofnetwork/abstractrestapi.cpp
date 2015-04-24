#include "abstractrestapi.h"
#include "abstractrestapi_p.h"

#include "restclient.h"

#include <QNetworkReply>
#include <QThread>

static const qlonglong NETWORK_SSL_ERROR_OFFSET = 1500;
static const qlonglong NETWORK_ERROR_OFFSET = 1000;

std::atomic<qulonglong> Proof::AbstractRestApiPrivate::lastUsedOperationId {0};

using namespace Proof;

AbstractRestApi::AbstractRestApi(const RestClientSP &restClient, AbstractRestApiPrivate &dd, QObject *parent)
    : ProofObject(dd, parent)
{
    setRestClient(restClient);
}

RestClientSP AbstractRestApi::restClient() const
{
    Q_D(const AbstractRestApi);
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

AbstractRestApi::ErrorCallbackType AbstractRestApi::generateErrorCallback(qulonglong &currentOperationId, RestApiError &error)
{
    return [&currentOperationId, &error]
           (qulonglong operationId, const Proof::RestApiError &_error) {
        if (currentOperationId != operationId)
            return false;
        error = _error;
        return true;
    };
}

AbstractRestApi::ErrorCallbackType AbstractRestApi::generateErrorCallback(qulonglong &currentOperationId, QString &errorMessage)
{
    return [&currentOperationId, &errorMessage]
           (qulonglong operationId, const Proof::RestApiError &_error) {
        if (currentOperationId != operationId)
            return false;
        errorMessage = QString("%1: %2").arg(_error.code).arg(_error.message);
        return true;
    };
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
        qCWarning(proofNetworkMiscLog) << "AbstractRestApi::get(): RestApi and RestClient should live in same thread."
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
        qCWarning(proofNetworkMiscLog) << "AbstractRestApi::post(): RestApi and RestClient should live in same thread."
                                       << "\nRestClient object is in thread =" << restClient->thread()
                                       << "\nRestApi is in thread =" << q->thread()
                                       << "\nrunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->post(method, query, body);
    setupReply(operationId, reply);
    return reply;
}

QNetworkReply *AbstractRestApiPrivate::put(qulonglong &operationId, const QString &method, const QUrlQuery &query, const QByteArray &body)
{
    Q_Q(AbstractRestApi);
    if (QThread::currentThread() != restClient->thread()) {
        qCWarning(proofNetworkMiscLog) << "AbstractRestApi::put(): RestApi and RestClient should live in same thread."
                                       << "\nRestClient object is in thread =" << restClient->thread()
                                       << "\nRestApi is in thread =" << q->thread()
                                       << "\nrunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->put(method, query, body);
    setupReply(operationId, reply);
    return reply;
}

QNetworkReply *AbstractRestApiPrivate::patch(qulonglong &operationId, const QString &method, const QUrlQuery &query, const QByteArray &body)
{
    Q_Q(AbstractRestApi);
    if (QThread::currentThread() != restClient->thread()) {
        qCWarning(proofNetworkMiscLog) << "AbstractRestApi::patch(): RestApi and RestClient should live in same thread."
                                       << "\nRestClient object is in thread =" << restClient->thread()
                                       << "\nRestApi is in thread =" << q->thread()
                                       << "\nrunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->patch(method, query, body);
    setupReply(operationId, reply);
    return reply;
}

QNetworkReply *AbstractRestApiPrivate::deleteResource(qulonglong &operationId, const QString &method, const QUrlQuery &query)
{
    Q_Q(AbstractRestApi);
    if (QThread::currentThread() != restClient->thread()) {
        qCWarning(proofNetworkMiscLog) << "AbstractRestApiPrivate::deleteResource(): RestApi and RestClient should live in same thread."
                                       << "\nRestClient object is in thread =" << restClient->thread()
                                       << "\nRestApi is in thread =" << q->thread()
                                       << "\nrunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->deleteResource(method, query);
    setupReply(operationId, reply);
    return reply;
}

void AbstractRestApiPrivate::replyFinished(qulonglong operationId, QNetworkReply *reply)
{
    Q_Q(AbstractRestApi);
    if (reply->error() == QNetworkReply::NetworkError::NoError
            || (reply->error() >= 100 && (reply->error() % 100) != 99)) {
        int errorCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (errorCode != 200 && errorCode != 201) {
            qCDebug(proofNetworkMiscLog) << "Error occurred for" << operationId
                                         << reply->request().url().path()
                                         << reply->request().url().query()
                                         << ": " << errorCode
                                         << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
            emit q->errorOccurred(operationId,
                                  RestApiError{RestApiError::Level::ServerError,
                                               errorCode,
                                               reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()});
            cleanupReply(operationId, reply);
        }
    }
}

void AbstractRestApiPrivate::replyErrorOccurred(qulonglong operationId, QNetworkReply *reply)
{
    Q_Q(AbstractRestApi);
    if (reply->error() != QNetworkReply::NetworkError::NoError
            && (reply->error() < 100 || (reply->error() % 100) == 99)) {
        int errorCode = NETWORK_ERROR_OFFSET
                + static_cast<int>(reply->error());
        qCDebug(proofNetworkMiscLog) << "Error occurred for" << operationId
                                     << reply->request().url().path()
                                     << reply->request().url().query()
                                     << ": " << errorCode << reply->errorString();
        emit q->errorOccurred(operationId,
                              RestApiError{RestApiError::Level::ClientError,
                                           errorCode,
                                           reply->errorString()});
        cleanupReply(operationId, reply);
    }
}


void AbstractRestApiPrivate::sslErrorsOccurred(qulonglong operationId, QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_Q(AbstractRestApi);
    for (const QSslError &error : errors) {
        if (error.error() != QSslError::SslError::NoError) {
            int errorCode = NETWORK_SSL_ERROR_OFFSET + static_cast<int>(error.error());
            qCDebug(proofNetworkMiscLog) << "SSL error occurred for" << operationId
                                         << reply->request().url().path()
                                         << reply->request().url().query()
                                         << ": " << errorCode << error.errorString();
            emit q->errorOccurred(operationId,
                                  RestApiError{RestApiError::Level::ClientError,
                                               errorCode,
                                               error.errorString()});
            cleanupReply(operationId, reply);
        }
    }
}

void AbstractRestApiPrivate::cleanupReply(qulonglong operationId, QNetworkReply *reply)
{
    Q_UNUSED(operationId);
    repliesIds.remove(reply);
}

QString AbstractRestApiPrivate::vendor() const
{
    return vendorValue;
}

void AbstractRestApiPrivate::setVendor(const QString &arg)
{
    vendorValue = arg;
}

void AbstractRestApiPrivate::notifyAboutJsonParseError(qulonglong operationId, const QJsonParseError &error)
{
    Q_Q(AbstractRestApi);
    QString jsonErrorString = QString("JSON error: %1").arg(error.errorString());
    emit q->errorOccurred(operationId,
                          RestApiError{RestApiError::Level::JsonParseError,
                                       error.error,
                                       jsonErrorString});
}

void AbstractRestApiPrivate::setupReply(qulonglong &operationId, QNetworkReply *reply)
{
    Q_Q(AbstractRestApi);
    operationId = ++lastUsedOperationId;
    repliesIds[reply] = operationId;
    QObject::connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                     q, [this, reply, operationId](QNetworkReply::NetworkError) {replyErrorOccurred(operationId, reply);});
}


QString RestApiError::toString() const
{
    if (level == Level::NoError)
        return QString();
    return QString("%1: %2").arg(code).arg(message);
}

void RestApiError::reset()
{
    level = Level::NoError;
    code = 0;
    message = QString();
}
