#ifndef PROOFOBJECT_H
#define PROOFOBJECT_H

#include "proofcore_global.h"

#include <QObject>
#include <QScopedPointer>

namespace Proof {

class ProofObjectPrivate;

class PROOF_CORE_EXPORT ProofObject : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ProofObject)
public:
    explicit ProofObject(QObject *parent = 0);
    ~ProofObject();

protected:
    ProofObject(ProofObjectPrivate &dd, QObject *parent = 0);
    QScopedPointer<ProofObjectPrivate> d_ptr;
};

}

#endif // PROOFOBJECT_H
