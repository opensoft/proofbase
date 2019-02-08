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
    ~UserQmlWrapper();
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
