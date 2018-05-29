#ifndef BASERESTAPI_P_H
#define BASERESTAPI_P_H

#include "proofcore/proofobject_p.h"
#include "proofcore/objectscache.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/baserestapi.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofcore/future.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

namespace Proof {
class BaseRestApi;
class PROOF_NETWORK_EXPORT BaseRestApiPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(BaseRestApi)
public:
    BaseRestApiPrivate() : ProofObjectPrivate() {}

    CancelableFuture<QByteArray> get(const QString &method, const QUrlQuery &query = QUrlQuery());
    CancelableFuture<QByteArray> post(const QString &method, const QUrlQuery &query = QUrlQuery(), const QByteArray &body = "");
    CancelableFuture<QByteArray> post(const QString &method, const QUrlQuery &query, QHttpMultiPart *multiParts);
    CancelableFuture<QByteArray> put(const QString &method, const QUrlQuery &query = QUrlQuery(), const QByteArray &body = "");
    CancelableFuture<QByteArray> patch(const QString &method, const QUrlQuery &query = QUrlQuery(), const QByteArray &body = "");
    CancelableFuture<QByteArray> deleteResource(const QString &method, const QUrlQuery &query = QUrlQuery());

    CancelableFuture<QByteArray> configureReply(CancelableFuture<QNetworkReply *> replyFuture);

    template<typename Result> CancelableFuture<Result>
    invalidArgumentsFailure(Failure &&f = Failure(QStringLiteral("Invalid arguments"),
                                                  NETWORK_MODULE_CODE, NetworkErrorCode::InvalidRequest)) const
    {
        auto promise = PromiseSP<Result>::create();
        promise->failure(std::move(f));
        return CancelableFuture<Result>(promise);
    }

    template<typename Unmarshaller,
             typename T = typename std::result_of<Unmarshaller(QByteArray)>::type>
    CancelableFuture<T> unmarshalReply(const CancelableFuture<QByteArray> &reply, Unmarshaller &&unmarshaller) const
    {
        auto promise = PromiseSP<T>::create();
        reply->onSuccess([promise, unmarshaller = std::forward<Unmarshaller>(unmarshaller)](const QByteArray &data) {
            promise->success(unmarshaller(data));
        });
        reply->onFailure([promise](const Failure &f) {promise->failure(f);});
        promise->future()->onFailure([reply](const Failure &) {reply.cancel();});
        return CancelableFuture<T>(promise);
    }

    virtual void processSuccessfulReply(QNetworkReply *reply, const PromiseSP<QByteArray> &promise);
    virtual void processErroredReply(QNetworkReply *reply, const PromiseSP<QByteArray> &promise);

    bool replyShouldBeHandledByError(QNetworkReply *reply) const;

