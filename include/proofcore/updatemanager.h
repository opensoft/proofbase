#ifndef PROOF_UPDATEMANAGER_H
#define PROOF_UPDATEMANAGER_H

#include "proofcore_global.h"

#include "proofcore/proofobject.h"

#include <QStringList>

namespace Proof {

class UpdateManagerPrivate;
class PROOF_CORE_EXPORT UpdateManager : public ProofObject
{
    Q_OBJECT
    Q_PROPERTY(bool supported READ supported CONSTANT)
    Q_PROPERTY(bool autoUpdateEnabled READ autoUpdateEnabled WRITE setAutoUpdateEnabled NOTIFY autoUpdateEnabledChanged)
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout NOTIFY timeoutChanged)
    Q_PROPERTY(QString aptSourcesListFilePath READ aptSourcesListFilePath WRITE setAptSourcesListFilePath NOTIFY
                   aptSourcesListFilePathChanged)
    Q_PROPERTY(QString currentVersion READ currentVersion WRITE setCurrentVersion NOTIFY currentVersionChanged)
    Q_PROPERTY(QString packageName READ packageName WRITE setPackageName NOTIFY packageNameChanged)
    Q_PROPERTY(QString newVersion READ newVersion NOTIFY newVersionChanged)
    Q_PROPERTY(bool newVersionInstallable READ newVersionInstallable NOTIFY newVersionInstallableChanged)
    Q_DECLARE_PRIVATE(UpdateManager)

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
