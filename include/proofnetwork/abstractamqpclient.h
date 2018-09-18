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
#ifndef QABSTRACTAMQPCLIENT_H
#define QABSTRACTAMQPCLIENT_H

#include "proofcore/proofobject.h"

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QSslConfiguration>

namespace Proof {

class AbstractAmqpClientPrivate;
class PROOF_NETWORK_EXPORT AbstractAmqpClient : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractAmqpClient)
public:
    quint16 port() const;
    void setPort(quint16 port);

    QString host() const;
    void setHost(const QString &host);

    QString virtualHost() const;
    void setVirtualHost(const QString &virtualHost);

    QString userName() const;
    void setUserName(const QString &userName);

    QString password() const;
    void setPassword(const QString &password);

    bool autoReconnect() const;
    void setAutoReconnect(bool value);

    qint16 channelMax() const;
    void setChannelMax(qint16 channelMax);

    qint32 frameMax() const;
    void setFrameMax(qint32 frameMax);

    qint16 heartbeatDelay() const;
    void setHeartbeatDelay(qint16 delay);

    QSslConfiguration sslConfiguration() const;
    void setSslConfiguration(const QSslConfiguration &config);

    bool isConnected() const;

    void connectToHost();
    void disconnectFromHost();

signals:
    void connected();
    void disconnected();

protected:
    AbstractAmqpClient(AbstractAmqpClientPrivate &dd, QObject *parent = nullptr);
};

} // namespace Proof

#endif // QABSTRACTAMQPCLIENT_H
