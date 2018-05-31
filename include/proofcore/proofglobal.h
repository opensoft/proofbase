#ifndef PROOFGLOBAL_H
#define PROOFGLOBAL_H

#include "proofcore/proofalgorithms.h"
#include "proofcore/proofcore_global.h"

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
} // namespace math
} // namespace Proof

#ifdef Q_CC_MSVC
#    include <SDKDDKVer.h>
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#    define PROOF_LIBRARY_INITIALIZER(X)                                                    \
        __pragma(warning(push)) __pragma(warning(disable : 4100)) static void X(void);      \
        BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) \
        {                                                                                   \
            switch (ul_reason_for_call) {                                                   \
            case DLL_PROCESS_ATTACH:                                                        \
                X();                                                                        \
            case DLL_THREAD_ATTACH:                                                         \
            case DLL_THREAD_DETACH:                                                         \
            case DLL_PROCESS_DETACH:                                                        \
                break;                                                                      \
            }                                                                               \
            return TRUE;                                                                    \
        }                                                                                   \
        __pragma(warning(pop)) static void X(void)
#else
#    define PROOF_LIBRARY_INITIALIZER(f) __attribute__((constructor)) static void f(void)
#endif // Q_CC_MSVC

#endif // PROOFGLOBAL_H
