/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "proofnetwork/smtpclient.h"

#include "proofseed/asynqro_extra.h"
#include "proofseed/tasks.h"

#include "proofcore/proofobject_p.h"

#include <QLinkedList>
#include <QSslSocket>
#include <QTcpSocket>

namespace Proof {
class SmtpClientPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(SmtpClient)

    bool sendTextMail(const QString &subject, const QString &body, const QString &from, const QStringList &to,
                      const QStringList &cc, const QStringList &bcc);

    QString userName;
    QString password;
    QString host;
    int port = 25;
    SmtpClient::ConnectionType connectionType = SmtpClient::ConnectionType::Plain;
};
} // namespace Proof

using namespace Proof;

SmtpClient::SmtpClient() : ProofObject(*new SmtpClientPrivate)
{}

QString SmtpClient::userName() const
{
    Q_D_CONST(SmtpClient);
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
    Q_D_CONST(SmtpClient);
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
    Q_D_CONST(SmtpClient);
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
    Q_D_CONST(SmtpClient);
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
    Q_D_CONST(SmtpClient);
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

void SmtpClient::sendTextMail(const QString &subject, const QString &body, const QString &from, const QStringList &to,
                              const QStringList &cc, const QStringList &bcc)
{
    Q_D(SmtpClient);
    tasks::run([d, subject, body, from, to, cc, bcc]() { d->sendTextMail(subject, body, from, to, cc, bcc); });
}

bool SmtpClientPrivate::sendTextMail(const QString &subject, const QString &body, const QString &from,
                                     const QStringList &to, const QStringList &cc, const QStringList &bcc)
{
    qCDebug(proofNetworkMiscLog) << "Sending email from" << from << "to" << to;
    QByteArray userNameBase64 = userName.toLocal8Bit().toBase64();
    QByteArray passwordBase64 = password.toLocal8Bit().toBase64();

    QString domain = from;
    QString senderEmail = from;

    QRegExp addressParser("([^<>]*@([^<>]*))|.*<(.*@(.*))>");
    if (addressParser.exactMatch(from)) {
        domain = addressParser.cap(2).isEmpty() ? addressParser.cap(4) : addressParser.cap(2);
        senderEmail = addressParser.cap(1).isEmpty() ? addressParser.cap(3) : addressParser.cap(1);
    }

    QByteArray message;
    QLinkedList<QString> mutableTo;
    for (const auto &recv : to) {
        QString recvEmail = recv;
        if (addressParser.exactMatch(recv))
            recvEmail = addressParser.cap(1).isEmpty() ? addressParser.cap(3) : addressParser.cap(1);
        message.append("To: " + recv.toUtf8() + "\n");
        mutableTo << recvEmail;
    }

    for (const auto &recv : cc) {
        QString recvEmail = recv;
        if (addressParser.exactMatch(recv))
            recvEmail = addressParser.cap(1).isEmpty() ? addressParser.cap(3) : addressParser.cap(1);
        message.append("Cc: " + recv.toUtf8() + "\n");
        mutableTo << recvEmail;
    }

    for (const auto &recv : bcc) {
        QString recvEmail = recv;
        if (addressParser.exactMatch(recv))
            recvEmail = addressParser.cap(1).isEmpty() ? addressParser.cap(3) : addressParser.cap(1);
        message.append("Bcc: " + recv.toUtf8() + "\n");
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

    enum class SmtpStates
    {
        Init,
        Tls,
        Auth,
        User,
        Pass,
        Mail,
        Rcpt,
        Data,
        Body,
        Quit,
        Close
    };

    SmtpStates state = SmtpStates::Init;
    bool result = false;

    QTcpSocket *socket = (connectionType == SmtpClient::ConnectionType::Plain) ? new QTcpSocket : new QSslSocket;
    std::function<bool()> readyReadCallback = [this, &socket, &state, message, senderEmail, domain, userNameBase64,
                                               passwordBase64, &mutableTo, &result]() {
        QString reply = socket->readAll();
        int stateReply = reply.leftRef(3).toInt();
        QByteArray toSend;
        //TODO: add support for actual response for EHLO with auth types here
        if (state == SmtpStates::Init && stateReply == 220) {
            toSend = QString("EHLO " + domain).toUtf8();
            state = SmtpStates::Auth;
        } else if (state == SmtpStates::Auth && stateReply == 250) {
            if (reply.contains(QLatin1String("STARTTLS")) && connectionType == SmtpClient::ConnectionType::StartTls) {
                toSend = "STARTTLS";
                state = SmtpStates::Tls;
            } else {
                toSend = "AUTH LOGIN";
                state = SmtpStates::User;
            }
        } else if (state == SmtpStates::Tls && stateReply == 220) {
            static_cast<QSslSocket *>(socket)->startClientEncryption();
            return true;
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
            qCDebug(proofNetworkMiscLog) << "Email successfully sent";
            result = true;
            socket->close();
            return true;
        } else {
            qCWarning(proofNetworkMiscLog) << "Unknown state happened during email sending:" << static_cast<int>(state)
                                           << ". Server reply:" << reply;
            state = SmtpStates::Close;
            socket->close();
            return true;
        }
        socket->write(toSend);
        socket->write("\r\n");
        return true;
    };

    std::function<bool()> errorCallback = [&socket]() {
        qCCritical(proofNetworkMiscLog) << "Error occurred during email sending:" << socket->error()
                                        << socket->errorString();
        socket->close();
        return true;
    };

    std::function<bool()> startTlsEncryptedCallback = [&socket, &state, domain]() {
        state = SmtpStates::Auth;
        socket->write(QString("EHLO " + domain).toUtf8());
        socket->write("\r\n");
        return true;
    };

    tasks::addSignalWaiter(socket,
                           static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
                           errorCallback);
    tasks::addSignalWaiter(socket, &QTcpSocket::readyRead, readyReadCallback);
    switch (connectionType) {
    case SmtpClient::ConnectionType::Ssl:
        static_cast<QSslSocket *>(socket)->connectToHostEncrypted(host, port);
        break;
    case SmtpClient::ConnectionType::StartTls:
    case SmtpClient::ConnectionType::Plain:
        socket->connectToHost(host, port);
        break;
    }
    tasks::fireSignalWaiters();
    while (socket->isOpen()) {
        tasks::addSignalWaiter(socket,
                               static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(
                                   &QAbstractSocket::error),
                               errorCallback);
        if (connectionType == SmtpClient::ConnectionType::StartTls && state == SmtpStates::Tls)
            tasks::addSignalWaiter(static_cast<QSslSocket *>(socket), &QSslSocket::encrypted, startTlsEncryptedCallback);
        tasks::addSignalWaiter(socket, &QTcpSocket::readyRead, readyReadCallback);
        tasks::fireSignalWaiters();
    }
    socket->deleteLater();

    return result;
}
