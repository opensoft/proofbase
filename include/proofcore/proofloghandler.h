#ifndef PROOFLOGHANDLER_H
#define PROOFLOGHANDLER_H

#include "proofcore/proofcore_global.h"

#include <QtGlobal>
#include <QString>

class PROOF_CORE_EXPORT ProofLogHandler
{
public:
    static ProofLogHandler *instance();

    void install(const QString &logFileBaseName = QString());
    void uninstall();

private:
    ProofLogHandler();
    ~ProofLogHandler();
    Q_DISABLE_COPY(ProofLogHandler)


private:
    static QString m_logFileBaseName;
    static QtMessageHandler m_coreHandler;
};

#endif // PROOFLOGHANDLER_H
