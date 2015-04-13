#ifndef PROOFOBJECT_H
#define PROOFOBJECT_H

#include "proofcore/proofcore_global.h"

#include <QObject>
#include <QThread>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QCoreApplication>

#include <functional>
#include <atomic>

namespace Proof {

enum class Call {
    Block,
    BlockEvents
};

class ProofObjectPrivate;

class PROOF_CORE_EXPORT ProofObject : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ProofObject)
public:
    explicit ProofObject(QObject *parent);
    ~ProofObject();

    template <class Callee, class Result, class MethodCallee, class... MethodArgs, class... Args>
    static typename std::enable_if<std::is_base_of<MethodCallee, Callee>::value && sizeof...(MethodArgs) == sizeof...(Args), bool>::type
    call(Callee *callee, Result (MethodCallee:: *method)(MethodArgs...), Proof::Call callType, Result &result, Args... args)
    {
        return callPrivate(callee, method, callType, &result, args...);
    }

    template <class Callee, class Result, class MethodCallee, class... MethodArgs, class... Args>
    static typename std::enable_if<std::is_base_of<MethodCallee, Callee>::value && sizeof...(MethodArgs) == sizeof...(Args), bool>::type
    call(Callee *callee, Result (MethodCallee:: *method)(MethodArgs...), Proof::Call callType, Args... args)
    {
        return callPrivate(callee, method, callType, nullptr, args...);
    }

    template <class Callee, class Result, class MethodCallee, class... MethodArgs, class... Args>
    static typename std::enable_if<std::is_base_of<MethodCallee, Callee>::value && sizeof...(MethodArgs) == sizeof...(Args), bool>::type
    call(Callee *callee, Result (MethodCallee:: *method)(MethodArgs...), Args... args)
    {
        return callPrivate(callee, method, args...);
    }

signals:
    void queuedCallRequested(qulonglong queuedCallId, QPrivateSignal);

protected:
    ProofObject(ProofObjectPrivate &dd, QObject *parent = 0);
    QScopedPointer<ProofObjectPrivate> d_ptr;

private:
    ProofObject() = delete;

    qulonglong nextQueuedCallId();

    template <class Callee, class ResultPtr, class Method, class... Args>
    static bool callPrivate(Callee *callee, Method method, Proof::Call callType, ResultPtr resultPtr, Args... args)
    {
        if (QThread::currentThread() == callee->thread())
            return false;

        ProofObject *invoker = invokerForCallee(callee);
        qulonglong currentId = invoker->nextQueuedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        std::atomic_bool done(false);

        auto f = std::bind([](qulonglong queuedCallId, ResultPtr resultPtr, std::atomic_bool *done,
                      Callee *callee, ProofObject *invoker,
                      QSharedPointer<QMetaObject::Connection> conn,
                      qulonglong currentId, Method method, Args... args) {
                if (currentId != queuedCallId)
                    return;
                doTheCall(resultPtr, callee, method, args...);
                cleanupAfterCall(callee, invoker, conn);
                *done = true;
        }, std::placeholders::_1, resultPtr, &done, callee, invoker, conn, currentId, method, args...);

        *conn = connect(invoker, &ProofObject::queuedCallRequested,
                        callee, f, callType == Proof::Call::BlockEvents ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
        emit invoker->queuedCallRequested(currentId, QPrivateSignal{});

        if (callType == Proof::Call::Block) {
            while (!done)
                QCoreApplication::processEvents();
        }
        return true;
    }

    template <class Callee, class Method, class... Args>
    static bool callPrivate(Callee *callee, Method method, Args... args)
    {
        if (QThread::currentThread() == callee->thread())
            return false;

        ProofObject *invoker = invokerForCallee(callee);
        qulonglong currentId = invoker->nextQueuedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();

        auto f = std::bind([](qulonglong queuedCallId,
                      Callee *callee, ProofObject *invoker,
                      QSharedPointer<QMetaObject::Connection> conn,
                      qulonglong currentId, Method method, Args... args) {
                if (currentId != queuedCallId)
                    return;
                doTheCall(nullptr, callee, method, args...);
                cleanupAfterCall(callee, invoker, conn);
        }, std::placeholders::_1, callee, invoker, conn, currentId, method, args...);
        *conn = connect(invoker, &ProofObject::queuedCallRequested,
                        callee, f, Qt::QueuedConnection);
        emit invoker->queuedCallRequested(currentId, QPrivateSignal{});

        return true;
    }


    template <class Callee, class Result, class Method, class... Args>
    static void doTheCall(Result *result, Callee* callee, Method method, Args... args)
    {
        *result = (*callee.*method)(args...);
    }

    template <class Callee, class Method, class... Args>
    static void doTheCall(void *result, Callee* callee, Method method, Args... args)
    {
        Q_UNUSED(result);
        (*callee.*method)(args...);
    }

    template <class Callee>
    static typename std::enable_if<!std::is_base_of<ProofObject, Callee>::value && std::is_base_of<QObject, Callee>::value, ProofObject *>::type
    invokerForCallee(Callee *callee)
    {
        Q_UNUSED(callee);
        return new ProofObject(0);
    }

    template <class Callee>
    static typename std::enable_if<std::is_base_of<ProofObject, Callee>::value, ProofObject *>::type
    invokerForCallee(Callee *callee)
    {
        return callee;
    }

    template <class Callee>
    static typename std::enable_if<!std::is_base_of<ProofObject, Callee>::value && std::is_base_of<QObject, Callee>::value, bool>::type
    cleanupAfterCall(Callee *callee, ProofObject *delayer, QSharedPointer<QMetaObject::Connection> conn)
    {
        Q_UNUSED(callee);
        Q_UNUSED(conn);
        delayer->deleteLater();
        return true;
    }

    template <class Callee>
    static typename std::enable_if<std::is_base_of<ProofObject, Callee>::value, bool>::type
    cleanupAfterCall(Callee *callee, ProofObject *delayer, QSharedPointer<QMetaObject::Connection> conn)
    {
        Q_UNUSED(callee);
        Q_UNUSED(delayer);
        disconnect(*conn);
        return true;
    }
};

}

#endif // PROOFOBJECT_H
