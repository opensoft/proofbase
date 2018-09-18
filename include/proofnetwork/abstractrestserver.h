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
#ifndef ABSTRACTRESTSERVER_H
#define ABSTRACTRESTSERVER_H

#include "proofseed/future.h"

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QScopedPointer>
#include <QStringList>
#include <QTcpServer>
#include <QUrlQuery>

#ifndef Q_MOC_RUN
#    define NO_AUTH_REQUIRED
#endif

namespace Proof {
using HealthStatusMap = QMap<QString, QPair<QDateTime, QVariant>>;

class AbstractRestServerPrivate;
class PROOF_NETWORK_EXPORT AbstractRestServer : public QTcpServer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractRestServer)
public:
    explicit AbstractRestServer();
    explicit AbstractRestServer(quint16 port);
    explicit AbstractRestServer(const QString &pathPrefix, quint16 port);
    ~AbstractRestServer();

    QString userName() const;
    QString password() const;
    QString pathPrefix() const;
    int port() const;
    RestAuthType authType() const;

    void setUserName(const QString &userName);
    void setPassword(const QString &password);
    void setPathPrefix(const QString &pathPrefix);
    void setPort(quint16 port);
    void setSuggestedMaxThreadsCount(int count = -1);
    void setAuthType(RestAuthType authType);

    void setCustomHeader(const QString &header, const QString &value);
    QString customHeader(const QString &header) const;
    bool containsCustomHeader(const QString &header) const;
    void unsetCustomHeader(const QString &header);

    void startListen();
    void stopListen();

signals:
    void userNameChanged(const QString &arg);
    void passwordChanged(const QString &arg);
    void pathPrefixChanged(const QString &arg);
    void portChanged(int arg);
    void authTypeChanged(Proof::RestAuthType arg);

protected slots:
    NO_AUTH_REQUIRED void rest_get_System_Status(QTcpSocket *socket, const QStringList &headers,
                                                 const QStringList &methodVariableParts, const QUrlQuery &query,
                                                 const QByteArray &body);
    NO_AUTH_REQUIRED void rest_get_System_RecentErrors(QTcpSocket *socket, const QStringList &headers,
                                                       const QStringList &methodVariableParts, const QUrlQuery &query,
                                                       const QByteArray &body);

protected:
    virtual FutureSP<HealthStatusMap> healthStatus(bool quick) const;

    void incomingConnection(qintptr socketDescriptor) override;

    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, int returnCode = 200,
                    const QString &reason = QString());
    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType,
                    const QHash<QString, QString> &headers, int returnCode = 200, const QString &reason = QString());
    void sendErrorCode(QTcpSocket *socket, int returnCode, const QString &reason, int errorCode,
                       const QStringList &args = QStringList());
    template <class Enum>
    void sendErrorCode(QTcpSocket *socket, int returnCode, const QString &reason, Enum errorCode,
                       const QStringList &args = QStringList())
    {
        sendErrorCode(socket, returnCode, reason, static_cast<int>(errorCode), args);
    }
    void sendBadRequest(QTcpSocket *socket, const QString &reason = QStringLiteral("Bad Request"));
    void sendNotFound(QTcpSocket *socket, const QString &reason = QStringLiteral("Not Found"));
    void sendNotAuthorized(QTcpSocket *socket, const QString &reason = QStringLiteral("Unauthorized"));
    void sendConflict(QTcpSocket *socket, const QString &reason = QStringLiteral("Conflict"));
    void sendInternalError(QTcpSocket *socket);
    bool checkBasicAuth(const QString &encryptedAuth) const;
    QString parseAuth(QTcpSocket *socket, const QString &header);

    AbstractRestServer(AbstractRestServerPrivate &dd, const QString &pathPrefix, quint16 port);
    QScopedPointer<AbstractRestServerPrivate> d_ptr;
};

} // namespace Proof
#endif // ABSTRACTRESTSERVER_H
