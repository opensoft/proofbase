#ifndef PROOF_ERRORMESSAGESREGISTRY_H
#define PROOF_ERRORMESSAGESREGISTRY_H

#include "proofnetwork/proofnetwork_global.h"

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

class ErrorMessagesRegistryPrivate;
class PROOF_NETWORK_EXPORT ErrorMessagesRegistry final
{
    Q_DECLARE_PRIVATE(ErrorMessagesRegistry)
public:
    ErrorMessagesRegistry(std::initializer_list<ErrorInfo> &&list);
    ~ErrorMessagesRegistry();
    ErrorInfo infoForCode(int code, const QVector<QString> &args = QVector<QString>()) const;

private:
    QScopedPointer<ErrorMessagesRegistryPrivate> d_ptr;
};

} // namespace Proof

#endif // PROOF_ERRORMESSAGESREGISTRY_H
