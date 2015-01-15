#ifndef PROOFOBJECT_H
#define PROOFOBJECT_H

#include "proofcore/proofcore_global.h"

#include <QObject>
#include <QThread>
#include <QScopedPointer>
#include <QSharedPointer>


namespace Proof {

class ProofObjectPrivate;

class PROOF_CORE_EXPORT ProofObject : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ProofObject)
public:
    explicit ProofObject(QObject *parent);
    ~ProofObject();

    template <class CallerType, class ReturnType, class Method, class ...Args>
    static typename std::enable_if<!std::is_base_of<ProofObject, CallerType>::value
                                    && std::is_base_of<QObject, CallerType>::value, bool>::type
    blockingDelayedCall(CallerType *caller, Method method, ReturnType *result, Args... args)
    {
        if (QThread::currentThread() == caller->thread())
            return false;

        ProofObject *temporaryObject = new ProofObject(0);
        connect(temporaryObject, &ProofObject::delayedCallRequested,
                caller, [&result, temporaryObject, caller, method, args...]() {
            result = (*caller.*method)(args...);
            temporaryObject->deleteLater();
        }, Qt::BlockingQueuedConnection);
        emit temporaryObject->delayedCallRequested(0, QPrivateSignal{});
        return true;
    }

    template <class CallerType, class Method, class ...Args>
    static typename std::enable_if<!std::is_base_of<ProofObject, CallerType>::value
                                    && std::is_base_of<QObject, CallerType>::value, bool>::type
    blockingDelayedCall(CallerType *caller, Method method, void *dummy, Args... args)
    {
        Q_UNUSED(dummy);
        if (QThread::currentThread() == caller->thread())
            return false;

        ProofObject *temporaryObject = new ProofObject(0);
        connect(temporaryObject, &ProofObject::delayedCallRequested,
                caller, [temporaryObject, caller, method, args...]() {
            (*caller.*method)(args...);
            temporaryObject->deleteLater();
        }, Qt::BlockingQueuedConnection);
        emit temporaryObject->delayedCallRequested(0, QPrivateSignal{});
        return true;
    }

    template <class CallerType, class ReturnType, class Method, class ...Args>
    static typename std::enable_if<std::is_base_of<ProofObject, CallerType>::value, bool>::type
    blockingDelayedProofedCall(CallerType *caller, Method method, ReturnType &result, Args... args)
    {
        if (QThread::currentThread() == caller->thread())
            return false;
        qulonglong currentId = caller->nextDelayedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        *conn = connect(caller, &ProofObject::delayedCallRequested,
                        caller, [&result, caller, conn, currentId, method, args...](qulonglong delayedCallId) {
            if (currentId != delayedCallId)
                return;
            result = (*caller.*method)(args...);
            disconnect(*conn);
        }, Qt::BlockingQueuedConnection);
        emit caller->delayedCallRequested(currentId, QPrivateSignal{});
        return true;
    }

    template <class CallerType, class Method, class ...Args>
    static typename std::enable_if<std::is_base_of<ProofObject, CallerType>::value, bool>::type
    blockingDelayedProofedCall(CallerType *caller, Method method, void *dummy, Args... args)
    {
        Q_UNUSED(dummy);
        if (QThread::currentThread() == caller->thread())
            return false;
        qulonglong currentId = caller->nextDelayedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        *conn = connect(caller, &ProofObject::delayedCallRequested,
                        caller, [caller, conn, currentId, method, args...](qulonglong delayedCallId) {
            if (currentId != delayedCallId)
                return;
            (*caller.*method)(args...);
            disconnect(*conn);
        }, Qt::BlockingQueuedConnection);
        emit caller->delayedCallRequested(currentId, QPrivateSignal{});
        return true;
    }

    template <class CallerType, class Method, class ...Args>
    static typename std::enable_if<!std::is_base_of<ProofObject, CallerType>::value
                                    && std::is_base_of<QObject, CallerType>::value, bool>::type
    delayedCall(CallerType *caller, Method method, Args... args)
    {
        if (QThread::currentThread() == caller->thread())
            return false;

        ProofObject *temporaryObject = new ProofObject(0);
        connect(temporaryObject, &ProofObject::delayedCallRequested,
                caller, [temporaryObject, caller, method, args...]() {
            (*caller.*method)(args...);
            temporaryObject->deleteLater();
        });
        emit temporaryObject->delayedCallRequested(0, QPrivateSignal{});
        return true;
    }

    template <class CallerType, class Method, class ...Args>
    static typename std::enable_if<std::is_base_of<ProofObject, CallerType>::value, bool>::type
    delayedProofedCall(CallerType *caller, Method method, Args... args)
    {
        if (QThread::currentThread() == caller->thread())
            return false;
        qulonglong currentId = caller->nextDelayedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        *conn = connect(caller, &ProofObject::delayedCallRequested,
                        caller, [caller, conn, currentId, method, args...](qulonglong delayedCallId) {
            if (currentId != delayedCallId)
                return;
            (*caller.*method)(args...);
            disconnect(*conn);
        });
        emit caller->delayedCallRequested(currentId, QPrivateSignal{});
        return true;
    }

signals:
    void delayedCallRequested(qulonglong delayedCallId, QPrivateSignal);

protected:
    ProofObject(ProofObjectPrivate &dd, QObject *parent = 0);
    QScopedPointer<ProofObjectPrivate> d_ptr;

private:
    ProofObject() = delete;

    qulonglong nextDelayedCallId();
};

}

#endif // PROOFOBJECT_H
