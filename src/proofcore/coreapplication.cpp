#include "coreapplication.h"
#include "coreapplication_p.h"

#include "logs.h"

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

void Proof::CoreApplicationPrivate::initApp()
{
    Proof::Logs::setup();
}

