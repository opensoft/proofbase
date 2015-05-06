#include "coreapplication.h"
#include "coreapplication_p.h"

#include "logs.h"
#include "settings.h"
#include "settingsgroup.h"

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_MAC)
#include <unistd.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <stdio.h>
#include <fcntl.h>
#endif

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_MAC)
constexpr int BACKTRACE_MAX_SIZE = 50;

static void signalHandler(int sig, siginfo_t *info, void *context)
{
    static bool handlerAlreadyCalled = false;
    if (handlerAlreadyCalled)
        return;
    handlerAlreadyCalled = true;

    int crashFileDescriptor = open("/tmp/last_proof_crash", O_WRONLY | O_TRUNC | O_CREAT, 0644);
    ucontext_t *uc = (ucontext_t *)context;
# ifdef Q_OS_LINUX
    void *caller = (void *) uc->uc_mcontext.fpregs->rip;
# else
    void *caller = (void *) uc->uc_mcontext->__ss.__rip;
# endif

    QString toLog = "#######################################";
    qCCritical(proofCoreCrashLog) << toLog;
    qCCritical(proofCoreCrashLog) << "Crash file /tmp/last_proof_crash opened with fd" << crashFileDescriptor;
    if (crashFileDescriptor != -1) {
        write(crashFileDescriptor, toLog.toLatin1().constData(), toLog.length());
        write(crashFileDescriptor, "\n", 1);
    }
    toLog = QString("signal %1 (%2), address is 0x%3 from 0x%4")
            .arg(sig)
            .arg(strsignal(sig))
            .arg((unsigned long long) info->si_addr, 0, 16)
            .arg((unsigned long long) caller, 0, 16);
    qCCritical(proofCoreCrashLog) << toLog;
    if (crashFileDescriptor != -1) {
        write(crashFileDescriptor, toLog.toLatin1().constData(), toLog.length());
        write(crashFileDescriptor, "\n", 1);
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
            qCCritical(proofCoreCrashLog) << toLog;
            if (crashFileDescriptor != -1) {
                write(crashFileDescriptor, toLog.toLatin1().constData(), toLog.length());
                write(crashFileDescriptor, "\n", 1);
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
            qCCritical(proofCoreCrashLog) << toLog;
            if (crashFileDescriptor != -1) {
                write(crashFileDescriptor, toLog.toLatin1().constData(), toLog.length());
                write(crashFileDescriptor, "\n", 1);
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

using namespace Proof;

CoreApplication::CoreApplication(int & argc, char **argv, const QString &orgName, const QString &appName)
    : QCoreApplication(argc, argv), d_ptr(new CoreApplicationPrivate)
{
    d_ptr->q_ptr = this;
    setOrganizationName(orgName);
    setApplicationName(appName);
    d_ptr->initApp();
}

CoreApplication::~CoreApplication()
{
}

Settings *CoreApplication::settings() const
{
    Q_D(const CoreApplication);
    return d->settings;
}

void Proof::CoreApplicationPrivate::initApp()
{
    Logs::setup();
    settings = new Proof::Settings(q_ptr);

    bool daemonized = false;
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
    if (q_ptr->arguments().count() == 2 && q_ptr->arguments().last() == "-d") {
        daemonized = true;
        daemon(0, 0);
    }
#endif

    SettingsGroup *logGroup = settings->group("logs", Settings::NotFoundPolicy::Add);
    Logs::setConsoleOutputEnabled(!daemonized && logGroup->value("console", true, Settings::NotFoundPolicy::Add).toBool());
    Logs::setLogsStoragePath(logGroup->value("custom_storage_path", "", Settings::NotFoundPolicy::Add).toString());
    QString logFileName = logGroup->value("filename", q_ptr->applicationName(), Settings::NotFoundPolicy::Add).toString();
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

    //TODO: Qt5.4: use noquote() here instead
    qCDebug(proofCoreMiscLog) << QString("%1 started").arg(q_ptr->applicationName()).toLatin1().constData();
}
