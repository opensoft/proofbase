#ifndef ABSTRACTRESTSERVER_H
#define ABSTRACTRESTSERVER_H

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QTcpServer>
#include <QScopedPointer>
#include <QStringList>
#include <QUrlQuery>

#ifndef Q_MOC_RUN
# define NO_AUTH_REQUIRED
#endif

namespace Proof {

class AbstractRestServerPrivate;
class PROOF_NETWORK_EXPORT AbstractRestServer : public QTcpServer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractRestServer)
public:
    explicit AbstractRestServer(QObject *parent = 0);
    explicit AbstractRestServer(const QString &pathPrefix, int port, RestAuthType authType, QObject *parent = 0);
    AbstractRestServer(const QString &userName, const QString &password, const QString &pathPrefix = QString(),
                       int port = 80, QObject *parent = 0);
    ~AbstractRestServer();

    QString userName() const;
    QString password() const;
    //TODO: 1.0: it is not used now. Remove or make it usable
    QString pathPrefix() const;
    int port() const;
    RestAuthType authType() const;

    void setUserName(const QString &userName);
    void setPassword(const QString &password);
    void setPathPrefix(const QString &pathPrefix);
    void setPort(int port);
    void setSuggestedMaxThreadsCount(int count = -1);
    void setAuthType(RestAuthType authType);

    void setCustomHeader(const QString &header, const QString &value);
    QString customHeader(const QString &header) const;
    bool containsCustomHeader(const QString &header) const;
    void unsetCustomHeader(const QString &header);

    Q_INVOKABLE void startListen();
    Q_INVOKABLE void stopListen();

signals:
    void userNameChanged(const QString &arg);
    void passwordChanged(const QString &arg);
    void pathPrefixChanged(const QString &arg);
    void portChanged(int arg);
    void authTypeChanged(Proof::RestAuthType arg);

protected slots:
    NO_AUTH_REQUIRED void rest_get_System_Status(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                                                 const QUrlQuery &query, const QByteArray &body);
    NO_AUTH_REQUIRED void rest_get_System_RecentErrors(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                                                       const QUrlQuery &query, const QByteArray &body);

protected:
    virtual QMap<QString, QPair<QDateTime, QVariant>> healthStatus(bool quick) const;

    void incomingConnection(qintptr socketDescriptor) override;

    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, int returnCode = 200, const QString &reason = QString());
    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, const QHash<QString, QString> &headers,
                    int returnCode = 200, const QString &reason = QString());
    void sendErrorCode(QTcpSocket *socket, int returnCode, const QString &reason, int errorCode, const QStringList &args = QStringList());
    template<class Enum>
    void sendErrorCode(QTcpSocket *socket, int returnCode, const QString &reason, Enum errorCode, const QStringList &args = QStringList())
    {
        sendErrorCode(socket, returnCode, reason, static_cast<int>(errorCode), args);
    }
    void sendBadRequest(QTcpSocket *socket, const QString &reason = "Bad Request");
    void sendNotFound(QTcpSocket *socket, const QString &reason = "Not Found");
    void sendNotAuthorized(QTcpSocket *socket, const QString &reason = "Unauthorized");
    void sendInternalError(QTcpSocket *socket);
    bool checkBasicAuth(const QString &encryptedAuth) const;
    QString parseAuth(QTcpSocket *socket, const QString &header);

    AbstractRestServer(AbstractRestServerPrivate &dd, const QString &userName, const QString &password,
                       const QString &pathPrefix, int port, RestAuthType authType, QObject *parent = 0);
    QScopedPointer<AbstractRestServerPrivate> d_ptr;
};

}
#endif // ABSTRACTRESTSERVER_H
