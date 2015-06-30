#ifndef USER_P_H
#define USER_P_H

#include "networkdataentity_p.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {
class PROOF_NETWORK_EXPORT UserPrivate : public NetworkDataEntityPrivate
{
    Q_DECLARE_PUBLIC(User)
public:
    explicit UserPrivate(const QString &userName);
    void updateFrom(const Proof::NetworkDataEntitySP &other) override;

    void setUserName(const QString &arg);
    void setFullName(const QString &arg);
    void setEmail(const QString &arg);

    QString userName;
    QString fullName;
    QString email;
};
}

#endif // USER_P_H
