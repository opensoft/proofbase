#include "proofglobal.h"

namespace Proof {
QString proofVersion() {
    return PROOF_VERSION;
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
}
