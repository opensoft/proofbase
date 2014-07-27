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
    setupEntity();
}

PROOF_NDE_WRAPPER_PROPERTY_IMPL_R(User, QString, userName)
PROOF_NDE_WRAPPER_PROPERTY_IMPL_R(User, QString, fullName)
PROOF_NDE_WRAPPER_PROPERTY_IMPL_R(User, QString, email)

PROOF_NDE_WRAPPER_TOOLS_IMPL(User)

void UserQmlWrapper::setupEntity()
{
    Q_D(UserQmlWrapper);
    UserSP user = d->entity<User>();
    Q_ASSERT(user);

    if (user) {
        connect(user.data(), &User::userNameChanged,
                this, &UserQmlWrapper::userNameChanged);
        connect(user.data(), &User::fullNameChanged,
                this, &UserQmlWrapper::fullNameChanged);
        connect(user.data(), &User::emailChanged,
                this, &UserQmlWrapper::emailChanged);
    }
}
