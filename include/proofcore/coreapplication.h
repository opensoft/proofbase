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
#ifndef PROOF_COREAPPLICATION_H
#define PROOF_COREAPPLICATION_H

#include "proofcore/proofcore_global.h"
#include "proofcore/proofobject.h"

#include <QDateTime>

#define proofApp Proof::CoreApplication::instance()

class QCoreApplication;

namespace Proof {
class UpdateManager;
class Settings;
class CoreApplicationPrivate;

class PROOF_CORE_EXPORT CoreApplication : public ProofObject
{
    Q_OBJECT
    Q_PROPERTY(QString prettifiedApplicationName READ prettifiedApplicationName CONSTANT)
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages CONSTANT)
    Q_PROPERTY(QVariantMap fullLanguageNames READ fullLanguageNames CONSTANT)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(int languageIndex READ languageIndex NOTIFY languageChanged)
    Q_PROPERTY(Proof::UpdateManager *updateManager READ updateManager CONSTANT)
    Q_PROPERTY(QDateTime startedAt READ startedAt CONSTANT)
    Q_DECLARE_PRIVATE(CoreApplication)
public:
    CoreApplication(int &argc, char **argv, const QString &orgName = QString(), const QString &appName = QString(),
                    const QString &version = QStringLiteral("0.0.0.0"),
                    const QStringList &defaultLoggingRules = QStringList());

    CoreApplication(QCoreApplication *app, const QString &orgName = QString(), const QString &appName = QString(),
                    const QString &version = QStringLiteral("0.0.0.0"),
                    const QStringList &defaultLoggingRules = QStringList());

    ~CoreApplication();

    QString prettifiedApplicationName() const;
    QStringList availableLanguages() const;
    QVariantMap fullLanguageNames() const;
    QString language() const;
    void setLanguage(const QString &language);
    int languageIndex() const;
    UpdateManager *updateManager() const;
    QDateTime startedAt() const;

    Settings *settings() const;

    Q_INVOKABLE void postInit();
    Q_INVOKABLE int exec();

    static CoreApplication *instance();

    static bool exists();

    static void addInitializer(const std::function<void()> &initializer);
    //Migration can't use qApp/proofApp since it is not guaranteed that they will exist or be properly configured during migration exec
    //All migrations should be added before app ctor is called
    //Max related version here means version when something is changed. I.e. it is not last non-changed version, it is first changed one
    //Migration may not be called at all if config version is greater or equal than both related app and related proof versions
    //Migration should minimize exec time in case when it was called but version is greater or equal than related
    //arguments: oldAppVersion, oldProofVersion, settings pointer
    //Versions are packed into quint64 using proofcore/helpers/versionhelper.h
    using Migration = std::function<void(quint64, quint64, Settings *)>;
    static void addMigration(quint64 maxRelatedVersion, Migration &&migration);
    static void addMigration(const QString &maxRelatedVersion, Migration &&migration);
    static void addMigrations(const QMap<quint64, QVector<Migration>> &migrations);
    static void addMigrations(const QMap<QString, QVector<Migration>> &migrations);

signals:
    void languageChanged(const QString &language);

protected:
    CoreApplication(CoreApplicationPrivate &dd, QCoreApplication *app, const QString &orgName = QString(),
                    const QString &appName = QString(), const QString &version = QStringLiteral("0.0.0.0"),
                    const QStringList &defaultLoggingRules = QStringList());
};

} // namespace Proof

#endif // PROOF_COREAPPLICATION_H
