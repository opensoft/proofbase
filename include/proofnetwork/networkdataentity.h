#ifndef ENTITY_H
#define ENTITY_H

#include "proofcore/proofobject.h"

#include "proofnetwork/networkdataentityhelpers.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

namespace Proof {
class NetworkDataEntityQmlWrapper;
class NetworkDataEntityPrivate;
//TODO: make NDEs thread-safe if will be needed
class PROOF_NETWORK_EXPORT NetworkDataEntity : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(NetworkDataEntity)
public:
    using NDE = NetworkDataEntityHelpers;
    bool isFetched() const;

    template <class Argument>
    static typename std::enable_if<std::is_base_of<NetworkDataEntity, Argument>::value, bool>::type
    isValidAndDirty(const QSharedPointer<Argument> &sp)
    {
        return sp && sp->isDirty();
    }

    void updateFrom(const Proof::NetworkDataEntitySP &other);
    virtual NetworkDataEntityQmlWrapper *toQmlWrapper(QObject *parent = nullptr) const = 0;

signals:
    void isFetchedChanged(bool arg);

protected:
    NetworkDataEntity();
    NetworkDataEntity(NetworkDataEntityPrivate &dd);
    void setFetched(bool fetched);

    virtual void updateSelf(const Proof::NetworkDataEntitySP &other);

    NetworkDataEntitySP selfPtr() const;

    template <typename Entity>
    QSharedPointer<Entity> castedSelfPtr() const
    {
        auto self = selfPtr();
        return self ? qSharedPointerCast<Entity>(self) : QSharedPointer<Entity>();
    }

    static void initSelfWeakPtr(const NetworkDataEntitySP &entity);
};
} // namespace Proof

#endif // ENTITY_H
