#ifndef NETWORKDATAENTITY_P_H
#define NETWORKDATAENTITY_P_H

#include "proofseed/spinlock.h"

#include "proofcore/proofobject_p.h"

#include "proofnetwork/networkdataentity.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QtGlobal>

#include <functional>

namespace Proof {
class PROOF_NETWORK_EXPORT NetworkDataEntityPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(NetworkDataEntity)
public:
    using NDE = NetworkDataEntity::NDE;
    NetworkDataEntityPrivate();
    ~NetworkDataEntityPrivate();

    bool isFetched = false;
    mutable NetworkDataEntityWP weakSelf;
    mutable SpinLock spinLock;
};
} // namespace Proof

#endif // NETWORKDATAENTITY_P_H
