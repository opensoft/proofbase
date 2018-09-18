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
#include "proofnetwork/emailnotificationhandler.h"

#include "proofcore/abstractnotificationhandler_p.h"
#include "proofcore/proofglobal.h"

#include "proofnetwork/smtpclient.h"

#include <QDateTime>
#include <QNetworkInterface>
#include <QSysInfo>

const static int SAME_PACK_TIMEOUT = 1000 * 60 * 60; //1 hour

namespace Proof {

class EmailNotificationHandlerPrivate : public AbstractNotificationHandlerPrivate
{
    Q_DECLARE_PUBLIC(EmailNotificationHandler)
    SmtpClientSP smtpClient;

    QString from;
    QStringList to;

    QHash<QString, QDateTime> packTimestamps;
};

} // namespace Proof

using namespace Proof;

EmailNotificationHandler::EmailNotificationHandler(const SmtpClientSP &smtpClient, const QString &from,
                                                   const QStringList &to, const QString &appId)
    : AbstractNotificationHandler(*new EmailNotificationHandlerPrivate, appId)
{
    Q_D(EmailNotificationHandler);
    d->smtpClient = smtpClient;
    d->from = QStringLiteral("%1<%2>").arg(qApp->applicationName(), from);
    d->to = to;
}

void EmailNotificationHandler::notify(const QString &message, ErrorNotifier::Severity severity, const QString &packId)
{
    Q_D(EmailNotificationHandler);

    if (severity == ErrorNotifier::Severity::Warning)
        return;
    if (!packId.isEmpty()) {
        if (d->packTimestamps.contains(packId)
            && d->packTimestamps[packId].msecsTo(QDateTime::currentDateTime()) < SAME_PACK_TIMEOUT)
            return;
        d->packTimestamps[packId] = QDateTime::currentDateTime();
    }

    QString severityName;
    switch (severity) {
    case ErrorNotifier::Severity::Error:
        severityName = QStringLiteral("Error");
        break;
    case ErrorNotifier::Severity::Critical:
        severityName = QStringLiteral("Critical error");
        break;
    case ErrorNotifier::Severity::Warning: //will never be used, added to supress warning
        severityName = QStringLiteral("Warning");
        break;
    }

    QString subject = QStringLiteral("%1 at %2").arg(severityName, qApp->applicationName());
    if (!d->appId.isEmpty())
        subject += QStringLiteral(" (%1)").arg(d->appId);
    QStringList ipsList;
    const auto allIfaces = QNetworkInterface::allInterfaces();
    for (const auto &interface : allIfaces) {
        const auto addressEntries = interface.addressEntries();
        for (const auto &address : addressEntries) {
            auto ip = address.ip();
            if (ip.isLoopback())
                continue;

            ipsList << QStringLiteral("%1 (%2)").arg(ip.toString(), interface.humanReadableName());
        }
    }

    QString fullMessage = QStringLiteral("Application: %1 (%2)\n"
                                         "Version: %3\n"
                                         "Proof version: %4\n"
                                         "OS: %5\n"
                                         "IP: %6\n"
                                         "Time: %7\n\n"
                                         "%8")
                              .arg(qApp->applicationName(), d->appId, qApp->applicationVersion(), Proof::proofVersion(),
                                   QSysInfo::prettyProductName(), ipsList.join(QStringLiteral("; ")),
                                   QDateTime::currentDateTime().toString(Qt::ISODate), message);
    d->smtpClient->sendTextMail(subject, fullMessage, d->from, d->to);
}

QString EmailNotificationHandler::id()
{
    return QStringLiteral("EmailNotificationHandler");
}
