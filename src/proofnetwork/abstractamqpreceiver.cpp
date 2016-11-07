#include "abstractamqpreceiver.h"
#include "abstractamqpreceiver_p.h"

using namespace Proof;

AbstractAmqpReceiver::AbstractAmqpReceiver(AbstractAmqpReceiverPrivate &dd, QObject *parent)
    : AbstractAmqpClient(dd, parent)
{

}

QString AbstractAmqpReceiver::queueName() const
{
    Q_D(const AbstractAmqpReceiver);
    return d->queueName;
}

void AbstractAmqpReceiver::setQueueName(const QString &queueName)
{
    Q_D(AbstractAmqpReceiver);
    d->queueName = queueName;
}

AbstractAmqpReceiverPrivate::AbstractAmqpReceiverPrivate()
    : AbstractAmqpClientPrivate()
{

}

void AbstractAmqpReceiverPrivate::connected()
{
    Q_Q(AbstractAmqpReceiver);
    auto newQueue = rabbitClient->createQueue(queueName);
    if (newQueue != queue){
        queue = newQueue;
        qCDebug(proofNetworkAmqpLog) << "Create queue:" << queueName;
        QObject::connect(queue, static_cast<void(QAmqpQueue::*)(QAMQP::Error)>(&QAmqpQueue::error), q, [this, q](QAMQP::Error error) {
            emit q->errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::InternalError, QString("Queue Error: %1").arg(error), false);
            qCDebug(proofNetworkAmqpLog) << "Queue Error:" << error;
        });

        QObject::connect(queue, &QAmqpQueue::opened, q, [this, q]() {
            qCDebug(proofNetworkAmqpLog) << "Queue opened" << q->sender();
            QObject::connect(queue, &QAmqpQueue::messageReceived, q, [this]() {amqpMessageReceived();});
            queue->consume(QAmqpQueue::coNoAck);
            emit q->connected();
        });
    }
}
