#include "abstractrestapi.h"
#include "abstractrestapi_p.h"

#include "restclient.h"

#include <QNetworkReply>

static const qlonglong NETWORK_SSL_ERROR_OFFSET = 1500;
static const qlonglong NETWORK_ERROR_OFFSET = 1000;

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
    d->replyFinishedConnection = QObject::connect(client.data(), &RestClient::finished,
                                      this, [d](QNetworkReply *reply){d->replyFinished(reply);});
    d->sslErrorsConnection = QObject::connect(client.data(), &RestClient::sslErrors,
                                      this, [d](QNetworkReply *reply, const QList<QSslError> &errors){d->sslErrorsOccurred(reply, errors);});
}

QNetworkReply *AbstractRestApiPrivate::get(const QString &method, const QUrlQuery &query)
{
    Q_Q(AbstractRestApi);
    QNetworkReply *reply = restClient->get(method, query);
    QObject::connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                     q, [this, reply](QNetworkReply::NetworkError) {replyErrorOccurred(reply);});
    return reply;
}

QNetworkReply *AbstractRestApiPrivate::post(const QString &method, const QUrlQuery &query, const QByteArray &body)
{
    Q_Q(AbstractRestApi);
    QNetworkReply *reply = restClient->post(method, query, body);
    QObject::connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                     q, [this, reply](QNetworkReply::NetworkError) {replyErrorOccurred(reply);});
    return reply;
}

void AbstractRestApiPrivate::replyFinished(QNetworkReply *reply)
{
    Q_Q(AbstractRestApi);
    if (reply->error() == QNetworkReply::NetworkError::NoError
            || (reply->error() >= 100 && (reply->error() % 100) != 99)) {
        int errorNumber = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (errorNumber != 200 && errorNumber != 201) {
            emit q->errorOccurred(AbstractRestApi::ErrorLevel::ServerError,
                                  errorNumber, reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
            cleanupReply(reply);
        }
    }
}

void AbstractRestApiPrivate::replyErrorOccurred(QNetworkReply *reply)
{
    Q_Q(AbstractRestApi);
    if (reply->error() != QNetworkReply::NetworkError::NoError
            && (reply->error() < 100 || (reply->error() % 100) == 99)) {
        int errorNumber = NETWORK_ERROR_OFFSET
                + static_cast<int>(reply->error());
        emit q->errorOccurred(AbstractRestApi::ErrorLevel::ClientError,
                              errorNumber, reply->errorString());
        cleanupReply(reply);
    }
}


void AbstractRestApiPrivate::sslErrorsOccurred(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_UNUSED(reply);
    Q_Q(AbstractRestApi);
    for (const QSslError &error : errors) {
        if (error.error() != QSslError::SslError::NoError) {
            int errorNumber = NETWORK_SSL_ERROR_OFFSET + static_cast<int>(error.error());
            emit q->errorOccurred(AbstractRestApi::ErrorLevel::ClientError, errorNumber, error.errorString());
            cleanupReply(reply);
        }
    }
}
