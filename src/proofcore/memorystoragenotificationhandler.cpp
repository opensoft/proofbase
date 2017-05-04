#include "memorystoragenotificationhandler.h"

#include "proofcore/abstractnotificationhandler_p.h"

#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

static const qlonglong MSECS_TO_KEEP = 1000 * 60 * 60 * 24; //24 hours

namespace Proof {
class MemoryStorageNotificationHandlerPrivate : public AbstractNotificationHandlerPrivate
{
    Q_DECLARE_PUBLIC(MemoryStorageNotificationHandler)

    QMultiMap<QDateTime, QString> messages;
    QPair<QDateTime, QString> lastMessage;
    mutable QMutex mutex;
    QTimer *cleanupTimer = nullptr;
};

} // namespace Proof

using namespace Proof;

MemoryStorageNotificationHandler::MemoryStorageNotificationHandler(const QString &appId)
    : AbstractNotificationHandler(*new MemoryStorageNotificationHandlerPrivate, appId)

{
    Q_D(MemoryStorageNotificationHandler);
    d->cleanupTimer = new QTimer(this);
    d->cleanupTimer->setInterval(1000 * 60 * 60);
    d->cleanupTimer->setTimerType(Qt::VeryCoarseTimer);
    connect(d->cleanupTimer, &QTimer::timeout, this, [this, d]() {
        QDateTime limiter = QDateTime::currentDateTimeUtc().addMSecs(-MSECS_TO_KEEP);
        d->mutex.lock();
        while (!d->messages.isEmpty() && d->messages.firstKey() < limiter)
            d->messages.remove(d->messages.firstKey());
        d->mutex.unlock();
    });
    d->cleanupTimer->start();
}

QMultiMap<QDateTime, QString> MemoryStorageNotificationHandler::messages() const
{
    Q_D(const MemoryStorageNotificationHandler);
    QMutexLocker locker(&d->mutex);
    return d->messages;
}

QPair<QDateTime, QString> MemoryStorageNotificationHandler::lastMessage() const
{
    Q_D(const MemoryStorageNotificationHandler);
    QMutexLocker locker(&d->mutex);
    return d->lastMessage;
}

void MemoryStorageNotificationHandler::notify(const QString &message, ErrorNotifier::Severity severity, const QString &packId)
{
    Q_UNUSED(packId)
    Q_UNUSED(severity)
    Q_D(MemoryStorageNotificationHandler);
    d->mutex.lock();
    d->lastMessage = qMakePair(QDateTime::currentDateTimeUtc(), message);
    d->messages.insert(d->lastMessage.first, message);
    d->mutex.unlock();
}

QString MemoryStorageNotificationHandler::id()
{
    return QStringLiteral("MemoryStorageNotificationHandler");
}
