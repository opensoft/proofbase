#include "proofcore_global.h"
#include "logs.h"

Q_LOGGING_CATEGORY(proofLog, "proof")
Q_LOGGING_CATEGORY(proofCoreLog, "proof.core")
Q_LOGGING_CATEGORY(proofCoreSettingsLog, "proof.core.settings")
Q_LOGGING_CATEGORY(proofCoreTaskChainLog, "proof.core.taskchain")
Q_LOGGING_CATEGORY(proofCoreCacheLog, "proof.core.cache")

__attribute__((constructor))
static void libraryInit()
{
}
