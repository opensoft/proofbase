#ifndef QABSTRACTAMQPCLIENT_H
#define QABSTRACTAMQPCLIENT_H

#include "proofcore/proofobject.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"

#include <QSslConfiguration>

namespace Proof {

class AbstractAmqpClientPrivate;
class PROOF_NETWORK_EXPORT AbstractAmqpClient : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractAmqpClient)

public:
    quint16 port() const;
    void setPort(quint16 port);

    QString host() const;
    void setHost(const QString &host);

    QString virtualHost() const;
    void setVirtualHost(const QString &virtualHost);

    QString userName() const;
    void setUserName(const QString &userName);

    QString password() const;
    void setPassword(const QString &password);

    bool autoReconnect() const;
    void setAutoReconnect(bool value);

    qint16 channelMax() const;
    void setChannelMax(qint16 channelMax);

    qint32 frameMax() const;
    void setFrameMax(qint32 frameMax);

    qint16 heartbeatDelay() const;
    void setHeartbeatDelay(qint16 delay);

    QSslConfiguration sslConfiguration() const;
    void setSslConfiguration(const QSslConfiguration &config);

    QString queueName() const;
    void setQueueName(const QString &queueName);

    bool isConnected() const;

    void connectToHost();
    void disconnectFromHost();

signals:
    void errorOccurred(long moduleCode, long errorCode, const QString &errorMessage, bool userFriendly);
    void connected();
    void disconnected();

protected:
    AbstractAmqpClient(AbstractAmqpClientPrivate &dd, QObject *parent = 0);

};

}

#endif // QABSTRACTAMQPCLIENT_H
