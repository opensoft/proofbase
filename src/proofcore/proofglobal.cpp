#include "proofglobal.h"

namespace Proof {
static bool proofUsesSettingsValue = true;
QString proofVersion() {
    return PROOF_VERSION; // clazy:skip=qstring-allocations
}

int proofVersionMajor() {
    return PROOF_VERSION_MAJOR;
}

int proofVersionYear() {
    return PROOF_VERSION_YEAR;
}

int proofVersionMonth() {
    return PROOF_VERSION_MONTH;
}

int proofVersionDay() {
    return PROOF_VERSION_DAY;
}

void setProofUsesSettings(bool value)
{
    proofUsesSettingsValue = value;
}

bool proofUsesSettings()
{
    return proofUsesSettingsValue;
}

}
