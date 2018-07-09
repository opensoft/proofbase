#ifndef PROOF_HTTPDOWNLOADER_H
#define PROOF_HTTPDOWNLOADER_H

#include "proofcore/proofobject.h"

#include "proofnetwork/proofnetwork_global.h"
#include "proofnetwork/proofnetwork_types.h"

#include <QUrl>

namespace Proof {

//This version downloads whole response before completing, don't use for big files
//TODO: add streaming version if needed
class HttpDownloaderPrivate;
class PROOF_NETWORK_EXPORT HttpDownloader : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(HttpDownloader)
public:
    explicit HttpDownloader(const Proof::RestClientSP &restClient, QObject *parent = nullptr);
    explicit HttpDownloader(QObject *parent = nullptr);
    FutureSP<QByteArray> download(const QUrl &url);
};
} // namespace Proof
#endif // PROOF_HTTPDOWNLOADER_H
