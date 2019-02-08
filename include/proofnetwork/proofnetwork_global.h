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
#ifndef PROOFNETWORK_GLOBAL_H
#define PROOFNETWORK_GLOBAL_H

#ifdef Proof_Network_EXPORTS
#    define PROOF_NETWORK_EXPORT Q_DECL_EXPORT
#else
#    define PROOF_NETWORK_EXPORT Q_DECL_IMPORT
#endif

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(proofNetworkMiscLog);
Q_DECLARE_LOGGING_CATEGORY(proofNetworkExtraLog);
Q_DECLARE_LOGGING_CATEGORY(proofNetworkAmqpLog);

namespace Proof {
namespace NetworkErrorCode {
enum Code
{
    ServerError = 1,
    ServiceUnavailable = 2,
    SslError = 3,
    InvalidReply = 4,
    InvalidRequest = 5,
    InvalidUrl = 6,
    InternalError = 7,
    AuthCredentialsError = 8,
    NoNetworkConnection = 9,
    NoInternetConnection = 10,
    HostNotFound = 11,
    MinCustomError = 100
};
} // namespace NetworkErrorCode
constexpr long NETWORK_MODULE_CODE = 300;
} // namespace Proof
#endif // PROOFNETWORK_GLOBAL_H
