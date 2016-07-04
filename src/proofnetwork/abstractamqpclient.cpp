#include "abstractamqpclient.h"
#include "abstractamqpclient_p.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QThread>

using namespace Proof;

static const int AUTO_RECONNECTION_TRIES = 3;

AbstractAmqpClient::AbstractAmqpClient(AbstractAmqpClientPrivate &dd, QObject *parent)
    : ProofObject(dd, parent)
{
    Q_D(AbstractAmqpClient);
    //TODO: Make lazy init for QAmqpClient for correct work in client network thread
    d->rabbitClient = new QAmqpClient();

    QObject::connect(d->rabbitClient, static_cast<void(QAmqpClient::*)(QAMQP::Error)>(&QAmqpClient::error), this, [this, d](QAMQP::Error error) {
        if (d->rabbitClient->autoReconnect() && d->autoReconnectionTries) {
            --d->autoReconnectionTries;
            qCDebug(proofNetworkAmqpLog) << "Client Connection Error:" << error << "Reconnection tries count:" << d->autoReconnectionTries;
            return;
        }

        d->rabbitClient->disconnectFromHost();
        emit errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::InternalError, QString("Client Error: %1").arg(error), false);
        qCDebug(proofNetworkAmqpLog) << "Client Error:" << error;
    });

    QObject::connect(d->rabbitClient, static_cast<void(QAmqpClient::*)(QAbstractSocket::SocketError)>(&QAmqpClient::socketError), this, [this, d](QAbstractSocket::SocketError error) {
        if (d->rabbitClient->autoReconnect() && d->autoReconnectionTries) {
            --d->autoReconnectionTries;
            qCDebug(proofNetworkAmqpLog) << "Client Connection Error:" << error << "Reconnection tries count:" << d->autoReconnectionTries;
            return;
        }

        d->rabbitClient->disconnectFromHost();
        emit errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::ServiceUnavailable, "Can't connect to qamqp server (Socket)", false);
        qCDebug(proofNetworkAmqpLog) << "Socket error" << error;
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

        auto queue = d->rabbitClient->createQueue(d->queueName);
        if (d->queue != queue){
            d->queue = queue;
            qCDebug(proofNetworkAmqpLog) << "Create queue:" << d->queueName;
            QObject::connect(d->queue, static_cast<void(QAmqpQueue::*)(QAMQP::Error)>(&QAmqpQueue::error), this, [this](QAMQP::Error error) {
                emit errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::InternalError, QString("Queue Error: %1").arg(error), false);
                qCDebug(proofNetworkAmqpLog) << "Queue Error:" << error;
            });

            QObject::connect(d->queue, &QAmqpQueue::opened, this, [this, d]() {
                qCDebug(proofNetworkAmqpLog) << "Queue opened" << sender();
                QObject::connect(d->queue, &QAmqpQueue::messageReceived, this, [this, d]() {d->amqpMessageReceived();});
                d->queue->consume(QAmqpQueue::coNoAck);
                emit connected();
            });
        }

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

QString AbstractAmqpClient::queueName() const
{
    Q_D(const AbstractAmqpClient);
    return d->queueName;
}

void AbstractAmqpClient::setQueueName(const QString &queueName)
{
    Q_D(AbstractAmqpClient);
    d->queueName = queueName;
}

bool AbstractAmqpClient::isConnected() const
{
    Q_D(const AbstractAmqpClient);
    return d->rabbitClient->isConnected();
}

void AbstractAmqpClient::connectToHost()
{
    Q_D(AbstractAmqpClient);

    if (!isConnected())
        d->rabbitClient->connectToHost();
}

void AbstractAmqpClient::disconnectFromHost()
{
    Q_D(AbstractAmqpClient);

    if (isConnected())
        d->rabbitClient->disconnectFromHost();
}

AbstractAmqpClientPrivate::AbstractAmqpClientPrivate() : ProofObjectPrivate()
{

}
