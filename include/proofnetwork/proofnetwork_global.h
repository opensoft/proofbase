#ifndef PROOFNETWORK_GLOBAL_H
#define PROOFNETWORK_GLOBAL_H

#ifdef PROOF_NETWORK_LIB
#  define PROOF_NETWORK_EXPORT Q_DECL_EXPORT
#else
#  define PROOF_NETWORK_EXPORT Q_DECL_IMPORT
#endif

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(proofNetworkLog)
Q_DECLARE_LOGGING_CATEGORY(proofNetworkMiscLog)
Q_DECLARE_LOGGING_CATEGORY(proofNetworkAmqpLog)

namespace Proof {
namespace NetworkErrorCode {
enum Code {
    ServerError = 1,
    InaccessibleService = 2,
    SslError = 3,
    InvalidReply = 4,
    InvalidRequest = 5,
    InvalidUrl = 6,
    InternalError = 7,
    MinCustomError = 100
};
}
constexpr long NETWORK_MODULE_CODE = 300;
}
#endif // PROOFNETWORK_GLOBAL_H
