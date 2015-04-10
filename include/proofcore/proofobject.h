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
    static typename std::enable_if<!std::is_base_of<ProofObject, Callee>::value
                                    && std::is_base_of<QObject, Callee>::value, bool>::type
    blockingDelayedCall(Callee *callee, Method method, bool blockEventLoop, Result &result, Args... args)
    {
        if (QThread::currentThread() == callee->thread())
            return false;

        ProofObject *temporaryObject = new ProofObject(0);
        std::atomic_bool done(false);
        auto f = std::bind([](qulonglong, Result *result, std::atomic_bool *done,
                           Callee *callee, ProofObject *temporaryObject,
                           Method method, Args... args) {
                result = (*callee.*method)(args...);
                temporaryObject->deleteLater();
                *done = true;
        }, std::placeholders::_1, &result, &done, callee, temporaryObject, method, args...);
        connect(temporaryObject, &ProofObject::delayedCallRequested,
                callee, f, blockEventLoop ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
        emit temporaryObject->delayedCallRequested(0, QPrivateSignal{});
        while (!done)
            QCoreApplication::processEvents();
        return true;
    }

    template <class Callee, class Method, class ...Args>
    static typename std::enable_if<!std::is_base_of<ProofObject, Callee>::value
                                    && std::is_base_of<QObject, Callee>::value, bool>::type
    blockingDelayedCall(Callee *callee, Method method, bool blockEventLoop, void *dummy, Args... args)
    {
        Q_UNUSED(dummy);
        if (QThread::currentThread() == callee->thread())
            return false;

        ProofObject *temporaryObject = new ProofObject(0);
        std::atomic_bool done(false);
        auto f = std::bind([](qulonglong, std::atomic_bool *done,
                           Callee *callee, ProofObject *temporaryObject,
                           Method method, Args... args) {
                (*callee.*method)(args...);
                temporaryObject->deleteLater();
                *done = true;
        }, std::placeholders::_1, &done, callee, temporaryObject, method, args...);
        connect(temporaryObject, &ProofObject::delayedCallRequested,
                callee, f, blockEventLoop ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
        emit temporaryObject->delayedCallRequested(0, QPrivateSignal{});
        while (!done)
            QCoreApplication::processEvents();
        return true;
    }

    template <class Callee, class Result, class Method, class ...Args>
    static typename std::enable_if<std::is_base_of<ProofObject, Callee>::value, bool>::type
    blockingDelayedCall(Callee *callee, Method method, bool blockEventLoop, Result &result, Args... args)
    {
        if (QThread::currentThread() == callee->thread())
            return false;
        qulonglong currentId = callee->nextDelayedCallId();
        std::atomic_bool done(false);
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        auto f = std::bind([](qulonglong delayedCallId, Result *result, std::atomic_bool *done,
                           Callee *callee, QSharedPointer<QMetaObject::Connection> conn,
                           qulonglong currentId, Method method, Args... args) {
                if (currentId != delayedCallId)
                    return;
                *result = (*callee.*method)(args...);
                disconnect(*conn);
                *done = true;
        }, std::placeholders::_1, &result, &done, callee, conn, currentId, method, args...);
        *conn = connect(callee, &ProofObject::delayedCallRequested,
                        callee, f, blockEventLoop ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
        emit callee->delayedCallRequested(currentId, QPrivateSignal{});
        while (!done)
            QCoreApplication::processEvents();
        return true;
    }

    template <class Callee, class Method, class ...Args>
    static typename std::enable_if<std::is_base_of<ProofObject, Callee>::value, bool>::type
    blockingDelayedCall(Callee *callee, Method method, bool blockEventLoop, void *dummy, Args... args)
    {
        Q_UNUSED(dummy);
        if (QThread::currentThread() == callee->thread())
            return false;
        qulonglong currentId = callee->nextDelayedCallId();
        std::atomic_bool done(false);
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        auto f = std::bind([](qulonglong delayedCallId, std::atomic_bool *done,
                           Callee *callee, QSharedPointer<QMetaObject::Connection> conn,
                           qulonglong currentId, Method method, Args... args) {
                if (currentId != delayedCallId)
                    return;
                (*callee.*method)(args...);
                disconnect(*conn);
                *done = true;
        }, std::placeholders::_1, &done, callee, conn, currentId, method, args...);
        *conn = connect(callee, &ProofObject::delayedCallRequested,
                        callee, f, blockEventLoop ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
        emit callee->delayedCallRequested(currentId, QPrivateSignal{});
        while (!done)
            QCoreApplication::processEvents();
        return true;
    }

    template <class Callee, class Method, class ...Args>
    static typename std::enable_if<!std::is_base_of<ProofObject, Callee>::value
                                    && std::is_base_of<QObject, Callee>::value, bool>::type
    delayedCall(Callee *callee, Method method, Args... args)
    {
        if (QThread::currentThread() == callee->thread())
            return false;

        ProofObject *temporaryObject = new ProofObject(0);
        auto f = std::bind([](qulonglong,
                           Callee *callee, ProofObject *temporaryObject,
                           Method method, Args... args) {
                (*callee.*method)(args...);
                temporaryObject->deleteLater();
        }, std::placeholders::_1, callee, temporaryObject, method, args...);
        connect(temporaryObject, &ProofObject::delayedCallRequested,
                callee, f);
        emit temporaryObject->delayedCallRequested(0, QPrivateSignal{});
        return true;
    }

    template <class Callee, class Method, class ...Args>
    static typename std::enable_if<std::is_base_of<ProofObject, Callee>::value, bool>::type
    delayedCall(Callee *callee, Method method, Args... args)
    {
        if (QThread::currentThread() == callee->thread())
            return false;
        qulonglong currentId = callee->nextDelayedCallId();
        auto conn = QSharedPointer<QMetaObject::Connection>::create();
        auto f = std::bind([](qulonglong delayedCallId,
                           Callee *callee, QSharedPointer<QMetaObject::Connection> conn,
                           qulonglong currentId, Method method, Args... args) {
                if (currentId != delayedCallId)
                    return;
                (*callee.*method)(args...);
                disconnect(*conn);
        }, std::placeholders::_1, callee, conn, currentId, method, args...);
        *conn = connect(callee, &ProofObject::delayedCallRequested,
                        callee, f);
        emit callee->delayedCallRequested(currentId, QPrivateSignal{});
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
