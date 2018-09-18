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
#include "proofnetwork/errormessagesregistry.h"

#include "proofseed/proofalgorithms.h"

namespace Proof {

class ErrorMessagesRegistryPrivate
{
    Q_DECLARE_PUBLIC(ErrorMessagesRegistry)
    ErrorMessagesRegistry *q_ptr;
    QHash<long, ErrorInfo> m_infos;
};

ErrorMessagesRegistry::ErrorMessagesRegistry(std::initializer_list<ErrorInfo> &&list)
    : d_ptr(new ErrorMessagesRegistryPrivate)
{
    d_ptr->q_ptr = this;

    for (const auto &info : list) {
        Q_ASSERT_X(!d_ptr->m_infos.contains(info.proofErrorCode), "ErrorMessagesRegistry", "Error codes must be unique");
        d_ptr->m_infos[info.proofErrorCode] = info;
    }
}

ErrorMessagesRegistry::~ErrorMessagesRegistry()
{}

ErrorInfo ErrorMessagesRegistry::infoForCode(int code, const QVector<QString> &args) const
{
    if (d_ptr->m_infos.contains(code)) {
        ErrorInfo info = d_ptr->m_infos[code];
        info.message = algorithms::reduce(args, [](const QString &acc, const QString &x) { return acc.arg(x); },
                                          info.message);
        return info;
    } else {
        return ErrorInfo{0, 0, QObject::tr("Unknown error"), true};
    }
}

} // namespace Proof
