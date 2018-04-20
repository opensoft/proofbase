#include "simplejsonamqpclient.h"

#include "jsonamqpclient_p.h"

namespace Proof {
class SimpleJsonAmqpClientPrivate : public JsonAmqpClientPrivate
{
    Q_DECLARE_PUBLIC(SimpleJsonAmqpClient)
    void handleJsonMessage(const QJsonDocument &json, const QString &routingKey, const QHash<QString, QVariant> &headers) override;
};
}

using namespace Proof;

SimpleJsonAmqpClient::SimpleJsonAmqpClient(QObject *parent)
    : JsonAmqpClient(*new SimpleJsonAmqpClientPrivate, parent)
{

}

void SimpleJsonAmqpClientPrivate::handleJsonMessage(const QJsonDocument &json, const QString &routingKey, const QHash<QString, QVariant> &headers)
{
    Q_Q(SimpleJsonAmqpClient);
    emit q->messageReceived(json, routingKey, headers);
}
