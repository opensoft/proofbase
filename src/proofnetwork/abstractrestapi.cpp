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
        if (!d->replies.contains(reply))
            return;
        d->replyFinished(d->replies[reply].first, reply);
    };
    auto sslErrorsOccurredCaller = [d](QNetworkReply *reply, const QList<QSslError> &errors) {
        if (!d->replies.contains(reply))
            return;
        d->sslErrorsOccurred(d->replies[reply].first, reply, errors);
    };

    d->replyFinishedConnection = QObject::connect(client.data(), &RestClient::finished, this, replyFinishedCaller);
    d->sslErrorsConnection = QObject::connect(client.data(), &RestClient::sslErrors, this, sslErrorsOccurredCaller);
}

QNetworkReply *AbstractRestApiPrivate::get(qulonglong &operationId, RestAnswerHandler &&handler, const QString &method, const QUrlQuery &query)
{
    Q_Q(AbstractRestApi);
    if (QThread::currentThread() != restClient->thread()) {
        qCWarning(proofNetworkMiscLog) << "AbstractRestApi::get(): RestApi and RestClient should live in same thread."
                                       << "\nRestClient object is in thread =" << restClient->thread()
                                       << "\nRestApi is in thread =" << q->thread()
                                       << "\nRunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->get(method, query, vendor);
    setupReply(operationId, reply, std::move(handler));
    return reply;
}

QNetworkReply *AbstractRestApiPrivate::post(qulonglong &operationId, RestAnswerHandler &&handler, const QString &method, const QUrlQuery &query, const QByteArray &body)
{
    Q_Q(AbstractRestApi);
    if (QThread::currentThread() != restClient->thread()) {
        qCWarning(proofNetworkMiscLog) << "AbstractRestApi::post(): RestApi and RestClient should live in same thread."
                                       << "\nRestClient object is in thread =" << restClient->thread()
                                       << "\nRestApi is in thread =" << q->thread()
                                       << "\nrunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->post(method, query, body, vendor);
    setupReply(operationId, reply, std::move(handler));
    return reply;
}

QNetworkReply *AbstractRestApiPrivate::post(qulonglong &operationId, RestAnswerHandler &&handler, const QString &method, const QUrlQuery &query, QHttpMultiPart *multiParts)
{
    Q_Q(AbstractRestApi);
    if (QThread::currentThread() != restClient->thread()) {
        qCWarning(proofNetworkMiscLog) << "AbstractRestApi::post(): RestApi and RestClient should live in same thread."
                                       << "\nRestClient object is in thread =" << restClient->thread()
                                       << "\nRestApi is in thread =" << q->thread()
                                       << "\nrunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->post(method, query, multiParts);
    setupReply(operationId, reply, std::move(handler));
    return reply;
}

QNetworkReply *AbstractRestApiPrivate::put(qulonglong &operationId, RestAnswerHandler &&handler, const QString &method, const QUrlQuery &query, const QByteArray &body)
{
    Q_Q(AbstractRestApi);
    if (QThread::currentThread() != restClient->thread()) {
        qCWarning(proofNetworkMiscLog) << "AbstractRestApi::put(): RestApi and RestClient should live in same thread."
                                       << "\nRestClient object is in thread =" << restClient->thread()
                                       << "\nRestApi is in thread =" << q->thread()
                                       << "\nrunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->put(method, query, body, vendor);
    setupReply(operationId, reply, std::move(handler));
    return reply;
}

QNetworkReply *AbstractRestApiPrivate::patch(qulonglong &operationId, RestAnswerHandler &&handler, const QString &method, const QUrlQuery &query, const QByteArray &body)
{
    Q_Q(AbstractRestApi);
    if (QThread::currentThread() != restClient->thread()) {
        qCWarning(proofNetworkMiscLog) << "AbstractRestApi::patch(): RestApi and RestClient should live in same thread."
                                       << "\nRestClient object is in thread =" << restClient->thread()
                                       << "\nRestApi is in thread =" << q->thread()
                                       << "\nrunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->patch(method, query, body, vendor);
    setupReply(operationId, reply, std::move(handler));
    return reply;
}

QNetworkReply *AbstractRestApiPrivate::deleteResource(qulonglong &operationId, RestAnswerHandler &&handler, const QString &method, const QUrlQuery &query)
{
    Q_Q(AbstractRestApi);
    if (QThread::currentThread() != restClient->thread()) {
        qCWarning(proofNetworkMiscLog) << "AbstractRestApiPrivate::deleteResource(): RestApi and RestClient should live in same thread."
                                       << "\nRestClient object is in thread =" << restClient->thread()
                                       << "\nRestApi is in thread =" << q->thread()
                                       << "\nrunning in thread =" << QThread::currentThread();
        return 0;
    }
    QNetworkReply *reply = restClient->deleteResource(method, query, vendor);
    setupReply(operationId, reply, std::move(handler));
    return reply;
}

void AbstractRestApiPrivate::replyFinished(qulonglong operationId, QNetworkReply *reply)
{
    Q_Q(AbstractRestApi);
    if (reply->error() == QNetworkReply::NetworkError::NoError
            || (reply->error() >= 100 && (reply->error() % 100) != 99)) {
        int errorCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (errorCode < 200 || 299 < errorCode) {
            QString message = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString().trimmed();
            QString mimeType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
            if (message.isEmpty() && mimeType == "text/plain")
                message = reply->readAll().trimmed();

            qCDebug(proofNetworkMiscLog) << "Error occurred for" << operationId
                                         << reply->request().url().path()
                                         << reply->request().url().query()
                                         << ": " << errorCode
                                         << message;
            emit q->errorOccurred(operationId,
                                  RestApiError{RestApiError::Level::ServerError,
                                               errorCode,
                                               message});
            cleanupReply(operationId, reply);
        }
    }

    if (replies.contains(reply)) {
        replies[reply].second(operationId, reply);
        cleanupReply(operationId, reply);
    }
}

void AbstractRestApiPrivate::replyErrorOccurred(qulonglong operationId, QNetworkReply *reply)
{
    Q_Q(AbstractRestApi);
    if (reply->error() != QNetworkReply::NetworkError::NoError
            && (reply->error() < 100 || (reply->error() % 100) == 99)) {
        int errorCode = NETWORK_ERROR_OFFSET + static_cast<int>(reply->error());
        QString errorString = reply->errorString();
        if (reply->error() == QNetworkReply::NetworkError::OperationCanceledError)
            errorString = QObject::tr("Service is inaccessible. Try again later");
        qCDebug(proofNetworkMiscLog) << "Error occurred for" << operationId
                                     << reply->request().url().path()
                                     << reply->request().url().query()
                                     << ": " << errorCode << errorString;
        emit q->errorOccurred(operationId,
                              RestApiError{RestApiError::Level::ClientError,
                                           errorCode,
                                           errorString});
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
    replies.remove(reply);
    reply->deleteLater();
}

void AbstractRestApiPrivate::notifyAboutJsonParseError(qulonglong operationId, const QJsonParseError &error)
{
    Q_Q(AbstractRestApi);
    QString jsonErrorString = QString(QObject::tr("JSON error: %1")).arg(error.errorString());
    emit q->errorOccurred(operationId,
                          RestApiError{RestApiError::Level::JsonParseError,
                                       error.error,
                                       jsonErrorString});
}

void AbstractRestApiPrivate::setupReply(qulonglong &operationId, QNetworkReply *reply, RestAnswerHandler &&handler)
{
    Q_Q(AbstractRestApi);
    operationId = ++lastUsedOperationId;
    replies[reply] = qMakePair(operationId, handler);
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
