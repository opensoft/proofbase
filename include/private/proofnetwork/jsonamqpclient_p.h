#ifndef JSONAMQPCLIENT_P_H
#define JSONAMQPCLIENT_P_H

#include "jsonamqpclient.h"
#include "abstractamqpclient_p.h"

namespace Proof {

class PROOF_NETWORK_EXPORT JsonAmqpClientPrivate : public AbstractAmqpClientPrivate
{
    Q_DECLARE_PUBLIC(JsonAmqpClient)

    void amqpMessageReceived() override;
    virtual void handleJsonMessage(const QJsonDocument &json);
};

}

#endif // JSONAMQPCLIENT_P_H
