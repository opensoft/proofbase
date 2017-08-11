#ifndef PROOFGLOBAL_H
#define PROOFGLOBAL_H

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
}

namespace algorithms {
//TODO: Predicate should accept two parameters (key and value), not an iterator
//TODO: add QHash
template<typename Key, typename T, typename Predicate>
void eraseIf(QMap<Key, T> &container, const Predicate &predicate)
{
    for (auto it = container.begin(); it != container.end();) {
        if (predicate(it))
            it = container.erase(it);
        else
            ++it;
    }
}

template<typename Container, typename Predicate>
void eraseIf(Container &container, const Predicate &predicate)
{
    container.erase(std::remove_if(container.begin(), container.end(), predicate), container.end());
}

template<typename Container, typename T>
auto addToContainer(Container &container, const T &value) -> decltype(container.push_back(value), void())
{
    container.push_back(value);
}

template<typename Container, typename T>
auto addToContainer(Container &container, const T &value) -> decltype(container.insert(value), void())
{
    container.insert(value);
}

//This one is not lazy, it copies values. It is fine for almost all our cases with DTOs and PODs, but still is O(n).
//If ever lazy variant will be needed probably we will need to use boost ranges or something similar then.
//There is no need for map btw, since we have std::transform. It will not work for QMap/QHash, so if we ever will need it - we can add later.
//Reduce can be achieved by std::accumulate.
template<typename Container, typename Predicate>
auto filter(const Container &container, const Predicate &predicate)
-> decltype(addToContainer(const_cast<Container&>(container), *(container.cbegin())),
            predicate(*(container.cbegin())),
            Container())
{
    Container result;
    auto it = container.cbegin();
    auto end = container.cend();
    for (; it != end; ++it) {
        if (predicate(*it))
            addToContainer(result, *it);
    }
    return result;
}

template<typename Container, typename Predicate>
auto filter(const Container &container, const Predicate &predicate)
-> decltype(const_cast<Container&>(container).insert(container.cbegin().key(), container.cbegin().value()),
            predicate(container.cbegin().key(), container.cbegin().value()),
            Container())
{
    Container result;
    auto it = container.cbegin();
    auto end = container.cend();
    for (; it != end; ++it) {
        if (predicate(it.key(), it.value()))
            result.insert(it.key(), it.value());
    }
    return result;
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
