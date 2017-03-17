#ifndef NETWORKDATAENTITYQMLWRAPPER_P_H
#define NETWORKDATAENTITYQMLWRAPPER_P_H

#include "proofcore/proofobject_p.h"
#include "proofnetwork/networkdataentity.h"
#include "proofnetwork/proofnetwork_global.h"

#include <QtGlobal>
#include <QSharedPointer>

namespace Proof {
class PROOF_NETWORK_EXPORT NetworkDataEntityQmlWrapperPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(NetworkDataEntityQmlWrapper)
public:
    NetworkDataEntityQmlWrapperPrivate() : ProofObjectPrivate() {}
    QSharedPointer<NetworkDataEntity> dataEntity;
    QObject *lambdaConnectContext = nullptr;

    template<class T>
    QSharedPointer<T> entity()
    {
        return qSharedPointerCast<T>(dataEntity);
    }

    template<class T>
    const QSharedPointer<T> entity() const
    {
        return qSharedPointerCast<T>(dataEntity);
    }
};

}

#endif // NETWORKDATAENTITYQMLWRAPPER_P_H
