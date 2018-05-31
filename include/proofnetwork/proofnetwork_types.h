#ifndef PROOFNETWORK_TYPES_H
#define PROOFNETWORK_TYPES_H

#include <QSharedPointer>
#include <QWeakPointer>

namespace Proof {

class NetworkDataEntity;
using NetworkDataEntitySP = QSharedPointer<NetworkDataEntity>;
using NetworkDataEntityWP = QWeakPointer<NetworkDataEntity>;

class User;
using UserSP = QSharedPointer<User>;
using UserWP = QWeakPointer<User>;

class RestClient;
using RestClientSP = QSharedPointer<RestClient>;
using RestClientWP = QWeakPointer<RestClient>;

class UrlQueryBuilder;
using UrlQueryBuilderSP = QSharedPointer<UrlQueryBuilder>;
using UrlQueryBuilderWP = QWeakPointer<UrlQueryBuilder>;

class SmtpClient;
using SmtpClientSP = QSharedPointer<SmtpClient>;
using SmtpClientWP = QWeakPointer<SmtpClient>;

enum class RestAuthType
{
    NoAuth,
    Basic,
    Wsse,
    BearerToken
};
} // namespace Proof

Q_DECLARE_METATYPE(Proof::RestAuthType)
#endif // PROOFNETWORK_TYPES_H
