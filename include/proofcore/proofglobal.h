/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef PROOFGLOBAL_H
#define PROOFGLOBAL_H

#include "proofseed/proofalgorithms.h"

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

#define Q_D_CONST(Class) const Class##Private *const d = const_cast<const Class *>(this)->d_func()
#define Q_Q_CONST(Class) const Class *const q = const_cast<const Class##Private *>(this)->q_func()

#ifdef Q_CC_MSVC
#    include <SDKDDKVer.h>
#    define WIN32_LEAN_AND_MEAN
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif // !NOMINMAX
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
