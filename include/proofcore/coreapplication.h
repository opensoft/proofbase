#ifndef PROOF_COREAPPLICATION_H
#define PROOF_COREAPPLICATION_H

#include "proofcore_global.h"

#include <QCoreApplication>

namespace Proof {

class CoreApplicationPrivate;
class PROOF_CORE_EXPORT CoreApplication : public QCoreApplication
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CoreApplication)
public:
    CoreApplication(int &argc, char **argv, const QString &orgName = QString(), const QString &appName = QString());
    ~CoreApplication();
private:
    QScopedPointer<CoreApplicationPrivate> d_ptr;
};

}

#endif // PROOF_COREAPPLICATION_H
