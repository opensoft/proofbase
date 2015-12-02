#include "abstractrestserver.h"

#include "proofcore/proofobject.h"
#include "proofnetwork/httpparser_p.h"

#include <QDir>
#include <QTcpSocket>
#include <QMetaObject>
#include <QMetaMethod>
#include <QUrlQuery>
#include <QSet>
#include <QMutex>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <algorithm>

static const QString REST_METHOD_PREFIX = QString("rest_");
static const int MIN_THREADS_COUNT = 5;

namespace {
class WorkerThread;

class MethodNode {
public:
    MethodNode();
    bool contains(const QString &name) const;
    void clear();

    operator QString();
    MethodNode &operator [](const QString &name);
    const MethodNode operator [](const QString &name) const;
    void setValue(const QString &_value);

private:
    QHash<QString, MethodNode> nodes;
    QString value = "";
};

struct WorkerThreadInfo
{
    explicit WorkerThreadInfo(WorkerThread *thread, quint32 socketCount)
        : thread(thread), socketCount(socketCount)
    {
    }

    WorkerThread *thread;
    quint32 socketCount;
};

struct SocketInfo
{
    SocketInfo()
    {
    }

    Proof::HttpParser parser;
    QMetaObject::Connection readyReadConnection;
    QMetaObject::Connection disconnectConnection;
    QMetaObject::Connection errorConnection;
};

class WorkerThread: public QThread
{
    Q_OBJECT
public:
    WorkerThread(Proof::AbstractRestServerPrivate *const _serverD);
    ~WorkerThread();

    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType,
                    const QHash<QString, QString> &headers, int returnCode, const QString &reason);
    void handleNewConnection(qintptr socketDescriptor);
    void deleteSocket(QTcpSocket *socket);
    void onReadyRead(QTcpSocket *socket);
    void stop();

private:
    Proof::AbstractRestServerPrivate * const serverD;
    QHash<QTcpSocket *, SocketInfo> sockets;
};
} // anonymous namespace

namespace Proof {

class AbstractRestServerPrivate: public QObject
{
    Q_DECLARE_PUBLIC(AbstractRestServer)
    friend WorkerThread;

public:
    AbstractRestServerPrivate() {}
    virtual ~AbstractRestServerPrivate() {}

private:
    AbstractRestServerPrivate(const AbstractRestServerPrivate &other) = delete;
    AbstractRestServerPrivate &operator=(const AbstractRestServerPrivate &other) = delete;
    AbstractRestServerPrivate(const AbstractRestServerPrivate &&other) = delete;
    AbstractRestServerPrivate &operator=(const AbstractRestServerPrivate &&other) = delete;

    void tryToCallMethod(QTcpSocket *socket, const QString &type, const QString &method, QStringList headers, const QByteArray &body);
    QStringList makeMethodName(const QString &type, const QString &name);
    QString findMethod(const QStringList &splittedMethod, QStringList &methodVariableParts);
    void fillMethods();
    void addMethodToTree(const QString &realMethod);

    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, const QHash<QString, QString> &headers,
                    int returnCode = 200, const QString &reason = QString());
    void registerSocket(QTcpSocket *socket);
    void deleteSocket(QTcpSocket *socket, WorkerThread *worker);

    AbstractRestServer *q_ptr = 0;
    int port = 0;
    QString userName;
    QString password;
    QString pathPrefix;
    QStringList splittedPathPrefix;
    QThread *serverThread = nullptr;
    QList<WorkerThreadInfo> threadPool;
    QSet<QTcpSocket *> sockets;
    QMutex socketsMutex;
    MethodNode methodsTreeRoot;
    int suggestedMaxThreadsCount;
    RestAuthType authType;
};

}

using namespace Proof;

AbstractRestServer::AbstractRestServer(QObject *parent)
    : AbstractRestServer(*new AbstractRestServerPrivate, "", "", "", 80, RestAuthType::NoAuth, parent)
{
}

AbstractRestServer::AbstractRestServer(const QString &pathPrefix, int port, RestAuthType authType, QObject *parent)
    : AbstractRestServer(*new AbstractRestServerPrivate, "", "", pathPrefix, port, authType, parent)
{
}

