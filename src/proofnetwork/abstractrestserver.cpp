#include "abstractrestserver.h"

#include <QDir>
#include <QTcpSocket>
#include <QMetaObject>
#include <QMetaMethod>
#include <QUrlQuery>
#include <QThreadPool>

static const QString REST_METHOD_PREFIX = QString("rest_");
static const int MAX_CONNECTION_THREADS_COUNT = 30;

namespace Proof {

struct MethodNode {
    MethodNode()
    {}

    bool contains(const QString &name) const
    {
        return nodes.contains(name);
    }

    void clear()
    {
        nodes.clear();
    }

    operator QString()
    {
        return value;
    }

    MethodNode &operator [](const QString &name)
    {
        return nodes[name];
    }

    const MethodNode operator [](const QString &name) const
    {
        return nodes[name];
    }

    void setValue(const QString &_value)
    {
        value = _value;
    }

private:
    QHash<QString, MethodNode> nodes;
    QString value = "";
};

class AbstractRestServerPrivate
{
    Q_DECLARE_PUBLIC(AbstractRestServer)
    friend class ServerConnectionTask;

public:
    AbstractRestServerPrivate() {}
    virtual ~AbstractRestServerPrivate() {}

private:
    AbstractRestServerPrivate(const AbstractRestServerPrivate &other) = delete;
    AbstractRestServerPrivate &operator=(const AbstractRestServerPrivate &other) = delete;
    AbstractRestServerPrivate(const AbstractRestServerPrivate &&other) = delete;
    AbstractRestServerPrivate &operator=(const AbstractRestServerPrivate &&other) = delete;

    void createNewConnection(QTcpSocket *socket);
    void handleRequest(QTcpSocket *socket);
    void tryToCallMethod(QTcpSocket *socket, const QString &type, const QString &method, QStringList headers, const QByteArray &body);
    QStringList makeMethodName(const QString &type, const QString &name);
    QString findMethod(const QStringList &splittedMethod, QStringList &methodVariableParts);
    void fillMethods();
    void addMethodToTree(const QString &realMethod);

    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, int returnCode = 200, const QString &reason = QString());

    AbstractRestServer *q_ptr = 0;
    int port = 0;
    QString userName;
    QString password;
    QString pathPrefix;
    QThread *thread = nullptr;
    QThreadPool *connectionThreadPool = nullptr;

    MethodNode methodsTreeRoot;
};

class ServerConnectionTask: public QRunnable
{
public:
    ServerConnectionTask(qintptr _socket, AbstractRestServerPrivate *const _server_d)
        : socket(_socket),
          server_d(_server_d)
    {
    }

    void run() override
    {
        QTcpSocket *tcpSocket = new QTcpSocket();
        if (!tcpSocket->setSocketDescriptor(socket)) {
            qCDebug(proofNetworkMiscLog) << "Can't create socket, error:" << tcpSocket->errorString();
            return;
        }
        server_d->createNewConnection(tcpSocket);
    }

private:
    qintptr socket;
    AbstractRestServerPrivate *const server_d;
};
}

using namespace Proof;

AbstractRestServer::AbstractRestServer(QObject *parent)
    : AbstractRestServer(*new AbstractRestServerPrivate, "", "", "", 80, parent)
{
}

AbstractRestServer::AbstractRestServer(const QString &userName, const QString &password, const QString &pathPrefix, int port, QObject *parent)
    : AbstractRestServer(*new AbstractRestServerPrivate, userName, password, pathPrefix, port, parent)
{
}

AbstractRestServer::AbstractRestServer(AbstractRestServerPrivate &dd, const QString &userName, const QString &password, const QString &pathPrefix, int port, QObject *parent)
    : QTcpServer(parent),
      d_ptr(&dd)
{
    Q_D(AbstractRestServer);
    d->q_ptr = this;

    qRegisterMetaType<QTcpSocket *>("QTcpSocket *");
    qRegisterMetaType<QUrlQuery>("QUrlQuery");

    d->thread = new QThread(this);
    d->connectionThreadPool = new QThreadPool(this);
    d->connectionThreadPool->setMaxThreadCount(MAX_CONNECTION_THREADS_COUNT > QThread::idealThreadCount()
                                               ? MAX_CONNECTION_THREADS_COUNT : QThread::idealThreadCount());
    d->port = port;
    d->userName = userName;
    d->password = password;
    d->pathPrefix = pathPrefix;

    moveToThread(d->thread);
    d->thread->moveToThread(d->thread);
    d->thread->start();
}

