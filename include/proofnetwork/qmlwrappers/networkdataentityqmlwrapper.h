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
#ifndef NETWORKDATAENTITYQMLWRAPPER_H
#define NETWORKDATAENTITYQMLWRAPPER_H

#include "proofcore/proofobject.h"

#include "proofnetwork/proofnetwork_global.h"

#include <QSharedPointer>

#define PROOF_NDE_WRAPPER_PROPERTY_IMPL_R(Nde, Type, Getter)    \
    Type Nde##QmlWrapper::Getter() const                        \
    {                                                           \
        const QSharedPointer<Nde> entity = this->entity<Nde>(); \
        Q_ASSERT(entity);                                       \
        return entity->Getter();                                \
    }

#define PROOF_NDE_WRAPPER_PROPERTY_IMPL_RW(Nde, Type, Getter, Setter) \
    PROOF_NDE_WRAPPER_PROPERTY_IMPL_R(Nde, Type, Getter)              \
    void Nde##QmlWrapper::Setter(const Type &arg)                     \
    {                                                                 \
        QSharedPointer<Nde> entity = this->entity<Nde>();             \
        Q_ASSERT(entity);                                             \
        if (arg != entity->Getter()) {                                \
            entity->Setter(arg);                                      \
            emit Getter##Changed(arg);                                \
        }                                                             \
    }

#define PROOF_NDE_WRAPPER_TOOLS(Nde) Proof::NetworkDataEntityQmlWrapper *clone() const override;

#define PROOF_NDE_WRAPPER_TOOLS_IMPL(Nde)                              \
    Proof::NetworkDataEntityQmlWrapper *Nde##QmlWrapper::clone() const \
    {                                                                  \
        const QSharedPointer<Nde> entity = this->entity<Nde>();        \
        Q_ASSERT(entity);                                              \
        return new Nde##QmlWrapper(entity);                            \
    }

namespace Proof {
class NetworkDataEntity;
class NetworkDataEntityQmlWrapperPrivate;
class PROOF_NETWORK_EXPORT NetworkDataEntityQmlWrapper : public ProofObject
{
    Q_OBJECT
    Q_PROPERTY(bool isFetched READ isFetched NOTIFY isFetchedChanged)
    Q_DECLARE_PRIVATE(NetworkDataEntityQmlWrapper)
public:
    Q_INVOKABLE virtual Proof::NetworkDataEntityQmlWrapper *clone() const = 0;
    bool isFetched() const;

    template <class T>
    QSharedPointer<T> entity()
    {
        return qSharedPointerCast<T>(entity());
    }

    template <class T>
    const QSharedPointer<T> entity() const
    {
        return qSharedPointerCast<T>(entity());
    }

    QSharedPointer<NetworkDataEntity> entity();
    const QSharedPointer<NetworkDataEntity> entity() const;

    void setEntity(const QSharedPointer<NetworkDataEntity> &networkDataEntity);

signals:
    void isFetchedChanged(bool isFetched);

protected:
    NetworkDataEntityQmlWrapper() = delete;
    explicit NetworkDataEntityQmlWrapper(const QSharedPointer<NetworkDataEntity> &networkDataEntity,
                                         QObject *parent = nullptr);
    explicit NetworkDataEntityQmlWrapper(const QSharedPointer<NetworkDataEntity> &networkDataEntity,
                                         NetworkDataEntityQmlWrapperPrivate &dd, QObject *parent = nullptr);

    virtual void setupEntity(const QSharedPointer<NetworkDataEntity> &old = QSharedPointer<NetworkDataEntity>()) = 0;

    QObject *entityConnectContext() const;
};
} // namespace Proof

#endif // NETWORKDATAENTITYQMLWRAPPER_H
