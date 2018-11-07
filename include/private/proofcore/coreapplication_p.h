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
#ifndef COREAPPLICATION_P_H
#define COREAPPLICATION_P_H

#include "proofcore/proofcore_global.h"
#include "proofcore/proofobject_p.h"

#include <QDateTime>
#include <QSet>
#include <QTranslator>
#include <QtCrypto>
#include <QtGlobal>

namespace Proof {
class Settings;
class UpdateManager;
class CoreApplication;
class PROOF_CORE_EXPORT CoreApplicationPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(CoreApplication)
protected:
    virtual void postInit();

    void initCrashHandler();
    void updatePrettifiedName();
    bool daemonizeIfNeeded();
    void initLogs(bool daemonized);
    void execMigrations();
    void initQca();
    void initTranslator();
    void initUpdateManager();

    void setLanguage(const QString &currentLanguage);

    QString prettifiedApplicationName;
    QDateTime startedAt = QDateTime::currentDateTimeUtc();
    Settings *settings = nullptr;
    UpdateManager *updateManager = nullptr;
    QSet<QString> translationPrefixes;
    QStringList availableLanguages;
    QVariantMap fullLanguageNames;
    QString currentLanguage = QStringLiteral("en");
    QVector<QTranslator *> installedTranslators;
    bool initialized = false;
    QScopedPointer<QCA::Initializer> qcaInit;
};
} // namespace Proof

#endif // COREAPPLICATION_P_H
