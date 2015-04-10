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

class ProofObjectPrivate;

class PROOF_CORE_EXPORT ProofObject : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ProofObject)
public:
    explicit ProofObject(QObject *parent);
    ~ProofObject();

    //TODO: change all binds to normal lambdas after switch to gcc>=4.9.0

    template <class Callee, class Result, class Method, class ...Args>
    static bool blockingDelayedCall(Callee *callee, Method method, bool blockEventLoop, Result &result, Args... args)
    {
        if (QThread::currentThread() == callee->thread())
            return false;
        ProofObject *delayer = delayerForObject(callee);
        qulonglong currentId = delayer->nextDelayedCallId();
        std::atomic_bool done(false);
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        auto f = std::bind([](qulonglong delayedCallId, Result *result, std::atomic_bool *done,
                           Callee *callee, ProofObject *delayer,
                           QSharedPointer<QMetaObject::Connection> conn,
                           qulonglong currentId, Method method, Args... args) {
                if (currentId != delayedCallId)
                    return;
                *result = (*callee.*method)(args...);
                cleanupDelayer(callee, delayer, conn);
                *done = true;
        }, std::placeholders::_1, &result, &done, callee, delayer, conn, currentId, method, args...);
        *conn = connect(delayer, &ProofObject::delayedCallRequested,
                        callee, f, blockEventLoop ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
        emit delayer->delayedCallRequested(currentId, QPrivateSignal{});
        while (!done)
            QCoreApplication::processEvents();

        return true;
    }

    template <class Callee, class Method, class ...Args>
    static bool blockingDelayedCall(Callee *callee, Method method, bool blockEventLoop, void *dummy, Args... args)
    {
        Q_UNUSED(dummy);
        if (QThread::currentThread() == callee->thread())
            return false;

        ProofObject *delayer = delayerForObject(callee);
        qulonglong currentId = delayer->nextDelayedCallId();
        std::atomic_bool done(false);
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        auto f = std::bind([](qulonglong delayedCallId, std::atomic_bool *done,
                           Callee *callee, ProofObject *delayer,
                           QSharedPointer<QMetaObject::Connection> conn,
                           qulonglong currentId, Method method, Args... args) {
                if (currentId != delayedCallId)
                    return;
                (*callee.*method)(args...);
                cleanupDelayer(callee, delayer, conn);
                *done = true;
        }, std::placeholders::_1, &done, callee, delayer, conn, currentId, method, args...);
        *conn = connect(delayer, &ProofObject::delayedCallRequested,
                        callee, f, blockEventLoop ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
        emit delayer->delayedCallRequested(currentId, QPrivateSignal{});
        while (!done)
            QCoreApplication::processEvents();
        return true;
    }

    template <class Callee, class Method, class ...Args>
    static bool delayedCall(Callee *callee, Method method, Args... args)
    {
        if (QThread::currentThread() == callee->thread())
            return false;
        ProofObject *delayer = delayerForObject(callee);
        qulonglong currentId = delayer->nextDelayedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        auto f = std::bind([](qulonglong delayedCallId,
                           Callee *callee, ProofObject *delayer,
                           QSharedPointer<QMetaObject::Connection> conn,
                           qulonglong currentId, Method method, Args... args) {
                if (currentId != delayedCallId)
                    return;
                (*callee.*method)(args...);
                cleanupDelayer(callee, delayer, conn);
        }, std::placeholders::_1, callee, delayer, conn, currentId, method, args...);
        *conn = connect(delayer, &ProofObject::delayedCallRequested,
                        callee, f);
        emit delayer->delayedCallRequested(currentId, QPrivateSignal{});
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

    template <class Callee>
    static typename std::enable_if<!std::is_base_of<ProofObject, Callee>::value
                                    && std::is_base_of<QObject, Callee>::value, ProofObject *>::type
    delayerForObject(Callee *callee)
    {
        Q_UNUSED(callee);
        return new ProofObject(0);
    }

    template <class Callee>
    static typename std::enable_if<std::is_base_of<ProofObject, Callee>::value, ProofObject *>::type
    delayerForObject(Callee *callee)
    {
        return callee;
    }

    template <class Callee>
    static typename std::enable_if<!std::is_base_of<ProofObject, Callee>::value
                                    && std::is_base_of<QObject, Callee>::value, bool>::type
    cleanupDelayer(Callee *callee, ProofObject *delayer, QSharedPointer<QMetaObject::Connection> conn)
    {
        Q_UNUSED(callee);
        Q_UNUSED(conn);
        delayer->deleteLater();
        return true;
    }

    template <class Callee>
    static typename std::enable_if<std::is_base_of<ProofObject, Callee>::value, bool>::type
    cleanupDelayer(Callee *callee, ProofObject *delayer, QSharedPointer<QMetaObject::Connection> conn)
    {
        Q_UNUSED(callee);
        Q_UNUSED(delayer);
        disconnect(*conn);
        return true;
    }
};

}

#endif // PROOFOBJECT_H
