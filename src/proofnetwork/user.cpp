/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
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
