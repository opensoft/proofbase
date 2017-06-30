#ifndef PROOFOBJECT_H
#define PROOFOBJECT_H

#include "proofcore/proofcore_global.h"
#include "proofcore/proofobjectprivatepointer.h"

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

    bool isDirty() const;

    template <class Callee, class Result, class Method, class... Args>
    static auto call(Callee *callee, Method method, Proof::Call callType, Result &result, Args &&... args)
        -> decltype((void)(result = (callee->*method)(std::forward<Args>(args)...)), bool{})
    {
        return callPrivate(callee, callee, method, callType, &result, std::forward<Args>(args)...);
    }

    template <class Callee, class Method, class... Args>
    static auto call(Callee *callee, Method method, Proof::Call callType, Args &&... args)
        -> decltype((void)(callee->*method)(std::forward<Args>(args)...), bool{})
    {
        return callPrivate(callee, callee, method, callType, nullptr, std::forward<Args>(args)...);
    }

    template <class Callee, class Method, class... Args>
    static auto call(Callee *callee, Method method, Args &&... args)
        -> decltype((void)(callee->*method)(std::forward<Args>(args)...), bool{})
    {
        return callPrivate(callee, callee, method, std::forward<Args>(args)...);
    }

    template <class Callee, class Result, class Method, class... Args>
    static auto call(QObject *context, Callee *callee, Method method, Proof::Call callType, Result &result, Args &&... args)
        -> decltype((void)(result = (callee->*method)(std::forward<Args>(args)...)), bool{})
    {
        return callPrivate(context, callee, method, callType, &result, std::forward<Args>(args)...);
    }

    template <class Callee, class Method, class... Args>
    static auto call(QObject *context, Callee *callee, Method method, Proof::Call callType, Args &&... args)
        -> decltype((void)(callee->*method)(std::forward<Args>(args)...), bool{})
    {
        return callPrivate(context, callee, method, callType, nullptr, std::forward<Args>(args)...);
    }

    template <class Callee, class Method, class... Args>
    static auto call(QObject *context, Callee *callee, Method method, Args &&... args)
        -> decltype((void)(callee->*method)(std::forward<Args>(args)...), bool{})
    {
        return callPrivate(context, callee, method, std::forward<Args>(args)...);
    }

signals:
    void queuedCallRequested(qulonglong queuedCallId, QPrivateSignal);

protected:
    ProofObject(ProofObjectPrivate &dd, QObject *parent = nullptr);
    ProofObjectPrivatePointer d_ptr;

private:
    ProofObject() = delete;

    qulonglong nextQueuedCallId() const;
    static ProofObject *defaultInvoker();

    template <class Callee, class ResultPtr, class Method, class... Args>
    static bool callPrivate(QObject *context, Callee *callee, Method method, Proof::Call callType, ResultPtr resultPtr, Args &&... args)
    {
        if (QThread::currentThread() == context->thread())
            return false;

        ProofObject *invoker = invokerForCallee(callee);
        qulonglong currentId = invoker->nextQueuedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        std::atomic_bool done(false);

        auto f = std::bind([](qulonglong queuedCallId, ResultPtr resultPtr, std::atomic_bool *done,
                           Callee *callee, qulonglong currentId, Method method,
                           typename std::decay<Args>::type &... args) {
                if (currentId != queuedCallId)
                    return;
                doTheCall(resultPtr, callee, method, std::forward<Args>(args)...);
                *done = true;
        }, std::placeholders::_1, resultPtr, &done, callee, currentId, method, std::forward<Args>(args)...);

        *conn = connect(invoker, &ProofObject::queuedCallRequested,
                        context, f, callType == Proof::Call::BlockEvents ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
        emit invoker->queuedCallRequested(currentId, QPrivateSignal{});

        while (!done)
            QCoreApplication::processEvents();
        disconnect(*conn);
        return true;
    }

    template <class Callee, class Method, class... Args>
    static bool callPrivate(QObject *context, Callee *callee, Method method, Args &&... args)
    {
        if (QThread::currentThread() == context->thread())
            return false;

        ProofObject *invoker = invokerForCallee(callee);
        qulonglong currentId = invoker->nextQueuedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();

        auto f = std::bind([](qulonglong queuedCallId, Callee *callee,
                           QSharedPointer<QMetaObject::Connection> conn,
                           qulonglong currentId, Method method,
                           typename std::decay<Args>::type &... args) {
                if (currentId != queuedCallId)
                    return;
                doTheCall(nullptr, callee, method, std::forward<Args>(args)...);
                disconnect(*conn);
        }, std::placeholders::_1, callee, conn, currentId, method, std::forward<Args>(args)...);
        *conn = connect(invoker, &ProofObject::queuedCallRequested,
                        context, f, Qt::QueuedConnection);
        emit invoker->queuedCallRequested(currentId, QPrivateSignal{});

        return true;
    }

    template <class Callee, class Result, class Method, class... Args>
    static void doTheCall(Result *result, Callee* callee, Method method, Args &&... args)
    {
        *result = (*callee.*method)(std::forward<Args>(args)...);
    }

    template <class Callee, class Method, class... Args>
    static void doTheCall(void *result, Callee* callee, Method method, Args &&... args)
    {
        Q_UNUSED(result)
        (*callee.*method)(std::forward<Args>(args)...);
    }

    template <class Callee>
    static typename std::enable_if<!std::is_base_of<ProofObject, Callee>::value, ProofObject *>::type
    invokerForCallee(Callee *callee)
    {
        Q_UNUSED(callee)
        return defaultInvoker();
    }

    template <class Callee>
    static typename std::enable_if<std::is_base_of<ProofObject, Callee>::value, ProofObject *>::type
    invokerForCallee(Callee *callee)
    {
        return callee;
    }
};

}

#endif // PROOFOBJECT_H
