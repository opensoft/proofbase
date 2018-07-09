#ifndef PROOFNETWORK_RESTAPICONSUMER_H
#define PROOFNETWORK_RESTAPICONSUMER_H

#include "proofnetwork/proofnetwork_global.h"

#include <qglobal.h>

#include <functional>

namespace Proof {

//TODO: 1.0: deprecated, remove
class PROOF_NETWORK_EXPORT RestApiConsumer
{
    Q_DISABLE_COPY(RestApiConsumer)
public:
protected:
    RestApiConsumer() {}
    virtual ~RestApiConsumer();
};
} // namespace Proof

#endif // PROOFNETWORK_RESTAPICONSUMER_H
