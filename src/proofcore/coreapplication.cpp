#include "coreapplication.h"
#include "coreapplication_p.h"

#include "logs.h"
#include "settings.h"
#include "settingsgroup.h"
#include "proofglobal.h"

#include <QDir>

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_MAC)
#include <unistd.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctime>
#endif

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_MAC)
constexpr int BACKTRACE_MAX_SIZE = 50;

static void signalHandler(int sig, siginfo_t *info, void *context)
{
    static bool handlerAlreadyCalled = false;
    if (handlerAlreadyCalled)
        return;
    handlerAlreadyCalled = true;

    alarm(10);
    QString crashFileName = QString("/tmp/proof_crash_%1").arg(time(0));
    int crashFileDescriptor = open(crashFileName.toLatin1().constData(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
    ucontext_t *uc = (ucontext_t *)context;
# ifdef Q_OS_LINUX
    void *caller = (void *) uc->uc_mcontext.fpregs->rip;
# else
    void *caller = (void *) uc->uc_mcontext->__ss.__rip;
# endif

    QString toLog = "#######################################";
    if (crashFileDescriptor != -1) {
        write(crashFileDescriptor, toLog.toLatin1().constData(), toLog.length());
        write(crashFileDescriptor, "\n", 1);
        write(STDOUT_FILENO, toLog.toLatin1().constData(), toLog.length());
        write(STDOUT_FILENO, "\n", 1);
    }
    toLog = QString("signal %1 (%2), address is 0x%3 from 0x%4")
            .arg(sig)
            .arg(strsignal(sig))
            .arg((unsigned long long) info->si_addr, 0, 16)
            .arg((unsigned long long) caller, 0, 16);
    if (crashFileDescriptor != -1) {
        write(crashFileDescriptor, toLog.toLatin1().constData(), toLog.length());
        write(crashFileDescriptor, "\n", 1);
        write(STDOUT_FILENO, toLog.toLatin1().constData(), toLog.length());
        write(STDOUT_FILENO, "\n", 1);
    }

    void *backtraceInfo[BACKTRACE_MAX_SIZE];
    int size = backtrace(backtraceInfo, BACKTRACE_MAX_SIZE);

    backtraceInfo[0] = backtraceInfo[1];
    backtraceInfo[1] = caller;

    char **backtraceArray = backtrace_symbols(backtraceInfo, size);

    if (!backtraceArray) {
        if (crashFileDescriptor != -1)
            close(crashFileDescriptor);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; ++i) {
# ifdef Q_OS_LINUX
        QRegExp re("^(.+)\\((.*)\\+([x0-9a-fA-F]*)\\)\\s+\\[(.+)\\]\\s*$");
# else
        QRegExp re("^\\d*\\s+(.+)\\s+(.+)\\s+(.+)\\s+\\+\\s+(\\d*)\\s*$");
# endif

        if (re.indexIn(backtraceArray[i]) < 0) {
            toLog = QString("[trace] #%1) %2")
                    .arg(i)
                    .arg(backtraceArray[i]);
            if (crashFileDescriptor != -1) {
                write(crashFileDescriptor, toLog.toLatin1().constData(), toLog.length());
                write(crashFileDescriptor, "\n", 1);
                write(STDOUT_FILENO, toLog.toLatin1().constData(), toLog.length());
                write(STDOUT_FILENO, "\n", 1);
            }
        } else {
# ifdef Q_OS_LINUX
            QString mangledName = re.cap(2).trimmed();
#else
            QString mangledName = re.cap(3).trimmed();
#endif
            char *name = abi::__cxa_demangle(mangledName.toLatin1().constData(), 0, 0, 0);
# ifdef Q_OS_LINUX
            toLog = QString("[trace] #%1) %2 : %3+%4 (%5)")
                    .arg(i)
                    .arg(re.cap(1).trimmed()) //scope
                    .arg(name ? name : mangledName) //name
                    .arg(re.cap(3).trimmed()) //offset
                    .arg(re.cap(4).trimmed()); //address
#else
            toLog = QString("[trace] #%1) %2 : %3+%4 (%5)")
                    .arg(i)
                    .arg(re.cap(1).trimmed()) //scope
                    .arg(name ? name : mangledName) //name
                    .arg(re.cap(4).trimmed()) //offset
                    .arg(re.cap(2).trimmed()); //address
#endif
            if (crashFileDescriptor != -1) {
                write(crashFileDescriptor, toLog.toLatin1().constData(), toLog.length());
                write(crashFileDescriptor, "\n", 1);
                write(STDOUT_FILENO, toLog.toLatin1().constData(), toLog.length());
                write(STDOUT_FILENO, "\n", 1);
            }
            free(name);
        }
    }

    free(backtraceArray);
    if (crashFileDescriptor != -1)
        close(crashFileDescriptor);
    exit(EXIT_FAILURE);
}
#endif

const QString TRANSLATIONS_PATH = ":/translations";

using namespace Proof;

CoreApplication::CoreApplication(int &argc, char **argv, const QString &orgName, const QString &appName, const QStringList &defaultLoggingRules)
    : QCoreApplication(argc, argv), d_ptr(new CoreApplicationPrivate)
{
    d_ptr->q_ptr = this;
    setOrganizationName(orgName);
    setApplicationName(appName);
    d_ptr->initApp(defaultLoggingRules);
}

CoreApplication::~CoreApplication()
{
}

Settings *CoreApplication::settings() const
{
    Q_D(const CoreApplication);
    return d->settings;
}

void CoreApplication::setLanguage(const QString &language)
{
    Q_D(CoreApplication);
    d->setLanguage(language);
    emit languageChanged(language);
}

QStringList CoreApplication::availableLanguages()
{
    Q_D(const CoreApplication);
    return d->availableLanguages;
}

QString CoreApplication::language() const
{
    Q_D(const CoreApplication);
    return d->language();
}

void Proof::CoreApplicationPrivate::initApp(const QStringList &defaultLoggingRules)
{
    Logs::setup(defaultLoggingRules);
    settings = new Proof::Settings(q_ptr);

    bool daemonized = false;
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
    if (q_ptr->arguments().contains("-d")) {
        daemonized = true;
        daemon(0, 0);
    }
#endif

    bool consoleOutputEnabled = true;
    QString logsStoragePath = "";
    QString logFileName = q_ptr->applicationName();

    if (Proof::proofUsesSettings()) {
        SettingsGroup *logGroup = settings->group("logs", Settings::NotFoundPolicy::Add);
        consoleOutputEnabled = !daemonized && logGroup->value("console", consoleOutputEnabled, Settings::NotFoundPolicy::Add).toBool();
        logsStoragePath = logGroup->value("custom_storage_path", logsStoragePath, Settings::NotFoundPolicy::Add).toString();
        logFileName = logGroup->value("filename", logFileName, Settings::NotFoundPolicy::Add).toString();
    }

    Logs::setConsoleOutputEnabled(consoleOutputEnabled);
    Logs::setLogsStoragePath(logsStoragePath);
    if (!logFileName.isEmpty())
        Logs::installFileHandler(logFileName);

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_MAC)
    static struct sigaction sigSegvAction;
    sigSegvAction.sa_sigaction = signalHandler;
    sigSegvAction.sa_flags = SA_SIGINFO;
    static struct sigaction sigAbrtAction;
    sigAbrtAction.sa_sigaction = signalHandler;
    sigAbrtAction.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &sigSegvAction, (struct sigaction *)NULL) != 0)
        qCWarning(proofCoreLoggerLog) << "No segfault handler is on your back.";
    if (sigaction(SIGABRT, &sigAbrtAction, (struct sigaction *)NULL) != 0)
        qCWarning(proofCoreLoggerLog) << "No abort handler is on your back.";
