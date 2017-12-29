#ifndef PROOF_AMQPPUBLISHER_H
#define PROOF_AMQPPUBLISHER_H

#include "proofnetwork/abstractamqpclient.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/proofnetwork_global.h"

#include "proofnetwork/3rdparty/qamqp/qamqpexchange.h"

#include <QSslConfiguration>

namespace Proof {

class AmqpPublisherPrivate;
class PROOF_NETWORK_EXPORT AmqpPublisher : public AbstractAmqpClient
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AmqpPublisher)
public:
    explicit AmqpPublisher(QObject *parent = nullptr);

    QString exchangeName() const;
    void setExchangeName(const QString &exchangeName);

    //TODO: add options if will be needed
    qulonglong publishMessage(const QString &message, const QString &routingKey);

    QAmqpExchange::ExchangeOptions exchangeOptions() const;

    bool createExchangeIfNotExists() const;
    void setCreateExchangeIfNotExists(bool createExchangeIfNotExists,
                                      QAmqpExchange::ExchangeOptions options = {QAmqpExchange::Durable});

signals:
    void messagePublished(qulonglong id);

private:
    void publishMessageImpl(const QString &message, const QString &routingKey, qulonglong publishId);
};

} // namespace Proof

#endif // PROOF_AMQPPUBLISHER_H
