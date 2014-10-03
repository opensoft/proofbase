#include "expirator.h"

#include "proofobject_p.h"

#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QTimerEvent>
#include <QSet>
#include <QMultiMap>

namespace Proof {
class ExpiratorPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(Expirator)
    QMultiMap<QDateTime, QSharedPointer<ProofObject>> m_controlledObjects;
    QMutex *m_mutex;
    QThread *m_thread;
    int m_timerId = 0;
};
}

using namespace Proof;

Expirator::Expirator()
    : ProofObject(*new ExpiratorPrivate)
{
    Q_D(Expirator);
    d->m_mutex = new QMutex();
    d->m_thread = new QThread(this);
    moveToThread(d->m_thread);
    d->m_thread->start();

    QTimer *timer = new QTimer();
    connect(timer, &QTimer::timeout, this, [this, timer](){
        Q_D(Expirator);
        d->m_timerId = startTimer(1000 * 60 * 10, Qt::VeryCoarseTimer);
        timer->deleteLater();
    }, Qt::QueuedConnection);
    timer->setSingleShot(true);
    timer->start();
}

Expirator::~Expirator()
{
    Q_D(Expirator);
    d->m_thread->quit();
    d->m_thread->wait(500);
    d->m_thread->terminate();
}

Expirator *Expirator::instance()
{
    static Expirator *inst = nullptr;
    if (!inst)
        inst = new Expirator;
    return inst;
}

void Expirator::addObject(const QSharedPointer<ProofObject> &object, const QDateTime &expirationTime)
{
    Q_D(Expirator);
    d->m_mutex->lock();
    d->m_controlledObjects.insertMulti(expirationTime, object);
    d->m_mutex->unlock();
}

void Expirator::timerEvent(QTimerEvent *ev)
{
    Q_D(Expirator);
    if (ev->timerId() != d->m_timerId)
        return;

    d->m_mutex->lock();
    QSet<QDateTime> toRemove;
    for (const QDateTime &time : d->m_controlledObjects.keys()) {
        if (time > QDateTime::currentDateTime())
            break;
        toRemove << time;
    }
    for (const QDateTime &time : toRemove)
        d->m_controlledObjects.remove(time);
    d->m_mutex->unlock();
}

