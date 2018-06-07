#ifndef BASERESTAPI_P_H
#define BASERESTAPI_P_H

#include "proofcore/future.h"
#include "proofcore/objectscache.h"
#include "proofcore/proofobject_p.h"

#include "proofnetwork/baserestapi.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/restclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace Proof {
struct PROOF_NETWORK_EXPORT RestApiReply
{
    RestApiReply() {}
    explicit RestApiReply(const QByteArray &data, const QHash<QByteArray, QByteArray> &headers,
                          const QByteArray &httpReason, int httpStatus)
        : data(data), headers(headers), httpReason(httpReason), httpStatus(httpStatus)
    {}
    static RestApiReply fromQNetworkReply(QNetworkReply *qReply);
    QByteArray data;
    QHash<QByteArray, QByteArray> headers;
    QByteArray httpReason;
    int httpStatus = 0;
};

class BaseRestApi;
class PROOF_NETWORK_EXPORT BaseRestApiPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(BaseRestApi)
public:
    BaseRestApiPrivate() : ProofObjectPrivate() {}

    CancelableFuture<RestApiReply> get(const QString &method, const QUrlQuery &query = QUrlQuery());
    CancelableFuture<RestApiReply> post(const QString &method, const QUrlQuery &query = QUrlQuery(),
                                        const QByteArray &body = "");
    CancelableFuture<RestApiReply> post(const QString &method, const QUrlQuery &query, QHttpMultiPart *multiParts);
    CancelableFuture<RestApiReply> put(const QString &method, const QUrlQuery &query = QUrlQuery(),
                                       const QByteArray &body = "");
    CancelableFuture<RestApiReply> patch(const QString &method, const QUrlQuery &query = QUrlQuery(),
                                         const QByteArray &body = "");
    CancelableFuture<RestApiReply> deleteResource(const QString &method, const QUrlQuery &query = QUrlQuery());

    CancelableFuture<RestApiReply> configureReply(CancelableFuture<QNetworkReply *> replyFuture);

    template <typename Result>
    CancelableFuture<Result> invalidArgumentsFailure(Failure &&f = Failure(QStringLiteral("Invalid arguments"),
                                                                           NETWORK_MODULE_CODE,
                                                                           NetworkErrorCode::InvalidRequest)) const
    {
        auto promise = PromiseSP<Result>::create();
        promise->failure(std::move(f));
        return CancelableFuture<Result>(promise);
    }

    template <typename Unmarshaller, typename T = typename std::result_of<Unmarshaller(RestApiReply)>::type>
    CancelableFuture<T> unmarshalReply(const CancelableFuture<RestApiReply> &reply, Unmarshaller &&unmarshaller) const
    {
        auto promise = PromiseSP<T>::create();
        reply->onSuccess([promise, unmarshaller = std::forward<Unmarshaller>(unmarshaller)](const RestApiReply &data) {
            promise->success(unmarshaller(data));
        });
        reply->onFailure([promise](const Failure &f) { promise->failure(f); });
        promise->future()->onFailure([reply](const Failure &) { reply.cancel(); });
        return CancelableFuture<T>(promise);
    }

    virtual void processSuccessfulReply(QNetworkReply *reply, const PromiseSP<RestApiReply> &promise);
    virtual void processErroredReply(QNetworkReply *reply, const PromiseSP<RestApiReply> &promise);

    bool replyShouldBeHandledByError(QNetworkReply *reply) const;

    // It is possible to have Entity deduced here too,
    // but it is kept as non-deducible intentionally to avoid any issues,
    // make type deduction stricter and make it similar to non-cached version
    template <typename Entity, typename Cache, typename KeyFunc, typename Key = typename Cache::key_type>
    auto entityUnmarshaller(Cache &cache, KeyFunc &&keyFunc) const
        -> decltype(std::function<Key(Entity *)>(keyFunc)(cache.value(Key()).data()) == Key(),
                    std::function<QSharedPointer<Entity>(const RestApiReply &)>())
    {
        return [this, &cache, keyFunc = std::function<Key(Entity *)>(keyFunc)](const RestApiReply &reply) {
            auto entity = parseEntity<Entity>(reply.data);
            if (!entity)
                return entity;
            auto fromCache = cache.add(keyFunc(entity.data()), entity);
            if (entity != fromCache) {
                fromCache->updateFrom(entity);
                entity = fromCache;
            }
            return entity;
        };
    }

    template <typename Entity>
    std::function<QSharedPointer<Entity>(const RestApiReply &)> entityUnmarshaller() const
    {
        return [this](const RestApiReply &reply) { return parseEntity<Entity>(reply.data); };
    }

    std::function<QString(const RestApiReply &)> stringUnmarshaller(const QString &attributeName) const
    {
        return [this, attributeName](const RestApiReply &reply) -> QString {
            QJsonParseError jsonError;
            QJsonDocument doc = QJsonDocument::fromJson(reply.data, &jsonError);
            if (jsonError.error != QJsonParseError::NoError) {
                return WithFailure(QStringLiteral("JSON error: %1").arg(jsonError.errorString()), NETWORK_MODULE_CODE,
                                   NetworkErrorCode::InvalidReply, Failure::NoHint, jsonError.error);
            }
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.value(attributeName).isUndefined() || !obj.value(attributeName).isString()) {
                    QString errorAttribute = algorithms::findIf(serverErrorAttributes, [obj](const QString &attr) {
                        return !obj.value(attr).isUndefined();
                    });
                    if (!errorAttribute.isEmpty()) {
                        return WithFailure(obj.value(errorAttribute).toString(), NETWORK_MODULE_CODE,
                                           NetworkErrorCode::InvalidReply);
                    }
                    return WithFailure(QStringLiteral("Can't fetch string from server response"), NETWORK_MODULE_CODE,
                                       NetworkErrorCode::InvalidReply);
                }
                return obj.value(attributeName).toString();
            }
            return WithFailure(QStringLiteral("Can't fetch string from server response"), NETWORK_MODULE_CODE,
                               NetworkErrorCode::InvalidReply);
        };
    }

    template <typename Entity, typename Cache, typename KeyFunc, typename Key = typename Cache::key_type>
    auto entitiesArrayUnmarshaller(Cache &cache, KeyFunc &&keyFunc, const QString &attributeName = QString()) const
        -> decltype(std::function<Key(Entity *)>(keyFunc)(cache.value(Key()).data()) == Key(),
                    std::function<QList<QSharedPointer<Entity>>(const RestApiReply &)>())
    {
        return [this, attributeName, &cache, keyFunc = std::function<Key(Entity *)>(keyFunc)](const RestApiReply &reply) {
            auto arr = parseEntitiesArray(reply.data, attributeName);
            QList<QSharedPointer<Entity>> result;
            result.reserve(arr.count());
            for (const QJsonValue &v : qAsConst(arr)) {
                auto entity = parseEntity<Entity>(v.toObject());
                if (!entity)
                    return QList<QSharedPointer<Entity>>();
                auto fromCache = cache.add(keyFunc(entity.data()), entity);
                if (entity != fromCache) {
                    fromCache->updateFrom(entity);
                    entity = fromCache;
                }
                result << entity;
            }
            return result;
        };
    }

    template <typename Entity>
    std::function<QList<QSharedPointer<Entity>>(const RestApiReply &)>
    entitiesArrayUnmarshaller(const QString &attributeName = QString()) const
    {
        return [this, attributeName](const RestApiReply &reply) {
            auto arr = parseEntitiesArray(reply.data, attributeName);
            QList<QSharedPointer<Entity>> result;
            result.reserve(arr.count());
            for (const QJsonValue &v : qAsConst(arr)) {
                auto entity = parseEntity<Entity>(v.toObject());
                if (!entity)
                    return QList<QSharedPointer<Entity>>();
                result << entity;
            }
            return result;
        };
    }

    std::function<QList<QString>(const RestApiReply &)>
    stringsArrayUnmarshaller(const QString &attributeName = QString()) const
    {
        return [this, attributeName](const RestApiReply &reply) {
            auto arr = parseEntitiesArray(reply.data, attributeName);
            QList<QString> result;
            result.reserve(arr.count());
            for (const QJsonValue &v : qAsConst(arr)) {
                QString string = v.toString();
                if (!string.isEmpty())
                    result << string;
            }
            return result;
        };
    }

    std::function<QList<qlonglong>(const RestApiReply &)> intsArrayUnmarshaller(const QString &attributeName = QString()) const
    {
        return [this, attributeName](const RestApiReply &reply) {
            auto arr = parseEntitiesArray(reply.data, attributeName);
            QList<qint64> result;
            result.reserve(arr.count());
            for (const QJsonValue &v : qAsConst(arr)) {
                if (v.isDouble())
                    result << v.toInt();
            }
            return result;
        };
    }

    std::function<bool(const RestApiReply &)> discardingUnmarshaller() const
    {
        return [](const RestApiReply &) { return true; };
    }

    QJsonArray parseEntitiesArray(const QByteArray &data, const QString &attributeName = QLatin1String()) const
    {
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            return WithFailure(QStringLiteral("JSON error: %1").arg(jsonError.errorString()), NETWORK_MODULE_CODE,
                               NetworkErrorCode::InvalidReply, Failure::NoHint, jsonError.error);
        }
        if (doc.isArray())
            return doc.array();
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.value(attributeName).isUndefined() || !obj.value(attributeName).isArray()) {
                QString errorAttribute = algorithms::findIf(serverErrorAttributes, [obj](const QString &attr) {
                    return !obj.value(attr).isUndefined();
                });
                if (!errorAttribute.isEmpty()) {
                    return WithFailure(obj.value(errorAttribute).toString(), NETWORK_MODULE_CODE,
                                       NetworkErrorCode::InvalidReply);
                }
                return WithFailure(QStringLiteral("Can't create list of entities from server response"),
                                   NETWORK_MODULE_CODE, NetworkErrorCode::InvalidReply);
            }
            return obj.value(attributeName).toArray();
        }
        return WithFailure(QStringLiteral("Can't create list of entities from server response"), NETWORK_MODULE_CODE,
                           NetworkErrorCode::InvalidReply);
    }

    void rememberReply(const CancelableFuture<RestApiReply> &reply);
    void abortAllReplies();

    template <typename Entity>
    QSharedPointer<Entity> parseEntity(const QByteArray &data) const
    {
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            return WithFailure(QStringLiteral("JSON error: %1").arg(jsonError.errorString()), NETWORK_MODULE_CODE,
                               NetworkErrorCode::InvalidReply, Failure::NoHint, jsonError.error);
        }
        return parseEntity<Entity>(doc.object());
    }

    template <typename Entity>
    QSharedPointer<Entity> parseEntity(const QJsonObject &obj) const
    {
        ;
        if (obj.isEmpty()) {
            return WithFailure(QStringLiteral("JSON error: empty entity data"), NETWORK_MODULE_CODE,
                               NetworkErrorCode::InvalidReply);
        }

        QSharedPointer<Entity> entity = Entity::fromJson(obj);
        if (!entity) {
            QString errorAttribute = algorithms::findIf(serverErrorAttributes, [obj](const QString &attr) {
                return !obj.value(attr).isUndefined();
            });
            if (!errorAttribute.isEmpty()) {
                return WithFailure(obj.value(errorAttribute).toString(), NETWORK_MODULE_CODE,
                                   NetworkErrorCode::InvalidReply);
            }
            return WithFailure(QStringLiteral("Can't create entity from server response"), NETWORK_MODULE_CODE,
                               NetworkErrorCode::InvalidReply);
        }
        return entity;
    }

    RestClientSP restClient;
    QString vendor;
    QStringList serverErrorAttributes;

private:

    QHash<qint64, CancelableFuture<RestApiReply>> allReplies;
    SpinLock allRepliesLock;
};
} // namespace Proof
#endif // BASERESTAPI_P_H
