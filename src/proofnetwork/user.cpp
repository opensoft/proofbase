#include "user.h"
#include "user_p.h"

using namespace Proof;

User::User(const QString &userName)
    : User(userName, *new UserPrivate)
{
}

User::User(const QString &userName, Proof::UserPrivate &dd, QObject *parent)
    : NetworkDataEntity(dd, parent)
{
    Q_D(User);
    d->setUserName(userName);
}

QString User::userName() const
{
    Q_D(const User);
    return d->userName;
}

QString User::fullName() const
{
    Q_D(const User);
    return d->fullName;
}

QString User::email() const
{
    Q_D(const User);
    return d->email;
}

UserQmlWrapper *User::toQmlWrapper(QObject *parent) const
{
    Q_D(const User);
    UserSP castedSelf = qSharedPointerCast<User>(d->weakSelf);
    Q_ASSERT(castedSelf);
    return new UserQmlWrapper(castedSelf, parent);
}

UserSP User::create(const QString &userName)
{
    UserSP result(new User(userName));
    result->d_func()->weakSelf = result.toWeakRef();
    return result;
}

UserSP User::defaultObject()
{
    static UserSP entity = create("");
    return entity;
}

void UserPrivate::updateFrom(const NetworkDataEntitySP &other)
{
    UserSP castedOther = qSharedPointerCast<User>(other);
    setUserName(castedOther->userName());
    setFullName(castedOther->fullName());
    setEmail(castedOther->email());

    NetworkDataEntityPrivate::updateFrom(other);
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
