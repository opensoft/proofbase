#include "updatemanager.h"
#include "proofcore/proofobject_p.h"

#include <QProcess>
#include <QTimer>
#include <QSet>
#include <QRegExp>

namespace {
class WorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit WorkerThread(Proof::UpdateManagerPrivate *updater);

    template<class Method, class... Args>
    void callUpdater(Method method, Args... args);

    template<class Method, class... Args>
    auto callUpdaterWithResult(Method method, Args &&... args)
        -> decltype((std::declval<Proof::UpdateManagerPrivate&>().*method)(std::forward<Args>(args)...));

private:
    template<class Result, class Method, class... Args>
    typename std::enable_if<!std::is_same<Result, void>::value, Result>::type
    doTheCall(Method method, Args &&... args);

    template<class Result, class Method, class... Args>
    typename std::enable_if<std::is_same<Result, void>::value>::type
    doTheCall(Method method, Args &&... args);

private:
    Proof::UpdateManagerPrivate *updater;
};
}

namespace Proof {
class UpdateManagerPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(UpdateManager)

    void start();

    void checkPassword(const QString &password);
    void checkForUpdates();
    void installVersion(QString version, const QString &password);

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
    void setNewVersion(const QString &arg);
    void setNewVersionInstallable(bool arg);

    void updateTimerState();

    static quint64 versionFromString(const QStringList &version);
    static quint64 versionFromString(const QString &version);
    static QString versionToString(quint64 version);

    QString aptSourcesListFilePathValue;
    QString packageNameValue;
    quint64 currentVersionValue = 0x0;
    int currentVersionMajor = 0;
    bool autoUpdateEnabledValue = true;
    QString newVersionValue;
    bool newVersionInstallableValue = false;
    WorkerThread *thread = nullptr;
    QTimer *timer = nullptr;

    bool started = false;
};

} // namespace Proof

using namespace Proof;

UpdateManager::UpdateManager(QObject *parent)
    : ProofObject(*new UpdateManagerPrivate, parent)
{
    Q_D(UpdateManager);
    d->thread = new WorkerThread(d);
    d->timer = new QTimer();
    d->timer->moveToThread(d->thread);
    d->timer->setTimerType(Qt::VeryCoarseTimer);
    d->timer->setInterval(30 * 60 * 1000); // 30 minutes
    connect(d->timer, &QTimer::timeout, d->timer, [d] {d->checkForUpdates();});
    d->thread->start();
}

UpdateManager::~UpdateManager()
{
    Q_D(UpdateManager);
    d->thread->quit();
    d->thread->wait(1000);
    delete d->thread;
    delete d->timer;
}

void UpdateManager::update(const QString &password)
{
    Q_D(UpdateManager);
    d->thread->callUpdater(&UpdateManagerPrivate::installVersion, QString(), password);
}

void UpdateManager::installVersion(const QString &version, const QString &password)
{
    Q_D(UpdateManager);
    d->thread->callUpdater(&UpdateManagerPrivate::installVersion, version, password);
}

void UpdateManager::checkPassword(const QString &password)
{
    Q_D(UpdateManager);
    d->thread->callUpdater(&UpdateManagerPrivate::checkPassword, password);
}

void UpdateManager::start()
{
    Q_D(UpdateManager);
    d->thread->callUpdater(&UpdateManagerPrivate::start);
}

bool UpdateManager::supported() const
{
#ifdef Q_OS_LINUX
    return true;
#else
    return false;
#endif
}

bool UpdateManager::autoUpdateEnabled() const
{
    Q_D(const UpdateManager);
    return d->thread->callUpdaterWithResult(&UpdateManagerPrivate::autoUpdateEnabled);
}

int UpdateManager::timeout() const
{
    Q_D(const UpdateManager);
    return d->thread->callUpdaterWithResult(&UpdateManagerPrivate::timeout);
}

QString UpdateManager::aptSourcesListFilePath() const
{
    Q_D(const UpdateManager);
    return d->thread->callUpdaterWithResult(&UpdateManagerPrivate::aptSourcesListFilePath);
}

