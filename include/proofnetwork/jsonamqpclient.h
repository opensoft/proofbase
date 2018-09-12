#ifndef JSONAMQPCLIENT_H
#define JSONAMQPCLIENT_H

#include "proofnetwork/abstractamqpreceiver.h"

namespace Proof {

class JsonAmqpClientPrivate;
class PROOF_NETWORK_EXPORT JsonAmqpClient : public AbstractAmqpReceiver
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(JsonAmqpClient)
protected:
    explicit JsonAmqpClient(JsonAmqpClientPrivate &dd, QObject *parent = nullptr);
    explicit JsonAmqpClient(QObject *parent = nullptr);
    void setCustomJsonMessageHandler(
        const std::function<void(const QJsonDocument &, const QString &, const QHash<QString, QVariant> &)> &&handler);
};

} // namespace Proof

#endif // JSONAMQPCLIENT_H
