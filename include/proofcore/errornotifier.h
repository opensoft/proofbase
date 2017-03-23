#ifndef PROOF_ERRORNOTIFIER_H
#define PROOF_ERRORNOTIFIER_H

#include "proofcore/proofobject.h"
#include "proofcore/proofcore_global.h"

namespace Proof {

//TODO: add messageType to allow skip same messages to be sent again and again in small period of time (useful for emails)
//TODO: add notifications to Logs
//TODO: add migrations support

class AbstractNotificationHandler;
class ErrorNotifierPrivate;
class PROOF_CORE_EXPORT ErrorNotifier : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ErrorNotifier)
public:
    enum class Severity {
        Warning,
        Error,
        Critical
    };

    static ErrorNotifier *instance();
    void notify(const QString &message, Severity severity = Severity::Error, const QString &packId = QString());

    //Will not register same handler type twice
    //In case of trying to register same handler type will replace old one
    template<class Handler>
    void registerHandler(Handler *handler)
    {
        registerHandler(Handler::id(), handler);
    }

    template<class Handler>
    void unregisterHandler()
    {
        unregisterHandler(Handler::id());
    }

    template<class Handler>
    Handler *handler()
    {
        return static_cast<Handler *>(handler(Handler::id()));
        //Handler handler HANDLER HaNdLeR
    }

private:
    ErrorNotifier();
    ~ErrorNotifier();

    void registerHandler(const QString &handlerId, AbstractNotificationHandler *handler);
    void unregisterHandler(const QString &handlerId);
    AbstractNotificationHandler *handler(const QString &handlerId);
};

} // namespace Proof

#endif // PROOF_ERRORNOTIFIER_H
