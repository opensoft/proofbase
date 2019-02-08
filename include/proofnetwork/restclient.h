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
#ifndef RESTCLIENT_H
#define RESTCLIENT_H

#include "proofseed/future.h"

#include "proofcore/proofobject.h"

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QUrlQuery>

class QNetworkReply;

namespace Proof {

class RestClientPrivate;

class PROOF_NETWORK_EXPORT RestClient : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RestClient)
public:
    explicit RestClient(bool ignoreSslErrors = false);
    RestClient(const RestClient &) = delete;
    RestClient(RestClient &&) = delete;
    RestClient &operator=(const RestClient &) = delete;
    RestClient &operator=(RestClient &&) = delete;
    ~RestClient();

    QString userName() const;
    void setUserName(const QString &arg);

    QString password() const;
    void setPassword(const QString &arg);

    QString clientName() const;
    void setClientName(const QString &arg);

    QString host() const;
    void setHost(const QString &arg);

    QString postfix() const;
    void setPostfix(const QString &arg);

    int port() const;
    void setPort(int arg);

    QString scheme() const;
    void setScheme(const QString &arg);

    QString token() const;
    void setToken(const QString &arg);

    RestAuthType authType() const;
    void setAuthType(RestAuthType arg);

    int msecsForTimeout() const;
    void setMsecsForTimeout(int arg);

    bool followRedirects() const;
    void setFollowRedirects(bool arg);

    void setCustomHeader(const QByteArray &header, const QByteArray &value);
    QByteArray customHeader(const QByteArray &header) const;
    bool containsCustomHeader(const QByteArray &header) const;
    void unsetCustomHeader(const QByteArray &header);

    void setCookie(const QNetworkCookie &cookie);
    QNetworkCookie cookie(const QString &name) const;
    bool containsCookie(const QString &name) const;
    void unsetCookie(const QString &name);

    CancelableFuture<QNetworkReply *> get(const QString &method, const QUrlQuery &query = QUrlQuery(),
                                          const QString &vendor = QString());
    CancelableFuture<QNetworkReply *> post(const QString &method, const QUrlQuery &query = QUrlQuery(),
                                           const QByteArray &body = "", const QString &vendor = QString());
    CancelableFuture<QNetworkReply *> post(const QString &method, const QUrlQuery &query, QHttpMultiPart *multiParts);
    CancelableFuture<QNetworkReply *> put(const QString &method, const QUrlQuery &query = QUrlQuery(),
                                          const QByteArray &body = "", const QString &vendor = QString());
    CancelableFuture<QNetworkReply *> patch(const QString &method, const QUrlQuery &query = QUrlQuery(),
                                            const QByteArray &body = "", const QString &vendor = QString());
    CancelableFuture<QNetworkReply *> deleteResource(const QString &method, const QUrlQuery &query = QUrlQuery(),
                                                     const QString &vendor = QString());
    CancelableFuture<QNetworkReply *> get(const QUrl &url);

signals:
    void userNameChanged(const QString &arg);
    void passwordChanged(const QString &arg);
    void clientNameChanged(const QString &arg);
    void hostChanged(const QString &arg);
    void postfixChanged(const QString &arg);
    void portChanged(int arg);
    void schemeChanged(const QString &arg);
    void tokenChanged(const QString &arg);
    void authTypeChanged(Proof::RestAuthType arg);
    void msecsForTimeoutChanged(qlonglong arg);
    void followRedirectsChanged(bool arg);
};

} // namespace Proof

#endif // RESTCLIENT_H
