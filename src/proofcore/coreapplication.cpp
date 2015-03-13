#include "coreapplication.h"
#include "coreapplication_p.h"

#include "logs.h"
#include "settings.h"
#include "settingsgroup.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#endif

#ifdef Q_OS_LINUX
constexpr int BACKTRACE_MAX_SIZE = 50;
static void signalHandler(int sig)
{
    qCCritical(proofCoreCrashLog) << "#######################################";
    qCCritical(proofCoreCrashLog) << QString("signal %1 (%2)").arg(sig).arg(strsignal(sig));

    void *backtraceInfo[BACKTRACE_MAX_SIZE];
    int size = backtrace(backtraceInfo, BACKTRACE_MAX_SIZE);

    char **backtraceArray = backtrace_symbols(backtraceInfo, size);

    if (!backtraceArray)
        exit(EXIT_FAILURE);

    for (int i = 0; i < size; ++i) {
        QRegExp re("^(.+)\\((.*)\\+([x0-9a-fA-F]*)\\)\\s+\\[(.+)\\]\\s*$");

        if (re.indexIn(backtraceArray[i]) < 0) {
            qCCritical(proofCoreCrashLog) << QString("[trace] #%1) %2")
                                             .arg(i)
                                             .arg(backtraceArray[i]);
        } else {
            char *name = abi::__cxa_demangle(re.cap(2).trimmed().toLatin1().constData(), 0, 0, 0);
            qCCritical(proofCoreCrashLog) << QString("[trace] #%1) %2 : %3+%4 (%5)")
                                                     .arg(i)
                                                     .arg(re.cap(1).trimmed()) //scope
                                                     .arg(name ? name : re.cap(2).trimmed()) //name
                                                     .arg(re.cap(3).trimmed()) //offset
                                                     .arg(re.cap(4).trimmed()); //address
            free(name);
        }
    }

    free(backtraceArray);
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
#ifdef Q_OS_LINUX
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

#ifdef Q_OS_LINUX
    signal(SIGSEGV, signalHandler);
#endif

}