AbstractRestServer::~AbstractRestServer()
{
    Q_D(AbstractRestServer);
    stopListen();
    d->thread->quit();
    d->thread->wait(1000);
    d->thread->terminate();
}

QString AbstractRestServer::userName() const
{
    Q_D(const AbstractRestServer);
    return d->userName;
}

QString AbstractRestServer::password() const
{
    Q_D(const AbstractRestServer);
    return d->password;
}

QString AbstractRestServer::pathPrefix() const
{
    Q_D(const AbstractRestServer);
    return d->pathPrefix;
}

int AbstractRestServer::port() const
{
    Q_D(const AbstractRestServer);
    return d->port;
}

void AbstractRestServer::setUserName(const QString &userName)
{
    Q_D(AbstractRestServer);
    if (d->userName != userName) {
        d->userName = userName;
        emit userNameChanged(d->userName);
    }
}

void AbstractRestServer::setPassword(const QString &password)
{
    Q_D(AbstractRestServer);
    if (d->password != password) {
        d->password = password;
        emit passwordChanged(d->password);
    }
}

void AbstractRestServer::setPathPrefix(const QString &pathPrefix)
{
    Q_D(AbstractRestServer);
    if (d->pathPrefix != pathPrefix) {
        d->pathPrefix = pathPrefix;
        emit pathPrefixChanged(d->pathPrefix);
    }
}

void AbstractRestServer::setPort(int port)
{
    Q_D(AbstractRestServer);
    if (d->port != port) {
        d->port = port;
        emit portChanged(d->port);
    }
}

void AbstractRestServer::startListen()
{
    Q_D(AbstractRestServer);
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, __func__, Qt::QueuedConnection);
    } else {
        d->fillMethods();
        bool isListen = listen(QHostAddress::Any, d->port);
        if (!isListen)
            qCCritical(proofNetworkMiscLog) << "Server can't start on port" << d->port;
    }
}

void AbstractRestServer::stopListen()
{
    if (QThread::currentThread() != thread())
        QMetaObject::invokeMethod(this, __func__, Qt::BlockingQueuedConnection);
    else
        close();
}

void AbstractRestServer::incomingConnection(qintptr socketDescriptor)
{
    Q_D(AbstractRestServer);
    ServerConnectionTask *task = new ServerConnectionTask(socketDescriptor, d);
    d->connectionThreadPool->start(task);
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
            .arg(d->userName)
            .arg(d->password).toLatin1().toBase64();
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
        if (parts.count() != 2 || parts.at(0) != "Basic")
            sendNotAuthorized(socket);
        else
            auth = parts.at(1);
    }
    return auth;
}

void AbstractRestServer::sendNotFound(QTcpSocket *socket, const QString &reason)
{
    sendAnswer(socket, "", "text/plain; charset=utf-8", 404, reason);
}

void AbstractRestServer::sendNotAuthorized(QTcpSocket *socket, const QString &reason)
{
    sendAnswer(socket, "", "text/plain; charset=utf-8", 401, reason);
}

void AbstractRestServer::sendInternalError(QTcpSocket *socket)
{
    sendAnswer(socket, "", "text/plain; charset=utf-8", 500, "Internal Server Error");
}


void AbstractRestServerPrivate::createNewConnection(QTcpSocket *socket)
{
    Q_ASSERT(socket);
    handleRequest(socket);
}

