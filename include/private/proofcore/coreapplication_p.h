#ifndef COREAPPLICATION_P_H
#define COREAPPLICATION_P_H

#include "coreapplication.h"
#include "proofcore_global.h"

#include <QtGlobal>
#include <QCoreApplication>

namespace Proof {
class Settings;
class PROOF_CORE_EXPORT CoreApplicationPrivate
{
    Q_DECLARE_PUBLIC(CoreApplication)
protected:
    void initApp();

    Settings *settings = nullptr;
    QCoreApplication *q_ptr = nullptr;
};
}

#endif // COREAPPLICATION_P_H
