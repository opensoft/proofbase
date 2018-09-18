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
#ifndef PROOF_ABSTRACTNOTIFICATIONHANDLER_H
#define PROOF_ABSTRACTNOTIFICATIONHANDLER_H

#include "proofcore/errornotifier.h"
#include "proofcore/proofcore_global.h"
#include "proofcore/proofobject.h"

#include <QString>

namespace Proof {
class AbstractNotificationHandlerPrivate;
class PROOF_CORE_EXPORT AbstractNotificationHandler : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractNotificationHandler)
public:
    QString appId() const;

    virtual void notify(const QString &message, ErrorNotifier::Severity severity, const QString &packId) = 0;
    //All subclasses should have static QString id() method which should return unique name for this handler

protected:
    AbstractNotificationHandler(AbstractNotificationHandlerPrivate &dd, const QString &appId);
};
} // namespace Proof

#endif // PROOF_ABSTRACTNOTIFICATIONHANDLER_H
