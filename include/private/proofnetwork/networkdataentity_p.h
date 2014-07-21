#ifndef NETWORKDATAENTITY_P_H
#define NETWORKDATAENTITY_P_H

#include "proofcore/proofobject_p.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"

#include <QtGlobal>

namespace Proof {
class PROOF_NETWORK_EXPORT NetworkDataEntityPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(NetworkDataEntity)
public:
    NetworkDataEntityPrivate() : ProofObjectPrivate() {}

    bool isFetched = false;
    NetworkDataEntityWP weakSelf;
};
}

#endif // NETWORKDATAENTITY_P_H
