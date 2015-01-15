#ifndef PROOFOBJECT_H
#define PROOFOBJECT_H

#include "proofcore/proofcore_global.h"

#include <QObject>
#include <QScopedPointer>

namespace Proof {

class ProofObjectPrivate;

class PROOF_CORE_EXPORT ProofObject : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ProofObject)
public:
    explicit ProofObject(QObject *parent);
    ~ProofObject();

protected:
    ProofObject(ProofObjectPrivate &dd, QObject *parent = 0);
    QScopedPointer<ProofObjectPrivate> d_ptr;

private:
    ProofObject() = delete;
};

}

#endif // PROOFOBJECT_H
