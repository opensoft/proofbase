#include "abstractrestapi.h"
#include "proofnetwork_global.h"
#include "proofnetwork_types.h"

Q_LOGGING_CATEGORY(proofNetworkMiscLog, "proof.network.misc")

__attribute__((constructor))
static void libraryInit()
{
    qRegisterMetaType<Proof::RestApiError>("Proof::RestApiError");
    qRegisterMetaType<Proof::RestAuthType>("Proof::RestAuthType");
}
