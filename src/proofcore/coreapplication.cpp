#include "coreapplication.h"
#include "coreapplication_p.h"

#include "logs.h"
#include "settings.h"
#include "settingsgroup.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
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
}
