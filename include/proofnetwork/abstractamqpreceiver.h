#ifndef PROOF_ABSTRACTAMQPRECEIVER_H
#define PROOF_ABSTRACTAMQPRECEIVER_H

#include "proofnetwork/3rdparty/qamqp/qamqpqueue.h"
#include "proofnetwork/abstractamqpclient.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QSslConfiguration>

namespace Proof {

class AbstractAmqpReceiverPrivate;
class PROOF_NETWORK_EXPORT AbstractAmqpReceiver : public AbstractAmqpClient
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractAmqpReceiver)
public:
    QString queueName() const;
    void setQueueName(const QString &queueName);

    bool createQueueIfNotExists() const;
    QAmqpQueue::QueueOptions queueOptions() const;

    void setCreateQueueIfNotExists(bool createQueueIfNotExists,
                                   QAmqpQueue::QueueOptions options = {QAmqpQueue::Durable, QAmqpQueue::AutoDelete});

protected:
    AbstractAmqpReceiver(AbstractAmqpReceiverPrivate &dd, QObject *parent = nullptr);
};

} // namespace Proof

#endif // PROOF_ABSTRACTAMQPRECEIVER_H
