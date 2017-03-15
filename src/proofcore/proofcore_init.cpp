#include "proofcore_global.h"
#include "logs.h"
#include "proofglobal.h"

Q_LOGGING_CATEGORY(proofCoreSettingsLog, "proof.core.settings")
Q_LOGGING_CATEGORY(proofCoreCrashLog, "proof.core.crash")
Q_LOGGING_CATEGORY(proofCoreLoggerLog, "proof.core.logger")
Q_LOGGING_CATEGORY(proofCoreUpdatesLog, "proof.core.updates")
Q_LOGGING_CATEGORY(proofCoreMiscLog, "proof.core.misc")
Q_LOGGING_CATEGORY(proofCoreTaskChainExtraLog, "proof.core.taskchain.extra")
Q_LOGGING_CATEGORY(proofCoreTaskChainStatsLog, "proof.core.taskchain.stats")
Q_LOGGING_CATEGORY(proofCoreCacheLog, "proof.core.cache")

PROOF_LIBRARY_INITIALIZER(libraryInit)
{
}
