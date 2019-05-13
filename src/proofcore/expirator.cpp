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
#include "proofcore/expirator.h"

#include "proofcore/proofobject_p.h"

#include <QMap>
#include <QMutex>
#include <QSet>
#include <QThread>
#include <QTimer>
#include <QTimerEvent>

namespace Proof {
class ExpiratorPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(Expirator)
    QMap<QDateTime, QSet<QSharedPointer<ProofObject>>> m_controlledObjects;
    QMutex *m_mutex = nullptr;
    QThread *m_thread = nullptr;
    int m_timerId = 0;
};
} // namespace Proof

using namespace Proof;

Expirator::Expirator() : ProofObject(*new ExpiratorPrivate)
{
    Q_D(Expirator);
    d->m_mutex = new QMutex();
    d->m_thread = new QThread(this);
    moveToThread(d->m_thread);
    d->m_thread->start();

    setCleanupInterval(60 * 10);
}

Expirator::~Expirator()
{
    Q_D(Expirator);
    if (d->m_timerId)
        call(this, &QObject::killTimer, Call::BlockEvents, d->m_timerId);
    d->m_thread->quit();
    if (!d->m_thread->wait(500))
        d->m_thread->terminate();
}

Expirator *Expirator::instance()
{
    static Expirator inst;
    return &inst;
}

void Expirator::addObject(const QSharedPointer<ProofObject> &object, QDateTime expirationTime)
{
    expirationTime.setTime(
        QTime(expirationTime.time().hour(), expirationTime.time().minute(), (expirationTime.time().second() / 5) * 5));
    expirationTime = expirationTime.addSecs(5);
    Q_D(Expirator);
    QMutexLocker lock(d->m_mutex);
    qCDebug(proofCoreCacheLog) << "Cache expirator adding object up to " << expirationTime;
    d->m_controlledObjects[expirationTime].insert(object);
}

void Expirator::setCleanupInterval(int seconds)
{
    if (safeCall(this, &Expirator::setCleanupInterval, seconds))
        return;
    Q_D(Expirator);
    if (d->m_timerId)
        killTimer(d->m_timerId);
    d->m_timerId = startTimer(1000 * seconds, Qt::VeryCoarseTimer);
    qCDebug(proofCoreCacheLog) << "Cache expirator timer started";
}

void Expirator::timerEvent(QTimerEvent *ev)
{
    Q_D(Expirator);
    if (ev->timerId() != d->m_timerId)
        return;
    QMutexLocker lock(d->m_mutex);
    for (auto it = d->m_controlledObjects.begin(); it != d->m_controlledObjects.end();) {
        if (it.key() > QDateTime::currentDateTime())
            break;
        it = d->m_controlledObjects.erase(it);
    }
}
