#ifndef PROOF_ERRORMESSAGESREGISTRY_H
#define PROOF_ERRORMESSAGESREGISTRY_H

#include "proofcore/proofcore_global.h"

#include <QHash>
#include <QPair>
#include <QString>

#include <initializer_list>

namespace Proof {

struct ErrorInfo
{
    long proofModuleCode;
    long proofErrorCode;
    QString message;
    bool userFriendly;
};

class PROOF_CORE_EXPORT ErrorMessagesRegistry
{
public:
    ErrorMessagesRegistry(std::initializer_list<ErrorInfo> list);
    ErrorInfo infoForCode(int code, const QVector<QString> &args = QVector<QString>()) const;

private:
    QHash<long, ErrorInfo> m_infos;
};

} // namespace Proof

#endif // PROOF_ERRORMESSAGESREGISTRY_H
