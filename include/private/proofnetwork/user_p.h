#ifndef USER_P_H
#define USER_P_H

#include "proofnetwork/networkdataentity_p.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/user.h"

namespace Proof {
class PROOF_NETWORK_EXPORT UserPrivate : public NetworkDataEntityPrivate
{
    Q_DECLARE_PUBLIC(User)
public:
    explicit UserPrivate(const QString &userName);

    void setUserName(const QString &arg);
    void setFullName(const QString &arg);
    void setEmail(const QString &arg);

    QString userName;
    QString fullName;
    QString email;
};
} // namespace Proof

#endif // USER_P_H
