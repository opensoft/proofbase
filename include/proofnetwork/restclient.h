#ifndef RESTCLIENT_H
#define RESTCLIENT_H

#include "proofcore/proofobject.h"
#include "proofnetwork/proofnetwork_global.h"

#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QByteArray>

class QNetworkReply;

namespace Proof {

class RestClientPrivate;

class PROOF_NETWORK_EXPORT RestClient : public ProofObject
{
    Q_OBJECT
    Q_ENUMS(AuthType)
    Q_DECLARE_PRIVATE(RestClient)
public:
    enum class AuthType
    {
        WithoutAuth,
        BasicAuth,
        WsseAuth,
        QuasiOAuth2Auth
    };

    explicit RestClient();

    QString userName() const;
    void setUserName(const QString &arg);

    QString password() const;
    void setPassword(const QString &arg);

    QString clientName() const;
    void setClientName(const QString &arg);

    QString host() const;
    void setHost(const QString &arg);

    int port() const;
    void setPort(int arg);

    QString scheme() const;
    void setScheme(const QString &arg);

    RestClient::AuthType authType() const;
    void setAuthType(RestClient::AuthType arg);

    qlonglong msecsForTimeout() const;
    void setMsecsForTimeout(qlonglong arg);

    void setCustomHeader(const QByteArray &header, const QByteArray &value);
    QByteArray customHeader(const QByteArray &header) const;
    bool containsCustomHeader(const QByteArray &header) const;
    void removeCustomHeader(const QByteArray &header);

    QNetworkReply *get(const QString &method, const QUrlQuery &query = QUrlQuery());
    QNetworkReply *post(const QString &method, const QUrlQuery &query = QUrlQuery(), const QByteArray &body = "");
    QNetworkReply *patch(const QString &method, const QUrlQuery &query = QUrlQuery(), const QByteArray &body = "");

    Q_INVOKABLE void authenticate();

signals:
    void userNameChanged(const QString &arg);
    void passwordChanged(const QString &arg);
    void clientNameChanged(const QString &arg);
    void hostChanged(const QString &arg);
    void portChanged(int arg);
    void schemeChanged(const QString &arg);
    void authTypeChanged(RestClient::AuthType arg);
    void msecsForTimeoutChanged(qlonglong arg);

    void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
    void encrypted(QNetworkReply *reply);
    void finished(QNetworkReply *reply);
    void networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void authenticationSucceed();
    void authenticationErrorOccurred(const QString &errorMessage);
};

}

#endif // RESTCLIENT_H
