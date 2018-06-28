#include "abstractrestserver.h"

#include "proofcore/coreapplication.h"
#include "proofcore/errornotifier.h"
#include "proofcore/memorystoragenotificationhandler.h"
#include "proofcore/proofglobal.h"
#include "proofcore/proofobject.h"

#include "proofnetwork/httpparser_p.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMutex>
#include <QNetworkInterface>
#include <QReadWriteLock>
#include <QSet>
#include <QSysInfo>
#include <QTcpSocket>
#include <QUrlQuery>

#include <algorithm>

static const QString REST_METHOD_PREFIX = QStringLiteral("rest_");
static const QString NO_AUTH_TAG = QStringLiteral("NO_AUTH_REQUIRED");
static const int MIN_THREADS_COUNT = 5;

namespace {
class WorkerThread;

class MethodNode
{
public:
    MethodNode();
    bool contains(const QString &name) const;
    void clear();

    operator QString();
    MethodNode &operator[](const QString &name);
    const MethodNode operator[](const QString &name) const;
    void setValue(const QString &value);

    QString tag() const;
    void setTag(const QString &tag);

private:
    QHash<QString, MethodNode> m_nodes;
    QString m_value;
    QString m_tag;
};

struct WorkerThreadInfo
{
    WorkerThreadInfo() {}
    explicit WorkerThreadInfo(WorkerThread *thread, long long socketCount) : thread(thread)
    {
        this->socketCount = socketCount;
    }

    WorkerThreadInfo(const WorkerThreadInfo &other) { *this = other; }

    WorkerThreadInfo &operator=(const WorkerThreadInfo &other)
    {
        thread = other.thread;
        socketCount.store(other.socketCount);
        return *this;
    }

    WorkerThread *thread = nullptr;
    std::atomic_llong socketCount{0};
};

struct SocketInfo
{
    SocketInfo() {}

    Proof::HttpParser parser;
    QMetaObject::Connection readyReadConnection;
    QMetaObject::Connection disconnectConnection;
    QMetaObject::Connection errorConnection;
};

class WorkerThread : public QThread
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
    Proof::AbstractRestServerPrivate *const serverD;
    QHash<QTcpSocket *, SocketInfo> sockets;
};
} // anonymous namespace

namespace Proof {

//TODO: refactor to remove qobject parent
//it is possible now to call methods of any class, not only qobjects
class AbstractRestServerPrivate : public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
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

    void tryToCallMethod(QTcpSocket *socket, const QString &type, const QString &method, const QStringList &headers,
                         const QByteArray &body);
    QStringList makeMethodName(const QString &type, const QString &name);
    MethodNode *findMethod(const QStringList &splittedMethod, QStringList &methodVariableParts);
    void fillMethods();
    void addMethodToTree(const QString &realMethod, const QString &tag);

    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType,
                    const QHash<QString, QString> &headers, int returnCode = 200, const QString &reason = QString());
    void registerSocket(QTcpSocket *socket);
    void deleteSocket(QTcpSocket *socket, WorkerThread *worker);

    AbstractRestServer *q_ptr = nullptr;
    int port = 0;
    QString userName;
    QString password;
    QString pathPrefix;
    QStringList splittedPathPrefix;
    QThread *serverThread = nullptr;
    QVector<WorkerThreadInfo> threadPool;
    QReadWriteLock threadPoolLock;
    QSet<QTcpSocket *> sockets;
    QMutex socketsMutex;
    MethodNode methodsTreeRoot;
    int suggestedMaxThreadsCount = MIN_THREADS_COUNT;
    RestAuthType authType = RestAuthType::NoAuth;
    QHash<QString, QString> customHeaders;
};

} // namespace Proof

using namespace Proof;

AbstractRestServer::AbstractRestServer() : AbstractRestServer(*new AbstractRestServerPrivate, QString(), 80)
{}

AbstractRestServer::AbstractRestServer(int port) : AbstractRestServer(*new AbstractRestServerPrivate, QString(), port)
{}

AbstractRestServer::AbstractRestServer(const QString &pathPrefix, int port)
    : AbstractRestServer(*new AbstractRestServerPrivate, pathPrefix, port)
{}

