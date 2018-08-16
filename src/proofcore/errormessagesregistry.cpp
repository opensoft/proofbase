#include "proofseed/proofalgorithms.h"

#include "proofcore/errormessagesregistry_p.h"

namespace Proof {

ErrorMessagesRegistry::ErrorMessagesRegistry(std::initializer_list<ErrorInfo> list)
{
    for (const auto &info : list) {
        Q_ASSERT_X(!m_infos.contains(info.proofErrorCode), "ErrorMessagesRegistry", "Error codes must be unique");
        m_infos[info.proofErrorCode] = info;
    }
}

ErrorInfo ErrorMessagesRegistry::infoForCode(int code, const QVector<QString> &args) const
{
    if (m_infos.contains(code)) {
        ErrorInfo info = m_infos[code];
        info.message = algorithms::reduce(args, [](const QString &acc, const QString &x) { return acc.arg(x); },
                                          info.message);
        return info;
    } else {
        return ErrorInfo{0, 0, QObject::tr("Unknown error"), true};
    }
}

} // namespace Proof