QString UpdateManager::currentVersion() const
{
    Q_D(const UpdateManager);
    return d->thread->callUpdaterWithResult(&UpdateManagerPrivate::currentVersion);
}

QString UpdateManager::packageName() const
{
    Q_D(const UpdateManager);
    return d->thread->callUpdaterWithResult(&UpdateManagerPrivate::packageName);
}

QString UpdateManager::newVersion() const
{
    Q_D(const UpdateManager);
    return d->thread->callUpdaterWithResult(&UpdateManagerPrivate::newVersion);
}

bool UpdateManager::newVersionInstallable() const
{
    Q_D(const UpdateManager);
    return d->thread->callUpdaterWithResult(&UpdateManagerPrivate::newVersionInstallable);
}

void UpdateManager::setAutoUpdateEnabled(bool arg)
{
    Q_D(UpdateManager);
    d->thread->callUpdaterWithResult(&UpdateManagerPrivate::setAutoUpdateEnabled, arg);
}

void UpdateManager::setTimeout(int arg)
{
    Q_D(UpdateManager);
    d->thread->callUpdaterWithResult(&UpdateManagerPrivate::setTimeout, arg);
}

void UpdateManager::setAptSourcesListFilePath(const QString &arg)
{
    Q_D(UpdateManager);
    d->thread->callUpdaterWithResult(&UpdateManagerPrivate::setAptSourcesListFilePath, arg);
}

void UpdateManager::setCurrentVersion(const QString &arg)
{
    Q_D(UpdateManager);
    d->thread->callUpdaterWithResult(&UpdateManagerPrivate::setCurrentVersion, arg);
}

void UpdateManager::setPackageName(const QString &arg)
{
    Q_D(UpdateManager);
    d->thread->callUpdaterWithResult(&UpdateManagerPrivate::setPackageName, arg);
}

void UpdateManagerPrivate::start()
{
    if (started)
        return;
    started = true;
    updateTimerState();
}

void UpdateManagerPrivate::checkPassword(const QString &password)
{
    Q_Q(UpdateManager);
#ifdef Q_OS_LINUX
    QScopedPointer<QProcess> checker(new QProcess);
    checker->setProcessChannelMode(QProcess::MergedChannels);
    checker->start(QString("sudo -S -k pwd"));
    if (checker->error() == QProcess::UnknownError) {
        if (!checker->waitForReadyRead()) {
            qCDebug(proofCoreUpdatesLog) << "No answer from command. Returning";
            emit q->passwordChecked(false);
            return;
        }
        QByteArray readBuffer;
        QByteArray currentRead;

        currentRead = checker->readAll();
        readBuffer.append(currentRead);
        currentRead = currentRead.trimmed();
        if (currentRead.contains("[sudo]") || currentRead.contains("password for")) {
            checker->write(QString("%1\n").arg(password).toLatin1());
            if (!checker->waitForReadyRead()) {
                qCDebug(proofCoreUpdatesLog) << "No answer from command. Returning";
                emit q->passwordChecked(false);
                return;
            }

            currentRead = checker->readAll();
            readBuffer.append(currentRead);
            currentRead = currentRead.trimmed();

            if (currentRead.contains("is not in the sudoers")) {
                qCDebug(proofCoreUpdatesLog) << "User not in sudoers list; log:\n" << readBuffer;
                emit q->passwordChecked(false);
                return;
            }
            if (currentRead.contains("Sorry, try again")) {
                qCDebug(proofCoreUpdatesLog) << "Sudo rejected the password; log:\n" << readBuffer;
                emit q->passwordChecked(false);
                return;
            }
        }
        checker->waitForFinished(-1);
        qCDebug(proofCoreUpdatesLog) << "Exitcode =" << checker->exitCode();
        emit q->passwordChecked(checker->exitCode() == 0);
    } else {
        qCDebug(proofCoreUpdatesLog) << "Process couldn't be started" << checker->error() << checker->errorString();
    }
#else
    Q_UNUSED(password);
    qCDebug(proofCoreUpdatesLog) << "Password check is not supported for this platform";
    emit q->passwordChecked(false);
#endif
}

