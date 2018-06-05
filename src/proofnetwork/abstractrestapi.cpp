#include "abstractrestapi.h"

static const int NETWORK_SSL_ERROR_OFFSET = 1500;
static const int NETWORK_ERROR_OFFSET = 1000;

using namespace Proof;

qlonglong AbstractRestApi::clientNetworkErrorOffset()
{
    return NETWORK_ERROR_OFFSET;
}

qlonglong AbstractRestApi::clientSslErrorOffset()
{
    return NETWORK_SSL_ERROR_OFFSET;
}

QString RestApiError::toString() const
{
    if (level == Level::NoError)
        return QString();
    return QStringLiteral("%1: %2").arg(code).arg(message);
}

void RestApiError::reset()
{
    level = Level::NoError;
    code = 0;
    message = QString();
}

bool RestApiError::isNetworkError() const
{
    return level == Level::ClientError && code > NETWORK_ERROR_OFFSET;
}

QNetworkReply::NetworkError RestApiError::toNetworkError() const
{
    if (isNetworkError())
        return (QNetworkReply::NetworkError)(code - NETWORK_ERROR_OFFSET);
    else
        return QNetworkReply::UnknownNetworkError;
}

Failure RestApiError::toFailure() const
{
    if (level == Level::NoError)
        return Failure();
    else
        return Failure(message, proofModuleCode, proofErrorCode,
                       userFriendly ? Failure::UserFriendlyHint : Failure::NoHint, code ? code : QVariant());
}

RestApiError RestApiError::fromFailure(const Failure &f)
{
    return RestApiError(f.exists ? Level::ServerError : Level::NoError, f.data.toInt(), f.moduleCode, f.errorCode,
                        f.message, f.hints & Failure::UserFriendlyHint);
}
