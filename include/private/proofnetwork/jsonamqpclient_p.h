#ifndef JSONAMQPCLIENT_P_H
#define JSONAMQPCLIENT_P_H

#include "jsonamqpclient.h"
#include "abstractamqpclient_p.h"

namespace Proof {

class JsonAmqpClientPrivate : public AbstractAmqpClientPrivate
{
    Q_DECLARE_PUBLIC(JsonAmqpClient)

    void amqpMessageReceived() override;
};

}

#endif // JSONAMQPCLIENT_P_H
