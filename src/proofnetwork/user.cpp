#include "proofnetwork/user.h"

#include "proofnetwork/user_p.h"

using namespace Proof;

User::User(const QString &userName) : User(*new UserPrivate(userName))
{}

User::User(Proof::UserPrivate &dd) : NetworkDataEntity(dd)
{}

QString User::userName() const
{
    Q_D_CONST(User);
    return d->userName;
}

QString User::fullName() const
{
    Q_D_CONST(User);
    return d->fullName;
}

QString User::email() const
{
    Q_D_CONST(User);
    return d->email;
}

UserQmlWrapper *User::toQmlWrapper(QObject *parent) const
{
    UserSP castedSelf = castedSelfPtr<User>();
    Q_ASSERT(castedSelf);
    return new UserQmlWrapper(castedSelf, parent);
}

UserSP User::create(const QString &userName)
{
    UserSP result(new User(userName));
    initSelfWeakPtr(result);
    return result;
}

UserPrivate::UserPrivate(const QString &userName) : userName(userName)
{
    setDirty(!userName.isEmpty());
}

void User::updateSelf(const NetworkDataEntitySP &other)
{
    Q_D(User);
    UserSP castedOther = qSharedPointerCast<User>(other);
    d->setUserName(castedOther->userName());
    d->setFullName(castedOther->fullName());
    d->setEmail(castedOther->email());

    NetworkDataEntity::updateSelf(other);
}

void UserPrivate::setUserName(const QString &arg)
{
    Q_Q(User);
    if (userName != arg) {
        userName = arg;
        emit q->userNameChanged(arg);
    }
}

void UserPrivate::setFullName(const QString &arg)
{
    Q_Q(User);
    if (fullName != arg) {
        fullName = arg;
        emit q->fullNameChanged(arg);
    }
}

void UserPrivate::setEmail(const QString &arg)
{
    Q_Q(User);
    if (email != arg) {
        email = arg;
        emit q->emailChanged(arg);
    }
}
