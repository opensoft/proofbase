#ifndef ABSTRACTBARCODERCONFIGURATOR_H
#define ABSTRACTBARCODERCONFIGURATOR_H

#include "proofcore/proofcore_global.h"

#include <QJsonObject>
#include <QObject>

namespace Proof {
//TODO: Make it public if needed
class PROOF_CORE_EXPORT AbstractBarcodeConfigurator : public QObject
{
    Q_OBJECT
public:
    explicit AbstractBarcodeConfigurator(QObject *parent = nullptr) : QObject(parent) {}

    virtual QString name() const = 0;
    virtual void handleCommand(const QJsonObject &command) = 0;
};
}

#endif // ABSTRACTBARCODERCONFIGURATOR_H
