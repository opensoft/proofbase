#ifndef JSONAMQPCLIENT_H
#define JSONAMQPCLIENT_H

#include "proofnetwork/abstractamqpreceiver.h"

namespace Proof {

class JsonAmqpClientPrivate;
class PROOF_NETWORK_EXPORT JsonAmqpClient : public AbstractAmqpReceiver
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(JsonAmqpClient)

public:
    explicit JsonAmqpClient(QObject *parent = nullptr);

signals:
    void messageReceived(const QJsonDocument &jsonMessage);

protected:
    JsonAmqpClient(JsonAmqpClientPrivate &dd, QObject *parent = nullptr);

};

}

#endif // JSONAMQPCLIENT_H
