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
#include "proofnetwork/jsonamqpclient.h"

#include "proofnetwork/jsonamqpclient_p.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QThread>

using namespace Proof;

JsonAmqpClient::JsonAmqpClient(JsonAmqpClientPrivate &dd, QObject *parent) : AbstractAmqpReceiver(dd, parent)
{}

void JsonAmqpClientPrivate::amqpMessageReceived()
{
    //HACK: Workaround for bug in QAQMP, sometimes after qamqp reconnect, signal messageReceived has been emitted, but queue actually is empty, so we check it here.
    if (queue->isEmpty())
        return;
    QAmqpMessage message = queue->dequeue();
    if (message.isValid()) {
        QJsonDocument messageDocument = QJsonDocument::fromJson(message.payload());
        qCDebug(proofNetworkAmqpLog) << "Queue message: " << messageDocument;
        handleJsonMessage(messageDocument, message.routingKey(), message.headers());
    } else {
        qCDebug(proofNetworkAmqpLog) << "Queue message is not valid: " << message.payload();
    }
}

void JsonAmqpClientPrivate::handleJsonMessage(const QJsonDocument &json, const QString &routingKey,
                                              const QHash<QString, QVariant> &headers)
{
    Q_UNUSED(routingKey)
    Q_UNUSED(headers)
    Q_UNUSED(json)
    //Nothing there
}
