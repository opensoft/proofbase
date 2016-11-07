#ifndef ABSTRACTAMQPRECEIVER_P_H
#define ABSTRACTAMQPRECEIVER_P_H

#include "proofnetwork/abstractamqpclient_p.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/abstractamqpreceiver.h"
#include "proofnetwork/3rdparty/qamqp/qamqpqueue.h"

namespace Proof {
class PROOF_NETWORK_EXPORT AbstractAmqpReceiverPrivate : public AbstractAmqpClientPrivate
{
    Q_DECLARE_PUBLIC(AbstractAmqpReceiver)

public:
    AbstractAmqpReceiverPrivate();

    void connected() override;
    virtual void amqpMessageReceived() = 0;

    QAmqpQueue *queue = nullptr;
    QString queueName;
};
}
#endif // ABSTRACTAMQPRECEIVER_P_H
