#include "abstractamqpclient.h"
#include "abstractamqpclient_p.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QThread>

using namespace Proof;

static const int AUTO_RECONNECTION_TRIES = 3;
static const int DISCONNECT_WAITING_TIMEOUT = 5000;

AbstractAmqpClient::AbstractAmqpClient(AbstractAmqpClientPrivate &dd, QObject *parent)
    : ProofObject(dd, parent)
{
    Q_D(AbstractAmqpClient);
    d->rabbitClient = new QAmqpClient(this);

    QObject::connect(d->rabbitClient, static_cast<void(QAmqpClient::*)(QAMQP::Error)>(&QAmqpClient::error), this, [this, d](QAMQP::Error error) {
        if (d->rabbitClient->autoReconnect()) {
            qCDebug(proofNetworkAmqpLog) << "Client Connection Error:" << error << "Reconnection tries before error emit:" << d->autoReconnectionTries;
            if (d->autoReconnectionTries > 0) {
                --d->autoReconnectionTries;
            } else {
                emit errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::InternalError, QString("Client Error: %1").arg(error), false);
                d->autoReconnectionTries = AUTO_RECONNECTION_TRIES;
            }
        } else {
            d->rabbitClient->disconnectFromHost();
            emit errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::InternalError, QString("Client Error: %1").arg(error), false);
            qCDebug(proofNetworkAmqpLog) << "Client Error:" << error;
        }
    });

    QObject::connect(d->rabbitClient, static_cast<void(QAmqpClient::*)(QAbstractSocket::SocketError)>(&QAmqpClient::socketError), this, [this, d](QAbstractSocket::SocketError error) {
        if (d->rabbitClient->autoReconnect()) {
            qCDebug(proofNetworkAmqpLog) << "Client Connection Error:" << error << "Reconnection tries before error emit:" << d->autoReconnectionTries;
            if (d->autoReconnectionTries > 0) {
                --d->autoReconnectionTries;
            } else {
                emit errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::ServiceUnavailable, "Can't connect to qamqp server (Socket)", false);
                d->autoReconnectionTries = AUTO_RECONNECTION_TRIES;
            }
        } else {
            d->rabbitClient->disconnectFromHost();
            emit errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::ServiceUnavailable, "Can't connect to qamqp server (Socket)", false);
            qCDebug(proofNetworkAmqpLog) << "Socket Error:" << error;
        }
    });

    QObject::connect(d->rabbitClient, &QAmqpClient::sslErrors, this, [this](const QList<QSslError> &errors) {
        emit errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::SslError, "Can't connect to qamqp server (SSL)", false);

        QString errorsString;
        for(const auto &error : errors)
            errorsString += QString("%1,\n").arg(error.errorString());
        errorsString.chop(2);
        qCDebug(proofNetworkAmqpLog) << "SSL Socket errors:" << errorsString;
    });

    QObject::connect(d->rabbitClient, &QAmqpClient::connected, this, [this, d]() {
        qCDebug(proofNetworkAmqpLog) << "Connected";
        d->autoReconnectionTries = AUTO_RECONNECTION_TRIES;
        d->connected();
    });

    QObject::connect(d->rabbitClient, &QAmqpClient::disconnected, this, &AbstractAmqpClient::disconnected);
}

quint16 AbstractAmqpClient::port() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->port();
}

void AbstractAmqpClient::setPort(quint16 port)
{
    Q_D(AbstractAmqpClient);
    d->rabbitClient->setPort(port);
}

QString AbstractAmqpClient::host() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->host();
}

void AbstractAmqpClient::setHost(const QString &host)
{
    Q_D(AbstractAmqpClient);
    d->rabbitClient->setHost(host);
}

QString AbstractAmqpClient::virtualHost() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->virtualHost();
}

void AbstractAmqpClient::setVirtualHost(const QString &virtualHost)
{
    Q_D(AbstractAmqpClient);
    d->rabbitClient->setVirtualHost(virtualHost);
}

QString AbstractAmqpClient::userName() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->username();
}

void AbstractAmqpClient::setUserName(const QString &username)
{
    Q_D(AbstractAmqpClient);
    d->rabbitClient->setUsername(username);
}

QString AbstractAmqpClient::password() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->password();
}

void AbstractAmqpClient::setPassword(const QString &password)
{
    Q_D(AbstractAmqpClient);
    d->rabbitClient->setPassword(password);
}

bool AbstractAmqpClient::autoReconnect() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->autoReconnect();
}

void AbstractAmqpClient::setAutoReconnect(bool value)
{
    Q_D(AbstractAmqpClient);
    d->rabbitClient->setAutoReconnect(value);
}

qint16 AbstractAmqpClient::channelMax() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->channelMax();
}

void AbstractAmqpClient::setChannelMax(qint16 channelMax)
{
    Q_D(AbstractAmqpClient);
    d->rabbitClient->setChannelMax(channelMax);
}

qint32 AbstractAmqpClient::frameMax() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->frameMax();
}

void AbstractAmqpClient::setFrameMax(qint32 frameMax)
{
    Q_D(AbstractAmqpClient);
    d->rabbitClient->setFrameMax(frameMax);
}

qint16 AbstractAmqpClient::heartbeatDelay() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->heartbeatDelay();
}

void AbstractAmqpClient::setHeartbeatDelay(qint16 delay)
{
    Q_D(AbstractAmqpClient);
    d->rabbitClient->setHeartbeatDelay(delay);
}

QSslConfiguration AbstractAmqpClient::sslConfiguration() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->sslConfiguration();
}

void AbstractAmqpClient::setSslConfiguration(const QSslConfiguration &config)
{
    Q_D(AbstractAmqpClient);
    d->rabbitClient->setSslConfiguration(config);
}

bool AbstractAmqpClient::isConnected() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->isConnected();
}

void AbstractAmqpClient::connectToHost()
{
    Q_D(AbstractAmqpClient);

    if (!call(this, &AbstractAmqpClient::connectToHost, Call::Block)) {
        if (!isConnected())
            d->rabbitClient->connectToHost();
    }
}

void AbstractAmqpClient::disconnectFromHost()
{
    Q_D(AbstractAmqpClient);

    if (!call(this, &AbstractAmqpClient::disconnectFromHost, Call::Block)) {
        if (isConnected()) {
            d->rabbitClient->disconnectFromHost();
            QTime timer;
            timer.start();
            while (isConnected() && timer.elapsed() < DISCONNECT_WAITING_TIMEOUT) {
                QThread::yieldCurrentThread();
                qApp->processEvents();
            }
        }
        //Just to make sure that everything is ok
        d->rabbitClient->abort();
    }
}

AbstractAmqpClientPrivate::AbstractAmqpClientPrivate() : ProofObjectPrivate()
{

}
