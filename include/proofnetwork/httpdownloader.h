#ifndef PROOF_HTTPDOWNLOADER_H
#define PROOF_HTTPDOWNLOADER_H

#include "proofnetwork/abstractrestapi.h"
#include "proofnetwork/proofnetwork_global.h"

namespace Proof {

//This version downloads whole response before completing, don't use for big files
//TODO: add streaming version if needed
class HttpDownloaderPrivate;
class PROOF_NETWORK_EXPORT HttpDownloader : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(HttpDownloader)
public:
    explicit HttpDownloader(QObject *parent = nullptr);
    FutureSP<QByteArray> download(const QUrl &url);
};
}
#endif // PROOF_HTTPDOWNLOADER_H
