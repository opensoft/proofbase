#include "proofnetwork/qmlwrappers/userqmlwrapper.h"

#include "proofnetwork/qmlwrappers/userqmlwrapper_p.h"
#include "proofnetwork/user.h"

using namespace Proof;
UserQmlWrapper::UserQmlWrapper(const UserSP &user, QObject *parent)
    : UserQmlWrapper(user, *new UserQmlWrapperPrivate, parent)
{}

UserQmlWrapper::UserQmlWrapper(const UserSP &user, UserQmlWrapperPrivate &dd, QObject *parent)
    : NetworkDataEntityQmlWrapper(user, dd, parent)
{
    setupEntity();
}

PROOF_NDE_WRAPPER_PROPERTY_IMPL_R(User, QString, userName)
PROOF_NDE_WRAPPER_PROPERTY_IMPL_R(User, QString, fullName)
PROOF_NDE_WRAPPER_PROPERTY_IMPL_R(User, QString, email)

PROOF_NDE_WRAPPER_TOOLS_IMPL(User)

void UserQmlWrapper::setupEntity(const QSharedPointer<NetworkDataEntity> &old)
{
    UserSP user = entity<User>();
    Q_ASSERT(user);

    connect(user.data(), &User::userNameChanged, this, &UserQmlWrapper::userNameChanged);
    connect(user.data(), &User::fullNameChanged, this, &UserQmlWrapper::fullNameChanged);
    connect(user.data(), &User::emailChanged, this, &UserQmlWrapper::emailChanged);

    auto castedOld = qSharedPointerCast<User>(old);
    if (castedOld) {
        if (castedOld->userName() != user->userName())
            emit userNameChanged(userName());
        if (castedOld->fullName() != user->fullName())
            emit fullNameChanged(fullName());
        if (castedOld->email() != user->email())
            emit emailChanged(email());
    }
}
