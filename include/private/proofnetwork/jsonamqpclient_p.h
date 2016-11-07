#ifndef JSONAMQPCLIENT_P_H
#define JSONAMQPCLIENT_P_H

#include "proofnetwork/jsonamqpclient.h"
#include "proofnetwork/abstractamqpreceiver_p.h"

namespace Proof {

class PROOF_NETWORK_EXPORT JsonAmqpClientPrivate : public AbstractAmqpReceiverPrivate
{
    Q_DECLARE_PUBLIC(JsonAmqpClient)

    void amqpMessageReceived() override;
    virtual void handleJsonMessage(const QJsonDocument &json);
};

}

#endif // JSONAMQPCLIENT_P_H
