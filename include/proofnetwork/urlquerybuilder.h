#ifndef PROOF_URLQUERYBUILDER_H
#define PROOF_URLQUERYBUILDER_H

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QScopedPointer>
#include <QUrlQuery>
#include <QDateTime>

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

}

#endif // PROOF_URLQUERYBUILDER_H
