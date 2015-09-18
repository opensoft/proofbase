#include "errormessagesregistry_p.h"

namespace Proof {

QString ErrorMessagesRegistry::messageForCode(int code, const QStringList &args) const
{
    if (m_messages.contains(code)) {
        QString message = m_messages[code];
        if (args.size() > 0) {
            for (int i = 0; i < args.size(); i++)
                message = message.arg(i);
        }
        return message;
    } else {
        return QObject::tr("Unknown error");
    }
}

} // namespace Proof

