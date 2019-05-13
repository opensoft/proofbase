/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "proofnetwork/httpdownloader.h"

#include "proofcore/proofobject_p.h"

#include "proofnetwork/baserestapi.h"
#include "proofnetwork/restclient.h"

#include <QNetworkReply>

namespace Proof {

class HttpDownloaderPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(HttpDownloader)

    void copyData(QNetworkReply *reply, QIODevice *dest) const;

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

RestClientSP HttpDownloader::restClient() const
{
    Q_D_CONST(HttpDownloader);
    return d->restClient;
}

Future<QByteArray> HttpDownloader::download(const QUrl &url)
{
    Q_D(HttpDownloader);

    if (!url.isValid()) {
        qCDebug(proofNetworkMiscLog) << "Url is not valid" << url;
        return Future<QByteArray>::failed(
            Failure(QStringLiteral("Url is not valid"), NETWORK_MODULE_CODE, NetworkErrorCode::InvalidUrl));
    }

    Promise<QByteArray> promise;
    d->restClient->get(url)
        .onSuccess([this, promise](QNetworkReply *reply) {
            auto errorHandler = [](QNetworkReply *reply, const Promise<QByteArray> &promise) {
                int errorCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                QString errorMessage = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString().trimmed();
                if (errorMessage.isEmpty())
                    errorMessage = QStringLiteral("Download failed");
                else
                    errorMessage = QStringLiteral("Download failed: %1").arg(errorMessage);
                promise.failure(Failure(errorMessage, NETWORK_MODULE_CODE, NetworkErrorCode::ServerError,
                                        Failure::NoHint, errorCode));
            };

            if (reply->isFinished()) {
                if (!promise.isFilled()) {
                    if (reply->error() == QNetworkReply::NetworkError::NoError)
                        promise.success(reply->readAll());
                    else
                        errorHandler(reply, promise);
                }
                reply->deleteLater();
            } else {
                connect(reply, &QNetworkReply::finished, this, [promise, reply, errorHandler]() {
                    if (promise.isFilled())
                        return;
                    if (reply->error() == QNetworkReply::NetworkError::NoError)
                        promise.success(reply->readAll());
                    else
                        errorHandler(reply, promise);
                    reply->deleteLater();
                });
                connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                        this, [promise, reply, errorHandler](QNetworkReply::NetworkError) {
                            if (promise.isFilled())
                                return;
                            errorHandler(reply, promise);
                            reply->deleteLater();
                        });

                connect(reply, &QNetworkReply::sslErrors, this, [promise, reply](const QList<QSslError> &errors) {
                    for (const QSslError &error : errors) {
                        if (error.error() != QSslError::SslError::NoError) {
                            int errorCode = BaseRestApi::clientSslErrorOffset() + static_cast<int>(error.error());
                            qCWarning(proofNetworkMiscLog)
                                << "SSL error occurred"
                                << reply->request().url().toDisplayString(QUrl::FormattingOptions(QUrl::FullyDecoded))
                                << ": " << errorCode << error.errorString();
                            if (promise.isFilled())
                                continue;
                            promise.failure(Failure(error.errorString(), NETWORK_MODULE_CODE,
                                                    NetworkErrorCode::SslError, Failure::UserFriendlyHint, errorCode));
                        }
                    }
                    reply->deleteLater();
                });
            }
        })
        .onFailure([promise](const Failure &f) { promise.failure(f); });
    return promise.future();
}

Future<QIODevice *> HttpDownloader::downloadTo(const QUrl &url, QIODevice *dest)
{
    Q_D(HttpDownloader);

    if (!url.isValid()) {
        qCDebug(proofNetworkMiscLog) << "Url is not valid" << url;
        return Future<QIODevice *>::failed(
            Failure(QStringLiteral("Url is not valid"), NETWORK_MODULE_CODE, NetworkErrorCode::InvalidUrl));
    }

    Promise<QIODevice *> promise;
    d->restClient->get(url)
        .onSuccess([this, d, dest, promise](QNetworkReply *reply) {
            auto errorHandler = [](QNetworkReply *reply, const Promise<QIODevice *> &promise) {
                int errorCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                QString errorMessage = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString().trimmed();
                if (errorMessage.isEmpty())
                    errorMessage = QStringLiteral("Download failed");
                else
                    errorMessage = QStringLiteral("Download failed: %1").arg(errorMessage);
                promise.failure(Failure(errorMessage, NETWORK_MODULE_CODE, NetworkErrorCode::ServerError,
                                        Failure::NoHint, errorCode));
            };

            if (reply->isFinished()) {
                if (!promise.isFilled()) {
                    if (reply->error() == QNetworkReply::NetworkError::NoError) {
                        d->copyData(reply, dest);
                        promise.success(dest);
                    } else {
                        errorHandler(reply, promise);
                    }
                }
                reply->deleteLater();
            } else {
                connect(reply, &QNetworkReply::downloadProgress, this,
                        [d, dest, promise, reply](qint64, qint64) {
                            if (promise.isFilled())
                                return;
                            d->copyData(reply, dest);
                        },
                        Qt::DirectConnection);
                connect(reply, &QNetworkReply::finished, this, [d, dest, promise, reply, errorHandler]() {
                    if (promise.isFilled())
                        return;
                    if (reply->error() == QNetworkReply::NetworkError::NoError) {
                        d->copyData(reply, dest);
                        promise.success(dest);
                    } else {
                        errorHandler(reply, promise);
                    }
                    reply->deleteLater();
                });
                connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                        this, [promise, reply, errorHandler](QNetworkReply::NetworkError) {
                            if (promise.isFilled())
                                return;
                            errorHandler(reply, promise);
                            reply->deleteLater();
                        });

                connect(reply, &QNetworkReply::sslErrors, this, [promise, reply](const QList<QSslError> &errors) {
                    for (const QSslError &error : errors) {
                        if (error.error() != QSslError::SslError::NoError) {
                            int errorCode = BaseRestApi::clientSslErrorOffset() + static_cast<int>(error.error());
                            qCWarning(proofNetworkMiscLog)
                                << "SSL error occurred"
                                << reply->request().url().toDisplayString(QUrl::FormattingOptions(QUrl::FullyDecoded))
                                << ": " << errorCode << error.errorString();
                            if (promise.isFilled())
                                continue;
                            promise.failure(Failure(error.errorString(), NETWORK_MODULE_CODE,
                                                    NetworkErrorCode::SslError, Failure::UserFriendlyHint, errorCode));
                        }
                    }
                    reply->deleteLater();
                });
            }
        })
        .onFailure([promise](const Failure &f) { promise.failure(f); });
    return promise.future();
}

void HttpDownloaderPrivate::copyData(QNetworkReply *reply, QIODevice *dest) const
{
    const unsigned LIMIT = 1u << 20u;
    if (!reply->bytesAvailable())
        return;
    if (reply->bytesAvailable() < LIMIT) {
        dest->write(reply->readAll());
        return;
    }
    char buffer[LIMIT];
    memset(buffer, 0, LIMIT);
    while (reply->bytesAvailable()) {
        int64_t read = reply->read(buffer, LIMIT);
        dest->write(buffer, read);
    }
}
