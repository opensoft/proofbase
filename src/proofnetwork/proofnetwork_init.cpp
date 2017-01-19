#include "abstractrestapi.h"
#include "proofnetwork_global.h"
#include "proofnetwork_types.h"
#include "proofservicerestapi.h"
#include "3rdparty/qamqp/qamqpglobal.h"
#include "proofcore/proofglobal.h"

Q_LOGGING_CATEGORY(proofNetworkMiscLog, "proof.network.misc")
Q_LOGGING_CATEGORY(proofNetworkAmqpLog, "proof.network.amqp")

PROOF_LIBRARY_INITIALIZER(libraryInit)
{
    qRegisterMetaType<Proof::RestApiError>("Proof::RestApiError");
    qRegisterMetaType<Proof::RestAuthType>("Proof::RestAuthType");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAMQP::Error>("QAMQP::Error");
    qRegisterMetaType<Proof::NetworkServices::VersionedEntityType>("Proof::NetworkServices::ApplicationType");
}
