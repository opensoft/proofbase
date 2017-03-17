#ifndef HUMANIZER_H
#define HUMANIZER_H

#include "proofcore/proofobject.h"
#include "proofcore/proofcore_global.h"

namespace Proof {

class PROOF_CORE_EXPORT Humanizer : public ProofObject
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

    Humanizer(QObject *parent = nullptr);

    Q_INVOKABLE QString humanizeTime(qlonglong seconds, Humanizer::TimeCategory stopAt = Humanizer::TimeCategory::StopAtSeconds);
    Q_INVOKABLE QString humanizeBytesSize(qlonglong bytes);
};

PROOF_CORE_EXPORT QString humanizeTime(qlonglong seconds, Humanizer::TimeCategory stopAt = Humanizer::TimeCategory::StopAtSeconds);
PROOF_CORE_EXPORT QString humanizeBytesSize(qlonglong bytes);

}

#endif // HUMANIZER_H
