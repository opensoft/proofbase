#ifndef EXPIRATOR_P_H
#define EXPIRATOR_P_H

#include "proofobject.h"

#include <QMultiMap>
#include <QDateTime>

class QThread;
class QMutex;

namespace Proof {

class ExpiratorPrivate;
class Expirator : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Expirator)
public:
    static Expirator *instance();
    void addObject(const QSharedPointer<ProofObject> &object, const QDateTime &expirationTime);

protected:
    void timerEvent(QTimerEvent *ev) override;

private:
    Expirator();
    ~Expirator();
    Expirator(const Expirator &other) = delete;
    Expirator &operator=(const Expirator &other) = delete;
    Expirator(const Expirator &&other) = delete;
    Expirator &operator=(const Expirator &&other) = delete;

};
}

#endif // EXPIRATOR_P_H
