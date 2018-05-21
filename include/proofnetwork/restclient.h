#ifndef RESTCLIENT_H
#define RESTCLIENT_H

#include "proofcore/proofobject.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofcore/future.h"

#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QNetworkCookie>
#include <QByteArray>

class QNetworkReply;

namespace Proof {

class RestClientPrivate;

class PROOF_NETWORK_EXPORT RestClient : public ProofObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RestClient)
public:
    explicit RestClient(bool ignoreSslErrors = false);
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

    CancelableFuture<QNetworkReply *> get(const QString &method, const QUrlQuery &query = QUrlQuery(), const QString &vendor = QString());
    CancelableFuture<QNetworkReply *> post(const QString &method, const QUrlQuery &query = QUrlQuery(), const QByteArray &body = "", const QString &vendor = QString());
    CancelableFuture<QNetworkReply *> post(const QString &method, const QUrlQuery &query, QHttpMultiPart *multiParts);
    CancelableFuture<QNetworkReply *> put(const QString &method, const QUrlQuery &query = QUrlQuery(), const QByteArray &body = "", const QString &vendor = QString());
    CancelableFuture<QNetworkReply *> patch(const QString &method, const QUrlQuery &query = QUrlQuery(), const QByteArray &body = "", const QString &vendor = QString());
    CancelableFuture<QNetworkReply *> deleteResource(const QString &method, const QUrlQuery &query = QUrlQuery(), const QString &vendor = QString());
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

    void finished(QNetworkReply *reply);//Remove
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);//Remove
};

}

#endif // RESTCLIENT_H
