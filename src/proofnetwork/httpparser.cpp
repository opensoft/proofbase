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
#include "proofnetwork/httpparser_p.h"

#include <QObject>
#include <QRegExp>

using namespace Proof;

const QRegExp HttpParser::FIRST_LINE_REG_EXP{"(.*) (.*) HTTP/1[.][01]\r\n"};
const QRegExp HttpParser::HEADER_REG_EXP{"((.*): (.*))\r\n"};

HttpParser::HttpParser()
{}

HttpParser::Result HttpParser::parseNextPart(QByteArray data) // clazy:exclude=function-args-by-ref
{
    Result result;
    do
        result = (this->*m_state)(data);
    while (result == Result::NeedMore && !data.isEmpty());
    return result;
}

QString HttpParser::method() const
{
    return m_method;
}

QString HttpParser::uri() const
{
    return m_uri;
}

QStringList HttpParser::headers() const
{
    return m_headers;
}

QByteArray HttpParser::body() const
{
    return m_data;
}

QString HttpParser::error() const
{
    return m_error;
}

HttpParser::Result HttpParser::initialState(QByteArray &data)
{
    Result result;
    int endLineIndex = data.indexOf("\n");
    if (endLineIndex != -1) {
        m_data.append(data.constData(), endLineIndex + 1);
        data.remove(0, endLineIndex + 1);
        QString startLine(m_data);
        m_data.clear();
        QRegExp firstLineRegExp = FIRST_LINE_REG_EXP;
        if (firstLineRegExp.indexIn(startLine) != -1) {
            m_method = firstLineRegExp.cap(1);
            m_uri = firstLineRegExp.cap(2);
            m_state = &HttpParser::headersState;
            result = Result::NeedMore;
        } else {
            m_error = QStringLiteral("Invalid start line: %1").arg(startLine);
            result = Result::Error;
        }
    } else {
        m_data.append(data);
        data.clear();
        result = Result::NeedMore;
    }
    return result;
}

HttpParser::Result HttpParser::headersState(QByteArray &data)
{
    Result result;
    int endLineIndex = data.indexOf("\n");
    if (endLineIndex != -1) {
        m_data.append(data.constData(), endLineIndex + 1);
        data.remove(0, endLineIndex + 1);
        QString header(m_data);
        m_data.clear();
        QRegExp headerRegExp = HEADER_REG_EXP;
        if (headerRegExp.indexIn(header) != -1) {
            result = Result::NeedMore;
            m_headers << headerRegExp.cap(1);
            if (headerRegExp.cap(2).compare(QLatin1String("Content-Length"), Qt::CaseInsensitive) == 0) {
                bool ok = false;
                m_contentLength = headerRegExp.cap(3).toULongLong(&ok);
                if (!ok) {
                    result = Result::Error;
                    m_error = QStringLiteral("Can't convert %1 to unsinged long long for \"Content-Length\"")
                                  .arg(headerRegExp.cap(3));
                }
            }
        } else if (header == QLatin1String("\r\n")) {
            if (m_contentLength != 0) {
                m_state = &HttpParser::bodyState;
                result = Result::NeedMore;
            } else {
                result = Result::Success;
            }
        } else {
            m_error = QStringLiteral("Invalid header: %1").arg(header);
            result = Result::Error;
        }
    } else {
        m_data.append(data);
        data.clear();
        result = Result::NeedMore;
    }
    return result;
}

HttpParser::Result HttpParser::bodyState(QByteArray &data)
{
    Result result;
    m_data.append(data);
    data.clear();
    if ((qulonglong)m_data.size() > m_contentLength) {
        m_error = QString();
        result = Result::Error;
    } else if ((qulonglong)m_data.size() == m_contentLength) {
        result = Result::Success;
    } else {
        result = Result::NeedMore;
    }
    return result;
}
