#include "abstractrestserver.h"

#include "proofcore/proofobject.h"

#include <QDir>
#include <QTcpSocket>
#include <QMetaObject>
#include <QMetaMethod>
#include <QUrlQuery>
#include <QQueue>
#include <QMutex>

static const QString REST_METHOD_PREFIX = QString("rest_");
static const int MAX_CONNECTION_THREADS_COUNT = 30;

namespace Proof {
class ServerConnectionTask;

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

class AbstractRestServerPrivate: public QObject
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

    void createNewConnection(QTcpSocket *socket, ServerConnectionTask *task);
    void removeFinishedConnection(QTcpSocket *socket);
    void freeTaskThread();

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
    QThread *serverThread = nullptr;
    QHash<QTcpSocket *, ServerConnectionTask *> activeConnectionTasks;
    QQueue<ServerConnectionTask *> waitingConnectionTasks;
    MethodNode methodsTreeRoot;
    QMutex connectionsMutex;
};

class ServerConnectionTask: public QThread
{
public:
    ServerConnectionTask(qintptr _socket_ptr, AbstractRestServerPrivate *const _server_d)
        : socket_ptr(_socket_ptr),
          server_d(_server_d)
    {
        moveToThread(this);
        QObject::connect(this, &QThread::finished, server_d, &AbstractRestServerPrivate::freeTaskThread);
    }

    ~ServerConnectionTask() {
        qCDebug(proofNetworkMiscLog) << "Task for Socket DESTROYED!";
    }

    void run() override
    {
        QTcpSocket *tcpSocket = new QTcpSocket();
        if (!tcpSocket->setSocketDescriptor(socket_ptr)) {
            qCDebug(proofNetworkMiscLog) << "Can't create socket, error:" << tcpSocket->errorString();
            return;
        }

        server_d->createNewConnection(tcpSocket, this);

        exec();
    }

    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, int returnCode, const QString &reason)
    {
        if (Proof::ProofObject::delayedCall(this,
                                            &ServerConnectionTask::sendAnswer,
                                            socket, body, contentType, returnCode, reason)) {
            return;
        }

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
            if (socket->state() != QTcpSocket::UnconnectedState)
                socket->waitForDisconnected();
        }
        delete socket;
        quit();
        server_d->removeFinishedConnection(socket);
    }

private:
    qintptr socket_ptr;
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

    d->serverThread = new QThread(this);
    d->port = port;
    d->userName = userName;
    d->password = password;
    d->pathPrefix = pathPrefix;

    moveToThread(d->serverThread);
    d->serverThread->moveToThread(d->serverThread);
    d->serverThread->start();
}

AbstractRestServer::~AbstractRestServer()
{
    Q_D(AbstractRestServer);
    stopListen();
    d->serverThread->quit();
    d->serverThread->wait(1000);
    d->serverThread->terminate();
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
    d->connectionsMutex.lock();
    if (d->activeConnectionTasks.count() >= MAX_CONNECTION_THREADS_COUNT)
        d->waitingConnectionTasks.enqueue(task);
    else
        task->start();
    d->connectionsMutex.unlock();
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


void AbstractRestServerPrivate::createNewConnection(QTcpSocket *socket, ServerConnectionTask *task)
{
    connectionsMutex.lock();
    activeConnectionTasks[socket] = task;
    connectionsMutex.unlock();
    handleRequest(socket);
}

void AbstractRestServerPrivate::removeFinishedConnection(QTcpSocket *socket)
{
    if (Proof::ProofObject::blockingDelayedCall(this,
                                        &AbstractRestServerPrivate::removeFinishedConnection, 0,
                                        socket)) {
        return;
    }

    connectionsMutex.lock();
    activeConnectionTasks.remove(socket);
    connectionsMutex.unlock();

    ServerConnectionTask *newTask = nullptr;
    connectionsMutex.lock();
    if (activeConnectionTasks.count() < MAX_CONNECTION_THREADS_COUNT && !waitingConnectionTasks.isEmpty())
        newTask = waitingConnectionTasks.dequeue();
    connectionsMutex.unlock();

    if (newTask)
        newTask->start();

}

void AbstractRestServerPrivate::freeTaskThread()
{
    QThread *taskThread = qobject_cast<QThread *>(sender());
    Q_ASSERT(taskThread);
    delete taskThread;
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
    connectionsMutex.lock();
    ServerConnectionTask *task = activeConnectionTasks.value(socket);
    connectionsMutex.unlock();

    if (task)
        task->sendAnswer(socket, body, contentType, returnCode, reason);
}
