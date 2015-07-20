#ifndef ABSTRACTRESTAPI_P_H
#define ABSTRACTRESTAPI_P_H

#include "proofcore/proofobject_p.h"
#include "proofcore/objectscache.h"
#include "proofnetwork/restclient.h"
#include "proofnetwork/abstractrestapi.h"
#include "proofnetwork/proofnetwork_global.h"

#include <QSslError>
#include <QHash>
#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QNetworkReply>

#include <functional>


#include <atomic>

class QTimer;
class QNetworkReply;

namespace Proof {
class AbstractRestApi;
class PROOF_NETWORK_EXPORT AbstractRestApiPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(AbstractRestApi)
public:
    AbstractRestApiPrivate() : ProofObjectPrivate() {}

    QNetworkReply *get(qulonglong &operationId, const QString &method, const QUrlQuery &query = QUrlQuery());
    QNetworkReply *post(qulonglong &operationId, const QString &method, const QUrlQuery &query = QUrlQuery(),
                        const QByteArray &body = "");
    QNetworkReply *post(qulonglong &operationId, const QString &method, const QUrlQuery &query,
                        QHttpMultiPart *multiParts);
    QNetworkReply *put(qulonglong &operationId, const QString &method, const QUrlQuery &query = QUrlQuery(),
                        const QByteArray &body = "");
    QNetworkReply *patch(qulonglong &operationId, const QString &method, const QUrlQuery &query = QUrlQuery(),
                        const QByteArray &body = "");
    QNetworkReply *deleteResource(qulonglong &operationId, const QString &method, const QUrlQuery &query = QUrlQuery());

    virtual void replyFinished(qulonglong operationId, QNetworkReply *reply);
    virtual void replyErrorOccurred(qulonglong operationId, QNetworkReply *reply);
    virtual void sslErrorsOccurred(qulonglong operationId, QNetworkReply *reply, const QList<QSslError> &errors);
    virtual void cleanupReply(qulonglong operationId, QNetworkReply *reply);

    void notifyAboutJsonParseError(qulonglong operationId, const QJsonParseError &error);

    //TODO: 1.0: add error message transmission from DTO
    template<class EntityKey, class Entity>
    QSharedPointer<Entity> parseEntity(QNetworkReply *reply, ObjectsCache<EntityKey, Entity> &cache,
                                       std::function<EntityKey(Entity *)> &&cacheKey, qulonglong operationId,
                                       QString *errorMessage = 0)
    {
        QJsonObject obj = parseEntityObject<Entity>(reply, operationId, errorMessage);
        if (obj.isEmpty())
            return QSharedPointer<Entity>();
        return parseEntity<EntityKey, Entity>(obj, cache, std::move(cacheKey), operationId, errorMessage);
    }

    template<class Entity>
    QSharedPointer<Entity> parseEntity(QNetworkReply *reply, qulonglong operationId, QString *errorMessage = 0)
    {
        QJsonObject obj = parseEntityObject<Entity>(reply, operationId, errorMessage);
        if (obj.isEmpty())
            return QSharedPointer<Entity>();
        return parseEntity<Entity>(obj, operationId, errorMessage);
    }

    template<class EntityKey, class Entity>
    QSharedPointer<Entity> parseEntity(const QJsonObject &jsonObject, ObjectsCache<EntityKey, Entity> &cache,
                                       std::function<EntityKey(Entity *)> &&cacheKey, qulonglong operationId,
                                       QString *errorMessage = 0)
    {
        QSharedPointer<Entity> entity = parseEntity<Entity>(jsonObject, operationId, errorMessage);
        if (entity) {
            QSharedPointer<Entity> fromCache = cache.value(cacheKey(entity.data()));
            if (fromCache) {
                fromCache->updateFrom(entity);
                entity = fromCache;
            } else {
                fromCache = cache.add(cacheKey(entity.data()), entity);
                if (entity != fromCache) {
                    fromCache->updateFrom(entity);
                    entity = fromCache;
                }
            }
        }
        return entity;
    }

    template<class Entity>
    QSharedPointer<Entity> parseEntity(const QJsonObject &jsonObject, qulonglong operationId, QString *errorMessage = 0)
    {
        Q_Q(AbstractRestApi);
        QSharedPointer<Entity> entity = Entity::fromJson(jsonObject);
        if (!entity) {
            for (const QString &attribute : serverErrorAttributes) {
                if (!jsonObject.value(attribute).isUndefined()) {
                    QString jsonErrorMessage = jsonObject.value(attribute).toString();
                    if (errorMessage) {
                        jsonErrorMessage.prepend(*errorMessage);
                        *errorMessage = jsonErrorMessage;
                    }
                    emit q->errorOccurred(operationId,
                                          RestApiError{RestApiError::Level::JsonServerError,
                                                       0,
                                                       jsonErrorMessage});
                    return entity;
                }
            }

            QString errorString("Can't create entity from server response");
            if (errorMessage) {
                errorString.prepend(*errorMessage);
                *errorMessage = errorString;
            }
            emit q->errorOccurred(operationId,
                                  RestApiError{RestApiError::Level::JsonDataError,
                                               0,
                                               errorString});
        }
        return entity;
    }

    QMetaObject::Connection replyFinishedConnection;
    QMetaObject::Connection sslErrorsConnection;
    RestClientSP restClient;
    QString vendor;
    QStringList serverErrorAttributes;

private:
    void setupReply(qulonglong &operationId, QNetworkReply *reply);

    template<class Entity>
    QJsonObject parseEntityObject(QNetworkReply *reply, qulonglong operationId, QString *errorMessage)
    {
        Q_Q(AbstractRestApi);
        QByteArray json = reply->readAll();
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(json, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            QString jsonErrorString = QString("JSON error: %1").arg(jsonError.errorString());
            if (errorMessage) {
                jsonErrorString.prepend(*errorMessage);
                *errorMessage = jsonErrorString;
            }
            emit q->errorOccurred(operationId,
                                  RestApiError{RestApiError::Level::JsonParseError,
                                               jsonError.error,
                                               jsonErrorString});
            return QJsonObject();
        } else if (doc.object().isEmpty()) {
            QString jsonErrorString = QString("JSON error: empty entity data").arg(jsonError.errorString());
            if (errorMessage) {
                jsonErrorString.prepend(*errorMessage);
                *errorMessage = jsonErrorString;
            }
            emit q->errorOccurred(operationId,
                                  RestApiError{RestApiError::Level::JsonParseError,
                                               0,
                                               jsonErrorString});
        }
        return doc.object();
    }

    static std::atomic<qulonglong> lastUsedOperationId;
    QHash<QNetworkReply *, qulonglong> repliesIds;
};
}
#endif // ABSTRACTRESTAPI_P_H
