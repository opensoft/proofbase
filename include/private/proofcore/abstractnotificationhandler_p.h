#ifndef ABSTRACTNOTIFICATIONHANDLER_P_H
#define ABSTRACTNOTIFICATIONHANDLER_P_H
#include "proofcore/proofcore_global.h"
#include "proofcore/proofobject_p.h"

namespace Proof {
class AbstractNotificationHandler;
class PROOF_CORE_EXPORT AbstractNotificationHandlerPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(AbstractNotificationHandler)

protected:
    QString appId;
};
} // namespace Proof
#endif // ABSTRACTNOTIFICATIONHANDLER_P_H
