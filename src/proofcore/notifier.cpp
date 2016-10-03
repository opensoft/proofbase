#include "notifier.h"

#include "proofobject_p.h"

namespace Proof {

class NotifierPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(Notifier)
    QHash<QString, AbstractNotificationHandler *> handlers;
    QStringList startBuffer;
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
    if (d->handlers.isEmpty())
        d->startBuffer << message;
    for (auto handler : d->handlers.values())
        handler->notify(message);
}

void Notifier::registerHandler(const QString &handlerId, AbstractNotificationHandler *handler)
{
    Q_D(Notifier);
    bool fireStartBuffer = d->handlers.isEmpty();
    unregisterHandler(handlerId);
    d->handlers.insert(handlerId, handler);
    if (fireStartBuffer) {
        for (const auto &message : d->startBuffer)
            notify(message);
        d->startBuffer.clear();
    }
}

void Notifier::unregisterHandler(const QString &handlerId)
{
    Q_D(Notifier);
    if (!d->handlers.contains(handlerId))
        return;
    delete d->handlers.take(handlerId);
}

AbstractNotificationHandler *Notifier::handler(const QString &handlerId)
{
    Q_D(Notifier);
    return d->handlers.value(handlerId, nullptr);
}