AbstractRestServer::AbstractRestServer(AbstractRestServerPrivate &dd, const QString &pathPrefix, int port)
    : QTcpServer(), d_ptr(&dd)
{
    Q_D(AbstractRestServer);
    d->q_ptr = this;

    d->serverThread = new QThread();
    d->port = port;
    setPathPrefix(pathPrefix);

    setSuggestedMaxThreadsCount();

    moveToThread(d->serverThread);
    d->serverThread->moveToThread(d->serverThread);
    d->serverThread->start();
}

AbstractRestServer::~AbstractRestServer()
{
    Q_D(AbstractRestServer);
    stopListen();
    d->threadPoolLock.lockForWrite();
    for (const WorkerThreadInfo &workerInfo : qAsConst(d->threadPool)) {
        workerInfo.thread->stop();
        workerInfo.thread->quit();
        workerInfo.thread->wait(1000);
        delete workerInfo.thread;
    }
    d->threadPoolLock.unlock();

    d->serverThread->quit();
    d->serverThread->wait(1000);
    d->serverThread->terminate();
    delete d->serverThread;
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
    QString loweredPathPrefix = pathPrefix.toLower();
    if (d->pathPrefix != loweredPathPrefix) {
        d->pathPrefix = loweredPathPrefix;
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

void AbstractRestServer::setCustomHeader(const QString &header, const QString &value)
{
    Q_D(AbstractRestServer);
    d->customHeaders[header] = value;
}

QString AbstractRestServer::customHeader(const QString &header) const
{
    Q_D(const AbstractRestServer);
    return d->customHeaders.value(header);
}

bool AbstractRestServer::containsCustomHeader(const QString &header) const
{
    Q_D(const AbstractRestServer);
    return d->customHeaders.contains(header);
}

void AbstractRestServer::unsetCustomHeader(const QString &header)
{
    Q_D(AbstractRestServer);
    d->customHeaders.remove(header);
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

void AbstractRestServer::rest_get_System_Status(QTcpSocket *socket, const QStringList &, const QStringList &,
                                                const QUrlQuery &query, const QByteArray &)
{
    QStringList ipsList;
    const auto allIfaces = QNetworkInterface::allInterfaces();
    for (const auto &interface : allIfaces) {
        const auto addressEntries = interface.addressEntries();
        for (const auto &address : addressEntries) {
            if (!address.ip().isLoopback())
                ipsList << QStringLiteral("%1 (%2)").arg(address.ip().toString(), interface.humanReadableName());
        }
    }

    bool quick = query.hasQueryItem(QStringLiteral("quick"));

    QString lastCrashAt(QStringLiteral("N/A"));
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_MAC)
    QByteArray homePath = qgetenv("HOME");
    QDir homeDir(homePath.isEmpty() ? "/tmp" : homePath);
    QFileInfoList crashes = homeDir.entryInfoList({"proof_crash_*"}, QDir::Files);
    if (!crashes.isEmpty()) {
        QDateTime mostRecentCrash = crashes.first().lastModified();
        for (const auto &crash : crashes) {
            if (crash.lastModified() > mostRecentCrash)
                mostRecentCrash = crash.lastModified();
        }
        lastCrashAt = mostRecentCrash.toUTC().toString(Qt::ISODate);
    }
#endif

    QJsonObject statusObj{{"app_type", qApp->applicationName()},
                          {"app_version", qApp->applicationVersion()},
                          {"proof_version", Proof::proofVersion()},
                          {"started_at", proofApp->startedAt().toString(Qt::ISODate)},
                          {"last_crash_at", lastCrashAt},
                          {"os", QSysInfo::prettyProductName()},
                          {"network_addresses", QJsonArray::fromStringList(ipsList)}};

    auto notificationsMemoryStorage = ErrorNotifier::instance()->handler<MemoryStorageNotificationHandler>();
    QPair<QDateTime, QString> lastError;
    if (notificationsMemoryStorage) {
        statusObj[QStringLiteral("app_id")] = notificationsMemoryStorage->appId();
        lastError = notificationsMemoryStorage->lastMessage();
    } else {
        lastError = qMakePair(QDateTime::currentDateTimeUtc(), QStringLiteral("Memory storage error handler not set"));
    }

    statusObj[QStringLiteral("last_error")] = lastError.first.isValid()
                                                  ? QJsonObject{{"timestamp", lastError.first.toString(Qt::ISODate)},
                                                                {"message", lastError.second}}
                                                  : QJsonValue();

    QJsonArray healthArray;
    const auto currentHealthStatus = healthStatus(quick);
    for (auto it = currentHealthStatus.cbegin(); it != currentHealthStatus.cend(); ++it) {
        healthArray.append(QJsonObject{{"name", it.key()},
                                       {"value", QJsonValue::fromVariant(it.value().second)},
                                       {"updated_at", it.value().first.toString(Qt::ISODate)}});
    }
    statusObj[QStringLiteral("health")] = healthArray;

    statusObj[QStringLiteral("generated_at")] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    sendAnswer(socket, QJsonDocument(statusObj).toJson(), QStringLiteral("text/json"));
}

void AbstractRestServer::rest_get_System_RecentErrors(QTcpSocket *socket, const QStringList &, const QStringList &,
                                                      const QUrlQuery &, const QByteArray &)
{
    auto notificationsMemoryStorage = ErrorNotifier::instance()->handler<MemoryStorageNotificationHandler>();
    auto lastErrors = notificationsMemoryStorage
                          ? notificationsMemoryStorage->messages()
                          : QMultiMap<QDateTime, QString>{{QDateTime::currentDateTimeUtc(),
                                                           QStringLiteral("Memory storage error handler not set")}};

    auto errorObjectBuilder = [](const QDateTime &time, const QString &error) -> QJsonObject {
        return QJsonObject{{"timestamp", time.toString(Qt::ISODate)}, {"message", error}};
    };

    QJsonArray recentErrorsArray;
    QList<QDateTime> uniqueErrorsKeys = lastErrors.uniqueKeys();
    std::reverse(uniqueErrorsKeys.begin(), uniqueErrorsKeys.end());
    for (const auto &time : uniqueErrorsKeys) {
        const auto allMessages = lastErrors.values(time);
        for (const auto &message : allMessages)
            recentErrorsArray.append(errorObjectBuilder(time, message));
    }
    sendAnswer(socket, QJsonDocument(recentErrorsArray).toJson(), QStringLiteral("text/json"));
}

QMap<QString, QPair<QDateTime, QVariant>> AbstractRestServer::healthStatus(bool) const
{
    return {};
}

void AbstractRestServer::incomingConnection(qintptr socketDescriptor)
{
    Q_D(AbstractRestServer);

    qCDebug(proofNetworkMiscLog) << "Incoming connection with socket descriptor" << socketDescriptor;

    WorkerThread *worker = nullptr;

    d->threadPoolLock.lockForRead();
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
    d->threadPoolLock.unlock();

    if (worker == nullptr) {
        worker = new WorkerThread(d);
        worker->start();
        d->threadPoolLock.lockForWrite();
        d->threadPool << WorkerThreadInfo(worker, 1);
        d->threadPoolLock.unlock();
    }

    worker->handleNewConnection(socketDescriptor);
}

void AbstractRestServer::sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType,
                                    int returnCode, const QString &reason)
{
    Q_D(AbstractRestServer);
    d->sendAnswer(socket, body, contentType, QHash<QString, QString>(), returnCode, reason);
}

void AbstractRestServer::sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType,
                                    const QHash<QString, QString> &headers, int returnCode, const QString &reason)
{
    Q_D(AbstractRestServer);
    d->sendAnswer(socket, body, contentType, headers, returnCode, reason);
}

