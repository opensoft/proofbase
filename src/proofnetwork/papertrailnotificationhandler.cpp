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
#include "proofnetwork/papertrailnotificationhandler.h"

#include "proofcore/abstractnotificationhandler_p.h"

#include <QDateTime>
#include <QTcpSocket>

namespace Proof {

class PapertrailNotificationHandlerPrivate : public AbstractNotificationHandlerPrivate
{
    Q_DECLARE_PUBLIC(PapertrailNotificationHandler)
    QTcpSocket *papertrailClient = nullptr;

    QString host;
    quint16 port = 0;
    QString senderName;

    QHash<QString, QDateTime> packTimestamps;
};

} // namespace Proof

using namespace Proof;

PapertrailNotificationHandler::PapertrailNotificationHandler(const QString &host, quint16 port,
                                                             const QString &senderName, const QString &appId)
    : AbstractNotificationHandler(*new PapertrailNotificationHandlerPrivate, appId)
{
    Q_D(PapertrailNotificationHandler);
    d->host = host;
    d->port = port;
    d->senderName = senderName;
    d->papertrailClient = new QTcpSocket(this);
    d->papertrailClient->setSocketOption(QAbstractSocket::SocketOption::KeepAliveOption, 1);

    connect(d->papertrailClient, &QTcpSocket::connected, this, [host, port]() {
        qCDebug(proofNetworkMiscLog) << QStringLiteral("connected to papertrail %1:%2").arg(host).arg(port);
    });
    connect(d->papertrailClient, &QTcpSocket::disconnected, this, [d]() {
        qCDebug(proofNetworkMiscLog) << QStringLiteral("disconnected from papertrail %1:%2 %3")
                                            .arg(d->host)
                                            .arg(d->port)
                                            .arg(d->papertrailClient->errorString());
    });
    connect(d->papertrailClient, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this,
            [d]() { d->papertrailClient->close(); });
    d->papertrailClient->connectToHost(d->host, d->port);
}

void PapertrailNotificationHandler::notify(const QString &message, ErrorNotifier::Severity severity, const QString &packId)
{
    Q_D(PapertrailNotificationHandler);
    if (safeCall(this, &PapertrailNotificationHandler::notify, message, severity, packId))
        return;

    switch (d->papertrailClient->state()) {
    case QTcpSocket::ClosingState:
        d->papertrailClient->close();
        [[fallthrough]];
    case QTcpSocket::UnconnectedState:
        d->papertrailClient->connectToHost(d->host, d->port);
        break;
    default:
        break;
    }

    QString severityName;
    switch (severity) {
    case ErrorNotifier::Severity::Error:
        severityName = QStringLiteral("ERROR");
        break;
    case ErrorNotifier::Severity::Critical:
        severityName = QStringLiteral("CRITICAL ERROR");
        break;
    case ErrorNotifier::Severity::Warning:
        severityName = QStringLiteral("WARNING");
        break;
    default:
        severityName = QStringLiteral("INFO");
        break;
    }

    QString fullMessage = QStringLiteral("%1 %2 %3 %4 | %5\r\n")
                              .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ss.zzz")),
                                   d->senderName, appId(), severityName, message);

    d->papertrailClient->write(fullMessage.toLocal8Bit());
}

QString PapertrailNotificationHandler::id()
{
    return QStringLiteral("PapertrailNotificationHandler");
}
