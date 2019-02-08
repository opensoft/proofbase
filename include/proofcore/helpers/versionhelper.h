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
#ifndef VERSIONHELPER_H
#define VERSIONHELPER_H

#include <QString>
#include <QStringList>

namespace Proof {
constexpr quint64 packVersion(quint16 major, quint16 year, quint16 month, quint16 day)
{
    return ((quint64)major << 48u) | ((quint64)year << 32u) | ((quint64)month << 16u) | ((quint64)day);
}

constexpr quint16 majorVersionPart(quint64 version)
{
    return version >> 48u;
}

constexpr quint64 dateVersionPart(quint64 version)
{
    return version & ~(0xFFFFull << 48u);
}

inline quint64 packVersion(const QStringList &version)
{
    if (version.count() < 4)
        return 0x0;
    return ((quint64)version[0].toShort() << 48u) | ((quint64)version[1].toShort() << 32u)
           | ((quint64)version[2].toShort() << 16u) | ((quint64)version[3].toShort());
}

inline quint64 packVersion(const QString &version)
{
    return packVersion(version.split(QStringLiteral(".")));
}

inline QString unpackVersionToString(quint64 version)
{
    return QStringLiteral("%1.%2.%3.%4")
        .arg(version >> 48u)
        .arg((version >> 32u) & 0xFFFFu)
        .arg((version >> 16u) & 0xFFFFu)
        .arg(version & 0xFFFFu);
}
} // namespace Proof

#endif // VERSIONHELPER_H