AbstractRestServer::AbstractRestServer(const QString &userName, const QString &password, const QString &pathPrefix, int port,
                                       QObject *parent)
    : AbstractRestServer(*new AbstractRestServerPrivate, userName, password, pathPrefix, port, RestAuthType::Basic, parent)
{
}

AbstractRestServer::AbstractRestServer(AbstractRestServerPrivate &dd, const QString &userName, const QString &password,
                                       const QString &pathPrefix, int port, RestAuthType authType, QObject *parent)
    : QTcpServer(parent),
      d_ptr(&dd)
{
    Q_D(AbstractRestServer);
    d->q_ptr = this;

    d->serverThread = new QThread(this);
    d->port = port;
    d->userName = userName;
    d->password = password;
    setPathPrefix(pathPrefix);
    d->authType = authType;

    setSuggestedMaxThreadsCount();

    moveToThread(d->serverThread);
    d->serverThread->moveToThread(d->serverThread);
    d->serverThread->start();
}

AbstractRestServer::~AbstractRestServer()
{
    Q_D(AbstractRestServer);
    stopListen();
    for (const WorkerThreadInfo &workerInfo : d->threadPool) {
        workerInfo.thread->stop();
        workerInfo.thread->quit();
        workerInfo.thread->wait(1000);
        delete workerInfo.thread;
    }

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

RestAuthType AbstractRestServer::authType() const
{
    Q_D(const AbstractRestServer);
    return d->authType;
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
        d->splittedPathPrefix = d->pathPrefix.split('/', QString::SkipEmptyParts);
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

void AbstractRestServer::setSuggestedMaxThreadsCount(int count)
{
    Q_D(AbstractRestServer);
    if (count < 0) {
        count = QThread::idealThreadCount();
        if (count < MIN_THREADS_COUNT)
            count = MIN_THREADS_COUNT;
        else
            count += 2;
    }
    d->suggestedMaxThreadsCount = count;
}

void AbstractRestServer::setAuthType(RestAuthType authType)
{
    Q_ASSERT(authType == RestAuthType::NoAuth || authType == RestAuthType::Basic);
    Q_D(AbstractRestServer);
    if (d->authType != authType) {
        d->authType = authType;
        emit authTypeChanged(d->authType);
    }
}

void AbstractRestServer::startListen()
{
    Q_D(AbstractRestServer);
    if (!ProofObject::call(this, &AbstractRestServer::startListen)) {
        d->fillMethods();
        bool isListen = listen(QHostAddress::Any, d->port);
        if (!isListen)
            qCCritical(proofNetworkMiscLog) << "Server can't start on port" << d->port;
    }
}

void AbstractRestServer::stopListen()
{
    if (!ProofObject::call(this, &AbstractRestServer::stopListen, Proof::Call::Block))
        close();
}

void AbstractRestServer::incomingConnection(qintptr socketDescriptor)
{
    Q_D(AbstractRestServer);

    qCDebug(proofNetworkMiscLog) << "Incoming connection with socket descriptor" << socketDescriptor;

    WorkerThread *worker = nullptr;

    if (!d->threadPool.isEmpty()) {
        auto iter = std::min_element(d->threadPool.begin(), d->threadPool.end(),
                                     [](const WorkerThreadInfo &lhs, const WorkerThreadInfo &rhs) {
                                         return lhs.socketCount < rhs.socketCount;
                                     });
        if (iter->socketCount == 0 || d->threadPool.count() >= d->suggestedMaxThreadsCount) {
            worker = iter->thread;
            ++iter->socketCount;
        }
    }

    if (worker == nullptr) {
        worker = new WorkerThread(d);
        worker->start();
        d->threadPool << WorkerThreadInfo{worker, 1};
    }

    worker->handleNewConnection(socketDescriptor);
}

void AbstractRestServer::sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, int returnCode, const QString &reason)
{
    Q_D(AbstractRestServer);
    d->sendAnswer(socket, body, contentType, QHash<QString, QString>(), returnCode, reason);
}

void AbstractRestServer::sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, const QHash<QString, QString> &headers, int returnCode, const QString &reason)
{
    Q_D(AbstractRestServer);
    d->sendAnswer(socket, body, contentType, headers, returnCode, reason);
}

