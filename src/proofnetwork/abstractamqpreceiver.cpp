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
#include "proofnetwork/abstractamqpreceiver.h"

#include "proofnetwork/abstractamqpreceiver_p.h"

using namespace Proof;

AbstractAmqpReceiver::AbstractAmqpReceiver(AbstractAmqpReceiverPrivate &dd, QObject *parent)
    : AbstractAmqpClient(dd, parent)
{}

QString AbstractAmqpReceiver::queueName() const
{
    Q_D_CONST(AbstractAmqpReceiver);
    return d->queueName;
}

void AbstractAmqpReceiver::setQueueName(const QString &queueName)
{
    Q_D(AbstractAmqpReceiver);
    d->queueName = queueName;
}

QString AbstractAmqpReceiver::newQueueBindingExchangeName() const
{
    Q_D_CONST(AbstractAmqpReceiver);
    return d->newQueueBindingExchangeName;
}

void AbstractAmqpReceiver::setNewQueueBindingExchangeName(const QString &exchangeName)
{
    Q_D(AbstractAmqpReceiver);
    d->newQueueBindingExchangeName = exchangeName;
}

QStringList AbstractAmqpReceiver::newQueueBindingRoutingKeys() const
{
    Q_D_CONST(AbstractAmqpReceiver);
    return d->newQueueBindingRoutingKeys;
}

void AbstractAmqpReceiver::setNewQueueBindingRoutingKeys(const QStringList &routingKeys)
{
    Q_D(AbstractAmqpReceiver);
    d->newQueueBindingRoutingKeys = routingKeys;
}

bool AbstractAmqpReceiver::createQueueIfNotExists() const
{
    Q_D_CONST(AbstractAmqpReceiver);
    return d->createdQueueIfNotExists;
}

QAmqpQueue::QueueOptions AbstractAmqpReceiver::queueOptions() const
{
    Q_D_CONST(AbstractAmqpReceiver);
    return d->queueOptions;
}

void AbstractAmqpReceiver::setCreateQueueIfNotExists(bool createQueueIfNotExists, QAmqpQueue::QueueOptions options)
{
    Q_D(AbstractAmqpReceiver);
    d->createdQueueIfNotExists = createQueueIfNotExists;
    d->queueOptions = options;
}

AbstractAmqpReceiverPrivate::AbstractAmqpReceiverPrivate() : AbstractAmqpClientPrivate()
{}

bool AbstractAmqpReceiverPrivate::startConsuming(QAmqpQueue *queue)
{
    Q_Q(AbstractAmqpReceiver);
    QObject::connect(queue, &QAmqpQueue::messageReceived, q, [this]() { amqpMessageReceived(); });
    bool consumeStarted = queue->consume(QAmqpQueue::coNoAck);
    if (consumeStarted) {
        queueState = QueueState::Consuming;
        emit q->connected();
    }
    return consumeStarted;
}

void AbstractAmqpReceiverPrivate::connected()
{
    Q_Q(AbstractAmqpReceiver);
    auto newQueue = rabbitClient->createQueue(queueName);
    queueState = QueueState::Opening;

    if (newQueue != queue) {
        qCDebug(proofNetworkAmqpLog) << "Create queue: " << queueName;
        queue = newQueue;
        QObject::connect(queue, static_cast<void (QAmqpQueue::*)(QAMQP::Error)>(&QAmqpQueue::error), q,
                         [this, q](QAMQP::Error error) {
                             if ((queueState == QueueState::Declared) && (error == QAMQP::PreconditionFailedError)
                                 && createdQueueIfNotExists) {
                                 queueState = QueueState::Reopening;
                                 queue->reset();
                                 queue->reopen();
                             } else {
                                 qCWarning(proofNetworkAmqpLog) << "Queue Error:" << error;
                                 emit q->errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::InternalError,
                                                       QStringLiteral("Queue Error: %1").arg(error), false);
                             }
                         });

        QObject::connect(queue, &QAmqpQueue::declared, q, [this]() {
            queueState = QueueState::Declared;
            startConsuming(queue);
            if (!newQueueBindingExchangeName.isEmpty()) {
                for (const auto &key : qAsConst(newQueueBindingRoutingKeys))
                    queue->bind(newQueueBindingExchangeName, key);
            }
        });

        QObject::connect(queue, &QAmqpQueue::opened, q, [this, q]() {
            qCDebug(proofNetworkAmqpLog) << "Queue opened " << q->sender();
            if (createdQueueIfNotExists && queueState == QueueState::Opening) {
                queueState = QueueState::Declared;
                queue->declare(queueOptions);
            } else {
                startConsuming(queue);
            }
        });
    }
}
