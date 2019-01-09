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
#ifndef PROOF_UPDATEMANAGER_H
#define PROOF_UPDATEMANAGER_H

#include "proofcore_global.h"

#include "proofcore/proofobject.h"

namespace Proof {

class UpdateManagerPrivate;
//TODO: Deprecated, remove apt-related stuff completely due to no real application in production for it in last two years
class PROOF_CORE_EXPORT UpdateManager : public ProofObject
{
    Q_OBJECT
    // clang-format off
    Q_PROPERTY(bool supported READ supported CONSTANT)
    Q_PROPERTY(bool autoUpdateEnabled READ autoUpdateEnabled WRITE setAutoUpdateEnabled NOTIFY autoUpdateEnabledChanged)
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout NOTIFY timeoutChanged)
    Q_PROPERTY(QString aptSourcesListFilePath READ aptSourcesListFilePath WRITE setAptSourcesListFilePath NOTIFY aptSourcesListFilePathChanged)
    Q_PROPERTY(QString currentVersion READ currentVersion WRITE setCurrentVersion NOTIFY currentVersionChanged)
    Q_PROPERTY(QString packageName READ packageName WRITE setPackageName NOTIFY packageNameChanged)
    Q_PROPERTY(QString newVersion READ newVersion NOTIFY newVersionChanged)
    Q_PROPERTY(bool newVersionInstallable READ newVersionInstallable NOTIFY newVersionInstallableChanged)
    Q_DECLARE_PRIVATE(UpdateManager)
    // clang-format on

public:
    explicit UpdateManager(QObject *parent = nullptr);
    ~UpdateManager();

    Q_INVOKABLE void update(const QString &password);
    Q_INVOKABLE void installVersion(const QString &version, const QString &password);
    Q_INVOKABLE void checkPassword(const QString &password);

    Q_INVOKABLE void start();

    bool supported() const;
    bool autoUpdateEnabled() const;
    int timeout() const;
    QString aptSourcesListFilePath() const;
    QString currentVersion() const;
    QString packageName() const;
    QString newVersion() const;
    bool newVersionInstallable() const;

    void setAutoUpdateEnabled(bool arg);
    void setTimeout(int arg);
    void setAptSourcesListFilePath(const QString &arg);
    void setCurrentVersion(const QString &arg);
    void setPackageName(const QString &arg);

signals:
    void autoUpdateEnabledChanged(bool autoUpdateEnabled);
    void timeoutChanged(int timeout);
    void currentVersionChanged(const QString &currentVersion);
    void packageNameChanged(const QString &packageName);
    void newVersionChanged(const QString &newVersion);
    void newVersionInstallableChanged(bool newVersionInstallable);
    void updateSucceeded();
    void updateFailed();
    void installationSucceeded();
    void installationFailed();
    void passwordChecked(bool isCorrect);
    void aptSourcesListFilePathChanged(const QString &aptSourcesListFilePath);
};

} // namespace Proof

#endif // PROOF_UPDATEMANAGER_H