void UpdateManagerPrivate::checkForUpdates()
{
#ifdef Q_OS_LINUX
    QScopedPointer<QProcess> updater(new QProcess);
    updater->setProcessChannelMode(QProcess::MergedChannels);
    if (aptSourcesListFilePathValue.isEmpty())
        updater->start("sudo apt-get update");
    else
        updater->start(QString("sudo apt-get update -o Dir::Etc::sourcelist=\"%1\"").arg(aptSourcesListFilePathValue));
    updater->waitForStarted();
    if (updater->error() == QProcess::UnknownError) {
        if (updater->waitForReadyRead()) {
            QByteArray data = updater->readAll().trimmed();
            if (data.contains("[sudo]") || data.contains("password for"))
                updater->kill();
        }
        updater->waitForFinished(-1);
        qCDebug(proofCoreUpdatesLog) << "apt-get update process finished with code =" << updater->exitCode();
    } else {
        qCDebug(proofCoreUpdatesLog) << "apt-get update process couldn't be started" << updater->error() << updater->errorString();
    }

    QScopedPointer<QProcess> checker(new QProcess);
    checker->start(QString("apt-cache --no-all-versions show %1").arg(packageNameValue));
    checker->waitForStarted();
    if (checker->error() == QProcess::UnknownError) {
        checker->waitForFinished();
        QList<QByteArray> lines = checker->readAll().trimmed().split('\n');
        QString version;
        for (const QByteArray &line : lines) {
            if (line.startsWith("Version: "))
                version = line;
            version.remove("Version: ");
        }
        QStringList splittedVersion = version.split(".");
        if (splittedVersion.count() < 4) {
            qCDebug(proofCoreUpdatesLog) << "Strange version found" << version << ". Returning.";
            return;
        }
        int foundVersionMajor = splittedVersion[0].toInt();
        quint64 foundVersion = versionFromString(splittedVersion);
        qCDebug(proofCoreUpdatesLog) << "Version found:" << QString("0x%1").arg(foundVersion, 16, 16, QLatin1Char('0'))
                                     << "; Current version is:" << QString("0x%1").arg(currentVersionValue, 16, 16, QLatin1Char('0'));
        if (foundVersion > currentVersionValue) {
            if (foundVersionMajor > currentVersionMajor)
                qCDebug(proofCoreUpdatesLog) << "Manual update needed because of different major version";
            else
                qCDebug(proofCoreUpdatesLog) << "Update from app is possible";
            setNewVersionInstallable(foundVersionMajor <= currentVersionMajor);
            setNewVersion(versionToString(foundVersion));
            if (autoUpdateEnabledValue)
                installVersion("", "");
        }
    } else {
        qCDebug(proofCoreUpdatesLog) << "process couldn't be started" << checker->error() << checker->errorString();
    }
#endif
}

