#ifndef PROOF_HTTPPARSER_P_H
#define PROOF_HTTPPARSER_P_H

#include <QByteArray>
#include <QStringList>

namespace Proof {

class HttpParser
{
public:
    enum class Result
    {
        NeedMore,
        Error,
        Success
    };

    HttpParser();
    Result parseNextPart(QByteArray data);

    QString method() const;
    QString uri() const;
    QStringList headers() const;
    QByteArray body() const;

    QString error() const;

private:
    Result initialState(QByteArray &data);
    Result headersState(QByteArray &data);
    Result bodyState(QByteArray &data);

private:
    using State = Result (HttpParser:: *)(QByteArray &);

    State m_state = &HttpParser::initialState;
    QByteArray m_data;
    qulonglong m_contentLength = 0;
    QString m_method;
    QString m_uri;
    QStringList m_headers;
    QString m_error;
};

} // namespace Proof

#endif // PROOF_HTTPPARSER_P_H
