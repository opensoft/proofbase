#ifndef QABSTRACTAMQPCLIENT_P_H
#define QABSTRACTAMQPCLIENT_P_H

#include "proofcore/proofobject_p.h"

#include "proofnetwork/abstractamqpclient.h"
#include "proofnetwork/proofnetwork_global.h"

#include "3rdparty/qamqp/src/qamqpclient.h"
#include "3rdparty/qamqp/src/qamqpmessage.h"
#include "3rdparty/qamqp/src/qamqpqueue.h"

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
