#include "abstractrestapi.h"
#include "proofnetwork_global.h"

Q_LOGGING_CATEGORY(proofNetworkLog, "proof.network")
Q_LOGGING_CATEGORY(proofNetworkCommonLog, "proof.network.common")

__attribute__((constructor))
static void libraryInit()
{
    qRegisterMetaType<Proof::RestApiError>("Proof::RestApiError");
}
