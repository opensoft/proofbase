#ifndef QABSTRACTAMQPCLIENT_P_H
#define QABSTRACTAMQPCLIENT_P_H

#include "proofcore/proofobject_p.h"
#include "proofnetwork/proofnetwork_global.h"
#include "abstractamqpclient.h"
#include "proofnetwork/3rdparty/qamqp/qamqpclient.h"
#include "proofnetwork/3rdparty/qamqp/qamqpqueue.h"
#include "proofnetwork/3rdparty/qamqp/qamqpmessage.h"

namespace Proof {

class PROOF_NETWORK_EXPORT AbstractAmqpClientPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(AbstractAmqpClient)

public:
    AbstractAmqpClientPrivate();
    QAmqpClient *m_rabbitClient = nullptr;
    QAmqpQueue *m_queue = nullptr;
    QString m_queueName;
    int m_autoReconnectionTries = 0;

    virtual void amqpMessageReceived() = 0;
};

}
#endif // QABSTRACTAMQPCLIENT_P_H
