#include "proofnetwork/errormessagesregistry.h"

#include "proofseed/proofalgorithms.h"

namespace Proof {

class ErrorMessagesRegistryPrivate
{
    Q_DECLARE_PUBLIC(ErrorMessagesRegistry)
    ErrorMessagesRegistry *q_ptr;
    QHash<long, ErrorInfo> m_infos;
};

ErrorMessagesRegistry::ErrorMessagesRegistry(std::initializer_list<ErrorInfo> &&list)
    : d_ptr(new ErrorMessagesRegistryPrivate)
{
    d_ptr->q_ptr = this;

    for (const auto &info : list) {
        Q_ASSERT_X(!d_ptr->m_infos.contains(info.proofErrorCode), "ErrorMessagesRegistry", "Error codes must be unique");
        d_ptr->m_infos[info.proofErrorCode] = info;
    }
}

ErrorMessagesRegistry::~ErrorMessagesRegistry()
{}

ErrorInfo ErrorMessagesRegistry::infoForCode(int code, const QVector<QString> &args) const
{
    if (d_ptr->m_infos.contains(code)) {
        ErrorInfo info = d_ptr->m_infos[code];
        info.message = algorithms::reduce(args, [](const QString &acc, const QString &x) { return acc.arg(x); },
                                          info.message);
        return info;
    } else {
        return ErrorInfo{0, 0, QObject::tr("Unknown error"), true};
    }
}

} // namespace Proof
