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
#ifndef PROOF_SMTPCLIENT_H
#define PROOF_SMTPCLIENT_H

#include "proofcore/proofobject.h"

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QStringList>

namespace Proof {
class SmtpClientPrivate;
class PROOF_NETWORK_EXPORT SmtpClient : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SmtpClient)
public:
    enum class ConnectionType
    {
        Plain,
        Ssl,
        StartTls
    };
    Q_ENUM(ConnectionType)

    explicit SmtpClient();

    QString userName() const;
    void setUserName(const QString &arg);

    QString password() const;
    void setPassword(const QString &arg);

    QString host() const;
    void setHost(const QString &arg);

    int port() const;
    void setPort(int arg);

    ConnectionType connectionType() const;
    void setConnectionType(ConnectionType arg);

    void sendTextMail(const QString &subject, const QString &body, const QString &from, const QStringList &to,
                      const QStringList &cc = QStringList(), const QStringList &bcc = QStringList());

signals:
    void userNameChanged(const QString &arg);
    void passwordChanged(const QString &arg);
    void hostChanged(const QString &arg);
    void portChanged(int arg);
    void connectionTypeChanged(Proof::SmtpClient::ConnectionType arg);
};

} // namespace Proof

#endif // PROOF_SMTPCLIENT_H
