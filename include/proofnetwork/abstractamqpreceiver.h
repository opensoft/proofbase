#ifndef PROOF_ABSTRACTAMQPRECEIVER_H
#define PROOF_ABSTRACTAMQPRECEIVER_H

#include "proofnetwork/abstractamqpclient.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include "3rdparty/qamqp/src/qamqpqueue.h"

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

    QString newQueueBindingExchangeName() const;
    void setNewQueueBindingExchangeName(const QString &exchangeName);

    QStringList newQueueBindingRoutingKeys() const;
    void setNewQueueBindingRoutingKeys(const QStringList &routingKeys);

    bool createQueueIfNotExists() const;
    QAmqpQueue::QueueOptions queueOptions() const;

    void setCreateQueueIfNotExists(bool createQueueIfNotExists,
                                   QAmqpQueue::QueueOptions options = {QAmqpQueue::Durable, QAmqpQueue::AutoDelete});

protected:
    AbstractAmqpReceiver(AbstractAmqpReceiverPrivate &dd, QObject *parent = nullptr);
};

} // namespace Proof

#endif // PROOF_ABSTRACTAMQPRECEIVER_H
