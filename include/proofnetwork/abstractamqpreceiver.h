#ifndef PROOF_ABSTRACTAMQPRECEIVER_H
#define PROOF_ABSTRACTAMQPRECEIVER_H

#include "proofnetwork/abstractamqpclient.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"

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

protected:
    AbstractAmqpReceiver(AbstractAmqpReceiverPrivate &dd, QObject *parent = nullptr);
};

} // namespace Proof

#endif // PROOF_ABSTRACTAMQPRECEIVER_H
