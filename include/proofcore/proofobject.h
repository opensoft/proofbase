#ifndef PROOFOBJECT_H
#define PROOFOBJECT_H

#include "proofcore/proofcore_global.h"

#include <QObject>
#include <QThread>
#include <QScopedPointer>
#include <QSharedPointer>

#include <functional>

namespace Proof {

class ProofObjectPrivate;

class PROOF_CORE_EXPORT ProofObject : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ProofObject)
public:
    explicit ProofObject(QObject *parent);
    ~ProofObject();

    //TODO: change all binds to normal lambdas after switch to gcc>=4.9.0

    template <class CallerType, class ReturnType, class Method, class ...Args>
    static typename std::enable_if<!std::is_base_of<ProofObject, CallerType>::value
                                    && std::is_base_of<QObject, CallerType>::value, bool>::type
    blockingDelayedCall(CallerType *caller, Method method, ReturnType &result, Args... args)
    {
        if (QThread::currentThread() == caller->thread())
            return false;

        ProofObject *temporaryObject = new ProofObject(0);
        auto f = std::bind([](qulonglong, ReturnType *result,
                           CallerType *caller, ProofObject *temporaryObject,
                           Method method, Args... args) {
                result = (*caller.*method)(args...);
                temporaryObject->deleteLater();
        }, std::placeholders::_1, &result, caller, temporaryObject, method, args...);
        connect(temporaryObject, &ProofObject::delayedCallRequested,
                caller, f, Qt::BlockingQueuedConnection);
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
        auto f = std::bind([](qulonglong,
                           CallerType *caller, ProofObject *temporaryObject,
                           Method method, Args... args) {
                (*caller.*method)(args...);
                temporaryObject->deleteLater();
        }, std::placeholders::_1, caller, temporaryObject, method, args...);
        connect(temporaryObject, &ProofObject::delayedCallRequested,
                caller, f, Qt::BlockingQueuedConnection);
        emit temporaryObject->delayedCallRequested(0, QPrivateSignal{});
        return true;
    }

    template <class CallerType, class ReturnType, class Method, class ...Args>
    static typename std::enable_if<std::is_base_of<ProofObject, CallerType>::value, bool>::type
    blockingDelayedCall(CallerType *caller, Method method, ReturnType &result, Args... args)
    {
        if (QThread::currentThread() == caller->thread())
            return false;
        qulonglong currentId = caller->nextDelayedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        auto f = std::bind([](qulonglong delayedCallId, ReturnType *result,
                           CallerType *caller, QSharedPointer<QMetaObject::Connection> conn,
                           qulonglong currentId, Method method, Args... args) {
            if (currentId != delayedCallId)
                return;
            *result = (*caller.*method)(args...);
            disconnect(*conn);
        }, std::placeholders::_1, &result, caller, conn, currentId, method, args...);
        *conn = connect(caller, &ProofObject::delayedCallRequested,
                        caller, f, Qt::BlockingQueuedConnection);
        emit caller->delayedCallRequested(currentId, QPrivateSignal{});
        return true;
    }

    template <class CallerType, class Method, class ...Args>
    static typename std::enable_if<std::is_base_of<ProofObject, CallerType>::value, bool>::type
    blockingDelayedCall(CallerType *caller, Method method, void *dummy, Args... args)
    {
        Q_UNUSED(dummy);
        if (QThread::currentThread() == caller->thread())
            return false;
        qulonglong currentId = caller->nextDelayedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        auto f = std::bind([](qulonglong delayedCallId,
                           CallerType *caller, QSharedPointer<QMetaObject::Connection> conn,
                           qulonglong currentId, Method method, Args... args) {
            if (currentId != delayedCallId)
                return;
            (*caller.*method)(args...);
            disconnect(*conn);
        }, std::placeholders::_1, caller, conn, currentId, method, args...);
        *conn = connect(caller, &ProofObject::delayedCallRequested,
                        caller, f, Qt::BlockingQueuedConnection);
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
        auto f = std::bind([](qulonglong,
                           CallerType *caller, ProofObject *temporaryObject,
                           Method method, Args... args) {
                (*caller.*method)(args...);
                temporaryObject->deleteLater();
        }, std::placeholders::_1, caller, temporaryObject, method, args...);
        connect(temporaryObject, &ProofObject::delayedCallRequested,
                caller, f);
        emit temporaryObject->delayedCallRequested(0, QPrivateSignal{});
        return true;
    }

    template <class CallerType, class Method, class ...Args>
    static typename std::enable_if<std::is_base_of<ProofObject, CallerType>::value, bool>::type
    delayedCall(CallerType *caller, Method method, Args... args)
    {
        if (QThread::currentThread() == caller->thread())
            return false;
        qulonglong currentId = caller->nextDelayedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        auto f = std::bind([](qulonglong delayedCallId,
                           CallerType *caller, QSharedPointer<QMetaObject::Connection> conn,
                           qulonglong currentId, Method method, Args... args) {
                if (currentId != delayedCallId)
                    return;
                (*caller.*method)(args...);
                disconnect(*conn);
        }, std::placeholders::_1, caller, conn, currentId, method, args...);
        *conn = connect(caller, &ProofObject::delayedCallRequested,
                        caller, f);
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
