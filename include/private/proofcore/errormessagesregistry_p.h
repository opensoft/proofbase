#ifndef PROOF_ERRORMESSAGESREGISTRY_H
#define PROOF_ERRORMESSAGESREGISTRY_H

#include "proofcore/proofcore_global.h"

#include <QPair>
#include <QString>
#include <QStringList>
#include <QHash>

#include <initializer_list>

namespace Proof {

class PROOF_CORE_EXPORT ErrorMessagesRegistry
{
public:
    template<class Enum>
    ErrorMessagesRegistry(std::initializer_list<QPair<Enum, QString>> list);

    QString messageForCode(int code, const QStringList &args = QStringList()) const;

private:
    QHash<int, QString> m_messages;
};

template<class Enum>
ErrorMessagesRegistry::ErrorMessagesRegistry(std::initializer_list<QPair<Enum, QString> > list)
{
    for (const auto &pair : list)
        m_messages[static_cast<int>(pair.first)] = pair.second;
}

} // namespace Proof

#endif // PROOF_ERRORMESSAGESREGISTRY_H
