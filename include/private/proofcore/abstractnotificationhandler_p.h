#ifndef ABSTRACTNOTIFICATIONHANDLER_P_H
#define ABSTRACTNOTIFICATIONHANDLER_P_H
#include "proofcore/proofobject_p.h"

#include "proofcore/proofcore_global.h"

namespace Proof {
class AbstractNotificationHandler;
class PROOF_CORE_EXPORT AbstractNotificationHandlerPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(AbstractNotificationHandler)
};
}
#endif // ABSTRACTNOTIFICATIONHANDLER_P_H
