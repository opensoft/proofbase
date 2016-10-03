#include "smtpclient.h"

#include "proofcore/proofobject_p.h"
#include "proofcore/taskchain.h"

#include <QTcpSocket>
#include <QSslSocket>

namespace Proof {
class SmtpClientPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(SmtpClient)

    QString userName;
    QString password;
    QString host;
    int port = 25;
    SmtpClient::ConnectionType connectionType = SmtpClient::ConnectionType::Plain;
};
}

using namespace Proof;

SmtpClient::SmtpClient()
    : ProofObject(*new SmtpClientPrivate)
{

}

QString SmtpClient::userName() const
{
    Q_D(const SmtpClient);
    return d->userName;
}

void SmtpClient::setUserName(const QString &arg)
{
    Q_D(SmtpClient);
    if (d->userName != arg) {
        d->userName = arg;
        emit userNameChanged(arg);
    }
}

QString SmtpClient::password() const
{
    Q_D(const SmtpClient);
    return d->password;
}

void SmtpClient::setPassword(const QString &arg)
{
    Q_D(SmtpClient);
    if (d->password != arg) {
        d->password = arg;
        emit passwordChanged(arg);
    }
}

QString SmtpClient::host() const
{
    Q_D(const SmtpClient);
    return d->host;
}

void SmtpClient::setHost(const QString &arg)
{
    Q_D(SmtpClient);
    if (d->host != arg) {
        d->host = arg;
        emit hostChanged(arg);
    }
}

int SmtpClient::port() const
{
    Q_D(const SmtpClient);
    return d->port;
}

void SmtpClient::setPort(int arg)
{
    Q_D(SmtpClient);
    if (d->port != arg) {
        d->port = arg;
        emit portChanged(arg);
    }
}
SmtpClient::ConnectionType SmtpClient::connectionType() const
{
    Q_D(const SmtpClient);
    return d->connectionType;
}

void SmtpClient::setConnectionType(SmtpClient::ConnectionType arg)
{
    Q_D(SmtpClient);
    if (d->connectionType != arg) {
        d->connectionType = arg;
        emit connectionTypeChanged(arg);
    }
}

void SmtpClient::sendMail(const QString &subject, const QString &body, const QString &from, const QStringList &to)
{
    auto taskChain = Proof::TaskChain::createChain();
    taskChain->addTask([this, taskChain, subject, body, from, to]() {
        sendMail(taskChain, subject, body, from, to);
    });
}

bool SmtpClient::sendMailSync(const QString &subject, const QString &body, const QString &from, const QStringList &to)
{
    auto taskChain = Proof::TaskChain::createChain();
    bool result = false;
    taskChain->waitForTask(taskChain->addTask([this, taskChain, subject, body, from, to, &result]() {
        result = sendMail(taskChain, subject, body, from, to);
    }));
    return result;
}

