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
#ifndef PROOF_URLQUERYBUILDER_H
#define PROOF_URLQUERYBUILDER_H

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QDateTime>
#include <QScopedPointer>
#include <QUrlQuery>

namespace Proof {

class UrlQueryBuilderPrivate;
class PROOF_NETWORK_EXPORT UrlQueryBuilder
{
    Q_DECLARE_PRIVATE(UrlQueryBuilder)
public:
    UrlQueryBuilder();
    virtual ~UrlQueryBuilder();

    void setCustomParam(const QString &name, const QString &value);
    void setCustomParam(const QString &name, qlonglong value);
    void setCustomParam(const QString &name, qulonglong value);
    void setCustomParam(const QString &name, long value);
    void setCustomParam(const QString &name, unsigned long value);
    void setCustomParam(const QString &name, int value);
    void setCustomParam(const QString &name, unsigned int value);
    void setCustomParam(const QString &name, const QDateTime &value);
    void setCustomParam(const QString &name, bool value);
    void setCustomParam(const QString &name, const char *value);
    void setCustomParam(const QString &name, double value);
    bool containsCustomParam(const QString &name) const;
    QString customParam(const QString &name) const;
    void unsetCustomParam(const QString &name);
    QUrlQuery toUrlQuery() const;

protected:
    UrlQueryBuilder(UrlQueryBuilderPrivate &dd);
    QScopedPointer<UrlQueryBuilderPrivate> d_ptr;
};

} // namespace Proof

#endif // PROOF_URLQUERYBUILDER_H
