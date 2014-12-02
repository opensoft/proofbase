#ifndef COREAPPLICATION_P_H
#define COREAPPLICATION_P_H

#include "proofcore_global.h"

#include <QtGlobal>
#include <QCoreApplication>

namespace Proof {
class Settings;
class CoreApplication;
class PROOF_CORE_EXPORT CoreApplicationPrivate
{
    Q_DECLARE_PUBLIC(CoreApplication)
protected:
    void initApp();

    Settings *settings = nullptr;
private:
    CoreApplication *q_ptr;
};
}

#endif // COREAPPLICATION_P_H
