#ifndef ABSTRACTAMQPRECEIVER_P_H
#define ABSTRACTAMQPRECEIVER_P_H

#include "proofnetwork/3rdparty/qamqp/qamqpqueue.h"
#include "proofnetwork/abstractamqpclient_p.h"
#include "proofnetwork/abstractamqpreceiver.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {
class PROOF_NETWORK_EXPORT AbstractAmqpReceiverPrivate : public AbstractAmqpClientPrivate
{
    Q_DECLARE_PUBLIC(AbstractAmqpReceiver)

public:
    AbstractAmqpReceiverPrivate();

    void connected() override;
    virtual void queueDeclared(QAmqpQueue *queue) = 0;
    virtual void amqpMessageReceived() = 0;

    enum class QueueState
    {
        OpeningState,
        ErrorState,
        ReopeningState,
        DeclaredState,
        ConsumingState
    };

    QAmqpQueue *queue = nullptr;
    QString queueName;
    bool createdQueueIfNotExists = false;
    QAmqpQueue::QueueOptions queueOptions = {QAmqpQueue::Durable, QAmqpQueue::AutoDelete};

    QueueState queueState = QueueState::ErrorState;

    bool startConsuming(QAmqpQueue *queue);
};
}
#endif // ABSTRACTAMQPRECEIVER_P_H
