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
