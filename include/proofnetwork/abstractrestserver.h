#ifndef ABSTRACTRESTSERVER_H
#define ABSTRACTRESTSERVER_H

#include "proofnetwork/proofnetwork_global.h"

#include <QTcpServer>
#include <QThread>
#include <QScopedPointer>

namespace Proof {

class AbstractRestServerPrivate;

class PROOF_NETWORK_EXPORT AbstractRestServer : public QTcpServer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractRestServer)
public:
    explicit AbstractRestServer(const QString &userName, const QString &password,
                                const QString &pathPrefix = QString(), int port = 80, QObject *parent = 0);
    ~AbstractRestServer();

    Q_INVOKABLE void startListen();

protected:
    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, int returnCode = 200, const QString &reason = QString());
    void sendNotFound(QTcpSocket *socket, const QString &reason = "Not Found");
    void sendNotAuthorized(QTcpSocket *socket, const QString &reason = "Unauthorized");
    void sendInternalError(QTcpSocket *socket);
    bool checkBasicAuth(const QString &encryptedAuth) const;
    QString parseAuth(QTcpSocket *socket, const QString &header);


    AbstractRestServer(AbstractRestServerPrivate &dd, const QString &userName, const QString &password, const QString &pathPrefix, int port, QObject *parent = 0);
    QScopedPointer<AbstractRestServerPrivate> d_ptr;
};

}
#endif // ABSTRACTRESTSERVER_H