void UpdateManagerPrivate::installVersion(QString version, const QString &password)
{
    Q_Q(UpdateManager);
#ifdef Q_OS_LINUX
    QScopedPointer<QProcess> updater(new QProcess);
    updater->setProcessChannelMode(QProcess::MergedChannels);
    bool isUpdate = version.isEmpty();
    QString package = isUpdate ? packageNameValue : QString("%1=%2").arg(packageNameValue, version);
    auto successSignal = isUpdate ? &UpdateManager::updateSucceeded : &UpdateManager::installationSucceeded;
    auto failSignal = isUpdate ? &UpdateManager::updateFailed : &UpdateManager::installationFailed;
    updater->start(QString("sudo -S -k apt-get --quiet --assume-yes --force-yes --allow-unauthenticated install %1").arg(package));
    updater->waitForStarted();
    if (updater->error() == QProcess::UnknownError) {
        if (!updater->waitForReadyRead()) {
            qCDebug(proofCoreUpdatesLog) << "No answer from apt-get. Returning";
            emit (q->*failSignal)();
            return;
        }
        QByteArray readBuffer;
        QByteArray currentRead;

        currentRead = updater->readAll();
        readBuffer.append(currentRead);
        currentRead = currentRead.trimmed();
        if (currentRead.contains("[sudo]") || currentRead.contains("password for")) {
            updater->write(QString("%1\n").arg(password).toLatin1());
            if (!updater->waitForReadyRead()) {
                qCDebug(proofCoreUpdatesLog) << "No answer from apt-get. Returning";
                emit (q->*failSignal)();
                return;
            }

            currentRead = updater->readAll();
            readBuffer.append(currentRead);
            currentRead = currentRead.trimmed();

            if (currentRead.contains("is not in the sudoers")) {
                qCDebug(proofCoreUpdatesLog) << "User not in sudoers list; log:\n" << readBuffer;
                emit (q->*failSignal)();
                return;
            }
            if (currentRead.contains("Sorry, try again")) {
                qCDebug(proofCoreUpdatesLog) << "Sudo rejected the password; log:\n" << readBuffer;
                emit (q->*failSignal)();
                return;
            }
        }
        updater->waitForFinished(-1);
        qCDebug(proofCoreUpdatesLog) << "Updated with exitcode =" << updater->exitCode() << "; log:\n" << readBuffer + updater->readAll().trimmed();
        if (updater->exitCode()) {
            emit (q->*failSignal)();
        } else {
            emit (q->*successSignal)();
            if (version.isEmpty())
                version = newVersionValue;
            if (versionFromString(version) >= versionFromString(newVersionValue))
                setNewVersion("");
            setCurrentVersion(version);
        }
    } else {
        qCDebug(proofCoreUpdatesLog) << "process couldn't be started" << updater->error() << updater->errorString();
    }
#else
    Q_UNUSED(version);
    Q_UNUSED(password);
    qCDebug(proofCoreUpdatesLog) << "Update is not supported for this platform";
    emit q->updateFailed();
#endif
}

bool UpdateManagerPrivate::autoUpdateEnabled() const
{
    return autoUpdateEnabledValue;
}

int UpdateManagerPrivate::timeout() const
{
    return timer->interval();
}

QString UpdateManagerPrivate::aptSourcesListFilePath() const
{
    return aptSourcesListFilePathValue;
}

QString UpdateManagerPrivate::currentVersion() const
{
    return versionToString(currentVersionValue);
}

QString UpdateManagerPrivate::packageName() const
{
    return packageNameValue;
}

QString UpdateManagerPrivate::newVersion() const
{
    return newVersionValue;
}

bool UpdateManagerPrivate::newVersionInstallable() const
{
    return newVersionInstallableValue;
}

void UpdateManagerPrivate::setAutoUpdateEnabled(bool arg)
{
    Q_Q(UpdateManager);
    if (autoUpdateEnabledValue != arg) {
        autoUpdateEnabledValue = arg;
        emit q->autoUpdateEnabledChanged(autoUpdateEnabledValue);
    }
}

void UpdateManagerPrivate::setTimeout(int arg)
{
    Q_Q(UpdateManager);
    if (timer->interval() != arg) {
        timer->setInterval(arg);
        emit q->timeoutChanged(timer->interval());
    }
}

void UpdateManagerPrivate::setAptSourcesListFilePath(const QString &arg)
{
    Q_Q(UpdateManager);
    if (aptSourcesListFilePathValue != arg) {
        aptSourcesListFilePathValue = arg;
        emit q->aptSourcesListFilePathChanged(aptSourcesListFilePathValue);
    }
}