#endif

    initTranslator();

    qCDebug(proofCoreMiscLog).noquote() << QString("%1 started").arg(q_ptr->applicationName()).toLatin1().constData() << "with config at" << Proof::Settings::filePath();
}

void CoreApplicationPrivate::initTranslator()
{
    QSet<QString> languagesSet;
    languagesSet.insert("en");
    QDir translationsDir(TRANSLATIONS_PATH);
    for (const QFileInfo &fileInfo : translationsDir.entryInfoList({"*.qm"}, QDir::Files)) {
        QString fileName = fileInfo.fileName();
        translationPrefixes.insert(fileName.mid(0, fileName.indexOf('.')));
        languagesSet.insert(fileName.mid(fileName.indexOf('.') + 1, -3).mid(0, 2));
    }
    availableLanguages = languagesSet.toList();
    qSort(availableLanguages.begin(), availableLanguages.end());
    SettingsGroup *localeGroup = settings->group("locale", Settings::NotFoundPolicy::Add);
    currentLanguage = localeGroup->value("language", "en", Settings::NotFoundPolicy::Add).toString();
}

void CoreApplicationPrivate::setLanguage(const QString &language)
{
    currentLanguage = language;
    SettingsGroup *localeGroup = settings->group("locale", Settings::NotFoundPolicy::Add);
    localeGroup->setValue("language", language);

    for (QTranslator *installedTranslator : installedTranslators) {
        if (installedTranslator)
            delete installedTranslator;
    }
    installedTranslators.clear();

    for (const QString &prefix : translationPrefixes) {
        qCDebug(proofCoreMiscLog) << "Language prefix" << prefix;
        QTranslator *translator = new QTranslator(q_ptr);
        installedTranslators.append(translator);
        translator->load(QString("%1.%2").arg(prefix, language), TRANSLATIONS_PATH);
        q_ptr->installTranslator(translator);
    }
}

QString CoreApplicationPrivate::language() const
{
    return currentLanguage;
}
