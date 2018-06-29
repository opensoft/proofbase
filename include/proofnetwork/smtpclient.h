#ifndef PROOF_SMTPCLIENT_H
#define PROOF_SMTPCLIENT_H

#include "proofcore/proofobject.h"

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QStringList>

namespace Proof {
class SmtpClientPrivate;
class PROOF_NETWORK_EXPORT SmtpClient : public ProofObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SmtpClient)
public:
    enum class ConnectionType
    {
        Plain,
        Ssl,
        StartTls
    };
    Q_ENUM(ConnectionType)

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

    void sendTextMail(const QString &subject, const QString &body, const QString &from, const QStringList &to,
                      const QStringList &cc = QStringList(), const QStringList &bcc = QStringList());

signals:
    void userNameChanged(const QString &arg);
    void passwordChanged(const QString &arg);
    void hostChanged(const QString &arg);
    void portChanged(int arg);
    void connectionTypeChanged(Proof::SmtpClient::ConnectionType arg);
};

} // namespace Proof

#endif // PROOF_SMTPCLIENT_H
