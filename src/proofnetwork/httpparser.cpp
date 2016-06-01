#include "httpparser_p.h"

#include <QObject>
#include <QRegExp>

static const QRegExp FIRST_LINE_REG_EXP("(.*) (.*) HTTP/1[.][01]\r\n");
static const QRegExp HEADER_REG_EXP("((.*): (.*))\r\n");

using namespace Proof;

HttpParser::HttpParser()
{
}

HttpParser::Result HttpParser::parseNextPart(QByteArray data)
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
            m_error = QString("Invalid start line: %1").arg(startLine);
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
            if (headerRegExp.cap(2).compare("Content-Length", Qt::CaseInsensitive) == 0) {
                bool ok = false;
                m_contentLength = headerRegExp.cap(3).toULongLong(&ok);
                if (!ok) {
                    result = Result::Error;
                    m_error = QString("Can't convert %1 to unsinged long long for \"Content-Length\"").arg(headerRegExp.cap(3));
                }
            }
        } else if (header == "\r\n") {
            if (m_contentLength != 0) {
                m_state = &HttpParser::bodyState;
                result = Result::NeedMore;
            } else {
                result = Result::Success;
            }
        } else {
            m_error = QString("Invalid header: %1").arg(header);
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
