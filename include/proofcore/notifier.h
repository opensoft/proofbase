#ifndef PROOF_NOTIFIER_H
#define PROOF_NOTIFIER_H

#include "proofcore/proofobject.h"
#include "proofcore/proofcore_global.h"

#include "abstractnotificationhandler.h"

#include <type_traits>

namespace Proof {

class NotifierPrivate;
//TODO: make register/unregister thread-safe if ever will be needed
class PROOF_CORE_EXPORT Notifier : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Notifier)
public:
    static Notifier *instance();
    void notify(const QString &message);

    //Will not register same handler type twice
    //In case of trying to register same handler type will replace old one
    template<class Handler,
             typename std::enable_if<std::is_base_of<AbstractNotificationHandler, Handler>::value, int>::type = 0>
    void registerHandler(Handler *handler)
    {
        registerHandler(Handler::id(), handler);
    }

    template<class Handler,
             typename std::enable_if<std::is_base_of<AbstractNotificationHandler, Handler>::value, int>::type = 0>
    void unregisterHandler()
    {
        unregisterHandler(Handler::id());
    }

    template<class Handler,
             typename std::enable_if<std::is_base_of<AbstractNotificationHandler, Handler>::value, int>::type = 0>
    Handler *handler()
    {
        return static_cast<Handler *>(handler(Handler::id()));
        //Handler handler HANDLER HaNdLeR
    }

private:
    Notifier();
    ~Notifier();

    void registerHandler(const QString &handlerId, AbstractNotificationHandler *handler);
    void unregisterHandler(const QString &handlerId);
    AbstractNotificationHandler *handler(const QString &handlerId);
};

} // namespace Proof

#endif // PROOF_NOTIFIER_H
