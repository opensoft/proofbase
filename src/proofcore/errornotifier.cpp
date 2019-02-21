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
#include "proofcore/errornotifier.h"

#include "proofcore/abstractnotificationhandler.h"
#include "proofcore/proofobject_p.h"

#include <QMutex>

namespace Proof {

using StartBuffer = QVector<std::tuple<QString, ErrorNotifier::Severity, QString>>;

class ErrorNotifierPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(ErrorNotifier)
    QHash<QString, AbstractNotificationHandler *> handlers;
    StartBuffer startBuffer;
    QMutex mutex;
};
} // namespace Proof

using namespace Proof;

ErrorNotifier::ErrorNotifier() : ProofObject(*new ErrorNotifierPrivate)
{}

ErrorNotifier::~ErrorNotifier()
{
    Q_D(ErrorNotifier);
    d->mutex.lock();
    qDeleteAll(d->handlers);
    d->handlers.clear();
    d->mutex.unlock();
}

ErrorNotifier *ErrorNotifier::instance()
{
    static ErrorNotifier inst;
    return &inst;
}

void ErrorNotifier::notify(const QString &message, Severity severity, const QString &packId)
{
    Q_D(ErrorNotifier);
    if (safeCall(this, &ErrorNotifier::notify, message, severity, packId))
        return;
    d->mutex.lock();
    if (d->handlers.isEmpty())
        d->startBuffer << std::make_tuple(message, severity, packId);
    for (AbstractNotificationHandler *handler : qAsConst(d->handlers))
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
