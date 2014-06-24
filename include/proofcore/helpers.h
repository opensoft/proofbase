#ifndef HELPERS_H
#define HELPERS_H

#include <QObject>

#include "proofcore_global.h"

namespace Proof {

class PROOF_CORE_EXPORT Humanizer : public QObject
{
    Q_OBJECT
    Q_ENUMS(TimeCategory)
public:
    enum TimeCategory {
        StopAtSeconds,
        StopAtMinutes,
        StopAtHours,
        StopAtDays,
        StopAtWeeks
    };

    static QString humanizeTime(qlonglong seconds, TimeCategory stopAt = TimeCategory::StopAtSeconds);
    static QString humanizeBytesSize(qlonglong bytes);
private:
    Humanizer(QObject *parent = 0);
    ~Humanizer();

};

}

#endif // HELPERS_H
