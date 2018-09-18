/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "proofcore/memorystoragenotificationhandler.h"

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
    connect(d->cleanupTimer, &QTimer::timeout, this, [d]() {
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
    Q_D_CONST(MemoryStorageNotificationHandler);
    QMutexLocker locker(&d->mutex);
    return d->messages;
}

QPair<QDateTime, QString> MemoryStorageNotificationHandler::lastMessage() const
{
    Q_D_CONST(MemoryStorageNotificationHandler);
    QMutexLocker locker(&d->mutex);
    return d->lastMessage;
}

void MemoryStorageNotificationHandler::notify(const QString &message, ErrorNotifier::Severity severity,
                                              const QString &packId)
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
