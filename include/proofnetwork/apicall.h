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
#ifndef PROOF_APICALL_H
#define PROOF_APICALL_H

#include "proofseed/future.h"

#include "proofnetwork/proofnetwork_global.h"

#include <QNetworkReply>

namespace Proof {
//TODO: Replace completely
//As it looks like - we don't need level (we use it only to check if error happened and if it was server error)
// and everything else can be easily covered with Failure directly
struct PROOF_NETWORK_EXPORT RestApiError
{
    enum class Level
    {
        NoError,
        AuthCredentialsError,
        ClientError,
        ServerError,
        JsonParseError,
        JsonServerError,
        JsonDataError
    };
    explicit RestApiError(Level level = Level::NoError, qlonglong code = 0, long proofModuleCode = NETWORK_MODULE_CODE,
                          long proofErrorCode = 0, const QString &message = QString(), bool userFriendly = false)
        : level(level), code(code), proofModuleCode(proofModuleCode), proofErrorCode(proofErrorCode), message(message),
          userFriendly(userFriendly)
    {}

    QString toString() const;
    void reset();
    bool isNetworkError() const;
    QNetworkReply::NetworkError toNetworkError() const;

    Failure toFailure() const;
    static RestApiError fromFailure(const Failure &f);

    Level level = Level::NoError;
    qlonglong code = 0;
    long proofModuleCode = NETWORK_MODULE_CODE;
    long proofErrorCode = 0;
    QString message;
    bool userFriendly = false;
};

//TODO: deprecated, remove when all clients will be moved to proper futures usage
//Can be used anywhere instead of chainedApiCall because they already require a background thread
template <typename T>
RestApiError runApiCall(const FutureSP<T> &caller, T &result)
{
    caller->wait();
    if (caller->failed())
        return RestApiError::fromFailure(caller->failureReason());
    result = caller->result();
    return RestApiError();
}

template <typename T>
RestApiError runApiCall(const FutureSP<T> &caller)
{
    caller->wait();
    return caller->failed() ? RestApiError::fromFailure(caller->failureReason()) : RestApiError();
}

template <typename T>
RestApiError runApiCall(const CancelableFuture<T> &caller, T &result)
{
    return runApiCall(caller.future(), result);
}

template <typename T>
RestApiError runApiCall(const CancelableFuture<T> &caller)
{
    return runApiCall(caller.future());
}

} // namespace Proof

Q_DECLARE_METATYPE(Proof::RestApiError)
#endif // PROOF_APICALL_H