void UpdateManagerPrivate::setCurrentVersion(const QString &arg)
{
    Q_Q(UpdateManager);
    QStringList splittedVersion = arg.split(".");
    if (splittedVersion.count() < 4)
        return;
    quint64 version = versionFromString(splittedVersion);
    if (currentVersionValue != version) {
        currentVersionMajor = splittedVersion[0].toInt();
        currentVersionValue = version;
        qCDebug(proofCoreUpdatesLog) << "Current version:" << QString("0x%1").arg(currentVersionValue, 16, 16, QLatin1Char('0'));
        emit q->currentVersionChanged(currentVersion());
        updateTimerState();
    }
}

void UpdateManagerPrivate::setPackageName(const QString &arg)
{
    Q_Q(UpdateManager);
    if (packageNameValue != arg) {
        packageNameValue = arg;
        emit q->packageNameChanged(packageNameValue);
        updateTimerState();
    }
}

void UpdateManagerPrivate::setNewVersion(const QString &arg)
{
    Q_Q(UpdateManager);
    if (newVersionValue != arg) {
        newVersionValue = arg;
        emit q->newVersionChanged(newVersionValue);
    }
}

void UpdateManagerPrivate::setNewVersionInstallable(bool arg)
{
    Q_Q(UpdateManager);
    if (newVersionInstallableValue != arg) {
        newVersionInstallableValue = arg;
        emit q->newVersionInstallableChanged(newVersionInstallableValue);
    }
}

void UpdateManagerPrivate::updateTimerState()
{
    if (started && currentVersionValue != 0 && !packageNameValue.isEmpty()) {
        if (!timer->isActive()) {
            QTimer::singleShot(1, thread, [this](){checkForUpdates();});
            timer->start();
        }
    } else {
        timer->stop();
    }
}

quint64 UpdateManagerPrivate::versionFromString(const QStringList &version)
{
    if (version.count() < 4)
        return 0x0;
    return ((quint64)version[0].toShort() << 48)
            | ((quint64)version[1].toShort() << 32)
            | ((quint64)version[2].toShort() << 16)
            | ((quint64)version[3].toShort());
}

quint64 UpdateManagerPrivate::versionFromString(const QString &version)
{
    return versionFromString(version.split("."));
}

QString UpdateManagerPrivate::versionToString(quint64 version)
{
    return QString("%1.%2.%3.%4")
            .arg(version >> 48)
            .arg((version >> 32) & 0xFFFF)
            .arg((version >> 16) & 0xFFFF)
            .arg(version & 0xFFFF);
}

WorkerThread::WorkerThread(UpdateManagerPrivate *updater)
    : updater(updater)
{
    moveToThread(this);
}

template<class Method, class... Args>
void WorkerThread::callUpdater(Method method, Args... args)
{
    if (!ProofObject::call(this, &WorkerThread::callUpdater<Method, Args...>, method, args...))
        (updater->*method)(args...);
}

template<class Method, class... Args>
auto WorkerThread::callUpdaterWithResult(Method method, Args &&... args)
    -> decltype((std::declval<Proof::UpdateManagerPrivate&>().*method)(std::forward<Args>(args)...))
{
    using Result = decltype((std::declval<Proof::UpdateManagerPrivate&>().*method)(std::forward<Args>(args)...));
    return doTheCall<Result>(method, std::forward<Args>(args)...);
}

template<class Result, class Method, class... Args>
typename std::enable_if<!std::is_same<Result, void>::value, Result>::type
WorkerThread::doTheCall(Method method, Args &&... args)
{
    Result result;
    if (!ProofObject::call(this, &WorkerThread::doTheCall<Result, Method, Args &&...>,
                           Call::Block, result, method, std::forward<Args>(args)...)) {
        result = (updater->*method)(std::forward<Args>(args)...);
    }
    return result;
}

template<class Result, class Method, class... Args>
typename std::enable_if<std::is_same<Result, void>::value>::type
WorkerThread::doTheCall(Method method, Args &&... args)
{
    if (!ProofObject::call(this, &WorkerThread::doTheCall<Result, Method, Args &&...>,
                           Call::Block, method, std::forward<Args>(args)...)) {
        (updater->*method)(std::forward<Args>(args)...);
    }
}

#include "updatemanager.moc"
