#ifndef COREAPPLICATION_P_H
#define COREAPPLICATION_P_H

#include <QtGlobal>

namespace Proof {
class CoreApplication;
class CoreApplicationPrivate
{
    Q_DECLARE_PUBLIC(CoreApplication)
protected:
    void initApp();
private:
    CoreApplication *q_ptr;
};
}

#endif // COREAPPLICATION_P_H
