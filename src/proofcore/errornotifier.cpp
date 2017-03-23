#include "errornotifier.h"

#include "proofobject_p.h"

#include "abstractnotificationhandler.h"

#include <QMutex>

namespace Proof {

using StartBuffer = QList<std::tuple<QString, ErrorNotifier::Severity, QString>>;

class ErrorNotifierPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(ErrorNotifier)
    QHash<QString, AbstractNotificationHandler *> handlers;
    StartBuffer startBuffer;
    QMutex mutex;
};
} // namespace Proof

using namespace Proof;

ErrorNotifier::ErrorNotifier()
    : ProofObject(*new ErrorNotifierPrivate)
{
}

ErrorNotifier::~ErrorNotifier()
{
    Q_D(ErrorNotifier);
    for (auto handler : d->handlers.values())
        delete handler;
}

ErrorNotifier *ErrorNotifier::instance()
{
    static ErrorNotifier inst;
    return &inst;
}

void ErrorNotifier::notify(const QString &message, Severity severity, const QString &packId)
{
    Q_D(ErrorNotifier);
    if (call(this, &ErrorNotifier::notify, message, severity, packId))
        return;
    d->mutex.lock();
    if (d->handlers.isEmpty())
        d->startBuffer << std::make_tuple(message, severity, packId);
    for (auto handler : d->handlers.values())
        handler->notify(message, severity, packId);
    d->mutex.unlock();
}

void ErrorNotifier::registerHandler(const QString &handlerId, AbstractNotificationHandler *handler)
{
    Q_D(ErrorNotifier);
    bool fireStartBuffer = d->handlers.isEmpty();
    unregisterHandler(handlerId);
    d->mutex.lock();
    d->handlers.insert(handlerId, handler);
    if (fireStartBuffer) {
        StartBuffer startBufferCopy = d->startBuffer;
        d->startBuffer.clear();
        d->mutex.unlock();
        for (const auto &message : startBufferCopy)
            notify(std::get<0>(message), std::get<1>(message), std::get<2>(message));
    } else {
        d->mutex.unlock();
    }
}

void ErrorNotifier::unregisterHandler(const QString &handlerId)
{
    Q_D(ErrorNotifier);
    if (!d->handlers.contains(handlerId))
        return;
    d->mutex.lock();
    delete d->handlers.take(handlerId);
    d->mutex.unlock();
}

AbstractNotificationHandler *ErrorNotifier::handler(const QString &handlerId)
{
    Q_D(ErrorNotifier);
    return d->handlers.value(handlerId, nullptr);
}
