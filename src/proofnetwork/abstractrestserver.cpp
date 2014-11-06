#include "abstractrestserver.h"

#include <QDir>
#include <QTcpSocket>
#include <QMetaObject>
#include <QMetaMethod>
#include <QUrlQuery>

static const QString REST_METHOD_PREFIX = QString("rest_");

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
    void tryToCallMethod(QTcpSocket *socket, const QString &type, const QString &method, QStringList headers, const QByteArray &body);
    QStringList makeMethodName(const QString &type, const QString &name);
    QString findMethod(const QStringList &splittedMethod, QStringList &methodVariablePart);
    void fillMethods();
    void addMethodToTree(const QString &realMethod);

    void sendAnswer(QTcpSocket *socket, const QByteArray &body, const QString &contentType, int returnCode = 200, const QString &reason = QString());

    AbstractRestServer *q_ptr = 0;
    int port = 0;
    QString userName;
    QString password;
    QString pathPrefix;
    QThread *thread;

    MethodNode methodsTreeRoot;
};
}

using namespace Proof;

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

    d->thread = new QThread(this);

    d->port = port;
    d->userName = userName;
    d->password = password;
    d->pathPrefix = pathPrefix;

    connect(this, &AbstractRestServer::newConnection, this, [d](){d->createNewConnection();});
    moveToThread(d->thread);
    d->thread->moveToThread(d->thread);
    d->thread->start();
}

AbstractRestServer::~AbstractRestServer()
{
    Q_D(AbstractRestServer);
    d->thread->quit();
    d->thread->wait(1000);
    d->thread->terminate();
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
            qCDebug(proofNetworkMiscLog) << "Server can't start on port" << d->port;
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
    socket->disconnectFromHost();
    if (socket->state() == QTcpSocket::UnconnectedState)
        delete socket;
}

QStringList AbstractRestServerPrivate::makeMethodName(const QString &type, const QString &name)
{
    QStringList splittedName = name.toLower().split("/", QString::SkipEmptyParts);
    splittedName.prepend(type.toLower());
    return splittedName;
}

QString AbstractRestServerPrivate::findMethod(const QStringList &splittedMethod, QStringList &methodVariablePart)
{
    Q_ASSERT(splittedMethod.count() >= 2);

    MethodNode *currentNode = &methodsTreeRoot;
    int i = 0;
    for (; i < splittedMethod.count(); ++i) {
        if (!currentNode->contains(splittedMethod[i]))
            break;
        currentNode = &(*currentNode)[splittedMethod[i]];
    }

    QString methodName = (*currentNode);

    if (!methodName.isEmpty())
        methodVariablePart = splittedMethod.mid(i);

    return methodName;
}

void AbstractRestServerPrivate::fillMethods()
{
    Q_Q(AbstractRestServer);
    methodsTreeRoot.clear();
    for (int i = 0; i < q->metaObject()->methodCount(); ++i) {
        if (q->metaObject()->method(i).methodType() == QMetaMethod::Slot) {
            QString currentMethod = QString(q->metaObject()->method(i).name());
            if (currentMethod.startsWith(REST_METHOD_PREFIX)) {
                addMethodToTree(currentMethod);
            }
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
        if (!currentNode->contains(splittedMethod[i])) {
            (*currentNode)[splittedMethod[i]] = MethodNode();
        }
        currentNode = &(*currentNode)[splittedMethod[i]];
    }
    currentNode->setValue(realMethod);


}

void AbstractRestServerPrivate::tryToCallMethod(QTcpSocket *socket, const QString &type, const QString &method, QStringList headers, const QByteArray &body)
{
    Q_Q(AbstractRestServer);
    QStringList splittedByParamsMethod = method.split('?');
    QStringList methodVariablePart;
    QUrlQuery queryParams;

    Q_ASSERT(splittedByParamsMethod.count());
    if (splittedByParamsMethod.count() > 1)
        queryParams = QUrlQuery(splittedByParamsMethod.at(1));

    QString methodName = findMethod(makeMethodName(type, splittedByParamsMethod.at(0)), methodVariablePart);
    if (!methodName.isEmpty()) {
        QString encryptedAuth;
        for (int i = 1; i < headers.count(); ++i) {
            if (headers.at(i).startsWith("Authorization")) {
                encryptedAuth = q->parseAuth(socket, headers.at(i));
                break;
            }
        }
        if (!encryptedAuth.isEmpty() && q->checkBasicAuth(encryptedAuth)) {
            QMetaObject::invokeMethod(q, methodName.toLatin1().constData(), Qt::AutoConnection,
                                      Q_ARG(QTcpSocket *,socket),
                                      Q_ARG(const QStringList &, headers),
                                      Q_ARG(const QStringList &, methodVariablePart),
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
    QTextStream os(socket);
    os.setAutoDetectUnicode(true);
    os << QString("HTTP/1.0 %1 %2\r\n"
                  "Server: proof\r\n"
                  "Content-Type: %3\r\n"
                  "%4"
                  "\r\n")
          .arg(returnCode)
          .arg(reason)
          .arg(contentType)
          .arg(!body.isEmpty() ? QString("Content-Size: %1\r\n").arg(body.size()) : QString())
          .toUtf8().constData()
       << body << "\r\n";
}
