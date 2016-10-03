#include "emailnotificationhandler.h"

#include "proofcore/abstractnotificationhandler_p.h"
#include "proofcore/proofglobal.h"
#include "smtpclient.h"

#include <QDateTime>
#include <QNetworkInterface>
#include <QSysInfo>

namespace Proof {

class EmailNotificationHandlerPrivate : public AbstractNotificationHandlerPrivate
{
    Q_DECLARE_PUBLIC(EmailNotificationHandler)
    SmtpClientSP smtpClient;

    QString from;
    QStringList to;
    QString appId;
};

} // namespace Proof

using namespace Proof;

EmailNotificationHandler::EmailNotificationHandler(const SmtpClientSP &smtpClient, const QString &from, const QStringList &to, const QString &appId)
    : AbstractNotificationHandler(*new EmailNotificationHandlerPrivate)
{
    Q_D(EmailNotificationHandler);
    d->smtpClient = smtpClient;
    d->from = QString("%1<%2>").arg(qApp->applicationName()).arg(from);
    d->to = to;
    d->appId = appId;
}

void EmailNotificationHandler::notify(const QString &message)
{
    Q_D(EmailNotificationHandler);
    QString subject = QString("Issue at %1").arg(qApp->applicationName());
    if (!d->appId.isEmpty())
        subject += QString(" (%1)").arg(d->appId);
    QStringList ipsList;
    for (const auto &interface : QNetworkInterface::allInterfaces()) {
        for (const auto &address : interface.addressEntries()) {
            auto ip = address.ip();
            if (ip.isLoopback())
                continue;

            ipsList << QString("%1 (%2)").arg(ip.toString(), interface.humanReadableName());
        }
    }
    QString fullMessage = QString("Application: %1 (%2)\n"
                                  "Version: %3\n"
                                  "Proof version: %4\n"
                                  "OS: %5\n"
                                  "IP: %6\n"
                                  "Time: %7\n\n"
                                  "%8")
            .arg(qApp->applicationName(), d->appId,
                 qApp->applicationVersion(), Proof::proofVersion(),
                 QSysInfo::prettyProductName(),
                 ipsList.join("; "),
                 QDateTime::currentDateTime().toString(Qt::ISODate),
                 message);
    d->smtpClient->sendMail(subject, fullMessage, d->from, d->to);
}

QString EmailNotificationHandler::id()
{
    return "EmailNotificationHandler";
}