void AbstractRestServerPrivate::handleRequest(QTcpSocket *socket)
{
    Q_Q(AbstractRestServer);
    QByteArray request;
    forever {
        QByteArray read = socket->read(1024);
        if (!read.isEmpty())
            request.append(read);
        if (!socket->waitForReadyRead(500))
            break;

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

    tryToCallMethod(socket, tokens[0], tokens[1], headersParts, body.toUtf8());
}

QStringList AbstractRestServerPrivate::makeMethodName(const QString &type, const QString &name)
{
    QStringList splittedName = name.split("/", QString::SkipEmptyParts);
    splittedName.prepend(type.toLower());
    return splittedName;
}

QString AbstractRestServerPrivate::findMethod(const QStringList &splittedMethod, QStringList &methodVariableParts)
{
    Q_ASSERT(splittedMethod.count() >= 2);

    MethodNode *currentNode = &methodsTreeRoot;
    int i = 0;
    for (; i < splittedMethod.count(); ++i) {
        QString loweredPart = splittedMethod[i].toLower();
        if (!currentNode->contains(loweredPart))
            break;
        currentNode = &(*currentNode)[loweredPart];
    }

    QString methodName = (*currentNode);

    if (!methodName.isEmpty())
        methodVariableParts = splittedMethod.mid(i);

    return methodName;
}

void AbstractRestServerPrivate::fillMethods()
{
    Q_Q(AbstractRestServer);
    methodsTreeRoot.clear();
    for (int i = 0; i < q->metaObject()->methodCount(); ++i) {
        if (q->metaObject()->method(i).methodType() == QMetaMethod::Slot) {
            QString currentMethod = QString(q->metaObject()->method(i).name());
            if (currentMethod.startsWith(REST_METHOD_PREFIX))
                addMethodToTree(currentMethod);
        }
    }
}

void AbstractRestServerPrivate::addMethodToTree(const QString &realMethod)
{
    QString method = realMethod.mid(QString(REST_METHOD_PREFIX).length());
    for (int i = 0; i < method.length(); ++i) {
        if (method[i].isUpper()) {
            method[i] = method[i].toLower();
            if (i > 0 && method[i - 1] != '_')
                method.insert(i++, '-');
        }
    }

    QStringList splittedMethod = method.split("_");
    Q_ASSERT(splittedMethod.count() >= 2);

    MethodNode *currentNode = &methodsTreeRoot;
    for (int i = 0; i < splittedMethod.count(); ++i) {
        if (!currentNode->contains(splittedMethod[i]))
            (*currentNode)[splittedMethod[i]] = MethodNode();
        currentNode = &(*currentNode)[splittedMethod[i]];
    }
    currentNode->setValue(realMethod);


}

void AbstractRestServerPrivate::tryToCallMethod(QTcpSocket *socket, const QString &type, const QString &method, QStringList headers, const QByteArray &body)
{
    Q_Q(AbstractRestServer);
    QStringList splittedByParamsMethod = method.split('?');
    QStringList methodVariableParts;
    QUrlQuery queryParams;

    Q_ASSERT(splittedByParamsMethod.count());
    if (splittedByParamsMethod.count() > 1)
        queryParams = QUrlQuery(splittedByParamsMethod.at(1));

    QString methodName = findMethod(makeMethodName(type, splittedByParamsMethod.at(0)), methodVariableParts);
    if (!methodName.isEmpty()) {
        QString encryptedAuth;
        for (int i = 1; i < headers.count(); ++i) {
            if (headers.at(i).startsWith("Authorization")) {
                encryptedAuth = q->parseAuth(socket, headers.at(i));
                break;
            }
        }
        if (!encryptedAuth.isEmpty() && q->checkBasicAuth(encryptedAuth)) {
            QMetaObject::invokeMethod(q, methodName.toLatin1().constData(), Qt::DirectConnection,
                                      Q_ARG(QTcpSocket *,socket),
                                      Q_ARG(const QStringList &, headers),
                                      Q_ARG(const QStringList &, methodVariableParts),
                                      Q_ARG(const QUrlQuery &, queryParams),
                                      Q_ARG(const QByteArray &, body));
        } else {
            q->sendNotAuthorized(socket);
        }
    } else {
        q->sendNotFound(socket, "Wrong method");
    }
}

void AbstractRestServerPrivate::sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, int returnCode, const QString &reason) {
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->write(QString("HTTP/1.0 %1 %2\r\n"
                              "Server: proof\r\n"
                              "Content-Type: %3\r\n"
                              "%4"
                              "\r\n")
                      .arg(returnCode)
                      .arg(reason)
                      .arg(contentType)
                      .arg(!body.isEmpty() ? QString("Content-Length: %1\r\n").arg(body.size()) : QString())
                      .toUtf8());

        socket->write(body);
        socket->flush();
        socket->disconnectFromHost();
    }
    if (socket->state() == QTcpSocket::UnconnectedState)
        delete socket;
}
