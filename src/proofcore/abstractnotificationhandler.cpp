#include "abstractnotificationhandler.h"
#include "abstractnotificationhandler_p.h"

using namespace Proof;

AbstractNotificationHandler::AbstractNotificationHandler(AbstractNotificationHandlerPrivate &dd, const QString &appId)
    : ProofObject(dd)
{
    Q_D(AbstractNotificationHandler);
    d->appId = appId;
}

QString AbstractNotificationHandler::appId() const
{
    Q_D(const AbstractNotificationHandler);
    return d->appId;
}

