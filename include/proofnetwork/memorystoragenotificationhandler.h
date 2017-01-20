#ifndef PROOF_MEMORYSTORAGENOTIFICATIONHANDLER_H
#define PROOF_MEMORYSTORAGENOTIFICATIONHANDLER_H

#include "proofcore/abstractnotificationhandler.h"
#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QString>
#include <QDateTime>
#include <QMultiMap>

namespace Proof {
class MemoryStorageNotificationHandlerPrivate;
class PROOF_NETWORK_EXPORT MemoryStorageNotificationHandler : public AbstractNotificationHandler
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MemoryStorageNotificationHandler)
public:
    MemoryStorageNotificationHandler(const QString &appId);

    QMultiMap<QDateTime, QString> messages() const;
    QPair<QDateTime, QString> lastMessage() const;

    void notify(const QString &message) override;

    static QString id();
};

} // namespace Proof

#endif // PROOF_MEMORYSTORAGENOTIFICATIONHANDLER_H
