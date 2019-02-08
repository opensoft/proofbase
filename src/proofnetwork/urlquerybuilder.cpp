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
#include "proofnetwork/urlquerybuilder.h"

#include "proofcore/proofglobal.h"

#include "proofnetwork/urlquerybuilder_p.h"

#include <QTimeZone>

namespace Proof {

UrlQueryBuilder::UrlQueryBuilder() : UrlQueryBuilder(*new UrlQueryBuilderPrivate)
{}

UrlQueryBuilder::~UrlQueryBuilder()
{}

void UrlQueryBuilder::setCustomParam(const QString &name, const QString &value)
{
    Q_D(UrlQueryBuilder);
    d->params[name] = value;
}

void UrlQueryBuilder::setCustomParam(const QString &name, qlonglong value)
{
    setCustomParam(name, QString::number(value));
}

void UrlQueryBuilder::setCustomParam(const QString &name, qulonglong value)
{
    setCustomParam(name, QString::number(value));
}

void UrlQueryBuilder::setCustomParam(const QString &name, long value)
{
    setCustomParam(name, QString::number(value));
}

void UrlQueryBuilder::setCustomParam(const QString &name, unsigned long value)
{
    setCustomParam(name, QString::number(value));
}

void UrlQueryBuilder::setCustomParam(const QString &name, int value)
{
    setCustomParam(name, QString::number(value));
}

void UrlQueryBuilder::setCustomParam(const QString &name, unsigned int value)
{
    setCustomParam(name, QString::number(value));
}

void UrlQueryBuilder::setCustomParam(const QString &name, const QDateTime &value)
{
    //ProFIT supports slightly different format from ISO8601 so we need to modify it accordingly
    QString result = value.toString(Qt::ISODate).replace(QLatin1String("T"), QLatin1String(" "));
    if (result.lastIndexOf(QRegExp(R"([-+]\d\d:?\d\d)")) == -1) {
        if (result.endsWith(QLatin1String("Z")))
            result.chop(1);
        result += value.timeZone().displayName(value, QTimeZone::OffsetName).replace(QLatin1String("UTC"), QString());
    }
    result.replace(QRegExp(R"(([-+])(\d\d):?(\d\d))"), QStringLiteral(R"(\1\2\3)"));
    setCustomParam(name, result);
}

void UrlQueryBuilder::setCustomParam(const QString &name, bool value)
{
    setCustomParam(name, value ? QStringLiteral("true") : QStringLiteral("false"));
}

void UrlQueryBuilder::setCustomParam(const QString &name, const char *value)
{
    setCustomParam(name, QString(value));
}

void UrlQueryBuilder::setCustomParam(const QString &name, double value)
{
    setCustomParam(name, QString::number(value, 'f', 3));
}

bool UrlQueryBuilder::containsCustomParam(const QString &name) const
{
    Q_D_CONST(UrlQueryBuilder);
    return d->params.contains(name);
}

QString UrlQueryBuilder::customParam(const QString &name) const
{
    Q_D_CONST(UrlQueryBuilder);
    return d->params.value(name, QString());
}

void UrlQueryBuilder::unsetCustomParam(const QString &name)
{
    Q_D(UrlQueryBuilder);
    d->params.remove(name);
}

QUrlQuery UrlQueryBuilder::toUrlQuery() const
{
    Q_D_CONST(UrlQueryBuilder);
    QUrlQuery urlQuery;
    for (auto it = d->params.cbegin(); it != d->params.cend(); ++it) {
        QString value = it.value();
        value.replace(QLatin1String("+"), QLatin1String("%2B"));
        urlQuery.addQueryItem(it.key(), value);
    }
    return urlQuery;
}

UrlQueryBuilder::UrlQueryBuilder(UrlQueryBuilderPrivate &dd) : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

} // namespace Proof
