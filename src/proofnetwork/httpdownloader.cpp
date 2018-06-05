#include "httpdownloader.h"

#include "proofcore/proofobject_p.h"

#include <QNetworkReply>

namespace Proof {

class HttpDownloaderPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(HttpDownloader)

    Proof::RestClientSP restClient;
};

} // namespace Proof

using namespace Proof;

HttpDownloader::HttpDownloader(const Proof::RestClientSP &restClient, QObject *parent)
    : ProofObject(*new HttpDownloaderPrivate, parent)
{
    Q_D(HttpDownloader);
    d->restClient = restClient;
}

HttpDownloader::HttpDownloader(QObject *parent) : HttpDownloader(Proof::RestClientSP::create(), parent)
{}

FutureSP<QByteArray> HttpDownloader::download(const QUrl &url)
{
    Q_D(HttpDownloader);

    if (!url.isValid()) {
        qCDebug(proofNetworkMiscLog) << "Url is not valid" << url;
        return Future<QByteArray>::fail(
            Failure(QStringLiteral("Url is not valid"), NETWORK_MODULE_CODE, NetworkErrorCode::InvalidUrl));
    }

    PromiseSP<QByteArray> promise = PromiseSP<QByteArray>::create();
    d->restClient->get(url)->onSuccess([this, promise](QNetworkReply *reply) {
        auto errorHandler = [](QNetworkReply *reply, const PromiseSP<QByteArray> &promise) {
            int errorCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            // TODO: See in BaseRestApi if you need more detailed error message
            QString errorMessage = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString().trimmed();
            if (errorMessage.isEmpty())
                errorMessage = QStringLiteral("Download failed");
            else
                errorMessage = QStringLiteral("Download failed: ") + errorMessage;
            promise->failure(
                Failure(errorMessage, NETWORK_MODULE_CODE, NetworkErrorCode::ServerError, Failure::NoHint, errorCode));
        };

        connect(reply, &QNetworkReply::finished, this, [promise, reply, errorHandler]() {
            if (promise->filled())
                return;
            if (reply->error() == QNetworkReply::NetworkError::NoError)
                promise->success(reply->readAll());
            else
                errorHandler(reply, promise);
            reply->deleteLater();
        });
        connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this,
                [promise, reply, errorHandler](QNetworkReply::NetworkError) {
                    if (promise->filled())
                        return;
                    errorHandler(reply, promise);
                    reply->deleteLater();
                });
    });
    return promise->future();
}
