#include "httpdownloader.h"

#include "proofnetwork/abstractrestapi_p.h"

#include <QNetworkReply>

namespace Proof {

class HttpDownloaderPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(HttpDownloader)

    Proof::RestClientSP restClient = Proof::RestClientSP::create();
};

}

using namespace Proof;

HttpDownloader::HttpDownloader(QObject *parent)
    : ProofObject(*new HttpDownloaderPrivate, parent)
{

}

FutureSP<QByteArray> HttpDownloader::download(const QUrl &url)
{
    Q_D(HttpDownloader);

    if (!url.isValid()) {
        qCDebug(proofNetworkMiscLog) << "Url is not valid" << url;
        return Future<QByteArray>::fail(Failure(QStringLiteral("Url is not valid"), NETWORK_MODULE_CODE, NetworkErrorCode::InvalidUrl));
    }

    FutureSP<QByteArray> future;
    if (!call(this, &HttpDownloader::download, Proof::Call::BlockEvents, future, url)) {
        PromiseSP<QByteArray> promise = PromiseSP<QByteArray>::create();
        future = promise->future();
        QNetworkReply *reply = d->restClient->get(url);

        auto errorHandler = [](QNetworkReply *reply, const PromiseSP<QByteArray> &promise) {
            int errorCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            // TODO: See in AbstractRestApi if you need more detailed error message
            QString errorMessage = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString().trimmed();
            if (errorMessage.isEmpty())
                errorMessage = QStringLiteral("Download failed");
            else
                errorMessage = QStringLiteral("Download failed: ") + errorMessage;
            promise->failure(Failure(errorMessage, NETWORK_MODULE_CODE, NetworkErrorCode::ServerError, Failure::NoHint, errorCode));
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
        connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                this, [promise, reply, errorHandler](QNetworkReply::NetworkError) {
            if (promise->filled())
                return;
            errorHandler(reply, promise);
            reply->deleteLater();
        });
    }
    return future;
}

bool HttpDownloader::event(QEvent *event)
{
    Q_D(HttpDownloader);
    if (event->type() == QEvent::ThreadChange)
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    else if (event->type() == QEvent::User && d->restClient)
        call(d->restClient.data(), &QObject::moveToThread, Proof::Call::BlockEvents, thread());

    return QObject::event(event);
}
