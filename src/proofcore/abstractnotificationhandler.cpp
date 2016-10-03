#include "abstractnotificationhandler.h"
#include "abstractnotificationhandler_p.h"

using namespace Proof;

AbstractNotificationHandler::AbstractNotificationHandler()
    : AbstractNotificationHandler(*new AbstractNotificationHandlerPrivate)
{
}

AbstractNotificationHandler::AbstractNotificationHandler(AbstractNotificationHandlerPrivate &dd)
    : ProofObject(dd)
{
}
