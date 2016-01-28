#include "errormessagesregistry_p.h"

namespace Proof {

ErrorMessagesRegistry::ErrorMessagesRegistry(std::initializer_list<ErrorInfo> list)
{
    for (const auto &info : list) {
        Q_ASSERT_X(!m_infos.contains(info.proofErrorCode), "ErrorMessagesRegistry", "Error codes must be unique");
        m_infos[info.proofErrorCode] = info;
    }
}

ErrorInfo ErrorMessagesRegistry::infoForCode(int code, const QStringList &args) const
{
    if (m_infos.contains(code)) {
        ErrorInfo info = m_infos[code];
        if (args.size() > 0) {
            for (int i = 0; i < args.size(); i++)
                info.message = info.message.arg(args[i]);
        }
        return info;
    } else {
        return ErrorInfo{0, 0, QObject::tr("Unknown error"), true};
    }
}

} // namespace Proof

