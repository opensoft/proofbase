#include "abstractrestserver.h"

#include <QDir>
#include <QTcpSocket>
#include <QDebug>

namespace Proof {
class AbstractRestServerPrivate
{
    Q_DECLARE_PUBLIC(AbstractRestServer)

public:
    AbstractRestServerPrivate() {}
    virtual ~AbstractRestServerPrivate() {}

private:
    AbstractRestServerPrivate(const AbstractRestServerPrivate &other) = delete;
    AbstractRestServerPrivate &operator=(const AbstractRestServerPrivate &other) = delete;
    AbstractRestServerPrivate(const AbstractRestServerPrivate &&other) = delete;
    AbstractRestServerPrivate &operator=(const AbstractRestServerPrivate &&other) = delete;

    void createNewConnection();
    void handleRequest(QTcpSocket *socket);

    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, int returnCode = 200, const QString &reason = QString());

    bool isAllowedMethod(const QString &method);

    AbstractRestServer *q_ptr = 0;
    int m_port = 0;
    QString m_userName;
    QString m_password;
    QThread m_thread;
};
}

using namespace Proof;

AbstractRestServer::AbstractRestServer(const QString &userName, const QString &password, int port, QObject *parent)
    : AbstractRestServer(*new AbstractRestServerPrivate, userName, password, port, parent)
{
}

AbstractRestServer::AbstractRestServer(AbstractRestServerPrivate &dd, const QString &userName, const QString &password, int port, QObject *parent)
    : QTcpServer(parent),
      d_ptr(&dd)
{
    Q_D(AbstractRestServer);
    d->q_ptr = this;
    d->m_port = port;
    d->m_userName = userName;
    d->m_password = password;

    connect(this, &AbstractRestServer::newConnection, this, [d](){d->createNewConnection();});
    moveToThread(&d->m_thread);
    d->m_thread.moveToThread(&d->m_thread);
    d->m_thread.start();
}

AbstractRestServer::~AbstractRestServer()
{
    Q_D(AbstractRestServer);
    d->m_thread.quit();
    d->m_thread.wait(1000);
    d->m_thread.terminate();
}

void AbstractRestServer::startListen()
{
    Q_D(AbstractRestServer);
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "startListen", Qt::QueuedConnection);
    } else {
        bool isListen = listen(QHostAddress::Any, d->m_port);
        if (!isListen)
            qDebug() << "Server down";
    }
}

void AbstractRestServer::sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, int returnCode, const QString &reason)
{
    Q_D(AbstractRestServer);
    d->sendAnswer(socket, body, contentType, returnCode, reason);
}

bool AbstractRestServer::checkBasicAuth(const QString &encryptedAuth) const
{
    Q_D(const AbstractRestServer);
    QByteArray auth = QString("%1:%2")
            .arg(d->m_userName)
            .arg(d->m_password).toLatin1().toBase64();
    if (encryptedAuth == auth)
        return true;
    return false;
}

QString AbstractRestServer::parseAuth(QTcpSocket *socket, const QString &header)
{
    QString auth;
    QStringList parts = header.split(":");
    if (parts.count() != 2) {
        sendInternalError(socket);
    } else {
        parts = parts.at(1).split(" ", QString::SkipEmptyParts);
        if (parts.count() != 2 || parts.at(0) != "Basic") {
            sendNotAuthorized(socket);
        } else {
            auth = parts.at(1);
        }
    }
    return auth;
}

void AbstractRestServer::sendNotFound(QTcpSocket *socket, const QString &reason)
{
    sendAnswer(socket, "", "text/plain; charset=utf-8", 404, reason);
}

void AbstractRestServer::sendNotAuthorized(QTcpSocket *socket)
{
    sendAnswer(socket, "", "text/plain; charset=utf-8", 401, "Unauthorized");
}

void AbstractRestServer::sendInternalError(QTcpSocket *socket)
{
    sendAnswer(socket, "", "text/plain; charset=utf-8", 500, "Internal Server Error");
}


void AbstractRestServerPrivate::createNewConnection()
{
    Q_Q(AbstractRestServer);
    if (q->hasPendingConnections()) {
        QTcpSocket *socket = q->nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead, q, [this, q]() {
             QTcpSocket *socket = qobject_cast<QTcpSocket *>(q->sender());
             Q_ASSERT(socket);
             handleRequest(socket);
        });
        QObject::connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
}

void AbstractRestServerPrivate::handleRequest(QTcpSocket *socket)
{
    Q_Q(AbstractRestServer);
    QByteArray request;
    forever {
        QByteArray read = socket->read(1024);
        if (read.isEmpty() && !socket->waitForReadyRead(100))
            break;
        request.append(read);
    }

    QStringList requestParts = QString(request).split("\r\n\r\n");
    if (requestParts.count() < 2) {
        q->sendInternalError(socket);
        return;
    }

    QStringList headersParts = requestParts.at(0).split("\r\n", QString::SkipEmptyParts);
    if (headersParts.count() < 2) {
        q->sendInternalError(socket);
        return;
    }

    QString body = requestParts.at(1);

    QStringList tokens = headersParts.at(0).split(" ");

    if (tokens.count() == 3 && tokens[0] == "GET") {
        if (isAllowedMethod(tokens[1])) {
            headersParts.removeFirst();
            q->handleRequest(socket, tokens[1], headersParts, body);
        } else {
            q->sendNotFound(socket, "Wrong method");
        }
        socket->disconnectFromHost();
        if (socket->state() == QTcpSocket::UnconnectedState)
            delete socket;
    } else {
        q->sendNotFound(socket, "Only GET requests allowed");
    }
}

bool AbstractRestServerPrivate::isAllowedMethod(const QString &method)
{
    Q_Q(AbstractRestServer);
    for (const QString &allowed : q->allowedMethods()) {
        if (method == allowed || method.startsWith(allowed + "/"))
            return true;
    }
    return false;
}

void AbstractRestServerPrivate::sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, int returnCode, const QString &reason) {
    QTextStream os(socket);
    os.setAutoDetectUnicode(true);
    os << QString("HTTP/1.0 %1 %2\r\n"
                  "Content-Type: %3\r\n"
                  "%4"
                  "\r\n")
          .arg(returnCode)
          .arg(reason)
          .arg(contentType)
          .arg(!body.isEmpty() ? QString("Content-Size: %1\r\n").arg(body.size()) : QString())
          .toLatin1().constData()
       << body << "\n";
}
