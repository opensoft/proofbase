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
#include "proofnetwork/apicall.h"

static const int NETWORK_ERROR_OFFSET = 1000;

using namespace Proof;

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
    return QNetworkReply::UnknownNetworkError;
}

Failure RestApiError::toFailure() const
{
    if (level == Level::NoError)
        return Failure();
    return Failure(message, proofModuleCode, proofErrorCode, userFriendly ? Failure::UserFriendlyHint : Failure::NoHint,
                   code ? code : QVariant());
}

RestApiError RestApiError::fromFailure(const Failure &f)
{
    return RestApiError(f.exists ? Level::ServerError : Level::NoError, f.data.toInt(), f.moduleCode, f.errorCode,
                        f.message, f.hints & Failure::UserFriendlyHint);
}
