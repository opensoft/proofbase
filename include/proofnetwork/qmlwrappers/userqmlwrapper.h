#ifndef USERQMLWRAPPER_H
#define USERQMLWRAPPER_H

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/qmlwrappers/networkdataentityqmlwrapper.h"

namespace Proof {
class UserQmlWrapperPrivate;
class PROOF_NETWORK_EXPORT UserQmlWrapper : public NetworkDataEntityQmlWrapper
{
    Q_OBJECT
    Q_PROPERTY(QString userName READ userName NOTIFY userNameChanged)
    Q_PROPERTY(QString fullName READ fullName NOTIFY fullNameChanged)
    Q_PROPERTY(QString email READ email NOTIFY emailChanged)
    Q_DECLARE_PRIVATE(UserQmlWrapper)
public:
    explicit UserQmlWrapper(const UserSP &user, QObject *parent = nullptr);
    PROOF_NDE_WRAPPER_TOOLS(User)

    QString userName() const;
    QString fullName() const;
    QString email() const;

signals:
    void userNameChanged(const QString &userName);
    void fullNameChanged(const QString &fullName);
    void emailChanged(const QString &email);

protected:
    explicit UserQmlWrapper(const UserSP &user, UserQmlWrapperPrivate &dd, QObject *parent = nullptr);
    void setupEntity(const QSharedPointer<NetworkDataEntity> &old = QSharedPointer<NetworkDataEntity>()) override;
};
} // namespace Proof

#endif // USERQMLWRAPPER_H
