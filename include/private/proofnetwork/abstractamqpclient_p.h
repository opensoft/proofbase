#ifndef QABSTRACTAMQPCLIENT_P_H
#define QABSTRACTAMQPCLIENT_P_H

#include "abstractamqpclient.h"

#include "proofcore/proofobject_p.h"

#include "proofnetwork/3rdparty/qamqp/qamqpclient.h"
#include "proofnetwork/3rdparty/qamqp/qamqpmessage.h"
#include "proofnetwork/3rdparty/qamqp/qamqpqueue.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {

class PROOF_NETWORK_EXPORT AbstractAmqpClientPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(AbstractAmqpClient)

public:
    AbstractAmqpClientPrivate();

    virtual void connected() = 0;

    QAmqpClient *rabbitClient = nullptr;
    int autoReconnectionTries = 0;
};

} // namespace Proof
#endif // QABSTRACTAMQPCLIENT_P_H
