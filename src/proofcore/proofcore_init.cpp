#include "proofcore_global.h"
#include "logs.h"

Q_LOGGING_CATEGORY(proofCoreSettingsLog, "proof.core.settings")
Q_LOGGING_CATEGORY(proofCoreLoggerLog, "proof.core.logger")
Q_LOGGING_CATEGORY(proofCoreTaskChainExtraLog, "proof.core.taskchain.extra")
Q_LOGGING_CATEGORY(proofCoreTaskChainStatsLog, "proof.core.taskchain.stats")
Q_LOGGING_CATEGORY(proofCoreCacheLog, "proof.core.cache")

__attribute__((constructor))
static void libraryInit()
{
}
