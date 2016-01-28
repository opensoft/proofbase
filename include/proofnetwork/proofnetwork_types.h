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

enum class RestAuthType
{
    NoAuth,
    Basic,
    Wsse,
    QuasiOAuth2, //TODO: Remove quasi support when production will moved on SH3
    OAuth2
};

namespace NetworkErrorCode {
enum Code {
    ServerError = 1,
    InaccessibleService = 2,
    SslError = 3,
    InvalidReply = 4,
    InvalidRequest = 5,
    InvalidUrl = 6,
    InternalError = 7,
    MinCustomError = 100
};
}
constexpr long NETWORK_MODULE_CODE = 300;

}

Q_DECLARE_METATYPE(Proof::RestAuthType)
#endif // PROOFNETWORK_TYPES_H
