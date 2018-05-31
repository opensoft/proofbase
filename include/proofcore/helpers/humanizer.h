#ifndef HUMANIZER_H
#define HUMANIZER_H

#include "proofcore/proofcore_global.h"
#include "proofcore/proofobject.h"

namespace Proof {

class PROOF_CORE_EXPORT Humanizer : public ProofObject
{
    Q_OBJECT
public:
    enum TimeCategory
    {
        StopAtSeconds,
        StopAtMinutes,
        StopAtHours,
        StopAtDays,
        StopAtWeeks
    };
    Q_ENUM(TimeCategory)

    Humanizer(QObject *parent = nullptr);

    Q_INVOKABLE QString humanizeTime(qlonglong seconds,
                                     Humanizer::TimeCategory stopAt = Humanizer::TimeCategory::StopAtSeconds);
    Q_INVOKABLE QString humanizeBytesSize(qlonglong bytes);
};

PROOF_CORE_EXPORT QString humanizeTime(qlonglong seconds,
                                       Humanizer::TimeCategory stopAt = Humanizer::TimeCategory::StopAtSeconds);
PROOF_CORE_EXPORT QString humanizeBytesSize(qlonglong bytes);

} // namespace Proof

#endif // HUMANIZER_H
