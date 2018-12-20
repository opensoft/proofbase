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
#ifndef BASERESTAPI_H
#define BASERESTAPI_H
#include "proofseed/future.h"

#include "proofcore/proofobject.h"

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"
#include "proofnetwork/restclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkReply>

namespace Proof {
struct PROOF_NETWORK_EXPORT RestApiReply
{
    RestApiReply() {}
    explicit RestApiReply(const QByteArray &data, const QHash<QByteArray, QByteArray> &headers,
                          const QByteArray &httpReason, int httpStatus);
    static RestApiReply fromQNetworkReply(QNetworkReply *qReply);
    QByteArray data;
    QHash<QByteArray, QByteArray> headers;
    QByteArray httpReason;
    int httpStatus = 0;
};

class BaseRestApiPrivate;
class PROOF_NETWORK_EXPORT BaseRestApi : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BaseRestApi)
public:
    RestClientSP restClient() const;

    virtual bool isLoggedOut() const;

    static int clientNetworkErrorOffset();
    static int clientSslErrorOffset();

    void abortAllRequests();

    QString location() const;
    void setLocation(const QString &location);

protected:
    BaseRestApi(const RestClientSP &restClient, QObject *parent = nullptr);
    BaseRestApi(const RestClientSP &restClient, BaseRestApiPrivate &dd, QObject *parent = nullptr);

    CancelableFuture<RestApiReply> get(const QString &method, const QUrlQuery &query = QUrlQuery());
    CancelableFuture<RestApiReply> post(const QString &method, const QUrlQuery &query = QUrlQuery(),
                                        const QByteArray &body = "");
    CancelableFuture<RestApiReply> post(const QString &method, const QUrlQuery &query, QHttpMultiPart *multiParts);
    CancelableFuture<RestApiReply> put(const QString &method, const QUrlQuery &query = QUrlQuery(),
                                       const QByteArray &body = "");
    CancelableFuture<RestApiReply> patch(const QString &method, const QUrlQuery &query = QUrlQuery(),
                                         const QByteArray &body = "");
    CancelableFuture<RestApiReply> deleteResource(const QString &method, const QUrlQuery &query = QUrlQuery());

    virtual void processSuccessfulReply(QNetworkReply *reply, const PromiseSP<RestApiReply> &promise);
    virtual void processErroredReply(QNetworkReply *reply, const PromiseSP<RestApiReply> &promise);

    virtual QVector<QString> serverErrorAttributes() const;
    virtual QString vendor() const;

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
                    QString errorAttribute = algorithms::findIf(serverErrorAttributes(), [obj](const QString &attr) {
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
                    std::function<QVector<QSharedPointer<Entity>>(const RestApiReply &)>())
    {
        return [this, attributeName, &cache, keyFunc = std::function<Key(Entity *)>(keyFunc)](const RestApiReply &reply) {
            auto arr = parseEntitiesArray(reply.data, attributeName);
            QVector<QSharedPointer<Entity>> result;
            result.reserve(arr.count());
            for (const QJsonValue &v : qAsConst(arr)) {
                auto entity = parseEntity<Entity>(v.toObject());
                if (!entity)
                    return QVector<QSharedPointer<Entity>>();
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
    std::function<QVector<QSharedPointer<Entity>>(const RestApiReply &)>
    entitiesArrayUnmarshaller(const QString &attributeName = QString()) const
    {
        return [this, attributeName](const RestApiReply &reply) {
            auto arr = parseEntitiesArray(reply.data, attributeName);
            QVector<QSharedPointer<Entity>> result;
            result.reserve(arr.count());
            for (const QJsonValue &v : qAsConst(arr)) {
                auto entity = parseEntity<Entity>(v.toObject());
                if (!entity)
                    return QVector<QSharedPointer<Entity>>();
                result << entity;
            }
            return result;
        };
    }

    std::function<QVector<QString>(const RestApiReply &)>
    stringsArrayUnmarshaller(const QString &attributeName = QString()) const
    {
        return [this, attributeName](const RestApiReply &reply) {
            auto arr = parseEntitiesArray(reply.data, attributeName);
            QVector<QString> result;
            result.reserve(arr.count());
            for (const QJsonValue &v : qAsConst(arr)) {
                QString string = v.toString();
                if (!string.isEmpty())
                    result << string;
            }
            return result;
        };
    }

    std::function<QVector<qlonglong>(const RestApiReply &)>
    intsArrayUnmarshaller(const QString &attributeName = QString()) const
    {
        return [this, attributeName](const RestApiReply &reply) {
            auto arr = parseEntitiesArray(reply.data, attributeName);
            QVector<qint64> result;
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

    QJsonArray parseEntitiesArray(const QByteArray &data, const QString &attributeName = QString()) const
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
                QString errorAttribute = algorithms::findIf(serverErrorAttributes(), [obj](const QString &attr) {
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
        if (obj.isEmpty()) {
            return WithFailure(QStringLiteral("JSON error: empty entity data"), NETWORK_MODULE_CODE,
                               NetworkErrorCode::InvalidReply);
        }

        QSharedPointer<Entity> entity = Entity::fromJson(obj);
        if (!entity) {
            QString errorAttribute = algorithms::findIf(serverErrorAttributes(), [obj](const QString &attr) {
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
};
} // namespace Proof

#endif // BASERESTAPI_H
