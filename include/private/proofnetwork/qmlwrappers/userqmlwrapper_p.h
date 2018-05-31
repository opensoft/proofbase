#ifndef USERQMLWRAPPER_P_H
#define USERQMLWRAPPER_P_H

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/qmlwrappers/networkdataentityqmlwrapper_p.h"

namespace Proof {
class UserQmlWrapper;
class PROOF_NETWORK_EXPORT UserQmlWrapperPrivate : public NetworkDataEntityQmlWrapperPrivate
{
    Q_DECLARE_PUBLIC(UserQmlWrapper)
};

} // namespace Proof

#endif // USERQMLWRAPPER_P_H
