#include "urlquerybuilder.h"
#include "urlquerybuilder_p.h"

#include <QTimeZone>

namespace Proof {

UrlQueryBuilder::UrlQueryBuilder()
    : UrlQueryBuilder(*new UrlQueryBuilderPrivate)
{
}

UrlQueryBuilder::~UrlQueryBuilder()
{

}

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
    if (result.lastIndexOf(QRegExp("[-+]\\d\\d:?\\d\\d")) == -1) {
        if (result.endsWith(QLatin1String("Z")))
            result.chop(1);
        result += value.timeZone().displayName(value, QTimeZone::OffsetName).replace(QLatin1String("UTC"), QLatin1String(""));
    }
    result.replace(QRegExp("([-+])(\\d\\d):?(\\d\\d)"), QStringLiteral("\\1\\2\\3"));
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
    Q_D(const UrlQueryBuilder);
    return d->params.contains(name);
}

QString UrlQueryBuilder::customParam(const QString &name) const
{
    Q_D(const UrlQueryBuilder);
    return d->params.value(name, QString());
}

void UrlQueryBuilder::unsetCustomParam(const QString &name)
{
    Q_D(UrlQueryBuilder);
    d->params.remove(name);
}

QUrlQuery UrlQueryBuilder::toUrlQuery() const
{
    Q_D(const UrlQueryBuilder);
    QUrlQuery urlQuery;
    for (auto it = d->params.cbegin(); it != d->params.cend(); ++it) {
        QString value = it.value();
        value.replace(QLatin1String("+"), QLatin1String("%2B"));
        urlQuery.addQueryItem(it.key(), value);
    }
    return urlQuery;
}

UrlQueryBuilder::UrlQueryBuilder(UrlQueryBuilderPrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

}