    // It is possible to have Entity deduced here too,
    // but it is kept as non-deducible intentionally to avoid any issues,
    // make type deduction stricter and make it similar to non-cached version
    template<typename Entity, typename Cache, typename KeyFunc,
             typename Key = typename Cache::key_type>
    auto entityUnmarshaller(Cache &cache, KeyFunc &&keyFunc) const
    ->decltype(std::function<Key(Entity *)>(keyFunc)(cache.value(Key()).data()) == Key(),
               std::function<QSharedPointer<Entity>(const QByteArray &)>())
    {
        return [this, &cache, keyFunc = std::function<Key(Entity *)>(keyFunc)](const QByteArray &data) {
            auto entity = parseEntity<Entity>(data);
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

    template<typename Entity>
    std::function<QSharedPointer<Entity>(const QByteArray &)> entityUnmarshaller() const
    {
        return [this](const QByteArray &data) {
            return parseEntity<Entity>(data);
        };
    }

    template<typename Entity, typename Cache, typename KeyFunc,
             typename Key = typename Cache::key_type>
    auto entitiesArrayUnmarshaller(Cache &cache, KeyFunc &&keyFunc, const QString &attributeName = QString()) const
    ->decltype(std::function<Key(Entity *)>(keyFunc)(cache.value(Key()).data()) == Key(),
               std::function<QList<QSharedPointer<Entity>>(const QByteArray &)>())
    {
        return [this, attributeName, &cache, keyFunc = std::function<Key(Entity *)>(keyFunc)](const QByteArray &data) {
            auto arr = parseEntitiesArray(data, attributeName);
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

    template<typename Entity>
    std::function<QList<QSharedPointer<Entity>>(const QByteArray &)>
    entitiesArrayUnmarshaller(const QString &attributeName = QString()) const
    {
        return [this, attributeName](const QByteArray &data) {
            auto arr = parseEntitiesArray(data, attributeName);
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

    std::function<QList<QString>(const QByteArray &)>
    stringsArrayUnmarshaller(const QString &attributeName = QString()) const
    {
        return [this, attributeName](const QByteArray &data) {
            auto arr = parseEntitiesArray(data, attributeName);
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

    std::function<QList<qlonglong>(const QByteArray &)>
    intsArrayUnmarshaller(const QString &attributeName = QString()) const
    {
        return [this, attributeName](const QByteArray &data) {
            auto arr = parseEntitiesArray(data, attributeName);
            QList<qint64> result;
            result.reserve(arr.count());
            for (const QJsonValue &v : qAsConst(arr)) {
                if (v.isDouble())
                    result << v.toInt();
            }
            return result;
        };
    }

    std::function<bool(const QByteArray &)> discardingUnmarshaller() const
    {
        return [](const QByteArray &){ return true; };
    }

    void rememberReply(const CancelableFuture<QByteArray> &reply);
    void abortAllReplies();

    RestClientSP restClient;
    QString vendor;
    QStringList serverErrorAttributes;

private:
    template<typename Entity>
    QSharedPointer<Entity> parseEntity(const QByteArray &data) const
    {
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            return WithFailure(QStringLiteral("JSON error: %1").arg(jsonError.errorString()),
                               NETWORK_MODULE_CODE, NetworkErrorCode::InvalidReply,
                               Failure::NoHint, jsonError.error);
        }
        return parseEntity<Entity>(doc.object());
    }

    template<typename Entity>
    QSharedPointer<Entity> parseEntity(const QJsonObject &obj) const
    {;
        if (obj.isEmpty()) {
            return WithFailure(QStringLiteral("JSON error: empty entity data"),
                               NETWORK_MODULE_CODE, NetworkErrorCode::InvalidReply);
        }

        QSharedPointer<Entity> entity = Entity::fromJson(obj);
        if (!entity) {
            QString errorAttribute = algorithms::findIf(serverErrorAttributes, [obj](const QString &attr) {
                return !obj.value(attr).isUndefined();
            });
            if (!errorAttribute.isEmpty()) {
                return WithFailure(obj.value(errorAttribute).toString(),
                                   NETWORK_MODULE_CODE, NetworkErrorCode::InvalidReply);
            }
            return WithFailure(QStringLiteral("Can't create entity from server response"),
                               NETWORK_MODULE_CODE, NetworkErrorCode::InvalidReply);
        }
        return entity;
    }

    QJsonArray parseEntitiesArray(const QByteArray &data, const QString &attributeName) const
    {
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            return WithFailure(QStringLiteral("JSON error: %1").arg(jsonError.errorString()),
                               NETWORK_MODULE_CODE, NetworkErrorCode::InvalidReply,
                               Failure::NoHint, jsonError.error);
        }
        if (doc.isArray()) {
            return doc.array();
        }
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.value(attributeName).isUndefined() || !obj.value(attributeName).isArray()) {
                QString errorAttribute = algorithms::findIf(serverErrorAttributes, [obj](const QString &attr) {
                    return !obj.value(attr).isUndefined();
                });
                if (!errorAttribute.isEmpty()) {
                    return WithFailure(obj.value(errorAttribute).toString(),
                                       NETWORK_MODULE_CODE, NetworkErrorCode::InvalidReply);
                }
                return WithFailure(QStringLiteral("Can't create entity from server response"),
                                   NETWORK_MODULE_CODE, NetworkErrorCode::InvalidReply);
            }
            return obj.value(attributeName).toArray();
        }
        return WithFailure(QStringLiteral("Can't create list of entities from server response"),
                           NETWORK_MODULE_CODE, NetworkErrorCode::InvalidReply);
    }

    QHash<qint64, CancelableFuture<QByteArray>> allReplies;
    SpinLock allRepliesLock;
};
}
#endif // BASERESTAPI_P_H
