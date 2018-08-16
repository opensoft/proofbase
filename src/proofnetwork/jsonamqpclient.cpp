#include "jsonamqpclient.h"

#include "proofnetwork/jsonamqpclient_p.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QThread>

using namespace Proof;

JsonAmqpClient::JsonAmqpClient(JsonAmqpClientPrivate &dd, QObject *parent) : AbstractAmqpReceiver(dd, parent)
{}

void JsonAmqpClientPrivate::amqpMessageReceived()
{
    //HACK: Workaround for bug in QAQMP, sometimes after qamqp reconnect, signal messageReceived has been emitted, but queue actually is empty, so we check it here.
    if (queue->isEmpty())
        return;
    QAmqpMessage message = queue->dequeue();
    if (message.isValid()) {
        QJsonDocument messageDocument = QJsonDocument::fromJson(message.payload());
        qCDebug(proofNetworkAmqpLog) << "Queue message: " << messageDocument;
        handleJsonMessage(messageDocument, message.routingKey(), message.headers());
    } else {
        qCDebug(proofNetworkAmqpLog) << "Queue message is not valid: " << message.payload();
    }
}

void JsonAmqpClientPrivate::handleJsonMessage(const QJsonDocument &json, const QString &routingKey,
                                              const QHash<QString, QVariant> &headers)
{
    Q_UNUSED(routingKey)
    Q_UNUSED(headers)
    Q_UNUSED(json)
    //Nothing there
}
