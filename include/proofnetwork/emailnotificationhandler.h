#ifndef PROOF_EMAILNOTIFICATIONHANDLER_H
#define PROOF_EMAILNOTIFICATIONHANDLER_H

#include "proofcore/abstractnotificationhandler.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QString>
#include <QStringList>

namespace Proof {

class EmailNotificationHandlerPrivate;
class PROOF_NETWORK_EXPORT EmailNotificationHandler : public AbstractNotificationHandler
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(EmailNotificationHandler)
public:
    EmailNotificationHandler(const SmtpClientSP &smtpClient, const QString &from, const QStringList &to, const QString &appId);
    void notify(const QString &message) override;

    static QString id();
};

} // namespace Proof

#endif // PROOF_EMAILNOTIFICATIONHANDLER_H
