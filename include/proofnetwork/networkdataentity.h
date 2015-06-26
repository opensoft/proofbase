#ifndef ENTITY_H
#define ENTITY_H

#include "proofcore/proofobject.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {
class NetworkDataEntityQmlWrapper;
class NetworkDataEntityPrivate;
//TODO: make NDEs thread-safe if will be needed
class PROOF_NETWORK_EXPORT NetworkDataEntity : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(NetworkDataEntity)
public:
    bool isFetched() const;

    void updateFrom(const Proof::NetworkDataEntitySP &other);
    virtual NetworkDataEntityQmlWrapper *toQmlWrapper(QObject *parent = 0) const = 0;

signals:
    void isFetchedChanged(bool arg);

protected:
    NetworkDataEntity() = delete;
    NetworkDataEntity(NetworkDataEntityPrivate &dd, QObject *parent = 0);
    void setFetched(bool fetched);

    static void makeWeakSelf(const NetworkDataEntitySP &entity);
};
}

#endif // ENTITY_H
