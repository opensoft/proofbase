#ifndef PROOFGLOBAL_H
#define PROOFGLOBAL_H

#include "proofcore/proofcore_global.h"
#include "proofcore/proofalgorithms.h"
#include <QString>

namespace Proof {
PROOF_CORE_EXPORT QString proofVersion();
PROOF_CORE_EXPORT int proofVersionMajor();
PROOF_CORE_EXPORT int proofVersionYear();
PROOF_CORE_EXPORT int proofVersionMonth();
PROOF_CORE_EXPORT int proofVersionDay();

PROOF_CORE_EXPORT void setProofUsesSettings(bool value);
PROOF_CORE_EXPORT bool proofUsesSettings();

namespace math {
static constexpr int round(double d)
{
#ifdef Q_CC_MSVC
    return d >= 0.0 ? int(d + 0.5) : int(d - double(int(d - 1)) + 0.5) + int(d - 1);
#else
    return qRound(d);
#endif
}
}
}

#ifdef Q_CC_MSVC
# pragma section(".CRT$XCU")
# define PROOF_LIBRARY_INITIALIZER(X) \
    static void X(); \
    __declspec(dllexport) \
    __declspec(allocate(".CRT$XCU")) \
    void(*proof_ctor_##X)() = &X; \
    static void X()
#else
# define PROOF_LIBRARY_INITIALIZER(f) \
    __attribute__((constructor))\
    static void f(void)
#endif // Q_CC_MSVC

#endif // PROOFGLOBAL_H
