#ifndef JSONAMQPCLIENT_P_H
#define JSONAMQPCLIENT_P_H

#include "proofnetwork/abstractamqpreceiver_p.h"
#include "proofnetwork/jsonamqpclient.h"

namespace Proof {

class PROOF_NETWORK_EXPORT JsonAmqpClientPrivate : public AbstractAmqpReceiverPrivate
{
    Q_DECLARE_PUBLIC(JsonAmqpClient)

    void amqpMessageReceived() override;
    virtual void handleJsonMessage(const QJsonDocument &json, const QString &routingKey,
                                   const QHash<QString, QVariant> &headers);

    std::function<void(const QJsonDocument &, const QString &, const QHash<QString, QVariant> &)> handler;
    bool customHandlerWasSet = false;
};

} // namespace Proof

#endif // JSONAMQPCLIENT_P_H
