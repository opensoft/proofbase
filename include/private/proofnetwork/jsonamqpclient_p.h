#ifndef JSONAMQPCLIENT_P_H
#define JSONAMQPCLIENT_P_H

#include "proofnetwork/abstractamqpreceiver_p.h"
#include "proofnetwork/jsonamqpclient.h"

namespace Proof {

class PROOF_NETWORK_EXPORT JsonAmqpClientPrivate : public AbstractAmqpReceiverPrivate
{
    Q_DECLARE_PUBLIC(JsonAmqpClient)

    void amqpMessageReceived() override;
    void queueDeclared(QAmqpQueue *queue) override;
    virtual void handleJsonMessage(const QJsonDocument &json, const QString &routingKey,
                                   const QHash<QString, QVariant> &headers);
};

} // namespace Proof

#endif // JSONAMQPCLIENT_P_H
