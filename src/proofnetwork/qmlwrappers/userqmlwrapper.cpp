#include "userqmlwrapper.h"
#include "userqmlwrapper_p.h"
#include "proofnetwork/user.h"

using namespace Proof;
UserQmlWrapper::UserQmlWrapper(const UserSP &user, QObject *parent)
    : UserQmlWrapper(user, *new UserQmlWrapperPrivate, parent)
{
}

UserQmlWrapper::UserQmlWrapper(const UserSP &user, UserQmlWrapperPrivate &dd, QObject *parent)
    : NetworkDataEntityQmlWrapper(user, dd, parent)
{
    if (user) {
        connect(user.data(), &User::userNameChanged,
                this, &UserQmlWrapper::userNameChanged);
        connect(user.data(), &User::fullNameChanged,
                this, &UserQmlWrapper::fullNameChanged);
        connect(user.data(), &User::emailChanged,
                this, &UserQmlWrapper::emailChanged);
    }
}

PROOF_NDE_WRAPPED_PROPERTY_IMPL_R(User, QString, userName)
PROOF_NDE_WRAPPED_PROPERTY_IMPL_R(User, QString, fullName)
PROOF_NDE_WRAPPED_PROPERTY_IMPL_R(User, QString, email)
