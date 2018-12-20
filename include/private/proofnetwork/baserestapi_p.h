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
#ifndef BASERESTAPI_P_H
#define BASERESTAPI_P_H

#include "proofseed/future.h"

#include "proofcore/objectscache.h"
#include "proofcore/proofobject_p.h"

#include "proofnetwork/baserestapi.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/restclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace Proof {

class BaseRestApi;
class PROOF_NETWORK_EXPORT BaseRestApiPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(BaseRestApi)
public:
    BaseRestApiPrivate() : ProofObjectPrivate() {}

    CancelableFuture<RestApiReply> configureReply(CancelableFuture<QNetworkReply *> replyFuture);
    bool replyShouldBeHandledByError(QNetworkReply *reply) const;
    Failure buildReplyFailure(QNetworkReply *reply);
    bool pingExternalResource(const QString &address);
    void rememberReply(const CancelableFuture<RestApiReply> &reply);

    RestClientSP restClient;
    QString location;

private:
    QHash<qint64, CancelableFuture<RestApiReply>> allReplies;
    SpinLock allRepliesLock;
};
} // namespace Proof
#endif // BASERESTAPI_P_H
