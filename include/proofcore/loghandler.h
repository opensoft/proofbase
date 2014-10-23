#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include "proofcore/proofcore_global.h"

#include <QtGlobal>
#include <QString>
#include <QMutex>

namespace Proof {

class PROOF_CORE_EXPORT LogHandler
{
public:
    static LogHandler *instance();
    static void setup();

    void install(const QString &fileName = QString());
    void uninstall();

private:
    LogHandler();
    ~LogHandler();
    LogHandler(const LogHandler &other) = delete;
    LogHandler &operator=(const LogHandler &other) = delete;
    LogHandler(const LogHandler &&other) = delete;
    LogHandler &operator=(const LogHandler &&other) = delete;
};
}

#endif // LOGHANDLER_H