void AbstractRestServer::sendErrorCode(QTcpSocket *socket, int returnCode, const QString &reason, int errorCode, const QStringList &args)
{
    QJsonObject body;
    body.insert("error_code", errorCode);
    if (!args.empty()) {
        QJsonArray jsonArgs;
        for (const auto &arg : args)
            jsonArgs << arg;
        body.insert("message_args", jsonArgs);
    }
    sendAnswer(socket, QJsonDocument(body).toJson(QJsonDocument::Compact), "application/json", returnCode, reason);
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

QStringList AbstractRestServerPrivate::makeMethodName(const QString &type, const QString &name)
{
    QStringList splittedName = name.split("/", QString::SkipEmptyParts);
    bool isStartedWithPrefix = splittedName.size() >= splittedPathPrefix.size() &&
            std::equal(splittedPathPrefix.cbegin(), splittedPathPrefix.cend(), splittedName.cbegin(),
                       [](const QString &prefixPart, const QString &namePart) { return prefixPart == namePart.toLower(); });
    if (isStartedWithPrefix == false)
        return QStringList();
    splittedName.erase(splittedName.begin(), splittedName.begin() + splittedPathPrefix.size());
    splittedName.prepend(type.toLower());
    return splittedName;
}

QString AbstractRestServerPrivate::findMethod(const QStringList &splittedMethod, QStringList &methodVariableParts)
{
    if (splittedMethod.count() < 2)
        return QString();

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
    qCDebug(proofNetworkMiscLog) << "Request for" << method << "associated with" << methodName << "at socket" << socket;

    if (!methodName.isEmpty()) {
        bool isAuthenticationSuccessful = true;
        if (authType == RestAuthType::Basic) {
            QString encryptedAuth;
            for (int i = 0; i < headers.count(); ++i) {
                if (headers.at(i).startsWith("Authorization")) {
                    encryptedAuth = q->parseAuth(socket, headers.at(i));
                    break;
                }
            }
            isAuthenticationSuccessful = (!encryptedAuth.isEmpty() && q->checkBasicAuth(encryptedAuth));
        }
        if (isAuthenticationSuccessful) {
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

void AbstractRestServerPrivate::sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, const QHash<QString, QString> &headers,
                                           int returnCode, const QString &reason)
{
    qCDebug(proofNetworkMiscLog) << "Replying" << returnCode << ":" << reason << "at socket" << socket;
    WorkerThread *worker = nullptr;
    {
        QMutexLocker lock(&socketsMutex);
        if (sockets.contains(socket))
            worker = qobject_cast<WorkerThread *>(socket->thread());
    }
    if (worker != nullptr)
        worker->sendAnswer(socket, body, contentType, headers, returnCode, reason);
}

void AbstractRestServerPrivate::registerSocket(QTcpSocket *socket)
{
    QMutexLocker lock(&socketsMutex);
    sockets.insert(socket);
}

void AbstractRestServerPrivate::deleteSocket(QTcpSocket *socket, WorkerThread *worker)
{
    if (!ProofObject::call(this, &AbstractRestServerPrivate::deleteSocket, socket, worker)) {
        {
            QMutexLocker lock(&socketsMutex);
            auto iter = sockets.find(socket);
            if (iter != sockets.end())
                sockets.erase(iter);
            else
                return;
        }
        delete socket;
        auto iter = std::find_if(threadPool.begin(), threadPool.end(), [worker](const WorkerThreadInfo &info) {
            return info.thread == worker;
        });
        --iter->socketCount;
    }
}

WorkerThread::WorkerThread(Proof::AbstractRestServerPrivate *const _server_d)
    : serverD(_server_d)
{
    moveToThread(this);
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::handleNewConnection(qintptr socketDescriptor)
{
    if (Proof::ProofObject::call(this, &WorkerThread::handleNewConnection, socketDescriptor))
        return;

    QTcpSocket *tcpSocket = new QTcpSocket();
    serverD->registerSocket(tcpSocket);
    SocketInfo info;
    info.readyReadConnection = connect(tcpSocket, &QTcpSocket::readyRead, this, [tcpSocket, this] { onReadyRead(tcpSocket); }, Qt::QueuedConnection);

    void (QTcpSocket:: *errorSignal)(QAbstractSocket::SocketError) = &QTcpSocket::error;
    info.errorConnection = connect(tcpSocket, errorSignal, this, [tcpSocket, this] {
        qCDebug(proofNetworkMiscLog) << "Socket error:" << tcpSocket->errorString();
    }, Qt::QueuedConnection);

    info.disconnectConnection = connect(tcpSocket, &QTcpSocket::disconnected, this, [tcpSocket, this] { deleteSocket(tcpSocket); }, Qt::QueuedConnection);

    if (!tcpSocket->setSocketDescriptor(socketDescriptor)) {
        qCDebug(proofNetworkMiscLog) << "Can't create socket, error:" << tcpSocket->errorString();
        serverD->deleteSocket(tcpSocket, this);
        return;
    }
    sockets[tcpSocket] = info;
    qCDebug(proofNetworkMiscLog) << "Handling socket descriptor" << socketDescriptor << "with socket" << tcpSocket;
}

void WorkerThread::deleteSocket(QTcpSocket *socket)
{
    sockets.remove(socket);
    serverD->deleteSocket(socket, this);
}

void WorkerThread::onReadyRead(QTcpSocket *socket)
{
    SocketInfo &info = sockets[socket];
    HttpParser::Result result = info.parser.parseNextPart(socket->readAll());
    switch (result) {
    case HttpParser::Result::Success:
        disconnect(info.readyReadConnection);
        serverD->tryToCallMethod(socket, info.parser.method(), info.parser.uri(), info.parser.headers(), info.parser.body());
        break;
    case HttpParser::Result::Error:
        qCDebug(proofNetworkMiscLog) << "Parse error:" << info.parser.error();
        disconnect(info.readyReadConnection);
        sendAnswer(socket, "", "text/plain; charset=utf-8", QHash<QString, QString>(), 400, "Bad Request");
        break;
    case HttpParser::Result::NeedMore:
        break;
    }
}

void WorkerThread::stop()
{
    if (!ProofObject::call(this, &WorkerThread::stop, Proof::Call::Block)) {
        for (QTcpSocket *socket : sockets.keys())
            deleteSocket(socket);
    }
}

void WorkerThread::sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType,
                              const QHash<QString, QString> &headers, int returnCode, const QString &reason)
{
    if (Proof::ProofObject::call(this,
                                 &WorkerThread::sendAnswer,
                                 socket, body, contentType, headers, returnCode, reason)) {
        return;
    }

    if (sockets.contains(socket) && socket->state() == QTcpSocket::ConnectedState) {
        QString additionalHeaders;
        if (!headers.isEmpty()) {
            QStringList stringified;
            for (const QString &key : headers.keys())
                stringified << QString("%1: %2").arg(key, headers[key]);
            additionalHeaders = stringified.join("\r\n") + "\r\n";
        }

        //TODO: Add support for keep-alive
        socket->write(QString("HTTP/1.1 %1 %2\r\n"
                              "Server: proof\r\n"
                              "Connection: closed\r\n"
                              "Content-Type: %3\r\n"
                              "%4"
                              "%5"
                              "\r\n")
                      .arg(QString::number(returnCode),
                           reason,
                           contentType,
                           !body.isEmpty() ? QString("Content-Length: %1\r\n").arg(body.size()) : QString(),
                           additionalHeaders)
                      .toUtf8());

        socket->write(body);
        connect(socket, &QTcpSocket::bytesWritten, this, [socket] {
            if (socket->bytesToWrite() == 0)
                socket->disconnectFromHost();
        });
    }
}

MethodNode::MethodNode()
{

}

bool MethodNode::contains(const QString &name) const
{
    return nodes.contains(name);
}

void MethodNode::clear()
{
    nodes.clear();
}

MethodNode::operator QString()
{
    return value;
}

MethodNode &MethodNode::operator [](const QString &name)
{
    return nodes[name];
}

const MethodNode MethodNode::operator [](const QString &name) const
{
    return nodes[name];
}

void MethodNode::setValue(const QString &_value)
{
    value = _value;
}

#include "abstractrestserver.moc"
