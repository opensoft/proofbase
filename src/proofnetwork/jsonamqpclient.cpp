#include "jsonamqpclient.h"

#include "proofnetwork/jsonamqpclient_p.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QThread>

using namespace Proof;

JsonAmqpClient::JsonAmqpClient(QObject *parent)
    : JsonAmqpClient(*new JsonAmqpClientPrivate, parent)
{
}

JsonAmqpClient::JsonAmqpClient(JsonAmqpClientPrivate &dd, QObject *parent)
    : AbstractAmqpClient(dd, parent)
{

}

void JsonAmqpClientPrivate::amqpMessageReceived()
{
    Q_Q(JsonAmqpClient);
    //HACK: Workaround for bug in QAQMP, sometimes after qamqp reconnect, signal messageReceived has been emitted, but queue actually is empty, so we check it here.
    if (queue->isEmpty())
        return;
    QAmqpMessage message = queue->dequeue();
    QJsonDocument messageDocument = QJsonDocument::fromJson(message.payload());
    qCDebug(proofNetworkAmqpLog) << "Queue message: " << messageDocument;
    emit q->messageReceived(messageDocument);
    handleJsonMessage(messageDocument);
}

void JsonAmqpClientPrivate::handleJsonMessage(const QJsonDocument &json)
{
    Q_UNUSED(json);
    //Nothing there
}