void AbstractRestServer::sendErrorCode(QTcpSocket *socket, int returnCode, const QString &reason, int errorCode,
                                       const QStringList &args)
{
    QJsonObject body;
    body.insert(QStringLiteral("error_code"), errorCode);
    if (!args.empty()) {
        QJsonArray jsonArgs;
        for (const auto &arg : args)
            jsonArgs << arg;
        body.insert(QStringLiteral("message_args"), jsonArgs);
    }
    sendAnswer(socket, QJsonDocument(body).toJson(QJsonDocument::Compact), QStringLiteral("application/json"),
               returnCode, reason);
}

bool AbstractRestServer::checkBasicAuth(const QString &encryptedAuth) const
{
    Q_D(const AbstractRestServer);
    QByteArray auth = QStringLiteral("%1:%2").arg(d->userName, d->password).toLatin1().toBase64();
    if (encryptedAuth == auth)
        return true;
    return false;
}

QString AbstractRestServer::parseAuth(QTcpSocket *socket, const QString &header)
{
    QString auth;
    QStringList parts = header.split(QStringLiteral(":"));
    if (parts.count() != 2) {
        sendInternalError(socket);
    } else {
        parts = parts.at(1).split(QStringLiteral(" "), QString::SkipEmptyParts);
        if (parts.count() != 2 || parts.at(0) != QLatin1String("Basic"))
            sendNotAuthorized(socket);
        else
            auth = parts.at(1);
    }
    return auth;
}

