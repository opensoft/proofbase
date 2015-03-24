#ifndef PROOFNETWORK_TYPES_H
#define PROOFNETWORK_TYPES_H

#include <QSharedPointer>
#include <QWeakPointer>

namespace Proof {

class NetworkDataEntity;
typedef QSharedPointer<NetworkDataEntity> NetworkDataEntitySP;
typedef QWeakPointer<NetworkDataEntity> NetworkDataEntityWP;

class User;
typedef QSharedPointer<User> UserSP;
typedef QWeakPointer<User> UserWP;

class RestClient;
typedef QSharedPointer<RestClient> RestClientSP;
typedef QWeakPointer<RestClient> RestClientWP;

class UrlQueryBuilder;
typedef QSharedPointer<UrlQueryBuilder> UrlQueryBuilderSP;
typedef QWeakPointer<UrlQueryBuilder> UrlQueryBuilderWP;

}
#endif // PROOFNETWORK_TYPES_H
