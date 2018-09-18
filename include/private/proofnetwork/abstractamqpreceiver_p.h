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
#ifndef ABSTRACTAMQPRECEIVER_P_H
#define ABSTRACTAMQPRECEIVER_P_H

#include "proofnetwork/abstractamqpclient_p.h"
#include "proofnetwork/abstractamqpreceiver.h"
#include "proofnetwork/proofnetwork_global.h"

#include "3rdparty/qamqp/src/qamqpqueue.h"

namespace Proof {
class PROOF_NETWORK_EXPORT AbstractAmqpReceiverPrivate : public AbstractAmqpClientPrivate
{
    Q_DECLARE_PUBLIC(AbstractAmqpReceiver)

public:
    AbstractAmqpReceiverPrivate();

    void connected() override;
    virtual void amqpMessageReceived() = 0;

    enum class QueueState
    {
        Opening,
        Error,
        Reopening,
        Declared,
        Consuming
    };

    QAmqpQueue *queue = nullptr;
    QString queueName;
    QString newQueueBindingExchangeName;
    QStringList newQueueBindingRoutingKeys;
    bool createdQueueIfNotExists = false;
    QAmqpQueue::QueueOptions queueOptions = {QAmqpQueue::Durable, QAmqpQueue::AutoDelete};

    QueueState queueState = QueueState::Error;

    bool startConsuming(QAmqpQueue *queue);
};
} // namespace Proof
#endif // ABSTRACTAMQPRECEIVER_P_H
