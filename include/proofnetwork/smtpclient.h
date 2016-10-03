#ifndef PROOF_SMTPCLIENT_H
#define PROOF_SMTPCLIENT_H

#include "proofcore/proofobject.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofcore/taskchain.h"

#include <QStringList>

namespace Proof {
class SmtpClientPrivate;
class PROOF_NETWORK_EXPORT SmtpClient : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SmtpClient)
    Q_ENUMS(ConnectionType)
public:
    enum class ConnectionType {
        Plain,
        Ssl
        //TODO: add StartTls support
    };

    explicit SmtpClient();

    QString userName() const;
    void setUserName(const QString &arg);

    QString password() const;
    void setPassword(const QString &arg);

    QString host() const;
    void setHost(const QString &arg);

    int port() const;
    void setPort(int arg);

    ConnectionType connectionType() const;
    void setConnectionType(ConnectionType arg);

    //TODO: 1.0: add cc, bcc, attachments, content type
    void sendMail(const QString &subject, const QString &body, const QString &from, const QStringList &to);
    bool sendMailSync(const QString &subject, const QString &body, const QString &from, const QStringList &to);
    bool sendMail(const Proof::TaskChainSP &taskChain, const QString &subject, const QString &body, const QString &from, const QStringList &to);

signals:
    void userNameChanged(const QString &arg);
    void passwordChanged(const QString &arg);
    void hostChanged(const QString &arg);
    void portChanged(int arg);
    void connectionTypeChanged(ConnectionType arg);
};

} // namespace Proof

#endif // PROOF_SMTPCLIENT_H
