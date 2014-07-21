#ifndef USERQMLWRAPPER_H
#define USERQMLWRAPPER_H

#include "proofnetwork/qmlwrappers/networkdataentityqmlwrapper.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"

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
    explicit UserQmlWrapper(const UserSP &user, QObject *parent = 0);

    QString userName() const;
    QString fullName() const;
    QString email() const;

signals:
    void userNameChanged(const QString &arg);
    void fullNameChanged(const QString &arg);
    void emailChanged(const QString &arg);

protected:
    explicit UserQmlWrapper(const UserSP &user, UserQmlWrapperPrivate &dd, QObject *parent = 0);
};
}

#endif // USERQMLWRAPPER_H