void AbstractRestServer::sendBadRequest(QTcpSocket *socket, const QString &reason)
{
    sendAnswer(socket, "", QStringLiteral("text/plain; charset=utf-8"), 400, reason);
}

void AbstractRestServer::sendNotFound(QTcpSocket *socket, const QString &reason)
{
    sendAnswer(socket, "", QStringLiteral("text/plain; charset=utf-8"), 404, reason);
}

void AbstractRestServer::sendNotAuthorized(QTcpSocket *socket, const QString &reason)
{
    sendAnswer(socket, "", QStringLiteral("text/plain; charset=utf-8"), 401, reason);
}

void AbstractRestServer::sendInternalError(QTcpSocket *socket)
{
    sendAnswer(socket, "", QStringLiteral("text/plain; charset=utf-8"), 500, QStringLiteral("Internal Server Error"));
}

QStringList AbstractRestServerPrivate::makeMethodName(const QString &type, const QString &name)
{
    QStringList splittedName = name.split(QStringLiteral("/"), QString::SkipEmptyParts);
    bool isStartedWithPrefix = splittedName.size() >= splittedPathPrefix.size()
                               && std::equal(splittedPathPrefix.cbegin(), splittedPathPrefix.cend(),
                                             splittedName.cbegin(),
                                             [](const QString &prefixPart, const QString &namePart) {
                                                 return prefixPart == namePart.toLower();
                                             });
    if (isStartedWithPrefix == false)
        return QStringList();
    splittedName.erase(splittedName.begin(), splittedName.begin() + splittedPathPrefix.size());
    splittedName.prepend(type.toLower());
    return splittedName;
}

MethodNode *AbstractRestServerPrivate::findMethod(const QStringList &splittedMethod, QStringList &methodVariableParts)
{
    if (splittedMethod.count() < 2)
        return nullptr;

    MethodNode *currentNode = &methodsTreeRoot;
    int i = 0;
    for (; i < splittedMethod.count(); ++i) {
        QString loweredPart = splittedMethod[i].toLower();
        if (!currentNode->contains(loweredPart))
            break;
        currentNode = &(*currentNode)[loweredPart];
    }

    QString methodName = (*currentNode);

    if (!methodName.isEmpty()) {
        QStringList partsForDecode = splittedMethod.mid(i);
        for (QString &part : partsForDecode)
            part = QString(QByteArray::fromPercentEncoding(part.toUtf8()));
        methodVariableParts = partsForDecode;
        return currentNode;
    }

    return nullptr;
}

void AbstractRestServerPrivate::fillMethods()
{
    Q_Q(AbstractRestServer);
    methodsTreeRoot.clear();
    for (int i = 0; i < q->metaObject()->methodCount(); ++i) {
        QMetaMethod method = q->metaObject()->method(i);
        if (method.methodType() == QMetaMethod::Slot) {
            QString currentMethod = QString(method.name());
            if (currentMethod.startsWith(REST_METHOD_PREFIX))
                addMethodToTree(currentMethod, method.tag());
        }
    }
}

void AbstractRestServerPrivate::addMethodToTree(const QString &realMethod, const QString &tag)
{
    QString method = realMethod.mid(QString(REST_METHOD_PREFIX).length());
    for (int i = 0; i < method.length(); ++i) {
        if (method[i].isUpper()) {
            method[i] = method[i].toLower();
            if (i > 0 && method[i - 1] != '_')
                method.insert(i++, '-');
        }
    }

    QStringList splittedMethod = method.split(QStringLiteral("_"));
    Q_ASSERT(splittedMethod.count() >= 2);

    MethodNode *currentNode = &methodsTreeRoot;
    for (int i = 0; i < splittedMethod.count(); ++i) {
        if (!currentNode->contains(splittedMethod[i]))
            (*currentNode)[splittedMethod[i]] = MethodNode();
        currentNode = &(*currentNode)[splittedMethod[i]];
    }
    currentNode->setValue(realMethod);
    currentNode->setTag(tag);
}

