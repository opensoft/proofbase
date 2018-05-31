#ifndef SIMPLEJSONAMQPCLIENT_H
#define SIMPLEJSONAMQPCLIENT_H

#include "proofnetwork/jsonamqpclient.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {

class SimpleJsonAmqpClientPrivate;
class PROOF_NETWORK_EXPORT SimpleJsonAmqpClient : public JsonAmqpClient
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SimpleJsonAmqpClient)
public:
    explicit SimpleJsonAmqpClient(QObject *parent = nullptr);

signals:
    void messageReceived(const QJsonDocument &json, const QString &routingKey, const QHash<QString, QVariant> &headers);
};
} // namespace Proof

#endif // SIMPLEJSONAMQPCLIENT_H
