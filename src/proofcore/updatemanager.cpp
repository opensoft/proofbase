#include "updatemanager.h"
#include "proofcore/proofobject_p.h"
#include "proofcore/notifier.h"

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

    void setAutoUpdateEnabled(bool arg);
    void setTimeout(int arg);
    void setAptSourcesListFilePath(const QString &arg);
    void setCurrentVersion(const QString &arg);
    void setPackageName(const QString &arg);
    void setNewVersion(quint64 arg);
    void setNewVersionInstallable(bool arg);

    void updateTimerState();

    static quint64 versionFromString(const QStringList &version);
    static quint64 versionFromString(const QString &version);
    static QString versionToString(quint64 version);

    QString aptSourcesListFilePath;
    QString packageName;
    quint64 currentVersion = 0x0;
    int currentVersionMajor = 0;
    bool autoUpdateEnabled = true;
    quint64 newVersion = 0x0;
    bool newVersionInstallable = false;
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
    d->timer->setInterval(60 * 60 * 1000); // 1 hour
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
    return d->autoUpdateEnabled;
}

int UpdateManager::timeout() const
{
    Q_D(const UpdateManager);
    return d->timer->interval();
}

QString UpdateManager::aptSourcesListFilePath() const
{
    Q_D(const UpdateManager);
    return d->aptSourcesListFilePath;
}

QString UpdateManager::currentVersion() const
{
    Q_D(const UpdateManager);
    return d->versionToString(d->currentVersion);
}

QString UpdateManager::packageName() const
{
    Q_D(const UpdateManager);
    return d->packageName;
}

QString UpdateManager::newVersion() const
{
    Q_D(const UpdateManager);
    return d->versionToString(d->newVersion);
}

bool UpdateManager::newVersionInstallable() const
{
    Q_D(const UpdateManager);
    return d->newVersionInstallable;
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
    if (aptSourcesListFilePath.isEmpty())
        updater->start("sudo -S apt-get update");
    else
        updater->start(QString("sudo -S apt-get update -o Dir::Etc::sourcelist=\"%1\" -o Dir::Etc::sourceparts=\"-\"").arg(aptSourcesListFilePath));
    updater->waitForStarted();
    if (updater->error() == QProcess::UnknownError) {
        bool errorSent = false;
        if (updater->bytesAvailable() || updater->waitForReadyRead()) {
            QByteArray data = updater->readAll().trimmed();
            if (data.contains("sudo") || data.contains("password for")) {
                qCDebug(proofCoreUpdatesLog) << "apt-get update process asked for sudo password";
                Notifier::instance()->notify(QString("Error occurred during looking for updates.\nApt-get update process asked for sudo password"));
                errorSent = true;
                updater->kill();
            }
        }
        updater->waitForFinished(-1);
        qCDebug(proofCoreUpdatesLog) << "apt-get update process finished with code =" << updater->exitCode();
        if (updater->exitCode() && !errorSent) {
            Notifier::instance()->notify(QString("Error occurred during looking for updates.\nApt-get process returned with exit code - %1.")
                                         .arg(updater->exitCode()));
        }
    } else {
        qCDebug(proofCoreUpdatesLog) << "apt-get update process couldn't be started" << updater->error() << updater->errorString();
        Notifier::instance()->notify(QString("Error occurred during looking for updates.\nApt-get couldn't be started.\n%1 - %2")
                                     .arg(updater->error()).arg(updater->errorString()));
    }

    QScopedPointer<QProcess> checker(new QProcess);
    checker->start(QString("apt-cache --no-all-versions show %1").arg(packageName));
    checker->waitForStarted();
    if (checker->error() == QProcess::UnknownError) {
        checker->waitForFinished();
        if (checker->exitCode()) {
            Notifier::instance()->notify(QString("Error occurred during looking for updates.\nApt-cache process returned with exit code - %1.")
                                         .arg(checker->exitCode()));
        }
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
            Notifier::instance()->notify(QString("Error occurred during looking for updates.\nStrange version %1 found.").arg(version));
            return;
        }
        int foundVersionMajor = splittedVersion[0].toInt();
        quint64 foundVersion = versionFromString(splittedVersion);
        qCDebug(proofCoreUpdatesLog) << "Version found:" << QString("0x%1").arg(foundVersion, 16, 16, QLatin1Char('0'))
                                     << "; Current version is:" << QString("0x%1").arg(currentVersion, 16, 16, QLatin1Char('0'));
        if (foundVersion > currentVersion) {
            if (foundVersionMajor > currentVersionMajor)
                qCDebug(proofCoreUpdatesLog) << "Manual update needed because of different major version";
            else
                qCDebug(proofCoreUpdatesLog) << "Update from app is possible";
            setNewVersionInstallable(foundVersionMajor <= currentVersionMajor);
            setNewVersion(foundVersion);
            if (autoUpdateEnabled)
                installVersion("", "");
        }
    } else {
        qCDebug(proofCoreUpdatesLog) << "process couldn't be started" << checker->error() << checker->errorString();
        Notifier::instance()->notify(QString("Error occurred during looking for updates.\nApt-get couldn't be started.\n%1 - %2")
                                     .arg(updater->error()).arg(updater->errorString()));
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
    QString package = isUpdate ? packageName : QString("%1=%2").arg(packageName, version);
    auto successSignal = isUpdate ? &UpdateManager::updateSucceeded : &UpdateManager::installationSucceeded;
    auto failSignal = isUpdate ? &UpdateManager::updateFailed : &UpdateManager::installationFailed;
    updater->start(QString("sudo -S -k apt-get --quiet --assume-yes --force-yes --allow-unauthenticated install %1").arg(package));
    updater->waitForStarted();
    if (updater->error() == QProcess::UnknownError) {
        if (!updater->waitForReadyRead()) {
            qCDebug(proofCoreUpdatesLog) << "No answer from apt-get. Returning";
            Notifier::instance()->notify("Error occurred during update.\nNo answer from apt-get.");
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
                Notifier::instance()->notify("Error occurred during update.\nNo answer from apt-get.");
                emit (q->*failSignal)();
                return;
            }

            currentRead = updater->readAll();
            readBuffer.append(currentRead);
            currentRead = currentRead.trimmed();

            if (currentRead.contains("is not in the sudoers")) {
                qCDebug(proofCoreUpdatesLog) << "User not in sudoers list; log:\n" << readBuffer;
                Notifier::instance()->notify(QString("Error occurred during update.\nUser not in sudoers list.\n\n%1").arg(readBuffer.constData()));
                emit (q->*failSignal)();
                return;
            }
            if (currentRead.contains("Sorry, try again")) {
                qCDebug(proofCoreUpdatesLog) << "Sudo rejected the password; log:\n" << readBuffer;
                Notifier::instance()->notify(QString("Error occurred during update.\nPassword rejected.\n\n%1").arg(readBuffer.constData()));
                emit (q->*failSignal)();
                return;
            }
        }
        updater->waitForFinished(-1);
        readBuffer.append(updater->readAll().trimmed());
        qCDebug(proofCoreUpdatesLog) << "Updated with exitcode =" << updater->exitCode() << "; log:\n" << readBuffer;
        if (updater->exitCode()) {
            Notifier::instance()->notify(QString("Error occurred during update.\n\n%1").arg(readBuffer.constData()));
            emit (q->*failSignal)();
        } else {
            emit (q->*successSignal)();
            if (version.isEmpty())
                version = versionToString(newVersion);
            if (versionFromString(version) >= newVersion)
                setNewVersion(0);
            setCurrentVersion(version);
        }
    } else {
        Notifier::instance()->notify(QString("Error occurred during update.\nApt-get couldn't be started.\n%1 - %2")
                                     .arg(updater->error()).arg(updater->errorString()));
        qCDebug(proofCoreUpdatesLog) << "process couldn't be started" << updater->error() << updater->errorString();
    }