void AbstractRestServerPrivate::tryToCallMethod(QTcpSocket *socket, const QString &type, const QString &method,
                                                const QStringList &headers, const QByteArray &body)
{
    Q_Q(AbstractRestServer);
    QStringList splittedByParamsMethod = method.split('?');
    QStringList methodVariableParts;
    QUrlQuery queryParams;

    Q_ASSERT(splittedByParamsMethod.count());
    if (splittedByParamsMethod.count() > 1)
        queryParams = QUrlQuery(splittedByParamsMethod.at(1));

    MethodNode *methodNode = findMethod(makeMethodName(type, splittedByParamsMethod.at(0)), methodVariableParts);
    QString methodName = methodNode ? (*methodNode) : QString();
    qCDebug(proofNetworkMiscLog) << "Request for" << method << "associated with" << methodName << "at socket" << socket;

    if (methodNode) {
        bool isAuthenticationSuccessful = true;
        if (authType == RestAuthType::Basic && methodNode->tag() != NO_AUTH_TAG) {
            QString encryptedAuth;
            for (int i = 0; i < headers.count(); ++i) {
                if (headers.at(i).startsWith(QLatin1String("Authorization"), Qt::CaseInsensitive)) {
                    encryptedAuth = q->parseAuth(socket, headers.at(i));
                    break;
                }
            }
            isAuthenticationSuccessful = (!encryptedAuth.isEmpty() && q->checkBasicAuth(encryptedAuth));
        }
        if (isAuthenticationSuccessful) {
            QMetaObject::invokeMethod(q, methodName.toLatin1().constData(), Qt::DirectConnection,
                                      Q_ARG(QTcpSocket *, socket), Q_ARG(const QStringList &, headers),
                                      Q_ARG(const QStringList &, methodVariableParts),
                                      Q_ARG(const QUrlQuery &, queryParams), Q_ARG(const QByteArray &, body));
        } else {
            q->sendNotAuthorized(socket);
        }
    } else {
        q->sendNotFound(socket, QStringLiteral("Wrong method"));
    }
}

void AbstractRestServerPrivate::sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType,
                                           const QHash<QString, QString> &headers, int returnCode, const QString &reason)
{
    WorkerThread *worker = nullptr;
    {
        QMutexLocker lock(&socketsMutex);
        if (sockets.contains(socket))
            worker = qobject_cast<WorkerThread *>(socket->thread());
    }
    if (worker != nullptr) {
        qCDebug(proofNetworkMiscLog) << "Replying" << returnCode << ":" << reason << "at socket" << socket;
        worker->sendAnswer(socket, body, contentType, headers, returnCode, reason);
    } else {
        qCDebug(proofNetworkMiscLog).noquote()
            << "Wanted to reply" << returnCode << ":" << reason << "at socket"
            << QStringLiteral("QTcpSocket(%1)").arg(reinterpret_cast<quint64>(socket), 0, 16)
            << "but it is dead already";
    }
}

void AbstractRestServerPrivate::registerSocket(QTcpSocket *socket)
{
    QMutexLocker lock(&socketsMutex);
    sockets.insert(socket);
}

void AbstractRestServerPrivate::deleteSocket(QTcpSocket *socket, WorkerThread *worker)
{
    {
        QMutexLocker lock(&socketsMutex);
        auto iter = sockets.constFind(socket);
        if (iter != sockets.end())
            sockets.erase(iter);
        else
            return;
    }
    delete socket;
    threadPoolLock.lockForRead();
    auto iter = std::find_if(threadPool.begin(), threadPool.end(),
                             [worker](const WorkerThreadInfo &info) { return info.thread == worker; });
    if (iter != threadPool.end())
        --iter->socketCount;
    threadPoolLock.unlock();
}

WorkerThread::WorkerThread(Proof::AbstractRestServerPrivate *const _server_d) : serverD(_server_d)
{
    moveToThread(this);
}

WorkerThread::~WorkerThread()
{}

