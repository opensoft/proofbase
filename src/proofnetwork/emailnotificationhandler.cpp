#include "emailnotificationhandler.h"

#include "smtpclient.h"

#include "proofcore/abstractnotificationhandler_p.h"
#include "proofcore/proofglobal.h"

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

    QString fullMessage = QString("Application: %1 (%2)\n"
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