#else
    Q_UNUSED(version);
    Q_UNUSED(password);
    qCDebug(proofCoreUpdatesLog) << "Update is not supported for this platform";
    emit q->updateFailed();
#endif
}

void UpdateManagerPrivate::setAutoUpdateEnabled(bool arg)
{
    Q_Q(UpdateManager);
    if (autoUpdateEnabled != arg) {
        autoUpdateEnabled = arg;
        emit q->autoUpdateEnabledChanged(autoUpdateEnabled);
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
    if (aptSourcesListFilePath != arg) {
        aptSourcesListFilePath = arg;
        emit q->aptSourcesListFilePathChanged(aptSourcesListFilePath);
    }
}

void UpdateManagerPrivate::setCurrentVersion(const QString &arg)
{
    Q_Q(UpdateManager);
    QStringList splittedVersion = arg.split(".");
    if (splittedVersion.count() < 4)
        return;
    quint64 version = versionFromString(splittedVersion);
    if (currentVersion != version) {
        currentVersionMajor = splittedVersion[0].toInt();
        currentVersion = version;
        qCDebug(proofCoreUpdatesLog) << "Current version:" << QString("0x%1").arg(currentVersion, 16, 16, QLatin1Char('0'));
        emit q->currentVersionChanged(q->currentVersion());
        updateTimerState();
    }
}

void UpdateManagerPrivate::setPackageName(const QString &arg)
{
    Q_Q(UpdateManager);
    if (packageName != arg) {
        packageName = arg;
        emit q->packageNameChanged(packageName);
        updateTimerState();
    }
}

void UpdateManagerPrivate::setNewVersion(quint64 arg)
{
    Q_Q(UpdateManager);
    if (newVersion != arg) {
        newVersion = arg;
        emit q->newVersionChanged(q->newVersion());
    }
}

void UpdateManagerPrivate::setNewVersionInstallable(bool arg)
{
    Q_Q(UpdateManager);
    if (newVersionInstallable != arg) {
        newVersionInstallable = arg;
        emit q->newVersionInstallableChanged(newVersionInstallable);
    }
}

void UpdateManagerPrivate::updateTimerState()
{
    if (started && currentVersion != 0 && !packageName.isEmpty()) {
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
    if (!version)
        return "";
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
