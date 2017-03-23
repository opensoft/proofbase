#ifndef PROOF_MEMORYSTORAGENOTIFICATIONHANDLER_H
#define PROOF_MEMORYSTORAGENOTIFICATIONHANDLER_H

#include "proofcore/abstractnotificationhandler.h"
#include "proofcore/proofcore_global.h"

#include <QString>
#include <QDateTime>
#include <QMultiMap>

namespace Proof {
class MemoryStorageNotificationHandlerPrivate;
class PROOF_CORE_EXPORT MemoryStorageNotificationHandler : public AbstractNotificationHandler
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MemoryStorageNotificationHandler)
public:
    MemoryStorageNotificationHandler(const QString &appId);

    QMultiMap<QDateTime, QString> messages() const;
    QPair<QDateTime, QString> lastMessage() const;

    void notify(const QString &message, ErrorNotifier::Severity severity, const QString &packId) override;

    static QString id();
};

} // namespace Proof

#endif // PROOF_MEMORYSTORAGENOTIFICATIONHANDLER_H