void WorkerThread::handleNewConnection(qintptr socketDescriptor)
{
    if (Proof::ProofObject::call(this, &WorkerThread::handleNewConnection, socketDescriptor))
        return;

    QTcpSocket *tcpSocket = new QTcpSocket();
    serverD->registerSocket(tcpSocket);
    SocketInfo info;
    info.readyReadConnection = connect(tcpSocket, &QTcpSocket::readyRead, this,
                                       [tcpSocket, this] { onReadyRead(tcpSocket); }, Qt::QueuedConnection);

    void (QTcpSocket::*errorSignal)(QAbstractSocket::SocketError) = &QTcpSocket::error;
    info.errorConnection = connect(tcpSocket, errorSignal, this,
                                   [tcpSocket] {
                                       qCWarning(proofNetworkMiscLog)
                                           << "RestServer: socket error:" << tcpSocket->errorString();
                                   },
                                   Qt::QueuedConnection);

    info.disconnectConnection = connect(tcpSocket, &QTcpSocket::disconnected, this,
                                        [tcpSocket, this] { deleteSocket(tcpSocket); }, Qt::QueuedConnection);

    if (!tcpSocket->setSocketDescriptor(socketDescriptor)) {
        qCWarning(proofNetworkMiscLog) << "RestServer: can't create socket, error:" << tcpSocket->errorString();
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
        serverD->tryToCallMethod(socket, info.parser.method(), info.parser.uri(), info.parser.headers(),
                                 info.parser.body());
        break;
    case HttpParser::Result::Error:
        qCWarning(proofNetworkMiscLog) << "RestServer: parse error:" << info.parser.error();
        disconnect(info.readyReadConnection);
        sendAnswer(socket, "", QStringLiteral("text/plain; charset=utf-8"), QHash<QString, QString>(), 400,
                   QStringLiteral("Bad Request"));
        break;
    case HttpParser::Result::NeedMore:
        break;
    }
}

void WorkerThread::stop()
{
    if (!ProofObject::call(this, &WorkerThread::stop, Proof::Call::Block)) {
        const auto allKeys = sockets.keys();
        for (QTcpSocket *socket : allKeys)
            deleteSocket(socket);
    }
}

void WorkerThread::sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType,
                              const QHash<QString, QString> &headers, int returnCode, const QString &reason)
{
    if (Proof::ProofObject::call(this, &WorkerThread::sendAnswer, socket, body, contentType, headers, returnCode, reason)) {
        return;
    }

    if (sockets.contains(socket) && socket->state() == QTcpSocket::ConnectedState) {
        QStringList additionalHeadersList;
        additionalHeadersList << QStringLiteral("Proof-Application: %1").arg(proofApp->prettifiedApplicationName());
        additionalHeadersList << QStringLiteral("Proof-%1-Version: %2")
                                     .arg(proofApp->prettifiedApplicationName(), qApp->applicationVersion());
        additionalHeadersList << QStringLiteral("Proof-%1-Framework-Version: %2")
                                     .arg(proofApp->prettifiedApplicationName(), Proof::proofVersion());
        for (auto it = serverD->customHeaders.cbegin(); it != serverD->customHeaders.cend(); ++it)
            additionalHeadersList << QStringLiteral("%1: %2").arg(it.key(), it.value());
        for (auto it = headers.cbegin(); it != headers.cend(); ++it)
            additionalHeadersList << QStringLiteral("%1: %2").arg(it.key(), it.value());
        QString additionalHeaders = additionalHeadersList.join(QStringLiteral("\r\n")) + "\r\n";

        //TODO: Add support for keep-alive
        socket->write(QStringLiteral("HTTP/1.1 %1 %2\r\n"
                                     "Server: proof\r\n"
                                     "Connection: closed\r\n"
                                     "Content-Type: %3\r\n"
                                     "%4"
                                     "%5"
                                     "\r\n")
                          .arg(QString::number(returnCode), reason, contentType,
                               !body.isEmpty() ? QStringLiteral("Content-Length: %1\r\n").arg(body.size()) : QString(),
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
{}

bool MethodNode::contains(const QString &name) const
{
    return m_nodes.contains(name);
}

void MethodNode::clear()
{
    m_nodes.clear();
}

MethodNode::operator QString()
{
    return m_value;
}

MethodNode &MethodNode::operator[](const QString &name)
{
    return m_nodes[name];
}

const MethodNode MethodNode::operator[](const QString &name) const
{
    return m_nodes[name];
}

void MethodNode::setValue(const QString &value)
{
    m_value = value;
}

QString MethodNode::tag() const
{
    return m_tag;
}

void MethodNode::setTag(const QString &tag)
{
    m_tag = tag;
}

#include "abstractrestserver.moc"
