#include "notifier.h"

#include "proofobject_p.h"

#include <QMutex>

namespace Proof {

class NotifierPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(Notifier)
    QHash<QString, AbstractNotificationHandler *> handlers;
    QStringList startBuffer;
    QMutex mutex;
};
} // namespace Proof

using namespace Proof;

Notifier::Notifier()
    : ProofObject(*new NotifierPrivate)
{
}

Notifier::~Notifier()
{
    Q_D(Notifier);
    for (auto handler : d->handlers.values())
        delete handler;
}

Notifier *Notifier::instance()
{
    static Notifier inst;
    return &inst;
}

void Notifier::notify(const QString &message)
{
    Q_D(Notifier);
    if (call(this, &Notifier::notify, message))
        return;
    d->mutex.lock();
    if (d->handlers.isEmpty())
        d->startBuffer << message;
    for (auto handler : d->handlers.values())
        handler->notify(message);
    d->mutex.unlock();
}

void Notifier::registerHandler(const QString &handlerId, AbstractNotificationHandler *handler)
{
    Q_D(Notifier);
    bool fireStartBuffer = d->handlers.isEmpty();
    unregisterHandler(handlerId);
    d->mutex.lock();
    d->handlers.insert(handlerId, handler);
    d->mutex.unlock();
    if (fireStartBuffer) {
        d->mutex.lock();
        QStringList startBufferCopy = d->startBuffer;
        d->startBuffer.clear();
        d->mutex.unlock();
        for (const auto &message : startBufferCopy)
            notify(message);
    }
}

void Notifier::unregisterHandler(const QString &handlerId)
{
    Q_D(Notifier);
    if (!d->handlers.contains(handlerId))
        return;
    d->mutex.lock();
    delete d->handlers.take(handlerId);
    d->mutex.unlock();
}

AbstractNotificationHandler *Notifier::handler(const QString &handlerId)
{
    Q_D(Notifier);
    return d->handlers.value(handlerId, nullptr);
}
