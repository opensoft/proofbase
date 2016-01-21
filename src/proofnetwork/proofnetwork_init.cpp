#include "abstractrestapi.h"
#include "proofnetwork_global.h"
#include "proofnetwork_types.h"
#include "3rdparty/qamqp/qamqpglobal.h"

Q_LOGGING_CATEGORY(proofNetworkMiscLog, "proof.network.misc")
Q_LOGGING_CATEGORY(proofNetworkAmqpLog, "proof.network.amqp")

__attribute__((constructor))
static void libraryInit()
{
    qRegisterMetaType<Proof::RestApiError>("Proof::RestApiError");
    qRegisterMetaType<Proof::RestAuthType>("Proof::RestAuthType");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAMQP::Error>("QAMQP::Error");
}
