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

bool AbstractAmqpReceiver::createQueueIfNotExists() const
{
    Q_D(const AbstractAmqpReceiver);
    return d->createdQueueIfNotExists;
}

QAmqpQueue::QueueOptions AbstractAmqpReceiver::queueOptions() const
{
    Q_D(const AbstractAmqpReceiver);
    return d->queueOptions;
}

void AbstractAmqpReceiver::setCreateQueueIfNotExists(bool createQueueIfNotExists, QAmqpQueue::QueueOptions options)
{
    Q_D(AbstractAmqpReceiver);
    d->createdQueueIfNotExists = createQueueIfNotExists;
    d->queueOptions = options;
}

AbstractAmqpReceiverPrivate::AbstractAmqpReceiverPrivate()
    : AbstractAmqpClientPrivate()
{
}

bool AbstractAmqpReceiverPrivate::startConsuming(QAmqpQueue *queue)
{
    Q_Q(AbstractAmqpReceiver);
    QObject::connect(queue, &QAmqpQueue::messageReceived, q, [this]() { amqpMessageReceived(); });
    bool consumeStarted = queue->consume(QAmqpQueue::coNoAck);
    if (consumeStarted) {
        queueState = QueueState::ConsumingState;
        emit q->connected();
    }
    return consumeStarted;
}

void AbstractAmqpReceiverPrivate::connected()
{
    Q_Q(AbstractAmqpReceiver);
    auto newQueue = rabbitClient->createQueue(queueName);
    queueState = QueueState::OpeningState;

    if (newQueue != queue) {
        qCDebug(proofNetworkAmqpLog) << "Create queue: " << queueName;
        queue = newQueue;
        QObject::connect(queue, static_cast<void (QAmqpQueue::*)(QAMQP::Error)>(&QAmqpQueue::error), q, [this, q](QAMQP::Error error) {
            if ((queueState == QueueState::DeclaredState) && (error == QAMQP::PreconditionFailedError) && createdQueueIfNotExists) {
                queueState = QueueState::ReopeningState;
                queue->reset();
                queue->reopen();
            } else {
                qCDebug(proofNetworkAmqpLog) << "Queue Error:" << error;
                emit q->errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::InternalError, QString("Queue Error: %1").arg(error), false);
            }
        });

        QObject::connect(queue, &QAmqpQueue::declared, q, [this, q]() {
            queueState = QueueState::DeclaredState;
            queueDeclared(queue);
        });

        QObject::connect(queue, &QAmqpQueue::opened, q, [this, q]() {
            qCDebug(proofNetworkAmqpLog) << "Queue opened " << q->sender();
            if (createdQueueIfNotExists && queueState == QueueState::OpeningState) {
                queueState = QueueState::DeclaredState;
                queue->declare(queueOptions);
            } else {
                startConsuming(queue);
            }
        });
    }
}
