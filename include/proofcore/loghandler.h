#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include "proofcore/proofcore_global.h"

#include <QtGlobal>
#include <QString>

namespace Proof {

class PROOF_CORE_EXPORT LogHandler
{
public:
    static LogHandler *instance();

    void install(const QString &logFileBaseName = QString());
    void uninstall();

private:
    LogHandler();
    ~LogHandler();
    LogHandler(const LogHandler &other) = delete;
    LogHandler &operator=(const LogHandler &other) = delete;
    LogHandler(const LogHandler &&other) = delete;
    LogHandler &operator=(const LogHandler &&other) = delete;

private:
    static QString m_logFileBaseName;
    static QtMessageHandler m_coreHandler;
};
}

#endif // LOGHANDLER_H