bool SmtpClient::sendMail(const TaskChainSP &taskChain, const QString &subject, const QString &body, const QString &from, const QStringList &to)
{
    Q_D(SmtpClient);
    qCDebug(proofNetworkMiscLog) << "Sending email from" << from << "to" << to;
    QByteArray userNameBase64 = d->userName.toLocal8Bit().toBase64();
    QByteArray passwordBase64 = d->password.toLocal8Bit().toBase64();

    QString domain = from;
    QString senderEmail = from;

    QRegExp addressParser("([^<>]*@([^<>]*))|.*<(.*@(.*))>");
    if (addressParser.exactMatch(from)) {
        domain = addressParser.cap(2).isEmpty() ? addressParser.cap(4) : addressParser.cap(2);
        senderEmail = addressParser.cap(1).isEmpty() ? addressParser.cap(3) : addressParser.cap(1);
    }

    QByteArray message;
    QStringList mutableTo;
    for (const auto &recv : to) {
        QString recvEmail = recv;
        if (addressParser.exactMatch(recv))
            recvEmail = addressParser.cap(1).isEmpty() ? addressParser.cap(3) : addressParser.cap(1);
        message.append("To: " + recv.toUtf8() + "\n");
        mutableTo << recvEmail;
    }

    message.append("From: " + from.toUtf8() + "\n");
    message.append("Subject: " + subject.toUtf8() + "\n");
    message.append("Mime-Version: 1.0;\n");
    message.append("Content-Type: text/plain; charset=\"utf8\";\n");
    message.append("Content-Transfer-Encoding: 8bit;\n");
    message.append("\n");
    message.append(body.toUtf8());
    message.replace("\n", "\r\n");
    message.replace("\r\n.\r\n", "\r\n..\r\n");

    enum class SmtpStates {Init, Auth, User, Pass, Mail, Rcpt, Data, Body, Quit, Close};

    SmtpStates state = SmtpStates::Init;
    bool result = false;

    QTcpSocket *socket = (d->connectionType == ConnectionType::Ssl) ? new QSslSocket : new QTcpSocket;
    std::function<bool()> readyReadCallback = [this, &socket, &state, message, senderEmail, domain, userNameBase64, passwordBase64, &mutableTo, &result]() {
        QString reply = socket->readAll();
        int stateReply = reply.left(3).toInt();
        QByteArray toSend;
        //TODO: add support for actual response for EHLO with auth types here
        if (state == SmtpStates::Init && stateReply == 220) {
            toSend = QString("EHLO " + domain).toUtf8();
            state = SmtpStates::Auth;
        } else if (state == SmtpStates::Auth && stateReply == 250) {
            toSend = "AUTH LOGIN";
            state = SmtpStates::User;
        } else if (state == SmtpStates::User && stateReply == 334) {
            toSend = userNameBase64;
            state = SmtpStates::Pass;
        } else if (state == SmtpStates::Pass && stateReply == 334) {
            toSend = passwordBase64;
            state = SmtpStates::Mail;
        } else if (state == SmtpStates::Mail && stateReply == 235) {
            toSend = QString("MAIL FROM: <" + senderEmail + ">").toUtf8();
            state = SmtpStates::Rcpt;
        } else if (state == SmtpStates::Rcpt && stateReply == 250) {
            if (!mutableTo.isEmpty()) {
                toSend = QString("RCPT TO: <" + mutableTo.takeFirst() + ">").toUtf8();
                state = mutableTo.isEmpty() ? SmtpStates::Data : SmtpStates::Rcpt;
            } else {
                toSend = "DATA";
                state = SmtpStates::Body;
            }
        } else if (state == SmtpStates::Data && stateReply == 250) {
            toSend = "DATA";
            state = SmtpStates::Body;
        } else if (state == SmtpStates::Body && stateReply == 354) {
            toSend = message + "\r\n.";
            state = SmtpStates::Quit;
        } else if (state == SmtpStates::Quit && stateReply == 250) {
            toSend = "QUIT";
            state = SmtpStates::Close;
        } else if (state == SmtpStates::Close) {
            result = true;
            socket->close();
            return true;
        } else {
            state = SmtpStates::Close;
            socket->close();
            return true;
        }
        socket->write(toSend);
        socket->write("\r\n");
        return true;
    };

    std::function<bool()> errorCallback = [this, &socket]() {
        socket->close();
        return true;
    };

    taskChain->addSignalWaiter(socket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), errorCallback);
    taskChain->addSignalWaiter(socket, &QTcpSocket::readyRead, readyReadCallback);
    switch (d->connectionType) {
    case ConnectionType::Ssl:
        static_cast<QSslSocket *>(socket)->connectToHostEncrypted(d->host, d->port);
        break;
    case ConnectionType::Plain:
        socket->connectToHost(d->host, d->port);
        break;
    }
    taskChain->fireSignalWaiters();
    while (socket->isOpen()) {
        taskChain->addSignalWaiter(socket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), errorCallback);
        taskChain->addSignalWaiter(socket, &QTcpSocket::readyRead, readyReadCallback);
        taskChain->fireSignalWaiters();
    }

    return result;
}


