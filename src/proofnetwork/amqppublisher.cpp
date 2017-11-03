#include "amqppublisher.h"
#include "proofnetwork/abstractamqpclient_p.h"

#include "proofnetwork/3rdparty/qamqp/qamqpclient.h"
#include "proofnetwork/3rdparty/qamqp/qamqpexchange.h"
#include "proofnetwork/3rdparty/qamqp/qamqpmessage.h"

namespace Proof {

class AmqpPublisherPrivate : public AbstractAmqpClientPrivate
{
    Q_DECLARE_PUBLIC(AmqpPublisher)

    void connected() override;

    QAmqpExchange *exchange = nullptr;
    QString exchangeName;
    std::atomic_ullong nextPublishId {1};
};

} // namespace Proof

using namespace Proof;

AmqpPublisher::AmqpPublisher(QObject *parent)
    : AbstractAmqpClient(*new AmqpPublisherPrivate, parent)
{

}

QString AmqpPublisher::exchangeName() const
{
    Q_D(const AmqpPublisher);
    return d->exchangeName;
}

void AmqpPublisher::setExchangeName(const QString &exchangeName)
{
    Q_D(AmqpPublisher);
    d->exchangeName = exchangeName;
}

qulonglong AmqpPublisher::publishMessage(const QString &message, const QString &routingKey)
{
    Q_D(AmqpPublisher);
    qulonglong result = d->nextPublishId++;
    publishMessageImpl(message, routingKey, result);
    return result;
}

void AmqpPublisher::publishMessageImpl(const QString &message, const QString &routingKey, qulonglong publishId)
{
    if (call(this, &AmqpPublisher::publishMessageImpl, message, routingKey, publishId))
        return;

    Q_D(AmqpPublisher);
    if (!d->exchange) {
        emit errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::InternalError, QStringLiteral("No amqp exchange found for message publishing"), false);
    } else {
        d->exchange->publish(message, routingKey);
        emit messagePublished(publishId);
    }
}

void AmqpPublisherPrivate::connected()
{
    Q_Q(AmqpPublisher);
    auto newExchange = rabbitClient->createExchange(exchangeName);
    if (newExchange != exchange){
        exchange = newExchange;
        qCDebug(proofNetworkAmqpLog) << "Create exchange:" << exchangeName;
        QObject::connect(exchange, static_cast<void(QAmqpExchange::*)(QAMQP::Error)>(&QAmqpExchange::error), q, [this, q](QAMQP::Error error) {
            emit q->errorOccurred(NETWORK_MODULE_CODE, NetworkErrorCode::InternalError, QStringLiteral("Exchange Error: %1").arg(error), false);
            qCWarning(proofNetworkAmqpLog) << "RabbitMQ exchange error:" << error;
        });

        QObject::connect(exchange, &QAmqpExchange::opened, q, [this, q]() {
            qCDebug(proofNetworkAmqpLog) << "RabbitMQ exchange opened" << q->sender();
            emit q->connected();
        });
    }
}
